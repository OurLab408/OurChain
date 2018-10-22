#include "contract/processing.h"
#include "util.h"

#include <string>
#include <fstream>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static fs::path contracts_dir;

const static fs::path &GetContractsDir()
{
    if(!contracts_dir.empty()) return contracts_dir;

    contracts_dir = GetDataDir() / "contracts";
    fs::create_directories(contracts_dir);

    return contracts_dir;
}

static int call_mkdll(const std::string &contract)
{
    int pid, status;

    pid = fork();
    if(pid == 0) {
        int fd = open((GetContractsDir().string() + "/err").c_str(),
                      O_WRONLY | O_APPEND | O_CREAT,
                      0664);
        dup2(fd, STDERR_FILENO);
        close(fd);

        execlp("ourcontract-mkdll",
               "ourcontract-mkdll",
               GetContractsDir().string().c_str(),
               contract.c_str(),
               NULL);
        exit(EXIT_FAILURE);
    }

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false) return -1;
    if (WEXITSTATUS(status) != 0) return -1;
    return 0;
}

static int call_rt(const std::string &contract, const std::vector<std::string> &args)
{
    int pid, status;

    pid = fork();
    if(pid == 0) {
        int fd = open((GetContractsDir().string() + "/err").c_str(),
                      O_WRONLY | O_APPEND | O_CREAT,
                      0664);
        dup2(fd, STDERR_FILENO);
        close(fd);

        const char **argv = (const char**)malloc((args.size() + 4) * sizeof(char*));
        argv[0] = "ourcontract-rt";
        argv[1] = GetContractsDir().string().c_str();
        argv[2] = contract.c_str();
        for (unsigned i = 0; i < args.size(); i++) argv[i + 3] = args[i].c_str();
        argv[args.size() + 3] = NULL;
        execvp("ourcontract-rt", (char* const*)argv);
        exit(EXIT_FAILURE);
    }

    waitpid(pid, &status, 0);
    if (WIFEXITED(status) == false) return -1;
    if (WEXITSTATUS(status) != 0) return -1;
    return 0;
}

bool ProcessContract(const std::string &txid, const Contract &contract)
{
    if (contract.action == contract_action::ACTION_NEW) {
        fs::path new_dir = GetContractsDir() / txid;
        fs::create_directories(new_dir);
        std::ofstream contract_code(new_dir.string() + "/code.c");
        contract_code.write(contract.code.c_str(), contract.code.size());
        contract_code.close();

        if (call_mkdll(txid) < 0) {
            /* TODO: clean up files */
            return false;
        }

        if (call_rt(txid, contract.args) < 0) {
            /* TODO: perform state recovery */
            return false;
        }
    } else if (contract.action == contract_action::ACTION_CALL) {
        if (call_rt(contract.callee.ToString(), contract.args) < 0) {
            /* TODO: perform state recovery */
            return false;
        }
    }

    return true;
}
