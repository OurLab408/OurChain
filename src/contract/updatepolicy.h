#ifndef CONTRACT_UPDATESTRATEGY_H
#define CONTRACT_UPDATESTRATEGY_H

#include "chain.h"
#include "contract/cache.h"
#include "primitives/transaction.h"
#include "util.h"
#include "validation.h"

enum UpdateStrategyType {
    UpdateStrategyTypeUnDo = 0,
    UpdateStrategyTypeRebuild = 1,
    UpdateStrategyTypeContinue = 2,
    UpdateStrategyTypeRollback = 3
};

std::string parseZmqMsg(bool isPure, const std::string& address, std::vector<std::string> parameters, std::string preTxid);

void zmqPushMessage(const std::string& message, std::string* buf);

class UpdateStrategy
{
public:
    virtual UpdateStrategyType getName() { return UpdateStrategyType::UpdateStrategyTypeUnDo; };
    virtual bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams) { return false; }
};

// 完全重建策略, 從頭開始重建合約狀態與快照
class UpdateStrategyRebuild : public UpdateStrategy
{
public:
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams);
    UpdateStrategyType getName()
    {
        return UpdateStrategyType::UpdateStrategyTypeRebuild;
    }
};

// 狀態相同時，不需要更新
class UpdateStrategyUnDo : public UpdateStrategy
{
public:
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams);
    UpdateStrategyType getName()
    {
        return UpdateStrategyType::UpdateStrategyTypeUnDo;
    }
};

// 繼續策略, 繼續從上一個合約狀態快照繼續更新
class UpdateStrategyContinue : public UpdateStrategy
{
public:
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams);
    UpdateStrategyType getName()
    {
        return UpdateStrategyType::UpdateStrategyTypeContinue;
    }
};

// 回滾策略, 回到上一個合約狀態快照
class UpdateStrategyRollback : public UpdateStrategy
{
public:
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapShot, CChain& chainActive, const Consensus::Params consensusParams);
    UpdateStrategyType getName()
    {
        return UpdateStrategyType::UpdateStrategyTypeRollback;
    }
};

// 檢視鏈狀態和快照狀態，決定更新當前快照策略
class UpdateStrategyFactory
{
public:
    UpdateStrategy* createUpdateStrategy(CChain& chainActive, ContractStateCache* cache);
};

#endif // CONTRACT_UPDATESTRATEGY_H