#include "contract/processing.h"
#include "amount.h"
#include "base58.h"
#include "script/standard.h"
#include "uint256.h"
#include "util.h"

#include <fstream>
#include <string>
#include <vector>

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

static void call_mkdll(const uint256& contract)
{
    fflush(stdout);
    fflush(stderr);

    pid_t pid = fork();

    if (pid < 0)
        throw std::runtime_error("fork() failed: " + std::string(strerror(errno)));

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
    if (waitpid(pid, &status, 0) < 0)
        throw std::runtime_error("waitpid() failed: " + std::string(strerror(errno)));

    if (WIFEXITED(status)) {
        const int exit_code = WEXITSTATUS(status);
        if (exit_code != 0)
            throw std::runtime_error("ourcontract-mkdll failed with exit code " + std::to_string(exit_code));
    } else {
        throw std::runtime_error("ourcontract-mkdll terminated abnormally.");
    }
}

using DlHandle = std::unique_ptr<void, int (*)(void*)>;

static void call_rt(ContractDB &cache,
    const uint256 &contract,
    const std::vector<std::string> &args,
    const CTransactionRef &curTx)
{
    auto contractPath = GetDataDir() / "contracts" / contract.GetHex() / "code.so";
    if (!fs::exists(contractPath)) {
        throw std::runtime_error("Contract not found: " + contractPath.string());
    }
    
    void *handle = dlopen(contractPath.c_str(), RTLD_NOW);
    if (!handle)
        throw std::runtime_error("Failed to load contract: " + std::string(dlerror()));
    
    DlHandle handle_ptr(handle, &dlclose);
    using ContractMainFunc = int (*)(const std::vector<std::string>&);
    ContractMainFunc contract_main = (ContractMainFunc)dlsym(handle_ptr.get(), "contract_main");

    if (!contract_main)
        throw std::runtime_error("Failed to load symbol 'contract_main': " + std::string(dlerror()));

    // Build contract args with contract address as args[0] (C-style)
    std::vector<std::string> contract_args;
    contract_args.push_back(contract.GetHex());  // args[0] = contract address
    contract_args.insert(contract_args.end(), args.begin(), args.end());  // append user args
    
    // Contract gets state reference directly via get_state() API
    int ret = contract_main(contract_args);

    if (ret != 0) {
        throw std::runtime_error("Contract execution failed with exit code: " + std::to_string(ret));
    }
}

void ExecuteContract(const Contract& contract, const CTransactionRef& curTx, ContractDB& cache)
{
    if (contract.action == contract_action::ACTION_NEW) {
        fs::path new_dir = GetContractsDir() / contract.address.GetHex();
        fs::create_directories(new_dir);
        std::ofstream contract_code(new_dir.string() + "/code.cpp");
        contract_code.write(contract.code.c_str(), contract.code.size());
        contract_code.close();
        call_mkdll(contract.address);
    } else if (contract.action == contract_action::ACTION_CALL) {
        call_rt(cache, contract.address, contract.args, curTx);
    }
}