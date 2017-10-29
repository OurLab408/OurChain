#include "ntunodeid.h"
#include "txmempool.h"

AddrPow myAddrPow;

void AddrPow::tests()
{
    {
        LOCK(cs_AddrPow);
        
        printf("\nAddress generation test:\n");
        fNew=1;
        double time;
        //--------------------------------------//
        struct timeval time_before, time_after;
        gettimeofday (&time_before, NULL);
        //--------------------------------------//
        printf("\nPoW: top !\n");
        genAddrPow();
        //--------------------------------------//
        gettimeofday (&time_after, NULL);
        time = ((time_after.tv_sec * 1000000 + time_after.tv_usec) - (time_before.tv_sec * 1000000 + time_before.tv_usec));
        time/=1000000;
        printf("\nTime to achieve PoW: %lf s\n", time);
        //--------------------------------------//
        printf("\nPrevious block Hash:\t%s\n", prevBlockHash.ToString().c_str());
        printf("Public key:\t\t");
        for( int i = 0; i < 65; i++)
        {
            printf("%02x", (unsigned char)publicKey[i]);
            if((i == 31) || (i == 63)) printf("\n\t\t\t");
        }
        printf("\n");
        printf("Final nonce:\t\t%08x\n", nonce);
        printf("Address hash:\t\t%s\n", addr.ToString().c_str());

        sign();
    }
    
    if(pindexBestHeader->nVersion >= NTU_SHARDING_VERSION)
        printf("\nGroup number:\t\t%u\n", this->getGroup(pindexBestHeader->nShardsForNextGen));
    else
        printf("\nGroup number:\t\t%u\n", this->getGroup(1));
    //printf("\nGroup number:\t\t%u\n", getGroup(mempool.nbGroups()));
    
    std::stringstream oss;
    this->SerializeAddr(oss);
    
    printf("\nSerialized size is:\t%lu char\n", oss.str().size());
    
    
    AddrPow testSerial;
    
    //testSerial.UnserializeAddr(oss);
    std::vector<unsigned char> result;
    result = this->getvch();
    testSerial.fromvch(result);
    
    testSerial.printTests("Serialization");
}

void AddrPow::printTests(std::string msg)
{
    LOCK(cs_AddrPow);
    
    printf("\nValid PoW?\t%s\n", this->checkPow() ? "Yes" : "No");

    printf("\nTest %s:\t%s\n\t\t\t",msg.c_str() ,this->prevBlockHash.ToString().c_str());
        for( int i = 0; i < 65; i++){
            printf("%02x", (unsigned char)this->publicKey[i]);
            if((i == 31) || (i == 63)) printf("\n\t\t\t");
        }
        printf("\n\t\t\t%08x\n\t\t\t%s\n", this->nonce, this->addr.ToString().c_str());
    printf("\nValid signature?\t%s\n", this->publicKey.Verify(addr, this->signature) ? "Yes" : "No");
}


unsigned int getGroupFromUint64(const uint64_t uint_in, const unsigned int numberOfGroups)
{
    unsigned int group;
    unsigned int n = numberOfGroups;
    unsigned int l = 0;
    unsigned int doubleSets = 0;
    
    while(!(n == 1))
    {
        n = n >> 1;
        ++l;
    }
    
    //Number of sets of l bits which need an additional bit for grouping
    doubleSets = numberOfGroups - (1 << l);
    
    uint64_t set=0;
    uint64_t mask=0;
    
    //This mask is used to copy only the bit used for the grouping process
    mask = (1 << (l + 1)) - 1;
    
    //Set of bits used for grouping
    set = uint_in & mask;    //TODO: deal with more than 2^64 groups
    
    if((uint_in & (mask >> 1)) < ((1 << l) - doubleSets))
    {
        group = set & (mask >> 1);
    }
    else
    {
        group = set << 1;
        group += group >> (l+1);
        group &= mask;
        group = group - ((1 << l) - doubleSets);
    }
    return group;
}

unsigned int getGroupFromUint256(const uint256 hash, const unsigned int numberOfGroups)
{
    return getGroupFromUint64(hash.GetUint64(0), numberOfGroups);
}

