#include "contract/state.h"

ContractState::ContractState(ContractStateCache* cache)
{
    this->cache = cache;
}

bool ContractState::SyncState(CChain& chainActive, const Consensus::Params consensusParams)
{
    int curHeight = 0;
    {
        LOCK(cs_main);
        UpdatePolicy *curUpdatePolicy = SelectUpdatePolicy(chainActive, cache);
        if (curUpdatePolicy->getType() == UpdatePolicyType::UpdatePolicy_DoNothing) {
            return true;
        }
        SnapShot *snapshot = cache->getSnapShot();
        if (!curUpdatePolicy->UpdateSnapShot(*cache, *snapshot, chainActive, consensusParams)) {
            LogPrintf("snapshot: update error\n");
            return false;
        }
        curHeight = cache->getBlockCache()->getHeighestBlock().blockHeight;
        if (isSaveCheckPointNow(curHeight)) {
            cache->saveCheckPoint();
        }
    }
    // save tmp state
    cache->getSnapShot()->getDBWrapper()->saveTmpState();
    if (isClearCheckPointNow(curHeight)) {
        cache->clearCheckPoint(5);
    }
    return true;
}

bool ContractState::isSaveCheckPointNow(int height)
{
    if (height == 0)
        return false;
    return true;
}

bool ContractState::isClearCheckPointNow(int height)
{
    if (height > 0 && height % 5 == 0)
        return true;
    return false;
}