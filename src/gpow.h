// Copyright (c) 2020- The OurChain Core developers
// Distributed under the Benevolence software license, see
// https://hackmd.io/KpMx2d-wQd2t_gwQ97D9Cg#Benevolence-license.

#ifndef GPOW_H
#define GPOW_H

#include <stdint.h>

#include "arith_uint256.h"
#include "tinyformat.h" // std::string
#include "uint256.h"
#include <serialize.h>

#include <queue>
#include <vector>

using namespace std;

//#define ENABLE_GPoW

#ifndef ENABLE_GPoW
typedef uint32_t GNonces;
#else

extern int Debug;
extern bool (*CONDITION)();

//#define my_debug
#define my_Eprintf(...)                                         \
    {                                                           \
        printf("\n0x%04x  %s(%i):", Debug, __FILE__, __LINE__); \
        printf(__VA_ARGS__);                                    \
        printf("\n");                                           \
    }
#ifdef my_debug
#define my_eprintf(...)                         \
    {                                           \
        if (Debug && CONDITION()) {             \
            if (Debug & 0x08) printf("\033c");  \
            if (Debug & 0x04) printf("\033[H"); \
            my_Eprintf(__VA_ARGS__);            \
            if (Debug & 0x02) getchar();        \
        }                                       \
    }
#else
#define my_eprintf(...) \
    {                   \
    }
#endif

typedef uint32_t NONCE_t;
typedef uint64_t NONCE_2t;
typedef NONCE_t T; // nonce type

const int GPOW_M = 8;
const T GPOW_N = 1024;
const T INNER_LOOP_COUNT = 0x10000;
const T GPOW_MaxTries = 999999;

static const int GNONCES_TYPE = 0x01;

static const NONCE_t CONSERVATIVE = (0x01 << 7);
static const NONCE_t NONCE_COMPRESSED = (0x01 << 6);

enum { REACH_n = -1,
    OUT_OF_NONCE = -2,
    OUT_OF_TRY = -3 };

extern uint8_t NONCE_TYPE; // :1　not Conservative :1 not Compressed :4 Bit Size (max 15)


extern T m;
extern T N;
extern uint32_t nBits;
extern bool _conservative, _nonce_compressed;
extern uint256 bnPowLimit;
extern uint256 bnTarget;
extern T nInnerLoopCount;     // the time for conservative n, 2N
extern T MaxTries, nMaxTries; // MaxTries: the time for aggressive n, 4N, half of block interval;

extern volatile bool fAllowedMining;

class arith_uint288;
extern arith_uint288 Avg;

inline bool isConservativeGPoW()
{
    return _conservative;
}

inline bool isNonceCompressed()
{
    return _nonce_compressed;
}

inline int GetConservativeDefault()
{
    return CONSERVATIVE;
}

inline int GetCompressedDefault()
{
    return NONCE_COMPRESSED;
}

extern T Getm();
extern T GetN();
extern uint64_t Now();
extern uint32_t GetNBits();
extern double fGetNBits();
extern double fGetUint256(uint256& hash);
extern bool CheckProofOfWork(uint256& hash);
extern bool GPoWSetCompact(uint256& bnTarget);
extern uint32_t GPoWGetCompact(uint256& bnTarget);
extern void (*InitArith)();
extern void (*ClearArith)();
extern void (*AddHash)(uint256& avg, uint256& hash);
extern void (*MinusHash)(uint256& avg, uint256& hash);
extern void (*AvgHash)(uint256& avg);
extern int CompareTo(uint256& a, uint256& b);

extern uint64_t TimeError, StatisticTime, LastTime; // ns
// extern T MaxTries, nMaxTries;
extern T CurrentTries, LastSuccess, FirstSuccess;
extern T TrueSuccess, TrueN, LateSuccess, LateN, OverSuccess, OverN, TotalFail;
extern double TrueMean, TrueVariance, LateMean, LateVariance, OverMean, OverVariance;

typedef pair<T, uint256> TP;

struct cmp {
    bool operator()(TP a, TP b)
    {
        return CompareTo(a.second, b.second) < 0;
    }
};

typedef std::priority_queue<TP, vector<TP>, cmp> PQ;

class GNonces
{
private:
    T nonce;
    T n;

public:
    std::deque<TP> q;
    PQ pq;

    GNonces()
    {
        SetNull();
        setn(0);
    }

    GNonces(T n)
    {
        SetNull();
        setn(n);
    }

    //  ~GNonces();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        //       if (ismDone()){
        //           READWRITE(VARINT(current));
        //           READWRITE(NONCE_TYPE);
        //       }
        READWRITE(VARINT(nonce));
    }

#if 0
   template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
       READWRITE(VARINT(current));
       READWRITE(NONCE_TYPE);
     //  for (int i=0; i<ARRAR_SIZE; i++) READWRITE(*(uint32_t *) &arr[i]);
       int i, current_size = (current+1)*NONCE_BIT_SIZE;         // current nonce bit size
       int j = current_size/ARRAY_ENTRY_BITS;

       if (current_size % ARRAY_ENTRY_BITS) --j; 
       for (i=0; i<j; i+=2) READWRITE(*(NONCE_4t *) &arr[i]);
       if (i!=j+1) READWRITE(*(T*) &arr[i]);  // if j is even
    }

    template<typename Stream>
    void Serialize(Stream& s) const
    {   
        if (ismDone()){
            ::Serialize(s, VARINT(current));
            ::Serialize(s, NONCE_TYPE);        
        }
        s.write((char*)arr, current_size);
    }

    template<typename Stream>
    void Unserialize(Stream& s)
    {
        if (ismDone()){
            ::Unserialize(s, VARINT(m));
            ::Unserialize(s, NONCE_TYPE);        
        }
        s.read((char*)arr, current_size);
    }
#endif

    inline void SetNull()
    {
        nonce = 0;
        while (!q.empty())
            q.pop_front();
        while (!pq.empty())
            pq.pop();
    }

#if 1
    explicit operator int()
    {
        return nonce;
    }
#endif

    int operator++()
    {
        ++nonce;
        return nonce;
    }

    int operator++(int)
    {
        int tmp = operator++(); // (*this)[current]++;
        if (tmp > 0) return tmp - 1;
        return tmp;
    }

    inline bool ismDone() const
    {
        if (nonce == nInnerLoopCount) return true;
        if (q.size() < m) return false;
        if (isConservativeGPoW()) return true;
        return (n && nonce >= n);
    }

    inline bool isNextmDone()
    {
        ++nonce;
        return ismDone();
    }

    bool NextNonce()
    {
        return (++(*this) > 0);
    }

    void operator=(T n) // for backward compatible nNonce = 0; (*this) = 0;
    {
        SetNull();
        setn(n);
    }

    bool operator<(const T b)
    {
        return nonce < b;
    }

    bool operator==(const T b)
    {
        return nonce == b;
    }

    void setn(T _n)
    {
        n = _n;
    }

    void setNonce(T _n)
    {
        nonce = _n;
    }

    T getNonce() const // const for blockchain.cpp:95
    {
        return nonce;
    }

    //    uint256 GetHash() const;
    std::string ToString() const;
};

void InitGPoW(T& _m, T& size, int& cons, int& comp, uint32_t& nbits);

template <typename B>
bool CheckGPoW(B& block);

template <typename B>
void NextGPoW(B& block)
{
    auto& theNonce = block.nNonce;
    auto& theQ = theNonce.q;
    auto& thePq = theNonce.pq;
    if (theNonce.ismDone()) {
        if (isConservativeGPoW()) return;
        theNonce.NextNonce();
    } else {
        nMaxTries = MaxTries;
        CurrentTries = 0;
        FirstSuccess = 0;
    }
    uint256 tmp;

    InitArith();
    while (nMaxTries > 0 && fAllowedMining) {
        uint256 hash = block.GetHash();
        if (CheckProofOfWork(hash)) {
            CurrentTries++;
            if (theQ.size() < m) {
                LastSuccess = theNonce.getNonce();
                theQ.push_back((TP)make_pair(LastSuccess, hash));
                AddHash(block.hashGPoW, hash);
                if ((theQ.size() == m)) {
                    FirstSuccess = LastSuccess;
                    if (isConservativeGPoW())
                        break;
                    else
                        for (auto const& e : theQ)
                            thePq.push(e); // move ??
                }
            } else {
                uint256 tmpH = thePq.top().second;
                if (CompareTo(hash, tmpH) < 0) {
                    LastSuccess = theNonce.getNonce();
                    MinusHash(block.hashGPoW, tmpH);
                    thePq.pop();
                    thePq.push((TP)make_pair(LastSuccess, hash));
                    AddHash(block.hashGPoW, hash);
                }
            }
        }

        if (theNonce.isNextmDone()) {
            if (theQ.size() < m) continue;
            break;
        }
        /*
        theNonce.SetNull();
        CurrentTries = 0;
        block.SetExtraNonce();               // extra nonce for GPoW
        */
        /*
        my_eprintf("MaxTries=%06i success#=%06i last=%06i first=%06i %s\nAvg    %s\n       %s",
        nMaxTries, CurrentTries, LastSuccess, FirstSuccess, theNonce.ToString().c_str(),
        block.gpow.ToString().c_str(),
        Avg.ToUint256(tmp).ToString().c_str());
        */
        --nMaxTries;
    }

    theNonce.setn(LastSuccess);
    AvgHash(block.hashGPoW);

    /*
    my_eprintf("MaxTries=%06i success#=%06i last=%06i first=%06i %s\nAvg    %s\n       %s",
    nMaxTries, CurrentTries, LastSuccess, FirstSuccess, theNonce.ToString().c_str(),
    block.gpow.ToString().c_str(),
    Avg.ToUint256(tmp).ToString().c_str());
    */
    ClearArith();

    /*
    uint64_t justnow = Now();
    StatisticTime += justnow-LastTime;
    LastTime = justnow;

    if (theQ.size() < m) {
        TotalFail++;
        my_eprintf("Out of max tries:%i, increase MaxTries or increase target.", nMaxTries);
    }
    else {
        double tmp = fGetUint256(block.gpow);
        if (LastSuccess >= N) {
            if (LastSuccess >= nInnerLoopCount) {
                OverSuccess++;
                OverN += LastSuccess;
                OverMean += tmp;
                OverVariance += tmp*tmp;
            } else {
                LateSuccess++;
                LateN+=LastSuccess;
                LateMean += tmp;
                LateVariance += tmp*tmp;
            }
        } else {
            TrueSuccess++;
            TrueN += LastSuccess;
            TrueMean += tmp;
            TrueVariance += tmp*tmp;
        }
    }
    */
}


template <typename B>
uint256 GetGPoW(B& block)
{
    return block.hashGPoW;
};


#endif

#endif // GPOW_H
