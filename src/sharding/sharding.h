#ifndef BITCOIN_SHARDING_SHARDING_H
#define BITCOIN_SHARDING_SHARDING_H

#include "uint256.h"

/* Get the tip of a given shard. */
CBlockIndex *get_shard_tip(unsigned shard);

#endif // BITCOIN_SHARDING_SHARDING_H
