#ifndef BITCOIN_CONTRACT_PROCESSING_H
#define BITCOIN_CONTRACT_PROCESSING_H

#include "contract/contract.h"
#include "primitives/transaction.h"
#include "amount.h"

#include <vector>

typedef unsigned char uchar;

int RecoverContractState(const uint256& contract);
int WriteContractState(const uint256& contract);
bool CheckBackup(const uint256& contract);

bool ProcessContract(const Contract &contract, std::vector<CTxOut> &vTxOut, std::vector<uchar> &state, CAmount balance,
					 std::vector<Contract> &nextContract,  bool fJustCheck);
					 
#endif // BITCOIN_CONTRACT_PROCESSING_H
