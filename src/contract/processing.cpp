#include "contract/processing.h"
#include "amount.h"
#include "base58.h"
#include "script/standard.h"
#include "uint256.h"
#include "util.h"

#include <fstream>

#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>

static fs::path contracts_dir;

const static fs::path& GetContractsDir()
{
    if (!contracts_dir.empty()) return contracts_dir;

    contracts_dir = GetDataDir() / "contracts";
    fs::create_directories(contracts_dir);

    return contracts_dir;
}

static bool call_mkdll(const uint256& contract)
{
    fflush(stdout);
    fflush(stderr);

    pid_t pid = fork();
    pid = fork();

    if (pid < 0) {
        LogPrintf("CRITICAL: fork() failed: %s\n", strerror(errno));
        return false;
    }

    if (pid == 0) {
        std::string err_path = (GetContractsDir().string() + "/" + contract.GetHex() + "/err");
        
        int fd = open(err_path.c_str(), O_WRONLY | O_APPEND | O_CREAT, 0664);
        if (fd < 0) {
            exit(127); 
        }

        if (dup2(fd, STDERR_FILENO) < 0) {
            exit(126);
        }
        close(fd);

        execlp("ourcontract-mkdll", "ourcontract-mkdll",
               GetContractsDir().string().c_str(), contract.GetHex().c_str(),
               (char*)NULL);
        
        exit(EXIT_FAILURE);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        LogPrintf("CRITICAL: waitpid() failed: %s\n", strerror(errno));
        return false;
    }

    if (WIFEXITED(status)) {
        const int exit_code = WEXITSTATUS(status);
        if (exit_code == 0) {
            return true;
        } else {
            LogPrintf("ourcontract-mkdll failed with exit code %d for contract %s\n", exit_code, contract.GetHex());
            return false;
        }
    }
    
    LogPrintf("ourcontract-mkdll terminated abnormally for contract %s\n", contract.GetHex());
    return false;
}

static bool call_rt(ContractDB &cache,
    const uint256 &contract,
    const std::vector<std::string> &args,
    const CTransactionRef &curTx)
{
    auto contractPath = GetDataDir() / "contracts" / contract.GetHex() / "code.so";
    if (!fs::exists(contractPath)) {
        LogPrintf("Contract not found: %s\n", contractPath.string());
        return false;
    }
    
    void *handle = dlopen(contractPath.c_str(), RTLD_NOW);
    if (!handle) {
        LogPrintf("Failed to load contract: %s\n", dlerror());
        return false;
    }
    try {
        json state = cache.getContractState(contract);
        int (*contract_main)(const std::vector<std::string>, json&) = (int (*)(const std::vector<std::string>, json&))dlsym(handle, "contract_main");
        if (!contract_main) {
            LogPrintf("Failed to load contract_main: %s\n", dlerror());
            dlclose(handle);
            return false;
        }

        int ret = contract_main(args, state);
        if (ret != 0)
            throw std::runtime_error("Contract execution failed");
        if (state != nullptr)
            cache.setContractState(contract, state);
    } catch (const std::exception &e) {
        LogPrintf("Failed to get contract state: %s\n", e.what());
        dlclose(handle);
        return false;
    } catch (...) {
        LogPrintf("Failed to get contract state: unknown error\n");
        dlclose(handle);
        return false;
    }
    dlclose(handle);
    return true;
}

bool ExecuteContract(const Contract& contract, const CTransactionRef& curTx, ContractDB& cache)
{
    if (contract.action == contract_action::ACTION_NEW) {
        fs::path new_dir = GetContractsDir() / contract.address.GetHex();
        fs::create_directories(new_dir);
        std::ofstream contract_code(new_dir.string() + "/code.cpp");
        contract_code.write(contract.code.c_str(), contract.code.size());
        contract_code.close();

        if (!call_mkdll(contract.address)) {
            return false;
        }
    } else if (contract.action == contract_action::ACTION_CALL) {
        if (!call_rt(cache, contract.address, contract.args, curTx)) {
            return false;
        }
    }
    return true;
}