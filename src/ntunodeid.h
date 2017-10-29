/********** NTU PATCH **********/

#include "arith_uint256.h"
#include <inttypes.h>
#include "validation.h"

#include "key.h"
#include <sys/time.h>
#include <serialize.h>
#include <sync.h>

#include <iostream>
#include <sstream>

unsigned int getGroupFromUint64(const uint64_t uint_in, const unsigned int numberOfGroups);

unsigned int getGroupFromUint256(const uint256 hash , const unsigned int numberOfGroups);

/** Address generated via a pow for the sharding process */
class AddrPow
{
private:
    //Address
    uint256 addr;
    
    uint256 prevBlockHash;
    
    CKey addrKey;
    //CPubKey is used to copy the public key because in CKey this key can only be accessed as const
    CPubKey publicKey;
    
    uint32_t nonce;
    
    //Signature generated when the address is finalized
    std::vector<unsigned char> signature;
    
    //Group number
    unsigned int group;
    
    //New addr, if 1, we should refresh the group number
    bool fNew;
    
    //Mutex to protect the object during the generation or during read/write operations
    mutable CCriticalSection cs_AddrPow;
    
public:

// Some compilers complain without a default constructor
    AddrPow() { fNew = 0; }

//Check if the address solve the PoW cryptopuzzle.
    bool checkPow()
    {
        
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);      //Declare hash object
        
        //Add the serialized elements to the hash object
        ss<<prevBlockHash;
        publicKey.Serialize(ss);
        ss<<nonce;
        
        //Finalize the hash
        addr = ss.GetHash();
        
        //Test the difficulty and return the result
        return (addr.GetUint64(3)<0x0000FFFFFFFFFFFF);
    }

//Check if the address solve the PoW cryptopuzzle at a given height in the blockchain. if -1 we use the latest block.
    bool checkPow(int height)   //check PoW for the given height
    {
        uint256 prevBlockHashAtH;
        if(height == -1)
            prevBlockHashAtH = pindexBestHeader->GetBlockHash();
        else if(height <= chainActive.Height())
            prevBlockHashAtH = chainActive[height]->GetBlockHash();
        else
        {
            //TODO: send some error message/return
            ;
        }
        return checkPow() && (prevBlockHashAtH == prevBlockHashAtH);   
    }

//Generate a correct address by solving a PoW cryptopuzzle
    bool genAddrPow()
    {    
        prevBlockHash = pindexBestHeader->GetBlockHash();
        nonce = 0;
        //if(&publicKey==nullptr) return false;                        //We need a public key
        
        //Get the hash of the most recent block on the active chain
        prevBlockHash = pindexBestHeader->GetBlockHash();
        
        while(!this->checkPow())  //While the PoW isn't OK, we increment the nonce.
        {
            nonce = nonce + 1;
        }
        
        fNew=1;
        return true;
    }

//Return the group number of the address. It depends on the number of groups
    unsigned int getGroup(const unsigned int numberOfGroups)
    {
        if(fNew)    //if we do not need to update the group, we don't to save time
        {
            fNew=0;
            /*
            We use the generic function declared in "ntunodeid.cpp" to get
            the group. We only have to pass the address and the number of
            groups.
            */
            group = getGroupFromUint256(addr, numberOfGroups);
        }
        return group;
    }

//Generate a public/private key. Replace the previous one.
    void newKey()
    {
        addrKey.MakeNewKey(false);
        publicKey = addrKey.GetPubKey();
    }

//Generate the signature of the address, avoid identity usurpation.
    void sign()
    {
        addrKey.Sign(addr, signature);
    }
    
    void tests();
    
    void printTests(std::string msg);

//The two following functions are created to interface with the CScript object from "/src/script/script.h" 

//Get the vector<unsigned char> serialization version of the address and signature.
    std::vector<unsigned char> getvch() const
    {
        std::stringstream oss;
        this->SerializeAddr(oss);
        
        std::vector<unsigned char> result;
        
        char c;
        
        while(oss.get(c))
        {
            result.push_back(c);
        }
        
        return result;
    }
    
//Get an address from a vector<unsigned char>
    void fromvch(std::vector<unsigned char> invch)
    {
        std::stringstream oss;
        
        for (std::vector<unsigned char>::const_iterator i = invch.begin(); i != invch.end(); ++i)
            oss << *i;
        
        this->UnserializeAddr(oss);
    }

//Serialization function   
    template<typename Stream>
    void SerializeAddr(Stream& s) const
    {       
        LOCK(cs_AddrPow);
        
        addr.Serialize(s);
        
        prevBlockHash.Serialize(s);
        publicKey.Serialize(s);
        Serialize(s, nonce);
        
        Serialize(s, signature);
    }

//Unserialization function
    template<typename Stream>
    void UnserializeAddr(Stream& s)
    {        
        LOCK(cs_AddrPow);
        
        addr.Unserialize(s);
        
        prevBlockHash.Unserialize(s);
        publicKey.Unserialize(s);
        Unserialize(s, nonce);
        
        Unserialize(s, signature);
        
        fNew=1;
    }
    
};

extern AddrPow myAddrPow;

/********** NTU PATCH END *****/

