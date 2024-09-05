#ifndef CONTRACT_DB_WRAPPER_H
#define CONTRACT_DB_WRAPPER_H

#include "util.h"
#include <boost/thread/shared_mutex.hpp>
#include <rocksdb/db.h>
#include <string>
#include <vector>

#include <boost/thread/shared_mutex.hpp>
#include <shared_mutex>
typedef std::unique_lock<std::shared_mutex> WriteLock;
typedef std::shared_lock<std::shared_mutex> ReadLock;

extern std::shared_mutex tmp_contract_db_mutex;

struct CheckPointInfo {
    std::string tipBlockHash;
    int id;
};

class ContractDBWrapper
{
private:
    rocksdb::DB* db;
    rocksdb::Status mystatus;
    rocksdb::WriteOptions writeOptions;

    // 保存當前狀態快照到目標位置
    void saveDuplicateState(fs::path path, std::string metadata);

public:
    fs::path getContractDBPath(std::string name)
    {
        return (GetDataDir() / "contracts" / name);
    }

    std::string curPath;
    const fs::path CheckPointPath = getContractDBPath("checkPoint");

    // pre db operation status
    rocksdb::Status getStatus();
    // is pre db operation ok
    bool isOk();
    // set critical save (directly write to disk)
    void setCriticalSave();
    // get iterator, for custom operation on database
    rocksdb::Iterator* getIterator();
    // connect contract DB
    ContractDBWrapper(std::string name);
    // connect read only contract DB (mode should be "readOnly")
    ContractDBWrapper(std::string name, std::string mode);
    // disconnect contract DB
    ~ContractDBWrapper();

    // get state
    std::string getState(std::string key);
    // set state
    void setState(std::string key, std::string value);
    // delete state
    void deleteState(std::string key);
    // clear all states
    void clearAllStates();
    // save contract checkPoint
    void saveCheckPoint(std::string tipBlockHash);
    // save tmp state to fix file place, read only user will read it
    void saveTmpState();
    // restore check point
    bool restoreCheckPoint(int targetBackupId);
    // remove old check point
    void removeOldCheckPoint(int maxCheckPointCount);
    // find check point list
    std::vector<CheckPointInfo> findCheckPointList();
};


#endif // CONTRACT_DB_WRAPPER_H