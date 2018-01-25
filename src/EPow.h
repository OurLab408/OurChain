#include <gmp.h>
#include <string.h>
#include <string>
#include "arith_uint256.h"
#include "primitives/block.h"
using namespace std;
//b04902091
void f1(mpq_t d,mpq_t x);
void f2(mpq_t d,mpq_t x);
uint256 GetEPow(uint256 hash2,uint256 hash);
bool CheckEPow(uint256 hash2,uint256 hash);
