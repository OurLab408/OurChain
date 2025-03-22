#ifndef CONTRACT_UPDATEPOLICY_H
#define CONTRACT_UPDATEPOLICY_H

#include "chain.h"
#include "contract/cache.h"
#include "primitives/transaction.h"
#include "util.h"
#include "validation.h"

enum UpdatePolicyType {
    UpdatePolicy_DoNothing = 0,
    UpdatePolicy_Rebuild = 1,
    UpdatePolicy_Forward = 2,
    UpdatePolicy_Rollback = 3
};

std::string MakeZmqMsg(bool isPure, const std::string& address, std::vector<std::string> parameters, std::string preTxid);

void SendZmq(const std::string& message, std::string* buf);

class UpdatePolicy
{
public:
    virtual UpdatePolicyType getType() const { return UpdatePolicyType::UpdatePolicy_DoNothing; };
    virtual bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams) { return false; }
};

// 檢視鏈狀態和快照狀態，決定更新當前快照策略
UpdatePolicy* SelectUpdatePolicy(CChain& chainActive, ContractStateCache* cache);

// 完全重建策略, 從頭開始重建合約狀態與快照
class Rebuild : public UpdatePolicy
{
public:
    UpdatePolicyType getType() const override { return UpdatePolicyType::UpdatePolicy_Rebuild; }
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams) override;
};

// 狀態相同時，不需要更新
class DoNothing : public UpdatePolicy
{
public:
    UpdatePolicyType getType() const override { return UpdatePolicyType::UpdatePolicy_DoNothing; }
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams) override;
};

// 繼續策略, 繼續從上一個合約狀態快照繼續更新
class Forward : public UpdatePolicy
{
public:
    UpdatePolicyType getType() const override { return UpdatePolicyType::UpdatePolicy_Forward; }
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams) override;
};

// 回滾策略, 回到上一個合約狀態快照
class Rollback : public UpdatePolicy
{
public:
    UpdatePolicyType getType() const override { return UpdatePolicyType::UpdatePolicy_Rollback; }
    bool UpdateSnapShot(ContractStateCache& cache, SnapShot& snapshot, CChain& chainActive, const Consensus::Params consensusParams) override;
};

#endif // CONTRACT_UPDATEPOLICY_H