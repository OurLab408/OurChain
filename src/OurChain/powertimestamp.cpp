#include "OurChain/powertimestamp.h"

#include "chrono"
#include "crypto/common.h"
#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"


uint256 CPowerTimestamp::GetHash() const
{
    return SerializeHash(*this);
}

std::string CPowerTimestamp::ToString() const
{
    std::string str;
    time_t t = nTime / 1000000000;
    str += strprintf("\nPowerTimestamp (\nhash=%s\nnTime=(%li.%09li) %s nNonce=%s",
        GetHash().ToString(),
        t, nTime % 1000000000, std::ctime(&t),
        nNonce.ToString());
    str += strprintf("\nTimeError=%iM:%lins", timeError, TimeError);
    str += strprintf("\ngpow=%s\n)", gpow.ToString());
    return str;
}
