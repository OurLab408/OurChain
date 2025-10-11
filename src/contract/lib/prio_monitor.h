/**
 * MODULE:      prio_monitor.h
 * COMPONENT:   API/Sync
 * PROJECT:     OurChain
 * DESCRIPTION:
 *   Priority-aware synchronization primitives:
 *     - CPrioritySemaphore<T>: semaphore that manages waiters by priority
 *     - CMonitor<T>: monitor with Hoare-style signaling (signal-and-wait),
 *       ensuring the signaling thread yields immediately to the resumed thread.
 *
 *   Notes:
 *     - T (priority type) must have a **total order** compatible with std::greater
 *       (i.e., defines a consistent operator< / operator>), so waiters can be
 *       ordered deterministically. (e.g., int, double, etc.).
 */
#ifndef CONTRACT_PMONITOR_H
#define CONTRACT_PMONITOR_H

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_types.hpp>
#include <queue>
#include <memory>
#include "sync.h"

template <typename T>
class CPrioritySemaphore {
private:
    using pqElem = std::pair<T, std::shared_ptr<boost::condition_variable>>;
    std::priority_queue<
        pqElem,
        std::vector<pqElem>,
        std::greater<pqElem>
    > pq;
    boost::mutex mutex;
    int value;

public:
    explicit CPrioritySemaphore(int init = 0) : value(init) {}

    void wait(T priority) 
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        if (value > 0) {
            value--;
            return;
        }
        auto my_cond = std::make_shared<boost::condition_variable>();
        pq.push({priority, my_cond});
        my_cond->wait(lock);
    }

    void post() 
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        std::shared_ptr<boost::condition_variable> cond_to_notify;
        if (pq.empty()) {
            value++;
        } else {
            cond_to_notify = pq.top().second;
            pq.pop();
        }
        
        if (cond_to_notify)
            cond_to_notify->notify_one();
    }
};

template <typename T>
class CMonitor {
private:
    CSemaphore mutex, next;
    CPrioritySemaphore<T> x_sem;
    int x_cnt, next_cnt;
    bool busy;

public:
    CMonitor() : mutex(1), next(0), x_sem(0) , x_cnt(0), next_cnt(0), busy(false) {}

    void acquire(T prio) 
    {
        mutex.wait();
        if (busy) {
            x_cnt++;
            if (next_cnt > 0)
                next.post();
            else
                mutex.post();
            x_sem.wait(prio);
            x_cnt--;
        }
        busy = true;
        if (next_cnt > 0)
            next.post();
        else
            mutex.post();
    }

    void release() 
    {
        mutex.wait();
        busy = false;
        if (x_cnt > 0) {
            next_cnt++;
            x_sem.post();
            next.wait();
            next_cnt--;
        }
        if (next_cnt > 0)
            next.post();
        else
            mutex.post();
    }

    ~CMonitor() = default;
};

#endif // CONTRACT_PMONITOR_H