#ifndef CONTRACT_DB_WRAPPER_H
#define CONTRACT_DB_WRAPPER_H

#include "util.h"
#include <rocksdb/db.h>
#include <rocksdb/utilities/backup_engine.h>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>
#include "json/json.hpp"

using json = nlohmann::json;
using WriteLock = std::unique_lock<std::shared_mutex>;
using ReadLock = std::shared_lock<std::shared_mutex>;

struct CheckpointInfo {
    int backupid;
    int height;
    std::string hash;
};

class ContractDBWrapper {
private:
    std::unique_ptr<rocksdb::DB> db;
    rocksdb::WriteOptions writeOptions;
    std::string dbPath;
    
    mutable std::shared_mutex backup_mutex_;

    mutable std::unique_ptr<rocksdb::BackupEngine> backupEngine_;
    rocksdb::BackupEngine* getBackupEngine() const;

    ContractDBWrapper() = default;

    static fs::path getDBPath(const std::string& name);

public:
    const fs::path checkpointPath;

    /**
     * @brief Constructor to open or create a read-write database.
     * @param name The name of the database.
     * @throws std::runtime_error If the database cannot be opened or created.
     */
    explicit ContractDBWrapper(const std::string& name);

    /**
     * @brief Static factory function to open a read-only database.
     * @param name The name of the database.
     * @return A unique_ptr to a ContractDBWrapper instance.
     * @throws std::runtime_error If the database does not exist or cannot be opened in read-only mode.
     */
    static std::unique_ptr<ContractDBWrapper> OpenForReadOnly(const std::string& name);

    virtual ~ContractDBWrapper() = default;

    // Disable copy and assignment, as std::unique_ptr is non-copyable.
    ContractDBWrapper(const ContractDBWrapper&) = delete;
    ContractDBWrapper& operator=(const ContractDBWrapper&) = delete;

    // --- Public API ---

    /**
     * @brief Enables synchronous writes for all subsequent write operations.
     * @note This makes writes safer against system crashes at the cost of performance,
     * as write operations will wait for the data to be physically persisted to storage.
     */
    virtual void enableSyncWrites();

    /**
     * @brief Gets the value for a given key.
     * @param key The key to look up.
     * @param value [out] The value is stored in this string if the key is found.
     * @return True if the key was found, false if the key was not found.
     * @throws std::runtime_error on database read errors (e.g., I/O error, corruption).
     */
    virtual bool getState(const std::string& key, std::string& value) const;

    /**
     * @brief Sets (inserts or updates) the value for a given key.
     * @param key The key to set.
     * @param value The value to associate with the key.
     * @throws std::runtime_error on database write errors.
     */
    virtual void setState(const std::string& key, const std::string& value);

    /**
     * @brief Deletes a key-value pair from the database.
     * @param key The key to delete.
     * @throws std::runtime_error on database write errors.
     */
    virtual void deleteState(const std::string& key);

    /**
     * @brief Deletes all key-value pairs in the default column family of the database.
     * @note This is generally much more efficient than deleting keys one by one.
     * @throws std::runtime_error on failure.
     */
    virtual void clearAll();

    /**
     * @brief Efficiently gets the last key-value pair from the database.
     * @param key [out] The last key is stored here if found.
     * @param value [out] The last value is stored here if found.
     * @return True on success, or false if the database is empty.
     */
    virtual bool tryGetLast(std::string& key, std::string& value) const;

    // --- Checkpoint Management ---

    /**
     * @brief Creates a new checkpoint (backup) of the current database state.
     * @param tipBlockHash An application-specific metadata string (e.g., a block hash) to tag this checkpoint.
     * @throws std::runtime_error on failure to create the checkpoint.
     */
    virtual void saveCheckpoint(const int height, const std::string& hash);

    /**
     * @brief Performs a destructive, in-place restore of the database from a specific checkpoint ID.
     *
     * @param targetBackupId The integer ID of the checkpoint to restore from. This ID
     * should be obtained from the `listCheckpoints()` method.
     *
     * @warning This is a highly destructive operation. It permanently overwrites the
     * current live database with the state from the historical checkpoint.
     * The state of the database before this call is NOT saved and will be lost.
     *
     * @throws std::runtime_error If the specified backup ID is invalid, if the restore
     * operation itself fails, or if the database cannot be
     * reopened after a successful file restore.
     */
    virtual void restoreToCheckpoint(int targetBackupId);

    /**
     * @brief Purges old checkpoints, keeping only the N most recent ones.
     * @param numCheckpointsToKeep The number of the most recent checkpoints to preserve.
     * @throws std::runtime_error on failure.
     */
    virtual void purgeOldCheckpoints(int numCheckpointsToKeep);

    /**
     * @brief Lists all available checkpoints in the backup storage.
     * @return A std::vector of CheckpointInfo, each containing a backup ID and its metadata.
     * @note Returns an empty vector if no checkpoints exist.
     * @throws std::runtime_error on failure (other than the backup directory not being found).
     */
    virtual std::vector<CheckpointInfo> listCheckpoints() const;
};

#endif // CONTRACT_DB_WRAPPER_H