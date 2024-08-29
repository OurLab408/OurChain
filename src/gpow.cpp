// Copyright (c) 2020- The OurChain Core developers
// Distributed under the Benevolence software license, see
// https://hackmd.io/KpMx2d-wQd2t_gwQ97D9Cg#Benevolence-license.

#include "arith_uint256.h"
#include "cstdlib"
#include "hash.h"
#include "string.h"
#include "uint256.h"
#include "util.h"
#include <univalue.h>

using namespace std;

#include "gpow.h"

int Debug = 1;
bool (*CONDITION)();

uint8_t NONCE_TYPE; // :1　not Conservative :1 not Compressed :4 Bit Size (max 15)

T m;
T N; // will be dicided by block interval
uint32_t nBits;
bool _conservative, _nonce_compressed;
uint256 bnPowLimit;
uint256 bnTarget;
const char* sMaxTarget = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
double fMaxTarget;

bool volatile fAllowedMining;

T nInnerLoopCount;

UniValue ToUniv(const GNonces& nNonce)
{
    UniValue result(UniValue::VOBJ);
    result.push_back(Pair("nonce", (int)nNonce.getNonce()));
    result.push_back(Pair("m", (int)m));
    result.push_back(Pair("nonce type", NONCE_TYPE));
    return result;
}

std::string GNonces::ToString() const
{
    std::string str;
    str += strprintf("\n(nonce=%06u m= %4u n= %6u %6u nBits=0x%08x NONCE_TYPE=0x%02x",
                     nonce,
                     m,
                     n, N,
                     GetNBits(),
                     NONCE_TYPE);
    str += strprintf("\nQ=%04u InnerLoopCount %i array=0x", q.size(), nInnerLoopCount);
    for (auto const& e : q) {
        str += strprintf("\n%06i %s", e.first, e.second.ToString());
    }
    PQ tmpq = pq;
    if (!tmpq.empty()) str += strprintf("\n");
    while (!tmpq.empty()) {
        str += strprintf("\n%06i %s", tmpq.top().first, tmpq.top().second.ToString());
        tmpq.pop();
    }
    str += strprintf("\n)");
    return str;
}

extern void SetArith(int n);
extern void ClearArith2();

void InitGPoW(T& _m, T& _n, int& cons, int& comp, uint32_t& nbits)
{
    m = _m;
    N = _n;
    // assert(m > 0);

    nBits = nbits;
    GPoWSetCompact(bnTarget);

    _conservative = (cons == CONSERVATIVE) || (cons == 1);
    _nonce_compressed = (comp == NONCE_COMPRESSED) || (comp == 1);

    NONCE_TYPE = CONSERVATIVE * _conservative | NONCE_COMPRESSED * _nonce_compressed;

    arith_uint288 tmp;
    uint256 max = uint256S(sMaxTarget);
    tmp += max;
    fMaxTarget = tmp.getdouble();
}


T Getm()
{
    return m;
}

T GetN()
{
    return N;
}

uint32_t GetNBits()
{
    return nBits;
}

double fGetUint256(uint256& hash)
{
    arith_uint288 tmp;
    tmp += hash;
    double d = tmp.getdouble();
    return d / fMaxTarget;
}

double fGetNBits()
{
    return fGetUint256(bnTarget);
}

int CompareTo(uint256& a, uint256& b)
{
    int WIDTH = a.size() >> 2;

    for (int i = WIDTH - 1; i >= 0; i--) {
        uint32_t _a = *(uint32_t*)(a.begin() + (i << 2)), _b = *(uint32_t*)(b.begin() + (i << 2));
        if (_a > _b) return 1;
        if (_a < _b) return -1;
    }
    return 0;
}

bool GPoWSetCompact(uint256& bnTarget)
{
    if (CompareTo(bnPowLimit, bnTarget) < 0) {
        my_eprintf("Target Out of PowLimit!\n%s\n%s", bnPowLimit.ToString().c_str(), bnTarget.ToString().c_str());
        return false;
    }
    bnTarget.SetNull();
    uint32_t nCompact = GetNBits();
    int nSize = nCompact >> 24; // assert(nSize < 30)
    uint32_t nWord = nCompact & 0x00ffffff;
    if (nSize <= 3) {
        nWord >>= 8 * (3 - nSize);
        *(uint32_t*)(bnTarget.begin()) = nWord;
    } else {
        nWord <<= 8;
        *(uint32_t*)(bnTarget.begin() + nSize - 4) = nWord;
    }
    //   my_eprintf("\n%s\n %i 0x%08x", bnTarget.ToString().c_str(), nSize, nWord);
    return true;
}

bool CheckProofOfWork(uint256& hash)
{
    return CompareTo(hash, bnTarget) < 0;
}

//=============================================================================================================

#include "GNonces.h"

#include "chrono"

uint64_t Now()
{
    std::chrono::high_resolution_clock m_clock;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(m_clock.now().time_since_epoch()).count();
}

uint64_t TimeError, StatisticTime, LastTime;
T MaxTries;
T CurrentTries, LastSuccess, FirstSuccess;
T TrueSuccess, TrueN, LateSuccess, LateN, OverSuccess, OverN, TotalFail;
double TrueMean, TrueVariance, LateMean, LateVariance, OverMean, OverVariance;

#include <chainparams.h>

mpz_t res;
mpq_t in1, in2, div1;

void (*InitArith)();
void (*ClearArith)();
void (*AddHash)(uint256& avg, uint256& hash);
void (*MinusHash)(uint256& avg, uint256& hash);
void (*AvgHash)(uint256& avg);

void InitArith1()
{
    mpz_inits(res, NULL);
    mpq_inits(in1, in2, div1, NULL);
};

void ClearArith1()
{
    mpz_clears(res, NULL);
    mpq_clears(in1, in2, div1, NULL);
};

void AddHash1(uint256& avg, uint256& hash)
{
    // init outside , remove str to hex
    string strAvg, strHash;
    strAvg = avg.GetHex();
    strHash = hash.GetHex();

    //    mpz_t res;
    //    mpz_inits(res, NULL);
    //    mpq_t in1, in2, div;
    //	  mpq_inits(in1, in2, div, NULL);
    gmp_sscanf(avg.GetHex().c_str(), "%Qx", in1);
    gmp_sscanf(hash.GetHex().c_str(), "%Qx", in2);
    mpq_set_ui(div1, 1, Getm());
    // printf("Check: %lf\n", mpq_get_d(div));
    // printf("Check hash1: %lf\n", mpq_get_d(in1));
    // printf("Check hash2: %lf\n", mpq_get_d(in2));
    mpq_mul(in2, in2, div1);
    // printf("Check hash2 / div: %lf\n", mpq_get_d(in2));
    mpq_add(in1, in1, in2);
    //./printf("Check res: %lf\n", mpq_get_d(in1));


    mpz_set_q(res, in1);
    uint256 cnum;
    char scnum[260];
    mpz_get_str(scnum, 16, res);
    avg.SetHex(scnum);

    //    mpq_clears(in1, in2, NULL);
    //    return cnum;
};


void AvgHash1(uint256& avg)
{
}

void MinusHash1(uint256& avg, uint256& hash)
{ // init outside , remove str to hex
    string strAvg, strHash;
    strAvg = avg.GetHex();
    strHash = hash.GetHex();

    //    mpz_t res;
    //    mpz_inits(res, NULL);
    //    mpq_t in1, in2, div;
    //    mpq_inits(in1, in2, div, NULL);
    gmp_sscanf(avg.GetHex().c_str(), "%Qx", in1);
    gmp_sscanf(hash.GetHex().c_str(), "%Qx", in2);
    mpq_set_ui(div1, 1, Getm());
    // printf("Check: %lf\n", mpq_get_d(div));
    // printf("Check hash1: %lf\n", mpq_get_d(in1));
    // printf("Check hash2: %lf\n", mpq_get_d(in2));
    mpq_mul(in2, in2, div1);
    // printf("Check hash2 / div: %lf\n", mpq_get_d(in2));
    mpq_sub(in1, in1, in2);
    //./printf("Check res: %lf\n", mpq_get_d(in1));


    mpz_set_q(res, in1);
    uint256 cnum;
    char scnum[260];
    mpz_get_str(scnum, 16, res);
    avg.SetHex(scnum);

    //   mpq_clears(in1, in2, NULL);
    //    return;
}


arith_uint288 Avg;

void InitArith2()
{
    (*(base_uint<288>*)&Avg) = 0;
};

void ClearArith2() {};

void AddHash2(uint256& avg, uint256& hash)
{
    Avg += hash;
}

void MinusHash2(uint256& avg, uint256& hash)
{
    arith_uint288 tmp;
    tmp += hash;
    Avg -= tmp;
}

void AvgHash2(uint256& avg)
{
    Avg /= Getm();
    Avg.ToUint256(avg);
}

typedef void (*FP0[2])();
typedef void (*FP1)(uint256& avg);
typedef void (*FP2[2])(uint256& avg, uint256& hash);


FP0 ArithFun0[2] = {{InitArith1, ClearArith1}, {InitArith2, ClearArith2}};
FP1 ArithFun1[2] = {AvgHash1, AvgHash2};
FP2 ArithFun2[2] = {{AddHash1, MinusHash1}, {AddHash2, MinusHash2}};

void SetArith(int n)
{
    InitArith = ArithFun0[n][0];
    ClearArith = ArithFun0[n][1];
    AvgHash = ArithFun1[n];
    AddHash = ArithFun2[n][0];
    MinusHash = ArithFun2[n][1];
}

arith_uint288& arith_uint288::operator+=(uint256& b)
{
    uint64_t carry = 0;
    int i, WIDTH = b.size() >> 2;
    for (i = 0; i < WIDTH; i++) {
        uint64_t n = carry + pn[i] + *(uint32_t*)(b.begin() + (i << 2));
        pn[i] = n & 0xffffffff;
        carry = n >> 32;
    }
    pn[i] += carry; // assume no further carry;
    return *this;
}

uint256& arith_uint288::ToUint256(uint256& b) // for average, assume bit 256-319 = 0;
{
    int WIDTH = b.size() >> 2;
    for (int i = 0; i < WIDTH; ++i)
        *(uint32_t*)(b.begin() + (i << 2)) = pn[i];
    return b;
    //    for (int i = 0, uint32_t* bp=b.begin(); i < WIDTH; ++i)     // arith_uint256.cpp:285, gpow.cpp:104
    //        *bp++ = pn[i];
}

/*
void PrinfNormalizedMeanNonce(uint256 hash, int decimals) {
    mpq_t in1, div, max, one;
        mpq_inits(in1, div, max, one, NULL);
    gmp_sscanf(hash.GetHex().c_str(),"%Qx",in1);
    mpq_set_str(max,"115792089237316195423570985008687907853269984665640564039457584007913129639936",10);
    mpq_set_ui(one, 1, 1);
    mpq_div(div, one, max);
    mpq_mul(in1, in1, div);
    printf("Normalized nonces mean: %.*lf\n", decimals, mpq_get_d(in1));
}
*/
