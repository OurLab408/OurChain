#include "chain.h"
#include "txmempool.h"
#include "nodeid.h"
#include "uint256.h"
#include "sharding.h"

/* See RPC getchaintips */
struct CompareBlocksByHeight
{
    bool operator()(const CBlockIndex* a, const CBlockIndex* b) const
    {
        /* Make sure that unequal blocks with the same height do not compare
           equal. Use the pointers themselves to make a distinction. */

        if (a->nHeight != b->nHeight)
          return (a->nHeight > b->nHeight);

        return a < b;
    }
};

const CBlockIndex *get_shard_tip(unsigned shard)
{
    /* See RPC getchaintips */
    std::set<const CBlockIndex*, CompareBlocksByHeight> setTips;
    std::set<const CBlockIndex*> setOrphans;
    std::set<const CBlockIndex*> setPrevs;

    for (const std::pair<const uint256, CBlockIndex*>& item : mapBlockIndex)
    {
        if (!chainActive.Contains(item.second)) {
            setOrphans.insert(item.second);
            setPrevs.insert(item.second->pprev);
        }
    }

    for (std::set<const CBlockIndex*>::iterator it = setOrphans.begin(); it != setOrphans.end(); ++it)
    {
        if (setPrevs.erase(*it) == 0) {
            setTips.insert(*it);
        }
    }

    setTips.insert(chainActive.Tip());

    for (const CBlockIndex* block : setTips) {
        /* TODO: Handle forks in a same shard */
        if (getGroupFromUint256(block->GetBlockHash(), mempool.nbGroups()) == shard) return block;
    }

    return NULL;
}
