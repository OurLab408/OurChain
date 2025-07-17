#include "chainparams.h"
#include "contract/contractserver.h"
#include "contract/contractdb.h"
#include "validation.h"
#include "contract/processing.h"
#include <future>
#include <vector>

ContractServer& ContractServer::getInstance() {
    static ContractServer instance;
    return instance;
}

ContractServer::ContractServer() : transactionPool_(THREAD_NUM) {}

ContractServer::~ContractServer() {
    if (!stop_flag_.load()) {
        shutdown();
    }
}

void ContractServer::start(boost::thread_group& threadGroup) {
    LogPrintf("ContractServer: Starting consumer thread...\n");
    // The consumer thread is now created and managed by the application's main thread group.
    threadGroup.create_thread([this]() { this->consumerLoop(); });
}

void ContractServer::submitBlock(CBlockIndex* pindex) {
    if (stop_flag_.load()) return;
    blockQueue_.push(pindex);
}

void ContractServer::shutdown() {
    // Use a lock to prevent race conditions if shutdown/interrupt are called concurrently
    static std::mutex shutdown_mutex;
    std::lock_guard<std::mutex> lock(shutdown_mutex);

    if (stop_flag_.load()) return; // Already shutting down

    LogPrintf("ContractServer: Shutting down gracefully...\n");
    stop_flag_.store(true);
    blockQueue_.push(nullptr);

    // Wait for the transaction pool to finish its current tasks and then join.
    transactionPool_.join();
}

void ContractServer::interrupt() {
    static std::mutex shutdown_mutex;
    std::lock_guard<std::mutex> lock(shutdown_mutex);

    if (stop_flag_.load()) return; // Already shutting down

    LogPrintf("ContractServer: Interrupting immediately...\n");
    stop_flag_.store(true);
    blockQueue_.push(nullptr);
    
    // Forcibly stop all tasks in the pool.
    transactionPool_.stop();
}

void ContractServer::processSingleBlock(CBlockIndex* pindex)
{
    std::unique_ptr<CBlock> block(new CBlock());
    if (!ReadBlockFromDisk(*block, pindex, Params().GetConsensus())) {
        LogPrintf("ERROR: Could not read block %s in ContractServer.\n", pindex->GetBlockHash().ToString());
        return;
    }

    std::vector<std::future<void>> futures;
    std::vector<CTransactionRef> dispatchedTxs;

    auto& cache = ContractDB::getInstance();

    for (const CTransactionRef& tx : block->vtx) {
        if (tx.get()->contract.action != ACTION_NONE) {
            // Wrap the execution logic in a packaged_task to get a future.
            using VoidTask = std::packaged_task<void()>;
            auto task = std::make_shared<VoidTask>([tx, &cache]() {
                ExecuteContract(tx.get()->contract, tx, cache);
            });

            futures.push_back(task->get_future());
            dispatchedTxs.push_back(tx);

            boost::asio::post(transactionPool_, [task]() { (*task)(); });
        }
    }

    // Wait for all transactions of the current block to complete before proceeding.
    LogPrintf("ContractServer: Waiting for %d contracts in block %d...\n", futures.size(), pindex->nHeight);
    for (size_t i = 0; i < futures.size(); ++i) {
        auto& future = futures[i];
        const auto& tx = dispatchedTxs[i];

        try {
            future.get();
        } catch(const std::exception& e) {
            LogPrintf("ERROR: Contract execution failed for tx %s: %s\n", 
                        tx->GetHash().ToString(), e.what());
        }
    }
    LogPrintf("ContractServer: Block %d processed.\n", pindex->nHeight);

    // Now that the block is fully processed, it's safe to update the tip
    // and handle checkpoint logic.
    cache.setTip(pindex->nHeight, pindex->GetBlockHash().ToString());
    
    const int CHECKPOINT_INTERVAL = 100;
    const int CHECKPOINTS_TO_KEEP = 10;
    if (pindex->nHeight > 0 && pindex->nHeight % CHECKPOINT_INTERVAL == 0) {
        LogPrintf("ContractServer: Saving checkpoint at height %d.\n", pindex->nHeight);
        cache.saveCheckpoint();
        cache.purgeOldCheckpoints(CHECKPOINTS_TO_KEEP);
    }
}

void ContractServer::consumerLoop() {
    RenameThread("contract-consumer");
    LogPrintf("ContractServer: Consumer loop started.\n");
    auto& cache = ContractDB::getInstance();

    while (!stop_flag_.load()) {
        {
            std::unique_lock<std::mutex> lock(server_mutex_);
            cv_paused_.wait(lock, [this] { return !paused_.load() || stop_flag_.load(); });
        }

        CBlockIndex* pindex = nullptr;
        blockQueue_.wait_and_pop(pindex);

        if (stop_flag_.load() || pindex == nullptr) {
            continue;
        }
        
        processSingleBlock(pindex);
    }
    LogPrintf("ContractServer: Consumer loop finished.\n");
}

void ContractServer::pauseAndClearQueue() {
    std::unique_lock<std::mutex> lock(server_mutex_);
    LogPrintf("ContractServer: Pausing and clearing queue...\n"); 
    paused_.store(true);
    blockQueue_.clear();
    transactionPool_.join(); 
    LogPrintf("ContractServer: Paused and transaction pool idle.\n");
}

void ContractServer::resume() {
    std::unique_lock<std::mutex> lock(server_mutex_);
    LogPrintf("ContractServer: Resuming...\n");
    paused_.store(false);
    cv_paused_.notify_one(); 
}