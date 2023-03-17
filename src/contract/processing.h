#ifndef BITCOIN_CONTRACT_PROCESSING_H
#define BITCOIN_CONTRACT_PROCESSING_H

#include "amount.h"
#include "contract/contract.h"
#include "primitives/transaction.h"

#include "util.h"
#include <leveldb/db.h>
#include <string>
#include <vector>

typedef unsigned char uchar;

bool ProcessContract(const Contract& contract, std::vector<CTxOut>& vTxOut, std::vector<uchar>& state, CAmount balance, std::vector<Contract>& nextContract);

class ContractDBWrapper
{
public:
    leveldb::DB* db;
    leveldb::Options options;
    leveldb::Status mystatus;
    // connect contract DB
    ContractDBWrapper();
    // disconnect contract DB
    ~ContractDBWrapper();
    // get state
    std::string getState(std::string key);
    // set state
    void setState(std::string key, void* buf, size_t size);
};

#endif // BITCOIN_CONTRACT_PROCESSING_H
