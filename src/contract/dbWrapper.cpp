#include "contract/dbWrapper.h"
#include <boost/filesystem.hpp>
#include "rocksdb/utilities/backup_engine.h"

std::shared_mutex tmp_contract_db_mutex;

ContractDBWrapper::ContractDBWrapper(std::string name)
{
    rocksdb::Options options;
    options.create_if_missing = true;
    fs::path path = getContractDBPath(name);
    TryCreateDirectories(path);
    mystatus = rocksdb::DB::Open(options, path.string(), &db);
    assert(mystatus.ok());
    this->curPath = path.string();
}

ContractDBWrapper::ContractDBWrapper(std::string name, std::string mode)
{
    assert(mode == "readOnly");
    fs::path path;
    rocksdb::Options options;
    path = getContractDBPath(name);
    if (mode == "readOnly") {
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        mystatus = rocksdb::DB::OpenForReadOnly(options, path.string(), &db);
    }
    if (!mystatus.ok()) {
        TryCreateDirectories(path);
        rocksdb::Options newOptions;
        newOptions.create_if_missing = true;
        mystatus = rocksdb::DB::Open(newOptions, path.string(), &db);
    }
    assert(mystatus.ok());
    this->curPath = path.string();
}

ContractDBWrapper::~ContractDBWrapper()
{
    delete db;
    db = nullptr;
}

rocksdb::Status ContractDBWrapper::getStatus() { return mystatus; }
bool ContractDBWrapper::isOk() { return mystatus.ok(); }
// set critical save
void ContractDBWrapper::setCriticalSave() { writeOptions.sync = true; }
rocksdb::Iterator *ContractDBWrapper::getIterator()
{
    return db->NewIterator(rocksdb::ReadOptions());
}

std::string ContractDBWrapper::getState(std::string key)
{
    std::string value;
    mystatus = db->Get(rocksdb::ReadOptions(), key, &value);
    return value;
}

void ContractDBWrapper::setState(std::string key, std::string value)
{
    mystatus = db->Put(rocksdb::WriteOptions(), key, value);
}

void ContractDBWrapper::deleteState(std::string key)
{
    mystatus = db->Delete(rocksdb::WriteOptions(), key);
}

void ContractDBWrapper::clearAllStates()
{
    // use delete range to clear all states
    mystatus = db->DeleteRange(
        rocksdb::WriteOptions(), rocksdb::Slice(), rocksdb::Slice());
    assert(mystatus.ok());
}

void ContractDBWrapper::saveDuplicateState(fs::path path, std::string metadata)
{
    rocksdb::BackupEngine *backup_engine;
    rocksdb::BackupEngineOptions backup_engine_options(path.string());
    mystatus = rocksdb::BackupEngine::Open(
        rocksdb::Env::Default(), backup_engine_options, &backup_engine);
    assert(mystatus.ok());
    backup_engine->CreateNewBackupWithMetadata(db, metadata);
    delete backup_engine;
}

// save contract checkPoint
void ContractDBWrapper::saveCheckPoint(std::string tipBlockHash)
{
    TryCreateDirectories(CheckPointPath);
    saveDuplicateState(CheckPointPath, tipBlockHash);
}

void ContractDBWrapper::saveTmpState()
{
    fs::path path = getContractDBPath("tmp");
    TryCreateDirectories(path);
    WriteLock lock(tmp_contract_db_mutex);
    // restore from cur checkPoint
    rocksdb::BackupEngine *backup_engine;
    rocksdb::BackupEngineOptions backup_engine_options(CheckPointPath.string());
    mystatus = rocksdb::BackupEngine::Open(
        rocksdb::Env::Default(), backup_engine_options, &backup_engine);
    assert(mystatus.ok());
    backup_engine->RestoreDBFromLatestBackup(path.string(), path.string());
    lock.unlock();
    delete backup_engine;
}

// find check point list
std::vector<CheckPointInfo> ContractDBWrapper::findCheckPointList()
{
    std::vector<CheckPointInfo> checkPointList;
    rocksdb::BackupEngine *backup_engine;
    rocksdb::BackupEngineOptions backup_engine_options(CheckPointPath.string());
    mystatus = rocksdb::BackupEngine::Open(
        rocksdb::Env::Default(), backup_engine_options, &backup_engine);
    assert(mystatus.ok());
    std::vector<rocksdb::BackupInfo> backup_info;
    backup_engine->GetBackupInfo(&backup_info);
    for (auto it = backup_info.begin(); it != backup_info.end(); it++) {
        CheckPointInfo info;
        info.tipBlockHash = it->app_metadata;
        info.id = it->backup_id;
        checkPointList.push_back(info);
    }
    delete backup_engine;
    return checkPointList;
}

bool ContractDBWrapper::restoreCheckPoint(int targetBackupId)
{
    mystatus = db->Close();
    assert(mystatus.ok());
    // restore from target checkPoint
    rocksdb::BackupEngine *backup_engine;
    rocksdb::BackupEngineOptions backup_engine_options(CheckPointPath.string());
    rocksdb::Status status = rocksdb::BackupEngine::Open(
        rocksdb::Env::Default(), backup_engine_options, &backup_engine);
    if (!status.ok()) {
        return false;
    }
    fs::path path = this->curPath;
    auto result = backup_engine->RestoreDBFromBackup(
        targetBackupId, path.string(), path.string());
    if (!result.ok()) {
        return false;
    }
    delete backup_engine;
    // reopen db
    mystatus = rocksdb::DB::Open(rocksdb::Options(), path.string(), &db);
    assert(mystatus.ok());
    return true;
}

void ContractDBWrapper::removeOldCheckPoint(int maxCheckPointCount)
{
    // rocksdb backup limit recent checkpoint
    rocksdb::BackupEngine *backup_engine;
    rocksdb::BackupEngineOptions backup_engine_options(CheckPointPath.string());
    rocksdb::Status status = rocksdb::BackupEngine::Open(
        rocksdb::Env::Default(), backup_engine_options, &backup_engine);
    assert(status.ok());
    backup_engine->PurgeOldBackups(maxCheckPointCount);
}