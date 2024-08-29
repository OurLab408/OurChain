#include "gpow.h"
#include "OurChain/gpowserver.h"
#include "timedata.h"
#include "util.h"

#include <event2/event.h>
#include <event2/util.h>
#include <future>
#include <unistd.h>

#include "support/events.h"


//! libevent event loop
static struct event_base* eventBase = 0;
//! HTTP server
struct event* eventGPoW = 0;
struct event* eventStopMining = 0;

// Half round duration
struct timeval tv{ROUND_INTERVAL, 0};
//struct timeval mining_tv{1, 800000}; // Valid mining duration
struct timeval mining_tv{1, 0}; // Valid mining duration
struct timeval now;

std::mutex cs_roundchange;
std::condition_variable cond_roundchange;
int64_t THIS_ROUND_START;

/** libevent event log callback */
static void libevent_log_cb(int severity, const char *msg)
{
#ifndef EVENT_LOG_WARN
// EVENT_LOG_WARN was added in 2.0.19; but before then _EVENT_LOG_WARN existed.
# define EVENT_LOG_WARN _EVENT_LOG_WARN
#endif
    if (severity >= EVENT_LOG_WARN) // Log warn messages and higher without debug category
        LogPrintf("libevent: %s\n", msg);
    else
        LogPrint(BCLog::LIBEVENT, "libevent: %s\n", msg);
}

void InterruptMining()
{
    std::lock_guard<std::mutex> lock(cs_roundchange);
    fAllowedMining = false;
}

void TuneRoundStartTime()
{
    InterruptMining();
    std::unique_lock<std::mutex> lock(cs_roundchange);
    while(1) {
        if ((GetAdjustedTime() - GENESIS_BLOCK_TIME) % ROUND_INTERVAL == ROUND_HALF_INTERVAL) {
            while(1) {
                cond_roundchange.wait_for(lock, std::chrono::milliseconds(5), []{ return ((GetAdjustedTime() - GENESIS_BLOCK_TIME) % ROUND_INTERVAL == 0);});
                if ((GetAdjustedTime() - GENESIS_BLOCK_TIME) % ROUND_INTERVAL == 0)
                    break;
            }
            break;
        }
    }
}

bool IsCurrentRoundBlock(CBlockIndex index)
{
    if (index.GetBlockTime() == THIS_ROUND_START)
        return true;
    return false;
}

bool InitGPoWServer()
{
    // Redirect libevent's logging to our own log
    event_set_log_callback(&libevent_log_cb);

    raii_event_base base_ctr = obtain_event_base();

    /* Create a new evhttp object to handle requests. */
    raii_event gpow_ctr = obtain_event(base_ctr.get(), -1, 0, nullptr, nullptr);
    raii_event nomining_ctr = obtain_event(base_ctr.get(), -1, 0, nullptr, nullptr);
    struct event* evgpow = gpow_ctr.get();
    if (!evgpow) {
        LogPrintf("GPoW Server: Could not create evgpow. Exiting.\n");
        return false;
    }

    eventBase = base_ctr.release();
    eventGPoW = gpow_ctr.release();
    eventStopMining = nomining_ctr.release();

    return true;
}

static void NoMiningCallback(evutil_socket_t fd, short event, void* arg)
{   
    //gettimeofday(&now, NULL);
    {
        std::lock_guard<std::mutex> lock(cs_roundchange);
        fAllowedMining = false;
    }
    cond_roundchange.notify_all();
    //LogPrintf("No Mining %ld.%06ld\n", now.tv_sec, now.tv_usec);
}

static void timeoutCallback(evutil_socket_t fd, short event, void* arg)
{
    //gettimeofday(&now, NULL);
    event_add(eventStopMining, &mining_tv);
    {
        std::lock_guard<std::mutex> lock(cs_roundchange);
        THIS_ROUND_START += ROUND_INTERVAL;
        fAllowedMining = true;
    }
    cond_roundchange.notify_all();
    //LogPrintf("DEBUG %ld.%06ld\n", now.tv_sec, now.tv_usec);
}


std::thread *threadGPoW;

bool StartGPoWServer()
{
    LogPrintf("GPoW Server Start\n");

    event_assign(eventGPoW, eventBase, -1, EV_PERSIST, timeoutCallback, (void*) eventGPoW);
    event_assign(eventStopMining, eventBase, -1, EV_TIMEOUT, NoMiningCallback, (void*) eventStopMining);

    // Tune start time
    TuneRoundStartTime();
    THIS_ROUND_START = GetAdjustedTime();
    fAllowedMining = true;

    event_add(eventGPoW, &tv);
    threadGPoW = new std::thread([&]() {
                    event_base_dispatch(eventBase);
                });
    
    return true;
}

void StopGPoWServer()
{
    if (eventBase) {
        event_base_loopbreak(eventBase);
        threadGPoW->join();
    }

    if (eventGPoW) {
        event_free(eventGPoW);
        eventGPoW = 0;
    }

    if (eventBase) {
        event_base_free(eventBase);
        eventBase = 0;
    }
}
