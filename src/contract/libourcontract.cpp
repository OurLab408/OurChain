#include "ourcontract.h"


bool call_contract(char *contract_id,
    char *func_name,
    const std::vector<std::string> &vec)
{
    auto contract_file = GetDataDir() / "contracts" / contract_id / "code.so";
    if (!fs::exists(contract_file)) {
        LogPrintf("Contract not found: %s\n", contract_file.string());
        return false;
    }
    void *handle = dlopen(contract_file.c_str(), RTLD_LAZY);
    if (!handle) {
        LogPrintf("Failed to load contract: %s\n", dlerror());
        return false;
    }
    int (*contract_func)(int, char **) =
            (int (*)(int, char **)) dlsym(handle, func_name);
    if (!contract_func) {
            LogPrintf("Failed to load contract_func: %s\n", dlerror());
            dlclose(handle);
            return false;
    }
    std::vector<char *> argv;
    argv.reserve(vec.size());
    for (const auto &s : vec)
        argv.push_back(const_cast<char *>(s.c_str()));
    int ret = contract_func(vec.size(), argv.data());
    dlclose(handle);
    return true;
}


void print(const char *s) {
    LogPrintf("Contract log: %s\n", s);
}