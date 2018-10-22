#ifndef BITCOIN_CONTRACT_PROCESSING_H
#define BITCOIN_CONTRACT_PROCESSING_H

#include "contract/contract.h"

bool ProcessContract(const std::string &txid, const Contract &contract);

#endif // BITCOIN_CONTRACT_PROCESSING_H
