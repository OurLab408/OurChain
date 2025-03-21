#ifndef CONTRACT_OBSERVER_H
#define CONTRACT_OBSERVER_H

#include "chain.h"
#include "contract/cache.h"
#include "contract/updatepolicy.h"
#include "primitives/transaction.h"
#include "util.h"
#include "validation.h"

class ContractObserver
{
public:
    ContractObserver(ContractStateCache* cache);
    // 當區塊鏈狀態改變時觸發, 用於更新合約狀態快照
    bool onChainStateSet(CChain& chainActive, const Consensus::Params consensusParams);

private:
    ContractStateCache* cache;
    UpdateStrategyFactory updateStrategyFactory;

    bool isSaveCheckPointNow(int height);
    bool isClearCheckPointNow(int height);
};

#endif // CONTRACT_OBSERVER_H