#ifndef CONTRACT_PROCESSING_H
#define CONTRACT_PROCESSING_H

#include "contract/contract.h"
#include "contract/dbWrapper.h"
#include "primitives/transaction.h"
#include "contract/contractdb.h"

#include <string>
#include <vector>

bool ExecuteContract(const Contract& contract, const CTransactionRef& curTx, ContractDB& cache);

#endif // CONTRACT_PROCESSING_H
