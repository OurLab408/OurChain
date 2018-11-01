// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"

//b04902091
uint256 CBlockHeader::GetHash() const
{
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        
        ss<<nVersion<<hashPrevBlock<<hashMerkleRoot<<nTime<<nBits<<nNonce<<nTimeNonce<<maxhash;
        
        return ss.GetHash();
}
uint256 CBlockHeader::GetHash2() const
{
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        
        ss<<nVersion<<hashPrevBlock<<hashMerkleRoot2<<nTime<<nBits<<nNonce2<<nTimeNonce2<<maxhash2;
        
        return ss.GetHash();
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, hashContractState=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        hashContractState.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
