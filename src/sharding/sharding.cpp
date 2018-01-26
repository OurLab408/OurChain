#include "chain.h"
#include "txmempool.h"
#include "nodeid.h"
#include "uint256.h"
#include "sharding.h"

/*
 * Compare the priority of the given two blocks for blockchain finalization.
 * Return true if priority(a) > priority(b).
 */
bool compare_block_priority(const CBlockIndex *a, const CBlockIndex *b)
{
    /* Complete implementation should not have equal priority */
    return a->GetBlockTime() < b->GetBlockTime();
}

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

CBlockIndex *get_shard_tip(unsigned shard)
{
    /* See RPC getchaintips */
    std::set<CBlockIndex*, CompareBlocksByHeight> setTips;
    std::set<CBlockIndex*> setOrphans;
    std::set<CBlockIndex*> setPrevs;

    for (const std::pair<const uint256, CBlockIndex*>& item : mapBlockIndex)
    {
        if (!chainActive.Contains(item.second)) {
            setOrphans.insert(item.second);
            setPrevs.insert(item.second->pprev);
        }
    }

    for (std::set<CBlockIndex*>::iterator it = setOrphans.begin(); it != setOrphans.end(); ++it)
    {
        if (setPrevs.erase(*it) == 0) {
            setTips.insert(*it);
        }
    }

    setTips.insert(chainActive.Tip());

    CBlockIndex *earliest = nullptr;
    for (CBlockIndex* block : setTips) {
        if (getGroupFromUint256(block->GetBlockHash(), mempool.nbGroups()) == shard && (earliest == nullptr || compare_block_priority(block, earliest) == true)) {
            earliest = block;
        }
    }

    return earliest;
}
