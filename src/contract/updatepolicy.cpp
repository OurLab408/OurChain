#include "contract/updatepolicy.h"
#include <boost/asio.hpp>
#include <future>
#include <stack>
#include <sys/wait.h>
#include <zmq.hpp>

static fs::path contracts_dir;

const static fs::path& GetContractsDir()
{
    if (!contracts_dir.empty()) return contracts_dir;

    contracts_dir = GetDataDir() / "contracts";
    fs::create_directories(contracts_dir);

    return contracts_dir;
}

std::string MakeZmqMsg(bool isPure, const std::string& address, vector<string> parameters, std::string preTxid)
{
    json j;
    j["isPure"] = isPure;
    j["address"] = address;
    j["parameters"] = parameters;
    j["preTxid"] = preTxid;
    return j.dump();
}

void SendZmq(const std::string& message, std::string* buf)
{
    zmq::context_t context(1);
    zmq::socket_t pusher(context, zmq::socket_type::req);
    pusher.connect("tcp://127.0.0.1:5559");
    // send message
    zmq::message_t zmq_message(message.size());
    memcpy(zmq_message.data(), message.data(), message.size());
    if (!pusher.send(zmq_message, zmq::send_flags::none)) {
        LogPrintf("send message error\n");
    }
    // receive response
    zmq::message_t response;
    if (!pusher.recv(response, zmq::recv_flags::none)) {
        LogPrintf("receive response error\n");
    }
    std::string recv_msg(static_cast<char*>(response.data()), response.size());
    LogPrintf("Contract Received response: %s\n", recv_msg.c_str());
    if (buf != nullptr) {
        *buf = recv_msg;
    }
    pusher.close();
}

static bool ProcessContracts(std::stack<CBlockIndex*> blockstack, ContractStateCache& cache, const Consensus::Params consensusParams)
{
    while (blockstack.size() > 0) {
        auto tmpBlock = blockstack.top();
        blockstack.pop();
        CBlock* block = new CBlock();
        if (!ReadBlockFromDisk(*block, tmpBlock, consensusParams)) {
            return false;
        }
        // init tp
        boost::asio::thread_pool pool(4);
        // put task to thread pool
        for (const CTransactionRef& tx : block->vtx) {
            if (tx.get()->contract.action == contract_action::ACTION_NEW) {
                boost::asio::post(pool, [tx, &cache]() {
                    auto contract = tx.get()->contract;
                    // contract address
                    fs::path new_dir = GetContractsDir() / contract.address.GetHex();
                    fs::create_directories(new_dir);
                    std::ofstream contract_code(new_dir.string() + "/code.cpp");
                    contract_code.write(contract.code.c_str(), contract.code.size());
                    contract_code.close();
                    // compile contract
                    int pid, status;
                    pid = fork();
                    if (pid == 0) {
                        int fd = open((GetContractsDir().string() + "/err").c_str(),
                            O_WRONLY | O_APPEND | O_CREAT,
                            0664);
                        dup2(fd, STDERR_FILENO);
                        close(fd);
                        execlp("ourcontract-mkdll",
                            "ourcontract-mkdll",
                            GetContractsDir().string().c_str(),
                            contract.address.GetHex().c_str(),
                            NULL);
                        exit(EXIT_FAILURE);
                    }
                    waitpid(pid, &status, 0);
                    if (WIFEXITED(status) == false || WEXITSTATUS(status) != 0) {
                        LogPrintf("compile contract %s error\n", contract.address.GetHex());
                    }
                    // Execute contract
                    string preTxid = tx.get()->GetHash().GetHex();
                    SendZmq(MakeZmqMsg(false, contract.address.GetHex(), contract.args, preTxid), nullptr);
                });
            } else if (tx.get()->contract.action == contract_action::ACTION_CALL) {
                boost::asio::post(pool, [tx, &cache]() {
                    auto contract = tx.get()->contract;
                    string preTxid = tx.get()->GetHash().GetHex();
                    SendZmq(MakeZmqMsg(false, contract.address.GetHex(), contract.args, preTxid), nullptr);
                });
            }
        }
        pool.join();
        delete block;
    }
    return true;
}

UpdatePolicy* SelectUpdatePolicy(CChain& chainActive, ContractStateCache* cache)
{
    int curHeight = chainActive.Height();
    uint256 curHash = chainActive.Tip()->GetBlockHash();
    BlockCache::blockIndex firstBlock;
    /* Return false if highest block height = -1  */
    if (!cache->getFirstBlockCache(firstBlock)) {
        return new Rebuild();
    }

    int cacheHeight = firstBlock.blockHeight;
    uint256 cacheHash = firstBlock.blockHash;
    if (curHash == cacheHash && curHeight == cacheHeight) {
        return new DoNothing();
    }
    if (cacheHeight < curHeight) {
        return new Forward();
    }

    return new Rollback();
}

bool DoNothing::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams)
{
    // Default is do nothing
    return true;
}

bool Rebuild::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams)
{
    cache.getSnapShot()->clear();
    cache.getBlockCache()->clear();
    auto blockstack = std::stack<CBlockIndex*>();
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        int height = pindex->nHeight;
        uint256 hash = pindex->GetBlockHash();
        cache.pushBlock(BlockCache::blockIndex(hash, height));
        blockstack.push(pindex);
    }
    // process all contracts in blockstack
    return ProcessContracts(blockstack, cache, consensusParams);
}

bool Forward::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams)
{
    auto blockstack = std::stack<CBlockIndex*>();
    auto tmpBlockIndex = std::stack<BlockCache::blockIndex>();
    BlockCache::blockIndex firstBlock;
    if (!cache.getFirstBlockCache(firstBlock)) {
        return false;
    }
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        // push block index to cache
        int height = pindex->nHeight;
        uint256 hash = pindex->GetBlockHash();
        if (height == firstBlock.blockHeight && hash == firstBlock.blockHash) {
            // find pre chain state
            while (tmpBlockIndex.size() > 0) {
                BlockCache::blockIndex tmpBlock = tmpBlockIndex.top();
                tmpBlockIndex.pop();
                cache.pushBlock(tmpBlock);
            }
            break;
        }
        if (height < firstBlock.blockHeight) {
            // release memory
            while (blockstack.size() > 0) {
                blockstack.pop();
            }
            // cannot find pre chain state, rollback
            Rollback algo;
            return algo.UpdateSnapShot(cache, snapshot, chainActive, consensusParams);
        }
        tmpBlockIndex.push(BlockCache::blockIndex(hash, height));
        blockstack.push(pindex);
    }
    return ProcessContracts(blockstack, cache, consensusParams);
}

bool Rollback::UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams)
{
    BlockCache::blockIndex firstBlock;
    if (!cache.getFirstBlockCache(firstBlock)) {
        return false;
    }
    auto checkPointInfoList = cache.getCheckPointList();
    std::vector<std::string> checkPointList;
    for (auto it = checkPointInfoList.begin(); it != checkPointInfoList.end(); it++) {
        checkPointList.push_back(it->tipBlockHash);
    }
    for (CBlockIndex* pindex = chainActive.Tip(); pindex != nullptr; pindex = pindex->pprev) {
        int height = pindex->nHeight;
        uint256 hash = pindex->GetBlockHash();

        if (height == firstBlock.blockHeight) {
            if (hash == firstBlock.blockHash) {
                // Checkpoint exist
                if (std::find(checkPointList.begin(), checkPointList.end(), hash.ToString()) != checkPointList.end()) {
                    // restore checkpoint
                    if (!cache.restoreCheckPoint(hash.ToString(), checkPointInfoList)) {
                        LogPrintf("rollback error can not continue in checkPoint \n");
                        assert(false);
                    }
                    Forward algo;
                    return algo.UpdateSnapShot(cache, snapshot, chainActive, consensusParams);
                }
            }
            cache.popBlock();
            if (!cache.getFirstBlockCache(firstBlock)) {
                // block is empty now
                Rebuild algo;
                return algo.UpdateSnapShot(cache, snapshot, chainActive, consensusParams);
            }
        } else {
            // cache index should not bigger than chain index
            while (firstBlock.blockHeight >= height) {
                cache.popBlock();
                if (!cache.getFirstBlockCache(firstBlock)) {
                    // block is empty now
                    Rebuild algo;
                    return algo.UpdateSnapShot(cache, snapshot, chainActive, consensusParams);
                }
            }
        }
    }
    Rebuild algo;
    return algo.UpdateSnapShot(cache, snapshot, chainActive, consensusParams);
}