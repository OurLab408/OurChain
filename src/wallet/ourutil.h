#pragma once

#include "amount.h"
#include "base58.h"
#include "chain.h"
#include "consensus/validation.h"
#include "contract/contract.h"
#include "core_io.h"
#include "init.h"
#include "httpserver.h"
#include "validation.h"
#include "net.h"
#include "policy/feerate.h"
#include "policy/fees.h"
#include "policy/policy.h"
#include "policy/rbf.h"
#include "rpc/mining.h"
#include "rpc/server.h"
#include "script/sign.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#include "wallet/coincontrol.h"
#include "wallet/feebumper.h"
#include "wallet/wallet.h"
#include "wallet/walletdb.h"

#include <stdint.h>
#include <fstream>

#include <univalue.h>

class OurUtil
{
public:
    static UniValue test(const JSONRPCRequest& request);
    static UniValue generatezkproof(const JSONRPCRequest& request);
private:
    OurUtil() {}
};
