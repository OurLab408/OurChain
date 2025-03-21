#include "contract/state.h"

ContractObserver::ContractObserver(ContractStateCache* cache)
{
    this->cache = cache;
    this->updateStrategyFactory = UpdateStrategyFactory();
}

bool ContractObserver::onChainStateSet(CChain& chainActive, const Consensus::Params consensusParams)
{
    int curHeight = 0;
    {
        LOCK(cs_main);
        auto curUpdateStrategy = updateStrategyFactory.createUpdateStrategy(chainActive, cache);
        if (curUpdateStrategy->getName() == UpdateStrategyType::UpdateStrategyTypeUnDo) {
            return true;
        }
        auto snapshot = cache->getSnapShot();
        if (!curUpdateStrategy->UpdateSnapShot(*cache, *snapshot, chainActive, consensusParams)) {
            LogPrintf("snapshot: update\n");
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

bool ContractObserver::isSaveCheckPointNow(int height)
{
    if (height == 0)
        return false;
    return true;
}

bool ContractObserver::isClearCheckPointNow(int height)
{
    if (height == 0)
        return false;
    if (height % 5 == 0)
        return true;
    return false;
}