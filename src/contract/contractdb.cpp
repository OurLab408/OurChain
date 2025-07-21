#include "contract/contractdb.h"
#include "contract/updatepolicy.h"
#include "validation.h"

void Snapshot::setState(const uint256& address, const json& state) {
    ContractDBWrapper::setState(address.ToString(), state.dump());
}

json Snapshot::getState(const uint256& address) const {
    std::string stateStr;
    
    if (ContractDBWrapper::getState(address.ToString(), stateStr)) {
        if (stateStr.empty()) return nullptr; // Handle case of empty but existing string
        return json::parse(stateStr);
    }
    // Return JSON null if the key is not found at all.
    return nullptr;
}

ContractDB::ContractDB()
    : snapshot(new Snapshot("contract_state"))
{}

// --- Public Methods ---

ContractDB& ContractDB::getInstance() {
    static ContractDB instance;
    return instance;
}

void ContractDB::setTip(const int height, const std::string hash) {
    currentTip.height = height;
    currentTip.hash = hash;
}

ContractDB::BlockInfo ContractDB::getTip() const {
    return currentTip;
}

void ContractDB::setContractState(const uint256& address, const json& state) {
    snapshot->setState(address, state);
}

json ContractDB::getContractState(const uint256& address) const {
    return snapshot->getState(address);
}

void ContractDB::clearAllState() { 
    snapshot->clearAll();
}

void ContractDB::saveCheckpoint() {
    if (currentTip.isValid()) {
        snapshot->saveCheckpoint(currentTip.height, currentTip.hash);
    }
}

void ContractDB::restoreToCheckpoint(int targetId) {
    snapshot->restoreToCheckpoint(targetId);
}

void ContractDB::purgeOldCheckpoints(int numToKeep) {
    snapshot->purgeOldCheckpoints(numToKeep);
}

std::vector<CheckpointInfo> ContractDB::listCheckpoints() const {
    return snapshot->listCheckpoints();
}

bool ContractDB::syncToChain(CChain& chainActive, const Consensus::Params& consensusParams)
{   
    std::unique_ptr<UpdatePolicy> curUpdatePolicy {SelectUpdatePolicy(chainActive, *this)};

    if (!curUpdatePolicy->UpdateSnapshot(*this, chainActive, consensusParams)) {
        LogPrintf("snapshot: update error\n");
        return false;
    }

    return true;
}