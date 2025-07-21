#ifndef CONTRACT_DB_H
#define CONTRACT_DB_H

#include "chain.h"
#include "primitives/transaction.h"
#include "contract/dbWrapper.h"
#include "json/json.hpp"

#include <memory>

using json = nlohmann::json;

class Snapshot: public ContractDBWrapper
{
public:
    explicit Snapshot(const std::string& name): ContractDBWrapper(name) {}
    virtual ~Snapshot() override = default;

    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;

    void setState(const uint256& address, const json& state);
    json getState(const uint256& address) const;
};

/**
 * @brief Acts as the central singleton manager for the entire contract state system.
 * @details It coordinates chain synchronization (via UpdatePolicy) and manages the
 * underlying data store (Snapshot) and its versioning (BlockInfo). This class
 * serves as the primary Facade for all external interactions.
 */
class ContractDB {
public:
    struct BlockInfo {
        int height = -1;
        std::string hash = {};

        BlockInfo() = default;

        // A helper to check if the info is valid.
        bool isValid() const { return height != -1; }
    };

    // --- Singleton Access ---
    static ContractDB& getInstance();
    
    ContractDB(const ContractDB&) = delete;
    ContractDB& operator=(const ContractDB&) = delete;

    // --- Core Public API ---
    void setTip(const int height, const std::string hash);
    BlockInfo getTip() const;

    void setContractState(const uint256& address, const json& state);
    json getContractState(const uint256& address) const;
    void clearAllState();

    void saveCheckpoint();
    void restoreToCheckpoint(int targetId);
    void purgeOldCheckpoints(int numToKeep);
    std::vector<CheckpointInfo> listCheckpoints() const;

    bool syncToChain(CChain& chainActive, const Consensus::Params& consensusParams);

private:
    ContractDB();
    ~ContractDB() = default;
    BlockInfo currentTip;

    std::unique_ptr<Snapshot> snapshot;
};

#endif