#ifndef CONTRACT_SERVER_H
#define CONTRACT_SERVER_H

#define CONTRACT_SERVER_THREADS 1

#include <json/json.hpp>
#include <mutex>
#include <stack>
#include <thread>
#include <vector>
#include <zmq.hpp>

using namespace std;

class ContractServer
{
public:
    ContractServer();
    ~ContractServer();
    bool start();
    bool stop();
    bool interrupt();

private:
    int numThreads;
    vector<thread> threadPool;
    thread proxyThreadInstance;

    void workerThread();
    void proxyThread();
};

extern ContractServer* server;

struct zmqMsg {
    string address;
    vector<string> parameters;
    bool isPure;
    string preTxid;
};

bool contractServerInit();
bool startContractServer();
bool stopContractServer();
bool interruptContractServer();

// contract interface
class ContractLocalState
{
private:
    string state;
    string preState;

public:
    ContractLocalState(std::string stateStr);
    ~ContractLocalState();
    void setState(string state);
    void setPreState(string preState);
    string getPreState();
    string getState();
    bool isStateNull();
};

struct ContractArguments;

struct ContractAPI {
    // read contract state
    std::function<string()> readContractState;
    // read pre contract state
    std::function<string()> readPreContractState;
    // write contract state
    std::function<bool(string*)> writeContractState;
    // write general contract interface output
    std::function<void(string, string)> generalContractInterfaceOutput;
    // print log
    std::function<void(string)> contractLog;
    // recursive call contract
    std::function<bool(ContractArguments*)> recursiveCall;
};

struct ContractArguments {
    // contract api interface
    ContractAPI api;
    // contract address
    string address;
    // is pure call contract or not
    bool isPureCall = false;
    // parameters
    vector<string> parameters = {};
    // pre txid
    string preTxid;
    // call stack
    stack<ContractLocalState*>* stateStack;
    // mtx
    mutex* mtx;
};

#endif // CONTRACT_SERVER_H