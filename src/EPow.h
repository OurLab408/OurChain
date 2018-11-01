#include <gmp.h>
#include <string.h>
#include <string>
#include "arith_uint256.h"
#include "primitives/block.h"
using namespace std;
//b04902091
int64_t RewardAdjustment(CBlock block);
uint256 GetEPowPerSecond(uint256 epow,uint32_t time);
uint256 GetEPow(uint256 hash2,uint256 hash);
bool CheckEPow(uint256 hash2,uint256 hash,uint32_t time);
