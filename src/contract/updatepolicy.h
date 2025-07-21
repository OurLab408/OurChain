#ifndef CONTRACT_UPDATEPOLICY_H
#define CONTRACT_UPDATEPOLICY_H

#include "chain.h"
#include "primitives/transaction.h"
#include "util.h"
#include <memory>

class ContractDB;

enum UpdatePolicyType {
    UpdatePolicy_Rebuild = 1,
    UpdatePolicy_Forward = 2,
    UpdatePolicy_Rollback = 3
};

class UpdatePolicy
{
public:
    virtual ~UpdatePolicy() = default;

    virtual UpdatePolicyType getType() const = 0;
    
    virtual bool UpdateSnapshot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams) = 0;
};

// The factory function returning a smart pointer is a great design.
std::unique_ptr<UpdatePolicy> SelectUpdatePolicy(CChain& chainActive, ContractDB& cache);

class Rebuild : public UpdatePolicy
{
public:
    UpdatePolicyType getType() const override { return UpdatePolicyType::UpdatePolicy_Rebuild; }
    bool UpdateSnapshot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams) override;
};

class Forward : public UpdatePolicy
{
public:
    UpdatePolicyType getType() const override { return UpdatePolicyType::UpdatePolicy_Forward; }
    bool UpdateSnapshot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams) override;
};

class Rollback : public UpdatePolicy
{
public:
    UpdatePolicyType getType() const override { return UpdatePolicyType::UpdatePolicy_Rollback; }
    bool UpdateSnapshot(ContractDB& cache, CChain& chainActive, const Consensus::Params& consensusParams) override;
};

#endif // CONTRACT_UPDATEPOLICY_H