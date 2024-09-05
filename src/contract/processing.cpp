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

static fs::path contracts_dir;

const static fs::path& GetContractsDir()
{
    if (!contracts_dir.empty()) return contracts_dir;

    contracts_dir = GetDataDir() / "contracts";
    fs::create_directories(contracts_dir);

    return contracts_dir;
}

static void exec_dll(const uint256& contract, const std::vector<std::string>& args, int fd_state_read[2], int fd_state_write[2])
{
    int fd_error = open((GetContractsDir().string() + "/err").c_str(),
        O_WRONLY | O_APPEND | O_CREAT,
        0664);
    dup2(fd_error, STDERR_FILENO);
    close(fd_error);
    // state & TX
    dup2(fd_state_read[1], STDOUT_FILENO);
    dup2(fd_state_write[0], STDIN_FILENO);
    close(fd_state_read[0]);
    close(fd_state_read[1]);
    close(fd_state_write[0]);
    close(fd_state_write[1]);
    const char** argv = (const char**)malloc((args.size() + 4) * sizeof(char*));
    argv[0] = "ourcontract-rt";
    argv[1] = GetContractsDir().string().c_str();
    std::string hex_ctid(contract.GetHex());
    argv[2] = hex_ctid.c_str();
    for (unsigned i = 0; i < args.size(); i++)
        argv[i + 3] = args[i].c_str();
    argv[args.size() + 3] = NULL;
    execvp("ourcontract-rt", (char* const*)argv);
    exit(EXIT_FAILURE);
}

static void read_state_from_json(json j, int& flag, FILE* pipe_state_write)
{
    if (!j.is_null()) {
        std::string newbuffer = j.dump();
        flag = newbuffer.size();
        fwrite((void*)&flag, sizeof(int), 1, pipe_state_write);
        fflush(pipe_state_write);
        fwrite((void*)newbuffer.data(), newbuffer.size(), 1, pipe_state_write);
        fflush(pipe_state_write);
    } else {
        // client will not recive data after flag is 0
        fwrite((void*)&flag, sizeof(int), 1, pipe_state_write);
        fflush(pipe_state_write);
    }
}

static void read_state_from_cache(ContractStateCache* cache, std::string& hex_ctid, int& flag, FILE* pipe_state_write)
{
    json j = cache->getSnapShot()->getContractState(hex_ctid);
    read_state_from_json(j, flag, pipe_state_write);
}

static void read_state_from_tmpDB(ContractDBWrapper* cache, std::string& hex_ctid, int& flag, FILE* pipe_state_write)
{
    json j = json::parse(cache->getState(hex_ctid));
    read_state_from_json(j, flag, pipe_state_write);
}

static int read_buffer_size(FILE* pipe_state_read)
{
    int size;
    int ret = fread((void*)&size, sizeof(int), 1, pipe_state_read);
    assert(ret >= 0);
    return size;
}

static void write_state_to_cache(ContractStateCache* cache, std::string& hex_ctid, int& size, FILE* pipe_state_read)
{
    char* tmp = (char*)malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    std::string tmp_str(tmp, size);
    assert(ret >= 0);
    cache->getSnapShot()->setContractState(uint256S(hex_ctid), json::parse(tmp_str));
    free(tmp);
}

static std::string read_char64(FILE* pipe_state_read)
{
    int size = sizeof(char) * 64;
    char* tmp = (char*)malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    assert(ret >= 0);
    std::string address(tmp, 64);
    free(tmp);
    return address;
}

static std::string write_state_as_string(std::string& hex_ctid, int& size, FILE* pipe_state_read)
{
    char* tmp = (char*)malloc(size);
    int ret = fread(tmp, 1, size, pipe_state_read);
    assert(ret >= 0);
    return std::string(tmp, size);
}

static int call_mkdll(const uint256& contract)
{
    int pid, status;
    pid = fork();
    if (pid == 0) {
        int fd = open((GetContractsDir().string() + "/err").c_str(),
            O_WRONLY | O_APPEND | O_CREAT,
            0664);
        dup2(fd, STDERR_FILENO);
        close(fd);
        execlp("ourcontract-mkdll",
            "ourcontract-mkdll",
            GetContractsDir().string().c_str(),
            contract.GetHex().c_str(),
            NULL);
        exit(EXIT_FAILURE);
    }

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false) return -1;
    if (WEXITSTATUS(status) != 0) return -1;
    return 0;
}

static int call_rt(ContractStateCache* cache, const uint256& contract, const std::vector<std::string>& args, const CTransactionRef& curTx)
{
    int pid, status;
    int fd_state_read[2], fd_state_write[2];
    if (pipe(fd_state_read) == -1) return -1;
    if (pipe(fd_state_write) == -1) return -1;

    pid = fork();
    if (pid == 0) {
        exec_dll(contract, args, fd_state_read, fd_state_write);
    }

    // read or write state or send money
    close(fd_state_read[1]);
    close(fd_state_write[0]);

    FILE* pipe_state_read = fdopen(fd_state_read[0], "rb");
    FILE* pipe_state_write = fdopen(fd_state_write[1], "wb");

    std::string hex_ctid(contract.GetHex());
    int flag;
    while (fread((void*)&flag, sizeof(int), 1, pipe_state_read) != 0) {
        if (flag == BYTE_READ_STATE) { // read state
            auto targetAddress = read_char64(pipe_state_read);
            read_state_from_cache(cache, targetAddress, flag, pipe_state_write);
        } else if (flag == BYTE_WRITE_STATE) { // write state
            int size = read_buffer_size(pipe_state_read);
            write_state_to_cache(cache, hex_ctid, size, pipe_state_read);
        } else if (flag == CHECK_RUNTIME_STATE) { // check mode (pure = 0, not pure = 1)
            flag = 1;
            fwrite((void*)&flag, sizeof(int), 1, pipe_state_write);
            fflush(pipe_state_write);
        } else if (flag == GET_PRE_TXID_STATE) {
            std::string txid = curTx.get()->vin[0].prevout.hash.ToString();
            fwrite((void*)txid.c_str(), sizeof(char) * 64, 1, pipe_state_write);
            fflush(pipe_state_write);
        } else {
            break;
        }
    }
    fclose(pipe_state_read);
    fclose(pipe_state_write);

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false) return -1;
    if (WEXITSTATUS(status) != 0) return -1;
    return 0;
}

std::string call_rt_pure(ContractDBWrapper* cache, const uint256& contract, const std::vector<std::string>& args)
{
    int pid, status;
    int fd_state_read[2], fd_state_write[2];
    if (pipe(fd_state_read) == -1) return "";
    if (pipe(fd_state_write) == -1) return "";

    pid = fork();
    if (pid == 0) {
        exec_dll(contract, args, fd_state_read, fd_state_write);
    }

    // read or write state or send money
    close(fd_state_read[1]);
    close(fd_state_write[0]);

    FILE* pipe_state_read = fdopen(fd_state_read[0], "rb");
    FILE* pipe_state_write = fdopen(fd_state_write[1], "wb");

    std::string hex_ctid(contract.GetHex());
    int flag;
    std::string result = "";
    while (fread((void*)&flag, sizeof(int), 1, pipe_state_read) != 0) {
        if (flag == BYTE_READ_STATE) { // read state
            auto targetAddress = read_char64(pipe_state_read);
            read_state_from_tmpDB(cache, targetAddress, flag, pipe_state_write);
        } else if (flag == BYTE_WRITE_STATE) { // write state
            int size = read_buffer_size(pipe_state_read);
            result = write_state_as_string(hex_ctid, size, pipe_state_read);
        } else if (flag == CHECK_RUNTIME_STATE) { // check mode (pure = 0, not pure = 1)
            flag = 0;
            fwrite((void*)&flag, sizeof(int), 1, pipe_state_write);
            fflush(pipe_state_write);
        } else if (flag == GET_PRE_TXID_STATE) {
            char* tmp = new char[64]{0};
            fwrite((void*)tmp, sizeof(char) * 64, 1, pipe_state_write);
            fflush(pipe_state_write);
        } else {
            break;
        }
    }
    fclose(pipe_state_read);
    fclose(pipe_state_write);

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false) return "";
    if (WEXITSTATUS(status) != 0) return "";
    return result;
}

bool ProcessContract(const Contract& contract, const CTransactionRef& curTx, ContractStateCache* cache)
{
    if (contract.action == contract_action::ACTION_NEW) {
        fs::path new_dir = GetContractsDir() / contract.address.GetHex();
        fs::create_directories(new_dir);
        std::ofstream contract_code(new_dir.string() + "/code.cpp");
        contract_code.write(contract.code.c_str(), contract.code.size());
        contract_code.close();

        if (call_mkdll(contract.address) < 0) {
            return false;
        }
        if (call_rt(cache, contract.address, contract.args, curTx) < 0) {
            return false;
        }
    } else if (contract.action == contract_action::ACTION_CALL) {
        if (call_rt(cache, contract.address, contract.args, curTx) < 0) {
            return false;
        }
    }

    return true;
}
