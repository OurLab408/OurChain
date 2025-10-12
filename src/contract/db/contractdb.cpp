#include "contractdb.h"
#include "../updatepolicy.h"
#include "validation.h"

void Snapshot::setState(const uint256& address, const json& state)
{
    ContractDBWrapper::setState(address.ToString(), state.dump());
}

json Snapshot::getState(const uint256& address) const
{
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
{
}

// --- Public Methods ---

ContractDB& ContractDB::getInstance()
{
    static ContractDB instance;
    return instance;
}

void ContractDB::setTip(const int height, const std::string hash)
{
    currentTip.height = height;
    currentTip.hash = hash;
}

ContractDB::BlockInfo ContractDB::getTip() const
{
    return currentTip;
}


void ContractDB::clearAllState()
{
    snapshot->clearAll();
}

// --- State Buffer Implementation (Your Design) ---

json& ContractDB::getState(const uint256& address)
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);

    std::string address_str = address.GetHex();

    // Lazy loading: if not in buffer, load from DB
    if (state_buffer_.find(address_str) == state_buffer_.end()) {
        json db_state = snapshot->getState(address);
        state_buffer_[address_str] = (db_state.is_null()) ? json::object() : db_state;
    }

    return state_buffer_[address_str];
}

void ContractDB::commitBuffer()
{
    std::lock_guard<std::mutex> lock(buffer_mutex_);

    // Batch write all buffer states to DB sequentially
    for (const auto& [address_str, state] : state_buffer_) {
        uint256 address = uint256S(address_str);
        snapshot->setState(address, state);
    }

    // Clear buffer after committing
    state_buffer_.clear();
}

void ContractDB::saveCheckpoint()
{
    if (currentTip.isValid()) {
        snapshot->saveCheckpoint(currentTip.height, currentTip.hash);
    }
}

void ContractDB::restoreToCheckpoint(int targetId)
{
    snapshot->restoreToCheckpoint(targetId);
}

void ContractDB::purgeOldCheckpoints(int numToKeep)
{
    snapshot->purgeOldCheckpoints(numToKeep);
}

std::vector<CheckpointInfo> ContractDB::listCheckpoints() const
{
    return snapshot->listCheckpoints();
}

bool ContractDB::syncToChain(CChain& chainActive, const Consensus::Params& consensusParams)
{
    std::unique_ptr<UpdatePolicy> curUpdatePolicy{SelectUpdatePolicy(chainActive, *this)};

    if (!curUpdatePolicy->UpdateSnapshot(*this, chainActive, consensusParams)) {
        LogPrintf("snapshot: update error\n");
        return false;
    }

    return true;
}
