#include "wallet/ourutil.h"
#include <linux/limits.h>
#include "rustlib/bindings.h"

/** secret is in hex format, point is in compressed format. 
 * the base point is "8b7d2d877a253c4b7733e1b91f05e0fcedf96bd11c2e572549b2a0f703727925".
 * use "0" for short.
 */
std::string enc(std::string secret, std::string point)
{
    char* res = encrypt_bjj(&(point[0]), &(secret[0]));
    std::string s(res);
    free_str(res);
    return s;
}

UniValue OurUtil::test(const JSONRPCRequest& request)
{
    std::string secret = request.params[0].get_str();
    std::string point = request.params[1].get_str();
    return enc(secret, point);
}


/** Perform subprocess and return stdout/stderr. */
std::string subp(std::string command)
{
    // stderr > stdout
    command.append(" 2>&1");
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "popen failed.";
    char buffer[PATH_MAX];
    std::string result;
    while (fgets(buffer, PATH_MAX, pipe) != NULL) {
        result += buffer;
        if (strlen(buffer) < PATH_MAX) result += "\n";
    }
    pclose(pipe);
    // remove last "\n"
    if (!result.empty()) result.pop_back();
    return result;
}

UniValue OurUtil::generatezkproof(const JSONRPCRequest& request)
{
    // TODO
    if (request.fHelp || request.params.size() != 2) {
        throw std::runtime_error(
            "generatezkproof \"txid\" \"inputs\"\n"
            "\nGenerate a zk prove of a smart contract.\n"
            "\nArguments:\n"
            "1. \"txid\"        (string) The txid of the deployment transaction\n"
            "2. \"inputs\"      (string) The inputs to the zk function, seperated by spaces\n"
            "\nResult\n"
            "message           (string) Content of the proof\n"
            "\nExamples:\n" +
            HelpExampleCli("generatezkproof", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\" \"1 20 907\""));
    }

    std::string zokrates_home = "~/ZoKrates";
    if (gArgs.IsArgSet("-zokrateshome")) {
        zokrates_home = gArgs.GetArgs("-zokrateshome")[0];
    }
    if (zokrates_home.empty())
        zokrates_home = "/";
    else if (zokrates_home.at(zokrates_home.length() - 1) != '/')
        zokrates_home += "/";
    std::string zokrates = zokrates_home + "target/release/zokrates";
    std::string zokrates_lib = zokrates_home + "zokrates_stdlib/stdlib";

    std::string contractId = request.params[0].get_str();
    std::string fields = request.params[1].get_str();
    std::string filepath = GetDataDir().string() + "/contracts/" + contractId + "/";
    std::string tmp = "deleteme";
    std::string code = filepath + "zk";
    std::string pk = filepath + "proving.key";
    std::string vk = filepath + "verification.key";

    // TODO currently we force using curve=bn128 and sceme=g16
    std::string result;

    result = "[compile]\n";
    result += subp(
        "cd " + filepath + " && " +
        "rm -rf " + tmp + " && " +
        "mkdir " + tmp + " && " +
        "cd " + tmp + " && " +
        zokrates + " compile --input " + code + " --stdlib-path " + zokrates_lib);
    result += "[compute witness]\n";
    result += subp(
        "cd " + filepath + tmp + " && " +
        zokrates + " compute-witness --arguments " + fields);
    result += "[generate proof]\n";
    result += subp(
        "cd " + filepath + tmp + " && " +
        zokrates + " generate-proof --proving-key-path " + pk);
    result += "[verify]\n";
    result += subp(
        "cd " + filepath + tmp + " && " +
        zokrates + " verify --verification-key-path " + vk);
    result += "[proof]\n";
    result = subp(
        "cd " + filepath + tmp + " && " +
        zokrates + " print-proof");
    return result;
}

/*
UniValue OurUtil::sendtoaddresslsm(const JSONRPCRequest& request)
{
    // TODO
    if (request.fHelp || request.params.size() != 2) {
        throw std::runtime_error(
            "callcontract \"txid\" ( \"argv[1]\" \"argv[2]\" ... )\n"
            "\nCall the main function of a smart contract.\n"
            "\nArguments:\n"
            "1. \"txid\"        (string) The txid of the deployment transaction\n"
            "2. \"argv[]\"      (string, optional) The arguments to be passed to the main function.\n"
            "\nResult\n"
            "\"txid\"           (string) The transaction id.\n"
            "\nExamples:\n" +
            HelpExampleCli("callcontract", "\"1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d\" \"arg\""));
    }

    CBitcoinAddress address(request.params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Bitcoin address");

    // Amount
    CAmount nAmount = AmountFromValue(request.params[1]);
    if (nAmount <= 0)
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount for send");

    // TBC
}
*/