#include <gmp.h>
#include <string.h>
#include <string>
#include "util.h"
#include "arith_uint256.h"
#include "primitives/block.h"
using namespace std;

#define MAXSUB "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
#define MINSUB "1"
#define MIN "0"
#define MAX "1000000000000000000000000000000000000000"

void f1(mpq_t d,mpq_t x,mpq_t M,mpq_t one){
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
void f2(mpq_t d,mpq_t x,mpq_t M,mpq_t one,mpq_t two){
	mpq_t n;
	mpq_inits(n,NULL);
	if(mpq_cmp(x,one)==0){
		mpq_add(n,M,one);
		mpq_mul(d,M,n);
		mpq_div(d,d,two);
	}
	else{
		f1(d,x,M,one);
		mpq_sub(n,one,x);
		mpq_div(d,d,n);
	}
	mpq_clear(n);
	return;
}
arith_uint256 EPowSub(const CBlockHeader& block){
	arith_uint256 maxhash, minhash;
	maxhash = UintToArith256(block.GetHash2());
	minhash = UintToArith256(block.GetHash());
	return maxhash - minhash;
}
int64_t RewardAdjustment(const CBlockHeader& block){
        return 1;
}

uint256 GetEPow(uint256 epowsub){
	string st;
	mpz_t in,zcnum;
	st=epowsub.GetHex();
	mpz_inits(in,zcnum,NULL);
	gmp_sscanf(st.c_str(),"%Zx",in);
	mpq_t n,p,np,n1p,n2p,temp,ans,Pij,M,Max,one,two;;
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
	f1(Pij,np,M,one);
	f1(temp,n1p,M,one);
	mpq_mul(temp,temp,two);
	mpq_sub(Pij,Pij,temp);
	f1(temp,n2p,M,one);
	mpq_add(Pij,Pij,temp);
	f2(ans,np,M,one,two);
	f2(temp,n1p,M,one,two);
	mpq_mul(temp,temp,two);
	mpq_sub(ans,ans,temp);
	f2(temp,n2p,M,one,two);
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
	mpz_clears(in,zcnum,NULL);
	mpq_clears(M,Max,n,p,np,n1p,n2p,temp,ans,Pij,one,two,NULL);

	return cnum;
}
uint256 GetEPowPerSecond(const CBlockHeader& block){
	uint256 epow, epowpersecond;
	epow = GetEPow(ArithToUint256(EPowSub(block)));
	uint32_t time = block.nTimeNonce - block.nTime;
	if(time == 0){
		return epow;
	}
	string st;
	char ste[260];
	st=epow.GetHex();
	mpz_t s,e,t;
	mpz_inits(s,e,t,NULL);
	gmp_sscanf(st.c_str(),"%Zx",e);
	mpz_set_ui(t,time);
	mpz_cdiv_q(s,e,t);
	mpz_get_str(ste,16,s);
	epowpersecond.SetHex(ste);
	mpz_clears(s,e,t,NULL);
	return epowpersecond;
}
bool CheckEPow(const CBlockHeader& block){
	uint256 epowpersecond, minpersecond, maxpersecond;
	arith_uint256 maxhash, minhash, epowsub, maxsub, minsub;
	maxhash = UintToArith256(block.GetHash2());
	minhash = UintToArith256(block.GetHash());
	if(maxhash != UintToArith256(block.maxhash)){
        LogPrintf("illegal MaxHash %s, Real MaxHash %s\n",block.maxhash.GetHex(),maxhash.GetHex());
        return false;
    }
    if(minhash > maxhash){
    	LogPrintf("MaxHash %s smaller than MinHash %s\n",maxhash.GetHex(),minhash.GetHex());
        return false;
    }
	maxsub.SetHex(MAXSUB);
	minsub.SetHex(MINSUB);
	epowsub = maxhash - minhash;
	if(epowsub > maxsub || epowsub < minsub){
		LogPrintf("this epow %s too big or samll,max %s min %s\n",epowsub.GetHex(),MAXSUB,MINSUB);
		return false;
	}
	maxpersecond.SetHex(MAX);
	minpersecond.SetHex(MIN);
	epowpersecond=GetEPowPerSecond(block);
	if(UintToArith256(epowpersecond)>UintToArith256(maxpersecond)||UintToArith256(epowpersecond)<UintToArith256(minpersecond)){
		LogPrintf("this epow per second %s too big or samll,max %s min %s\n",epowpersecond.GetHex(),MAX,MIN);
		return false;
	}
	return true;
}
int CompareEPow(const CBlockHeader& block1, const CBlockHeader& block2){
	arith_uint256 epowsub1, epowsub2;
	epowsub1 = EPowSub(block1);
	epowsub2 = EPowSub(block2);
	if(epowsub1 == epowsub2){
		return 0;
	}
	if(epowsub1 > epowsub2){
		return 1;
	}
	return -1;
}
int CompareEPowPerSecond(const CBlockHeader& block1, const CBlockHeader& block2){
	arith_uint256 epowpersecond1, epowpersecond2;
	epowpersecond1 = UintToArith256(GetEPowPerSecond(block1));
	epowpersecond2 = UintToArith256(GetEPowPerSecond(block2));
	if(epowpersecond1 == epowpersecond2){
		return 0;
	}
	if(epowpersecond1 > epowpersecond2){
		return 1;
	}
	return -1;
}