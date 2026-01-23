// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "crypto/common.h"
#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, hashContractState=%s, nTime=%u",
                   GetHash().ToString(),
                   nVersion,
                   hashPrevBlock.ToString(),
                   hashMerkleRoot.ToString(),
                   hashContractState.ToString(),
                   nTime);
#if ENABLE_GPoW
    s << strprintf(", nPrecisionTime=%u, hashGPoW=%s", nPrecisionTime, hashGPoW.ToString());
#endif
    s << strprintf(", nBits=%08x, nNonce=%s, vtx=%u\n",
                   nBits,
                   nNonce.ToString(),
                   vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
