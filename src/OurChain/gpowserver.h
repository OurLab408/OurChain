#ifndef OURCHAIN_GPOWSERVER_H
#define OURCHAIN_GPOWSERVER_H

#include <string>
#include <stdint.h>
#include <functional>

#include <mutex>
#include <condition_variable>

#include "chain.h"

/* OurChain Time-driven mechanism */
static const int ROUND_INTERVAL = 2;
static const int ROUND_HALF_INTERVAL = ROUND_INTERVAL / 2;
static const int64_t GENESIS_BLOCK_TIME = 1701836965;
static const uint32_t GENESIE_BLOCK_PRECISION_TIME = 158478;

//extern pthread_t chtid;
//extern sigset_t round_signal_set;

extern std::mutex cs_roundchange;
extern std::condition_variable cond_roundchange;
extern int64_t THIS_ROUND_START;

void InterruptMining();
void TuneRoundStartTime();
bool IsCurrentRoundBlock(CBlockIndex index);

bool InitGPoWServer();
bool StartGPoWServer();
void StopGPoWServer();


#endif // OURCHAIN_GPOWSERVER_H
