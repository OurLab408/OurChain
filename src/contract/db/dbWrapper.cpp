#include "dbWrapper.h"
#include <stdexcept>

// --- Private Helper Functions ---

// Returns the full path for a database or a related directory
fs::path ContractDBWrapper::getDBPath(const std::string& name) {
    return (GetDataDir() / "contracts" / name);
}

rocksdb::BackupEngine* ContractDBWrapper::getBackupEngine() const {
    if (!backupEngine_) {
        TryCreateDirectories(checkpointPath);
        rocksdb::BackupEngine* be_ptr;
        rocksdb::BackupEngineOptions backup_options(checkpointPath.string());
        rocksdb::Status s = rocksdb::BackupEngine::Open(
            rocksdb::Env::Default(), 
            backup_options, 
            &be_ptr
        );
        if (!s.ok()) {
            throw std::runtime_error("Failed to open backup engine: " + s.ToString());
        }
        backupEngine_.reset(be_ptr);
    }
    return backupEngine_.get();
}

// --- Constructors & Destructor ---

ContractDBWrapper::ContractDBWrapper(const std::string& name)
    : checkpointPath(getDBPath("checkpoints"))
{
    fs::path path = getDBPath(name);
    TryCreateDirectories(path);

    this->dbPath = path.string();

    rocksdb::Options options;
    options.create_if_missing = true;
    
    rocksdb::DB* raw_db_ptr;
    rocksdb::Status s = rocksdb::DB::Open(options, path.string(), &raw_db_ptr);

    if (!s.ok()) {
        throw std::runtime_error("Failed to open/create database '" + path.string() + "': " + s.ToString());
    }
    
    db.reset(raw_db_ptr);
}

std::unique_ptr<ContractDBWrapper> ContractDBWrapper::OpenForReadOnly(const std::string& name) {
    rocksdb::Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();

    fs::path path = getDBPath(name);
    
    rocksdb::DB* raw_db_ptr;
    rocksdb::Status s = rocksdb::DB::OpenForReadOnly(options, path.string(), &raw_db_ptr);

    if (!s.ok()) {
        // If opening read-only fails, throw an exception immediately.
        throw std::runtime_error("Failed to open database for read-only '" + path.string() + "': " + s.ToString());
    }
    
    auto wrapper = std::unique_ptr<ContractDBWrapper>(new ContractDBWrapper());
    wrapper->db.reset(raw_db_ptr);
    wrapper->dbPath = path.string();
    
    return wrapper;
}

// --- Public API ---

void ContractDBWrapper::enableSyncWrites() {
    writeOptions.sync = true;
}

bool ContractDBWrapper::getState(const std::string& key, std::string& value) const {
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), key, &value);

    if (s.ok()) {
        return true; // Found, 'value' has been populated.
    }
    if (s.IsNotFound()) {
        return false; // Not found.
    }
    // For other types of errors, throw an exception.
    throw std::runtime_error("Failed to get state for key '" + key + "': " + s.ToString());
}

void ContractDBWrapper::setState(const std::string& key, const std::string& value) {
    rocksdb::Status s = db->Put(writeOptions, key, value);
    if (!s.ok()) {
        throw std::runtime_error("Failed to set state for key '" + key + "': " + s.ToString());
    }
}

void ContractDBWrapper::deleteState(const std::string& key) {
    rocksdb::Status s = db->Delete(writeOptions, key);
    if (!s.ok()) {
        throw std::runtime_error("Failed to delete state for key '" + key + "': " + s.ToString());
    }
}

void ContractDBWrapper::clearAll() {
    rocksdb::Status s = db->DeleteRange(writeOptions, db->DefaultColumnFamily(), rocksdb::Slice(), rocksdb::Slice());
    if (!s.ok()) {
        throw std::runtime_error("Failed to clear all states: " + s.ToString());
    }
}

// --- Checkpoint Management ---

void ContractDBWrapper::saveCheckpoint(const int height, const std::string& hash) {
    WriteLock lock(backup_mutex_);
    json metadata;
    metadata["hash"] = hash;
    metadata["height"] = height;

    // Use the helper to get the engine, then perform the operation.
    rocksdb::Status s = getBackupEngine()->CreateNewBackupWithMetadata(db.get(), metadata.dump());
    if (!s.ok()) {
        throw std::runtime_error("Failed to save checkpoint: " + s.ToString());
    }
}

void ContractDBWrapper::restoreToCheckpoint(int targetBackupId) {
    WriteLock lock(backup_mutex_); // Exclusive lock for a destructive operation

    db->Close();
    db.reset();

    rocksdb::BackupEngine* backupEngine = getBackupEngine();
    rocksdb::Status s = backupEngine->RestoreDBFromBackup(targetBackupId, this->dbPath, this->dbPath);
    if (!s.ok()) {
        throw std::runtime_error("Failed to restore from backup ID " + std::to_string(targetBackupId) + ": " + s.ToString());
    }
    // If restore was successful, reopen the database.
    rocksdb::DB* raw_db_ptr;
    s = rocksdb::DB::Open(rocksdb::Options(), this->dbPath, &raw_db_ptr);
    if(!s.ok()) {
        throw std::runtime_error("Failed to reopen database after restore: " + s.ToString());
    }
    db.reset(raw_db_ptr);
}

void ContractDBWrapper::purgeOldCheckpoints(int numCheckpointsToKeep) {
    WriteLock lock(backup_mutex_); // Exclusive lock needed to delete backups

    // Use the helper to get the engine
    rocksdb::Status s = getBackupEngine()->PurgeOldBackups(numCheckpointsToKeep);
    if (!s.ok()) {
        throw std::runtime_error("Failed to purge old backups: " + s.ToString());
    }
}

std::vector<CheckpointInfo> ContractDBWrapper::listCheckpoints() const { 
    ReadLock lock(backup_mutex_); // Shared lock is sufficient to list info

    // Handle case where backup dir might not exist yet before trying to open it
    if (!fs::exists(checkpointPath)) {
        return {}; // Return an empty vector
    }
    
    std::vector<rocksdb::BackupInfo> backup_info;
    // Use the helper to get the engine
    getBackupEngine()->GetBackupInfo(&backup_info);
    
    std::vector<CheckpointInfo> result;
    result.reserve(backup_info.size());
    for (const auto& it : backup_info) {
        CheckpointInfo info;
        try {
            json metadata = json::parse(it.app_metadata);
            info.hash = metadata.at("hash").get<std::string>();
            info.height = metadata.at("height").get<int>();
            info.backupid = it.backup_id;
            result.push_back(info);
        } catch (const json::exception& e) {
            continue; // Safely ignore checkpoints with old/malformed metadata
        }
    }
    return result;
}