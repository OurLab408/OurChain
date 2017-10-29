// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"

uint256 CBlockHeader::GetHash() const
{
/*********** NTU PATCH **********/
    if(nVersion < NTU_SHARDING_VERSION)   //If the version stand before the sharding patch
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        
        ss<<nVersion<<hashPrevBlock<<hashMerkleRoot<<nTime<<nBits<<nNonce;
        
        return ss.GetHash();
    }
    else    //If the version is a sharded one -> different header
    {
/*********** NTU PATCH END ******/
    return SerializeHash(*this);
/*********** NTU PATCH **********/
    }
/*********** NTU PATCH END ******/
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
