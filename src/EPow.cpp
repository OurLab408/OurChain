#include <gmp.h>
#include <string.h>
#include <string>
#include "util.h"
#include "arith_uint256.h"
#include "primitives/block.h"
using namespace std;
//b04902091
/*this change in block.cpp and block.h
uint256 CBlockHeader::GetHash() const
{
    if(nVersion < NTU_SHARDING_VERSION)   //If the version stand before the sharding patch
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        
        ss<<nVersion<<hashPrevBlock<<hashMerkleRoot<<nTime<<nBits<<nNonce;
        
        return ss.GetHash();
    }
    else    //If the version is a sharded one -> different header
    {
        return SerializeHash(*this);
    }
}
uint256 CBlockHeader::GetHash2() const
{
    if(nVersion < NTU_SHARDING_VERSION)   //If the version stand before the sharding patch
    {
        CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
        
        ss<<nVersion<<hashPrevBlock<<hashMerkleRoot2<<nTime<<nBits<<nNonce2;
        
        return ss.GetHash();
    }
    else    //If the version is a sharded one -> different header
    {
        return SerializeHash(*this);
    }
}
*/
mpq_t M,Max,one,two;
void f1(mpq_t d,mpq_t x){
	if(mpq_cmp(x,one)==0){
		mpq_set(d,M);
		return;
	}
	else{
		mpq_t n;
		mpq_inits(n,NULL);
		mpq_sub(n,one,x);
		mpq_div(d,x,n);
		mpq_clear(n);
		return;
	}
}
void f2(mpq_t d,mpq_t x){
	mpq_t n;
	mpq_inits(n,NULL);
	if(mpq_cmp(x,one)==0){
		mpq_add(n,M,one);
		mpq_mul(d,M,n);
		mpq_div(d,d,two);
	}
	else{
		f1(d,x);
		mpq_sub(n,one,x);
		mpq_div(d,d,n);
	}
	mpq_clear(n);
	return;
}
uint256 GetEPow(uint256 hash2,uint256 hash){
	string st2,st;
	st=hash.GetHex();
	st2=hash2.GetHex();
	LogPrintf("my min hash %s max hash %s\n",st,st2);
/*main(){
	char st[300];
	scanf("%s",st);*/
	mpz_t in2,in,zcnum;
	mpz_inits(in2,in,zcnum,NULL);
//	gmp_sscanf(st,"%Zx",in);
	gmp_sscanf(st2.c_str(),"%Zx",in2);
	gmp_sscanf(st.c_str(),"%Zx",in);
	mpz_sub(in,in2,in);
	mpq_t n,p,np,n1p,n2p,temp,ans,Pij;
	mpq_inits(M,Max,n,p,np,n1p,n2p,temp,ans,Pij,one,two,NULL);
	mpq_set_str(Max,"115792089237316195423570985008687907853269984665640564039457584007913129639936",10);//2^256
	mpq_set_str(M,"115792089237316195423570985008687907853269984665640564039457584007913129639936",10);//need change
	mpq_set_ui(one,1,1);//2^256
	mpq_set_ui(two,2,1);
	mpq_div(p,one,M);
	mpq_set_z(n,in);
	mpq_mul(np,n,p);
	mpq_sub(temp,n,one);
	mpq_mul(n1p,temp,p);
	mpq_sub(temp,n,two);
	mpq_mul(n2p,temp,p);
	f1(Pij,np);
	f1(temp,n1p);
	mpq_mul(temp,temp,two);
	mpq_sub(Pij,Pij,temp);
	f1(temp,n2p);
	mpq_add(Pij,Pij,temp);
	f2(ans,np);
	f2(temp,n1p);
	mpq_mul(temp,temp,two);
	mpq_sub(ans,ans,temp);
	f2(temp,n2p);
	mpq_add(ans,ans,temp);
	mpq_div(ans,ans,Pij);
	uint256 cnum;
	mpz_set_q(zcnum,ans);
	if(mpq_cmp(ans,Max)>0){
		cnum.SetHex("10000000000000000000000000000000000000000000000000000000000000000");
	}
	else{
		char scnum[260];
		mpz_get_str(scnum,16,zcnum);
		cnum.SetHex(scnum);
	}
	return cnum;
//	gmp_printf("ans=%Ze\n",zcnum);
	mpz_clears(in2,in,zcnum,NULL);
	mpq_clears(M,Max,n,p,np,n1p,n2p,temp,ans,Pij,one,two,NULL);
}
bool CheckEPow(uint256 hash2,uint256 hash){
	if(hash.GetHex() == "0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"){
		return true;
	}
	uint256 cnum,min,max;
	max.SetHex("100000000");
	min.SetHex("5");
	cnum=GetEPow(hash2,hash);
	LogPrintf("my epow %s max %s min %s\n",cnum.GetHex(),max.GetHex(),min.GetHex());
	if(UintToArith256(cnum)>UintToArith256(max)||UintToArith256(cnum)<UintToArith256(min)){
		return false;
	}
	return true;
}
