#include <cstdio>

#include "consensus/merkle.h"
#include "gmp.h"
#include "primitives/block.h"
#include "string.h"
#include "util.h"
#include "utilstrencodings.h"
using namespace std;

/*
static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, GNonces nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, GNonces nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}
*/
/*
uint256 GetNonceMean(CBlock block)
{
    // Copy block nNonce
    GNonces nNonce_copy;
    for (int i = 0; i < GetGPoWM(); i++) {
        nNonce_copy = block.nNonce;
        block.nNonce= 0;
    }

    // Paste nNonce one by one
    uint256 mean;
    mean.SetNull();
    mpz_t res; // for integer
    mpz_inits(res, NULL);
    mpq_t in1, in2, div; // for fraction
    mpq_inits(in1, in2, div, NULL);
    gmp_sscanf(mean.GetHex().c_str(),"%Qx",in1);
    mpq_set_ui(div, 1, GetGPoWM());
    for (int i = 0; i < GetGPoWM(); i++) {
        block.nNonce = nNonce_copy;
        gmp_sscanf(block.GetHash().GetHex().c_str(),"%Qx",in2);
        mpq_mul(in2, in2, div);
        mpq_add(in1, in1, in2);
        mpq_inits(in2, NULL);
    }
    mpz_set_q(res, in1);
    char scnum[260];
    mpz_get_str(scnum, 16, res);
    mean.SetHex(scnum);

    mpq_clears(in1, in2, div, NULL);
    mpz_clears(res, NULL);
    return mean;
}

void PrintNormalizedMeanNonce(uint256 hash, int decimals) {
    mpq_t in1, div, max, one;
        mpq_inits(in1, div, max, one, NULL);
    gmp_sscanf(hash.GetHex().c_str(),"%Qx",in1);
    mpq_set_str(max, "115792089237316195423570985008687907853269984665640564039457584007913129639936", 10);
    mpq_set_ui(one, 1, 1);
    mpq_div(div, one, max);
    mpq_mul(in1, in1, div);
    FILE *fp = NULL;
    fp = fopen("record.txt","w+");
    for (double i = 0; i < 50; i++)
        fprintf(fp, "%lf\t%.*lf\n", i, decimals, mpq_get_d(in1));

    printf("Normalized nonces mean: %.*lf\n", decimals, mpq_get_d(in1));
}
*/
//#define MINE
//#define TEST_RANDOM

int main(int argc, char** argv)
{
    /*
        gArgs.ParseParameters(argc, argv);
        InitGPoWParameter();

        printf("m: %d\n", GetGPoWM());
        printf("Nonce_bits: %d\n", GetNonceBitSize());
        printf("difficulty: %d\n", GetNBits());

    #ifdef MINE
        // Mining
        CBlock genesis;
        uint32_t t = time(NULL);
        uint8_t done = 0;
        fprintf(stderr, "Mining main genesis block...\n\ntime = %u\n", t);
        for (; ; ++t) {
            for (uint32_t n = 0; ; ++n) {
                if ((n & 0xfffff) == 0) fprintf(stderr, "\rnonce = %u\n", n);
                genesis = CreateGenesisBlock(t, n, GetNBits(), 1, 50 * COIN);
                if (CheckGeneralProofOfWork(genesis.GetHash(), genesis.nBits)) {
                    done = 1;
                    break;
                }
                if (n == 4294967295) break;
            }

            if (done == 1) {
                break;
            }
        }
        uint256 mean = GetNonceMean(genesis);
        printf("Avg: %s\n", mean.GetHex().c_str());
        printf("Ori: %s\n", genesis.GetHash().GetHex().c_str());
        PrintNormalizedMeanNonce(mean, 30);

    #elif defined(TEST_RANDOM)
        // Test SHA256 is random
        uint32_t test_num = gArgs.GetArg("-num", 100);

        CBlock genesis;
        FILE *fp = NULL;
        char filename[40] = {'\0'};
        mpq_t max, one, div, hash;
        mpq_inits(max, one, div, hash, NULL);
        mpq_set_str(max, "115792089237316195423570985008687907853269984665640564039457584007913129639936", 10);
        mpq_set_ui(one, 1, 1);
        mpq_div(div, one, max);
        mpq_clears(one, max, NULL);

        uint32_t t = time(NULL);
        uint16_t count = 0;
        sprintf(filename, "record/record_%u.txt", count);
        fp = fopen(filename, "w+");
        for (uint32_t n = 0; ; ++n) {
            if ((n % 1000000 == 0) && (n != 0)) {
                printf("1\n");
                fclose(fp);
                count++;
                sprintf(filename, "record/record_%d.txt", count);
                fp = fopen(filename, "w+");
            }
            genesis = CreateGenesisBlock(t, n, GetNBits(), 1, 50 * COIN);
            gmp_sscanf(genesis.GetHash().GetHex().c_str(), "%Qx", hash);
            mpq_mul(hash, hash, div);
            fprintf(fp, "%d\t%.*lf\n", n, 30, mpq_get_d(hash));
            mpq_inits(hash, NULL);

            if (n == 4294967295) break;
            if (n == test_num) {
                fclose(fp);
                break;
            }
        }

        mpq_clears(hash, div, NULL);
    #endif
    */
    return 0;
}
