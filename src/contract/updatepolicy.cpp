#include "contract/updatepolicy.h"
#include "contract/contractdb.h"
#include "processing.h"
#include "validation.h"
#include <boost/asio.hpp>
#include <future>
#include <stack>
#include <sys/wait.h>
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

static bool ProcessBlockStack(std::stack<CBlockIndex*>& blockstack, ContractDB& cache, boost::asio::thread_pool& pool, const Consensus::Params& consensusParams)
{
    while (!blockstack.empty()) {
        auto tmpBlockIndex {blockstack.top()};
        blockstack.pop();
        
        std::unique_ptr<CBlock> block(new CBlock());
        if (!ReadBlockFromDisk(*block, tmpBlockIndex, consensusParams)) {
            return false;
        }

        for (const CTransactionRef& tx : block->vtx) {
            if (tx.get()->contract.action != ACTION_NONE) {
                boost::asio::post(pool, [tx, &cache]() {
                    const auto& contract = tx.get()->contract;
                    if (!ExecuteContract(contract, tx, cache)) {
                        LogPrintf("Contract execution failed: %s\n", contract.address.GetHex());
                    }
                });
            }
        }
    }
    return true; 
}

std::unique_ptr<UpdatePolicy> SelectUpdatePolicy(CChain& chainActive, ContractDB& cache)
{
    auto tip {cache.getTip()};
    if (!tip.isValid()) {
        return std::unique_ptr<UpdatePolicy>(new Rebuild());
    }

    if (chainActive.Tip()->GetBlockHash().ToString() == tip.hash) {
        return std::unique_ptr<UpdatePolicy>(new DoNothing());
    }
    
    if (tip.height < chainActive.Height()) {
        return std::unique_ptr<UpdatePolicy>(new Forward());
    }

    return std::unique_ptr<UpdatePolicy>(new Rollback());
}

bool DoNothing::UpdateSnapShot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams)
{
    return true;
}

bool Rebuild::UpdateSnapShot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams)
{
    auto tip {cache.getTip()};
    if (tip.isValid()) {
        try {
            cache.clearAllState();
        } catch (const std::runtime_error& e) {
            LogPrintf("CRITICAL: Rebuild failed during clear contract db's state: %s. Return false.\n", e.what());
            return false;
        }
    }

    auto blockstack {std::stack<CBlockIndex*>()};
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        blockstack.push(pindex);
    }
    
    boost::asio::thread_pool pool(4);
    if (!ProcessBlockStack(blockstack, cache, pool, consensusParams)) {
        return false;
    }
    pool.join();
    
    cache.setTip(chainActive.Tip()->nHeight, chainActive.Tip()->GetBlockHash().ToString());

    return true;
}

bool Forward::UpdateSnapShot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams)
{
    auto tip {cache.getTip()};
    // Verify that the chain is consistent.
    CBlockIndex* cached_tip_index = chainActive[tip.height];
    if (!cached_tip_index || cached_tip_index->GetBlockHash().ToString() != tip.hash) {
         // The cache's tip is not on the main chain. A reorg/rollback is needed.
        return Rollback().UpdateSnapShot(cache, chainActive, consensusParams);
    }

    auto blockstack {std::stack<CBlockIndex*>()};
    // Find the common ancestor and stack up the new blocks to be processed.
    for (CBlockIndex* pindex = chainActive.Tip(); pindex && pindex->nHeight > tip.height; pindex = pindex->pprev) {
        blockstack.push(pindex);
    }
    
    boost::asio::thread_pool pool(4);
    if (!ProcessBlockStack(blockstack, cache, pool, consensusParams)) {
        return false;
    }
    pool.join();

    cache.setTip(chainActive.Tip()->nHeight, chainActive.Tip()->GetBlockHash().ToString());

    return true;
}

bool Rollback::UpdateSnapShot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams)
{
    // Find the highest block hash on the main chain that is also a valid checkpoint.
    std::vector<CheckpointInfo> checkPointInfoList {cache.listCheckpoints()};
    CBlockIndex* pindex {chainActive.Tip()};
    
    while(pindex) {
        const auto& hashStr = pindex->GetBlockHash().ToString();
        auto it = std::find_if(checkPointInfoList.begin(), checkPointInfoList.end(), 
            [&](const CheckpointInfo& info){ return info.hash == hashStr; });

        if (it != checkPointInfoList.end()) {
            // Found a valid checkpoint on the main chain. Restore to it.
            try {
                cache.restoreToCheckpoint(it->backupid);
                cache.setTip(it->height, it->hash);
                cache.clearAllState();
                // After restoring, we need to move forward to the active chain's tip.
                return Forward().UpdateSnapShot(cache, chainActive, consensusParams);
            } catch (const std::runtime_error& e) {
                LogPrintf("CRITICAL: Rollback failed during restore to checkpoint %s: %s. Forcing rebuild.\n", it->hash, e.what());
                // If restore fails, the state is uncertain. A full rebuild is the only safe option.
                return Rebuild().UpdateSnapShot(cache, chainActive, consensusParams);
            }
        }
        pindex = pindex->pprev;
    }

    // If no valid checkpoint is found on the active chain, we must rebuild from scratch.
    LogPrintf("Rollback required, but no valid checkpoint found on the active chain. Forcing rebuild.\n");
    return Rebuild().UpdateSnapShot(cache, chainActive, consensusParams);
}