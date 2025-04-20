#include "contract/processing.h"
#include "amount.h"
#include "base58.h"
#include "primitives/transaction.h"
#include "script/standard.h"
#include "uint256.h"
#include "util.h"

#include <fstream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dlfcn.h>

static fs::path contracts_dir;

const static fs::path &GetContractsDir()
{
    if (!contracts_dir.empty())
        return contracts_dir;

    contracts_dir = GetDataDir() / "contracts";
    fs::create_directories(contracts_dir);

    return contracts_dir;
}

static void exec_dll(const uint256 &contract,
    const std::vector<std::string> &args,
    int fd_state_read[2],
    int fd_state_write[2])
{
    int fd_error = open((GetContractsDir().string() + "/err").c_str(),
        O_WRONLY | O_APPEND | O_CREAT, 0664);
    dup2(fd_error, STDERR_FILENO);
    close(fd_error);
    // state & TX
    dup2(fd_state_read[1], STDOUT_FILENO);
    dup2(fd_state_write[0], STDIN_FILENO);
    close(fd_state_read[0]);
    close(fd_state_read[1]);
    close(fd_state_write[0]);
    close(fd_state_write[1]);
    const char **argv =
        (const char **) malloc((args.size() + 4) * sizeof(char *));
    argv[0] = "ourcontract-rt";
    argv[1] = GetContractsDir().string().c_str();
    std::string hex_ctid(contract.GetHex());
    argv[2] = hex_ctid.c_str();
    for (unsigned i = 0; i < args.size(); i++)
        argv[i + 3] = args[i].c_str();
    argv[args.size() + 3] = NULL;
    execvp("ourcontract-rt", (char *const *) argv);
    exit(EXIT_FAILURE);
}

static void read_state_from_json(json j, int &flag, FILE *pipe_state_write)
{
    if (!j.is_null()) {
        std::string newbuffer = j.dump();
        flag = newbuffer.size();
        fwrite((void *) &flag, sizeof(int), 1, pipe_state_write);
        fflush(pipe_state_write);
        fwrite(
            (void *) newbuffer.data(), newbuffer.size(), 1, pipe_state_write);
        fflush(pipe_state_write);
    } else {
        // client will not recive data after flag is 0
        fwrite((void *) &flag, sizeof(int), 1, pipe_state_write);
        fflush(pipe_state_write);
    }
}

static void read_state_from_cache(ContractStateCache *cache,
    std::string &hex_ctid,
    int &flag,
    FILE *pipe_state_write)
{
    json j = cache->getSnapShot()->getContractState(hex_ctid);
    read_state_from_json(j, flag, pipe_state_write);
}

static void read_state_from_tmpDB(ContractDBWrapper *cache,
    std::string &hex_ctid,
    int &flag,
    FILE *pipe_state_write)
{
    json j = json::parse(cache->getState(hex_ctid));
    read_state_from_json(j, flag, pipe_state_write);
}

static int read_buffer_size(FILE *pipe_state_read)
{
    int size;
    int ret = fread((void *) &size, sizeof(int), 1, pipe_state_read);
    assert(ret >= 0);
    return size;
}

static void write_state_to_cache(ContractStateCache *cache,
    std::string &hex_ctid,
    int &size,
    FILE *pipe_state_read)
{
    char *tmp = (char *) malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    std::string tmp_str(tmp, size);
    assert(ret >= 0);
    cache->getSnapShot()->setContractState(
        uint256S(hex_ctid), json::parse(tmp_str));
    free(tmp);
}

static std::string read_char64(FILE *pipe_state_read)
{
    int size = sizeof(char) * 64;
    char *tmp = (char *) malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    assert(ret >= 0);
    std::string address(tmp, 64);
    free(tmp);
    return address;
}

static std::string write_state_as_string(std::string &hex_ctid,
    int &size,
    FILE *pipe_state_read)
{
    char *tmp = (char *) malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    assert(ret >= 0);
    return std::string(tmp, size);
}

static bool call_mkdll(const uint256 &contract_addr)
{
    int pid, status;
    pid = fork();
    if (pid == 0) {
        int fd = open((GetContractsDir().string() + "/" + contract_addr.GetHex() + "/err").c_str(),
            O_WRONLY | O_APPEND | O_CREAT, 0664);
        dup2(fd, STDERR_FILENO);
        close(fd);
        execlp("ourcontract-mkdll", "ourcontract-mkdll",
            GetContractsDir().string().c_str(), contract_addr.GetHex().c_str(),
            NULL);
        exit(EXIT_FAILURE);
    }

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false)
        return false;
    if (WEXITSTATUS(status) != 0)
        return false;
    return true;
}

static bool call_rt(ContractStateCache *cache,
    const uint256 &contract_addr,
    const std::vector<std::string> &args,
    const CTransactionRef &curTx)
{
    auto contractPath = GetDataDir() / "contracts" / contract_addr.GetHex() / "code.so";
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
        int (*contract_main)(int, char**) = (int (*)(int, char**)) dlsym(handle, "contract_main");
        if (!contract_main) {
            LogPrintf("Failed to load contract_main: %s\n", dlerror());
            dlclose(handle);
            return false;
        }
        int argc = args.size();
        std::vector<char*> argv;
        argv.reserve(argc);
        for (const auto& s : args)
            argv.push_back(const_cast<char*>(s.c_str()));

        int ret = contract_main(argc, argv.data());
        if (ret != 0)
            throw std::runtime_error("Contract execution failed");
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

bool ExecuteContract(const Contract &contract,
    const CTransactionRef &curTx,
    ContractStateCache *cache)
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
        if (!call_rt(cache, contract.address, contract.args, curTx)) {
            return false;
        }
    } else if (contract.action == contract_action::ACTION_CALL) {
        if (!call_rt(cache, contract.address, contract.args, curTx)) {
            return false;
        }
    }
    return true;
}