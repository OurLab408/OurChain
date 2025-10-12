#ifndef CONTRACT_PROCESSING_H
#define CONTRACT_PROCESSING_H

#include "contract/contract.h"
#include "contract/db/contractdb.h"
#include "contract/db/dbWrapper.h"
#include "primitives/transaction.h"

void ExecuteContract(const Contract& contract, const CTransactionRef& current_tx, ContractDB& contract_db);

#endif // CONTRACT_PROCESSING_H
