#ifndef CONTRACT_CACHE_H
#define CONTRACT_CACHE_H
#include "contract/contract.h"
#include "contract/dbWrapper.h"
#include "json/json.hpp"

using json = nlohmann::json;


class SnapShot
{
public:
    SnapShot(std::string name);
    ~SnapShot();

    void setContractState(uint256 address, json state);
    json getContractState(std::string address);
    void clear();
    // duplicate sanpshot to checkPoint folder
    void saveCheckPoint(std::string tipBlockHash);
    ContractDBWrapper *getDBWrapper();

private:
    ContractDBWrapper *dbWrapper;
};

// 用來追蹤對合約模塊來說的當前區塊鏈狀態
class BlockCache
{
public:
    struct blockIndex {
        uint256 blockHash;
        int blockHeight;

        blockIndex()
        {
            blockHash = uint256();
            blockHeight = -1;
        }
        blockIndex(uint256 blockHash, int blockHeight)
        {
            this->blockHash = blockHash;
            this->blockHeight = blockHeight;
        }
    };

    BlockCache();
    ~BlockCache();
    void clear();
    void setBlockIndex(uint256 blockHash, int blockHeight);
    uint256 getBlockHash(int blockHeight);
    blockIndex getHeighestBlock();
    void removeBlockIndex(int blockHeight);

private:
    ContractDBWrapper *dbWrapper;
    std::string intToKey(int num)
    {
        std::string key;
        key.resize(sizeof(int));
        for (size_t i = 0; i < sizeof(int); ++i) {
            key[sizeof(int) - i - 1] = (num >> (i * 8)) & 0xFF;
        }
        return key;
    }
    int keyToInt(const std::string &key)
    {
        int num = 0;
        for (size_t i = 0; i < sizeof(int); ++i) {
            num = (num << 8) | static_cast<unsigned char>(key[i]);
        }
        return num;
    }
};

class ContractStateCache
{
public:
    ContractStateCache();
    ~ContractStateCache();
    // 當前合約快照
    SnapShot *getSnapShot();
    void clearSnapShot();
    // 合約快照所追蹤的區塊鏈狀態(最高區塊高度)
    bool getFirstBlockCache(BlockCache::blockIndex &blockIndex);
    BlockCache *getBlockCache();
    // 更新合約狀態追蹤的區塊鏈狀態
    void pushBlock(BlockCache::blockIndex blockIndex);
    // 更新合約狀態追蹤的區塊鏈狀態
    void popBlock();
    // 保存合約狀態快照(checkPoint)
    void saveCheckPoint();
    // 恢復合約狀態快照到目標 checkPoint
    bool restoreCheckPoint(std::string tipBlockHash,
        std::vector<CheckPointInfo> checkPointList);
    // clear old checkPoint in checkPoint folder(保留幾個 block 內的 check
    // point)
    void clearCheckPoint(int maxCheckPointCount);
    // get checkPoint list
    std::vector<CheckPointInfo> getCheckPointList();
    // get contract dir
    fs::path getContractPath(std::string name);

private:
    BlockCache *blockCache;
    SnapShot *snapShot;
};

#endif // CONTRACT_CACHE_H
