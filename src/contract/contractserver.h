#ifndef CONTRACT_SERVER_H
#define CONTRACT_SERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include "contract/threadsafe_queue.h"

class CBlockIndex; // Forward declaration

#define THREAD_NUM 4

/**
 * @brief A singleton background service for processing contract transactions.
 *
 * This class implements a producer-consumer pattern. It manages a work queue
 * for incoming blocks and a thread pool for executing contract transactions
 * asynchronously, ensuring the main chain-syncing process is not blocked.
 */
class ContractServer {
public:
    /**
     * @brief Gets the single, global instance of the ContractServer.
     */
    static ContractServer& getInstance();

    ContractServer(const ContractServer&) = delete;
    ContractServer& operator=(const ContractServer&) = delete;

    /**
     * @brief Starts the main consumer thread. Should be called once during app initialization.
     * @param threadGroup The application's main thread group for lifecycle management.
     */
    void start(boost::thread_group& threadGroup);

    /**
     * @brief Submits a block to the queue for asynchronous processing.
     * This method is thread-safe and non-blocking.
     * @param pindex A pointer to the block index to be processed.
     */
    void submitBlock(CBlockIndex* pindex);

    /**
     * @brief Signals the server to perform a graceful shutdown.
     * Waits for pending tasks to complete.
     */
    void shutdown();

    /**
     * @brief Signals the server to interrupt all operations and stop immediately.
     */
    void interrupt();

    /**
     * @brief Pauses the consumer loop and clears all pending work.
     * Waits for any in-flight block to finish processing before returning.
     */
    void pauseAndClearQueue();

    /**
     * @brief Resumes the consumer loop after a pause.
     */
    void resume();

private:
    ContractServer();
    ~ContractServer();

    // The main loop for the consumer thread.
    void consumerLoop();
    void processSingleBlock(CBlockIndex* pindex);

    // The queue holds block indices waiting to be processed.
    ThreadSafeQueue<CBlockIndex*> blockQueue_;

    // The thread pool executes individual contract transactions.
    boost::asio::thread_pool transactionPool_;
    
    // An atomic flag to signal all threads to stop.
    std::atomic<bool> stop_flag_{false};

    std::atomic<bool> paused_{false};
    std::mutex server_mutex_; // A mutex to protect pausing/resuming state
    std::condition_variable cv_paused_; // To signal when pause is complete
};

#endif //CONTRACT_SERVER_H