#include "contract/updatepolicy.h"
#include "contract/contractdb.h"
#include "contract/contractserver.h"
#include "processing.h"
#include "validation.h"
#include <stack>
#include <algorithm>

const static fs::path& GetContractsDir()
{
    static fs::path contracts_dir = []{
        fs::path dir = GetDataDir() / "contracts";
        fs::create_directories(dir);
        return dir;
    }();
    return contracts_dir;
}

static bool ProcessBlockStack(std::stack<CBlockIndex*>& blockstack)
{
    ContractServer& server = ContractServer::getInstance();

    while (!blockstack.empty()) {
        CBlockIndex* pindex = blockstack.top();
        blockstack.pop();
        server.submitBlock(pindex);
    }
    return true;
}

std::unique_ptr<UpdatePolicy> SelectUpdatePolicy(CChain& chainActive, ContractDB& cache)
{
    auto tip {cache.getTip()};
    if (!tip.isValid()) {
        return std::unique_ptr<UpdatePolicy>(new Rebuild());
    }
    
    if (tip.height < chainActive.Height() && tip.hash == (chainActive[tip.height])->GetBlockHash().ToString()) {
        return std::unique_ptr<UpdatePolicy>(new Forward());
    }

    return std::unique_ptr<UpdatePolicy>(new Rollback());
}

bool Rebuild::UpdateSnapshot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams)
{
    auto blockstack {std::stack<CBlockIndex*>()};
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        blockstack.push(pindex);
    }
    
    if (!ProcessBlockStack(blockstack)) {
        return false;
    }

    return true;
}

bool Forward::UpdateSnapshot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams)
{
    auto tip {cache.getTip()};
    auto blockstack {std::stack<CBlockIndex*>()};
    // Find the common ancestor and stack up the new blocks to be processed.
    for (CBlockIndex* pindex = chainActive.Tip(); pindex && pindex->nHeight > tip.height; pindex = pindex->pprev) {
        blockstack.push(pindex);
    }
    
    if (!ProcessBlockStack(blockstack)) {
        return false;
    }

    return true;
}

bool Rollback::UpdateSnapshot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams)
{
    LogPrintf("Reorg detected. Starting rollback procedure...\n");

    /* Clear block queue and terminate transaction pool's child thread */
    auto& server = ContractServer::getInstance();
    server.pauseAndClearQueue();

    // RAII-style guard to ensure the server is always resumed, even if exceptions occur.
    struct ServerResumer {
        ContractServer& srv;
        ~ServerResumer() { srv.resume(); }
    } resumer{server};

    try {
        // Find the highest block hash on the main chain that is also a valid checkpoint.
        std::vector<CheckpointInfo> checkPointInfoList {cache.listCheckpoints()};
        CBlockIndex* pindex {chainActive.Tip()};
        
        while (pindex) {
            const auto& hashStr = pindex->GetBlockHash().ToString();
            auto it = std::find_if(checkPointInfoList.begin(), checkPointInfoList.end(), 
                [&](const CheckpointInfo& info){ return info.hash == hashStr; });

            if (it != checkPointInfoList.end()) {
                // Found a valid checkpoint on the main chain. Restore to it.
                cache.restoreToCheckpoint(it->backupid);
                cache.setTip(it->height, it->hash);
                // After restoring, we need to move forward to the active chain's tip.
                return Forward().UpdateSnapshot(cache, chainActive, consensusParams);
            }
            pindex = pindex->pprev;
        }
        LogPrintf("No valid checkpoint found on the active chain. Forcing rebuild.\n");
        cache.clearAllState();
        cache.setTip(-1, {});
        return Rebuild().UpdateSnapshot(cache, chainActive, consensusParams);
    } catch (const std::runtime_error& e) {
        LogPrintf("CRITICAL: Rollback procedure failed: %s\n", e.what());
        return false;
    }

    return true;
}