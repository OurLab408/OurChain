#ifndef THREADSAFE_QUEUE
#define THREADSAFE_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cv_.notify_one(); // Notify one waiting thread
    }

    // wait_and_pop will block until an item is available.
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]{ return !queue_.empty(); });
        value = std::move(queue_.front());
        queue_.pop();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        // The standard way to clear a std::queue is to swap it with an empty one.
        std::queue<T> empty_queue;
        queue_.swap(empty_queue);
    }
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

#endif //THREADSAFE_QUEUE