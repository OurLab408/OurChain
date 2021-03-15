#include <gmp.h>
#include <string.h>
#include <string>
#include "arith_uint256.h"
#include "primitives/block.h"
using namespace std;
//b04902091
int64_t RewardAdjustment(const CBlockHeader& block);
uint256 GetEPowPerSecond(const CBlockHeader& block);
uint256 GetEPow(uint256 epowsub);
bool CheckEPow(const CBlockHeader& block);
int CompareEPow(const CBlockHeader& block1, const CBlockHeader& block2);
int CompareEPowPerSecond(const CBlockHeader& block1, const CBlockHeader& block2);