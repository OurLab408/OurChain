#include "OurChain/powertimestamp.h"
#include "arith_uint256.h"
#include "gpow.h"
#include "util.h"
#include <cmath>

const int GPOW_M = 8;
const T GPOW_N = 1024;
const char* NBITS = "0x207fffff";
const int TIME_ERROR = 0x06000000; // 96<<20 = 100,663,296 ns
const int TIME_ERROR_MASK = 0x0FF00000;
T INNER_LOOP_COUNT = 0x10000;
T nMaxTries;
T MAX_TRIES = 999999;
const char* POW_LIMIT = "ffffff0000000000000000000000000000000000000000000000000000000000";

int Arith = 1;
int Round = 10;

bool Test()
{
    return true;
    return CurrentTries < 10;
}

extern void SetArith(int n);

void InitGPoWParameter()
{
    int cons, comp;
    uint32_t nbits;
    T m, N;

    m = gArgs.GetArg("-m", GPOW_M);
    N = gArgs.GetArg("-N", GPOW_N);
    assert(m <= N);
    /* use GetArg(string, string) because we don't want the hex-code being change to int64_t */
    nbits = std::stoul(gArgs.GetArg("-nBits", NBITS), nullptr, 16);
    // nbits = gArgs.GetArg("-number2nBits", NBITS);
    cons = gArgs.GetArg("-CONSERVATIVE", GetConservativeDefault());
    comp = gArgs.GetArg("-NONCE_COMPRESSED", GetCompressedDefault());
    bnPowLimit = uint256S(gArgs.GetArg("-powLimit", POW_LIMIT));

    TimeError = gArgs.GetArg("-TimeError", TIME_ERROR);
    if (TimeError > TIME_ERROR_MASK) TimeError = TIME_ERROR_MASK;
    TimeError &= TIME_ERROR_MASK;

    if (N != GPOW_N) {
        INNER_LOOP_COUNT = N << 1;
        MAX_TRIES = N << 2;
    }
    nInnerLoopCount = gArgs.GetArg("-InnerLoopCount", INNER_LOOP_COUNT);
    MaxTries = gArgs.GetArg("-MaxTries", MAX_TRIES);
    CurrentTries = 0;

    DEBUG = gArgs.GetArg("-DEBUG", DEBUG);

    Arith = gArgs.GetArg("-Arith", Arith); // = 0 or 1
    Round = gArgs.GetArg("-Round", Round);

    CONDITION = Test;

    InitGPoW(m, N, cons, comp, nbits);
    SetArith(Arith);
}

#include "GNonces.h"

#define No0(n) (n ? n : 1)

int main(int argc, char** argv)
{
    gArgs.ParseParameters(argc, argv);
    InitGPoWParameter();

    my_Eprintf("m= %i N= %i nBits=0x%08x %10.7e Round= %i Arith= %i Type=0x%02x",
               Getm(), GetN(), GetNBits(), fGetNBits(), Round, Arith, NONCE_TYPE);

    int N = GetN();

    CPowerTimestamp pt(N);
    my_Eprintf("%s\n", pt.ToString().c_str());
    /*

    StatisticTime = 0;
    TrueSuccess = LateSuccess = OverSuccess = 0;
    TrueN = LateN = OverN = TotalFail = 0;
    TrueMean = TrueVariance = LateMean = LateVariance = OverMean = OverVariance = 0.0;

    int r = Round;

    uint64_t justnow;
    justnow = LastTime = Now();
    while (r--) CPowerTimestamp pt(N);
    justnow = Now() - justnow;

    LateMean += TrueMean; OverMean += LateMean;
    LateVariance += TrueVariance; OverVariance+= LateVariance;

    TrueMean /= No0(TrueSuccess);
    TrueVariance /= No0(TrueSuccess);
    TrueVariance -= TrueMean*TrueMean;

    LateMean /= No0(TrueSuccess+LateSuccess);
    LateVariance /= No0(TrueSuccess+LateSuccess);
    LateVariance -= LateMean*LateMean;

    T TotalSuccess = TrueSuccess + LateSuccess + OverSuccess;

    OverMean /= No0(TotalSuccess);
    OverVariance /= No0(TotalSuccess);
    OverVariance -= OverMean*OverMean;

    assert(TotalSuccess+TotalFail == (T) Round);

    my_Eprintf("timediff %li %li ns/ %i rounds = %8.3f us %i %8.3f , %i %8.3f , %i %8.3f , %i %e %e %e %e %e %e , %e %e %e",
    justnow, StatisticTime, Round, StatisticTime/Round/1000.0,
    TrueSuccess, 1.0*TrueN/No0(TrueSuccess), LateSuccess, 1.0*LateN/No0(LateSuccess),
    OverSuccess, 1.0*OverN/No0(OverSuccess), TotalFail,
    TrueMean, TrueVariance, LateMean, LateVariance, OverMean, OverVariance,
    sqrt(TrueVariance)/No0(TrueMean), sqrt(LateVariance)/No0(LateMean), sqrt(OverVariance)/No0(OverMean));
    */

    // my_eprintf("%s %s", pt0.ToString().c_str(), pt.ToString().c_str());

    /*
    NextGPoW(pt);
    my_Eprintf("timediff %li ns %s", pt.nTime-pt0.nTime, pt.ToString().c_str());
    pt.SetTimestamp();
    NextGPoW(pt);
    my_Eprintf("timediff %li ns %s", pt.nTime-pt0.nTime, pt.ToString().c_str());
    */
    return 0;
}
