#ifndef GNONCES_H
#define GNONCES_H

#include "chainparams.h"
#include "hash.h"
#include <gmp.h>
#include <queue>
#include <utility>

template <typename B>
bool CheckGPoW(B& block)
{
    auto& theNonce = block.nNonce;

    T n = theNonce.getNonce();
    if (n) {
        if (!theNonce.ismDone()) return false;
        uint256 gpow, hash;
        auto& qq = isConservativeGPoW() ? theNonce.q : theNonce.pq;
        InitArith();
        for (auto const& e : qq) {
            theNonce.setNonce(e.first);
            hash = block.GetHash();
            if (CompTo(hash, e.second) != 0)
                return false;
            if (CheckProofOfWork(hash))
                AddHash(gpow, hash);
            else
                return false;
        }
        AvgHash(gpow);
        ClearArith();
        block.gpow = gpow;
    } else {
        if (!block.nTime)
            block.SetTimestamp();
        NextGPoW(block);
    }
    return true;
}

#endif