// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"

#include "stdio.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

uint256 CBlockHeader::GetHash() const
{
#ifdef DEBUG
    int pid, status;
    pid = fork();
    if (pid == 0) {
        FILE * pFile;
        pFile = fopen("~/check.txt","a");
        fprintf(pFile, "Check: \n");
        for (int i = 0; i < GPOW_M; i++) {
            fprintf(pFile, "\t%d", this->nNonce[i].x);
        }
        fprintf(pFile, "\n");
        exit(EXIT_FAILURE);
    }
    waitpid(pid, &status, 0);
#endif
    return SerializeHash(*this);
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, hashContractState=%s, nTime=%u, nBits=%08x, gpow_m=%d, nonce_bits=%u, nNonce_size=%u, vtx_size=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        hashContractState.ToString(),
        nTime, nBits, nGpow_m, nNonce_bit_size, sizeof(nNonce) / sizeof(nNonce[0]),
        vtx.size());
    s << "Nonce\n";
    for(nonce_type i : nNonce) {
        s << " " << i.x << "\n";
    }
    s << "vtx\n";
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
