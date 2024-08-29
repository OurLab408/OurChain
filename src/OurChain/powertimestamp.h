#ifndef POWERTIMESTAMP_H
#define POWERTIMESTAMP_H

#include "gpow.h"
#include "serialize.h"
#include "uint256.h"

const int ERR_SHIFT = 20;

class CPowerTimestamp
{
public:
    uint64_t nTime = 0;
    GNonces nNonce;
    uint32_t nBits;
    uint8_t timeError; // M ns, same in different instances for now
    uint256 gpow;

    CPowerTimestamp()
    {
        timeError = TimeError >> ERR_SHIFT;
        nBits = GetNBits();
        SetTimestamp();
        NextGPoW(*this);
    }

    CPowerTimestamp(T n)
    {
        timeError = TimeError >> ERR_SHIFT;
        nBits = GetNBits();
        SetTimestamp();
        nNonce.setn(n);
        NextGPoW(*this);
    }

    inline bool operator<(CPowerTimestamp b)
    {
        int64_t t = nTime - b.nTime;

        //       assert(timeError==b.timeError);
        if (labs(t) > TimeError) return t < 0;
        return CompareTo(gpow, b.gpow) < 0;
    }

    inline bool operator>(CPowerTimestamp b)
    {
        int64_t t = nTime - b.nTime;

        //       assert(timeError==b.timeError);
        if (labs(t) > TimeError) return t > 0;
        return CompareTo(gpow, b.gpow) > 0;
    }

    inline bool operator==(const CPowerTimestamp b)
    {
        //       assert(timeError==b.timeError);
        return nTime == b.nTime && gpow == b.gpow;
    }

    inline bool operator!=(const CPowerTimestamp b)
    {
        //       assert(timeError==b.timeError);
        return !(*this == b);
    }

    inline bool operator<=(const CPowerTimestamp b)
    {
        //       assert(timeError==b.timeError);
        return !(*this > b);
    }

    inline bool operator>=(CPowerTimestamp b)
    {
        //       assert(timeError==b.timeError);
        return !(*this < b);
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(nTime);
        READWRITE(nNonce);
        READWRITE(nBits);
        READWRITE(timeError);
    }

    uint256 GetHash() const;

    inline void SetExtraNonce()
    {
        SetTimestamp();
    }

    inline void SetTimestamp()
    {
        nNonce.SetNull();
        gpow.SetNull();
        nTime = Now();
    }

    inline uint64_t GetTimestamp() const
    {
        return nTime;
    }

    std::string ToString() const;
};

// CPowerTimestamp CreatePowerTimestamp(uint64_t nTime, GNonces nNonce, uint32_t nBits);
// bool CheckGPoW(CPowerTimestamp& pt);

#endif
