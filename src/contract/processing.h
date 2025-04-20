#ifndef CONTRACT_PROCESSING_H
#define CONTRACT_PROCESSING_H

#include "contract/cache.h"
#include "contract/contract.h"
#include "contract/dbWrapper.h"
#include "contract/state.h"
#include "primitives/transaction.h"

#include <leveldb/db.h>
#include <string>
#include <vector>
#include "chain.h"
#include "util.h"
#include "validation.h"

#define BYTE_READ_STATE 0
#define BYTE_WRITE_STATE 1
#define CHECK_RUNTIME_STATE 2
#define GET_PRE_TXID_STATE 3

bool ExecuteContract(const Contract &contract,
    const CTransactionRef &curTx,
    ContractStateCache *cache);

#endif // CONTRACT_PROCESSING_H
