#include "contract/server.h"
#include "util.h"
#include "validation.h"
#include <atomic>
#include <condition_variable>
#include <dlfcn.h>
#include <json/json.hpp>
#include <mutex>
#include <stack>
#include <sys/syscall.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <zmq.hpp>

using namespace std;
using namespace zmq;
using namespace nlohmann;

ContractServer* server = nullptr;
atomic<bool> stopFlag(false);

ContractLocalState::ContractLocalState(std::string stateStr)
{
    // try to parse json
    json j = json::parse(stateStr);
    // if failed, set state to null
    if (j.is_null()) {
        this->state = "null";
        return;
    }
    this->state = stateStr;
}

ContractLocalState::~ContractLocalState()
{
}

string ContractLocalState::getState()
{
    return this->state;
}

void ContractLocalState::setState(string state)
{
    this->state = state;
}

bool ContractLocalState::isStateNull()
{
    return this->state == "null";
}

void ContractLocalState::setPreState(string preState)
{
    this->preState = preState;
}

string ContractLocalState::getPreState()
{
    if (this->preState.empty()) {
        return "null";
    }
    return this->preState;
}

static bool readContractCache(string* state, string* hex_ctid)
{
    json j = contractStateCache->getSnapShot()->getContractState(*hex_ctid);
    *state = j.dump();
    return true;
}

static bool writeContractCache(string* state, string* hex_ctid)
{
    if (*state == "null") {
        *state = "{}";
    }
    json j = json::parse(*state);
    contractStateCache->getSnapShot()->setContractState(uint256S(*hex_ctid), j);
    return true;
}

static bool callContract(string* hex_ctid, ContractArguments* arg)
{
    auto contractPath = contractStateCache->getContractPath(*hex_ctid) / "code.so";
    if (!fs::exists(contractPath)) {
        LogPrintf("Contract not found: %s\n", contractPath.string());
        return false;
    }
    // execute contract
    void* handle = dlopen(contractPath.c_str(), RTLD_LAZY);
    if (!handle) {
        LogPrintf("Failed to load contract: %s\n", dlerror());
        return false;
    }
    try {
        dlerror();
        int (*contract_main)(ContractArguments*) = (int (*)(ContractArguments*))dlsym(handle, "contract_main");
        if (!contract_main) {
            LogPrintf("Failed to load contract_main: %s\n", dlerror());
            dlclose(handle);
            return false;
        }
        auto error = dlerror();
        if (error != nullptr) {
            throw runtime_error("Contract dlsym failed");
        }
        int ret = contract_main(arg);
        if (ret != 0) {
            throw runtime_error("Contract execution failed");
        }
    } catch (const std::exception& e) {
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

ContractServer::ContractServer()
{
    numThreads = CONTRACT_SERVER_THREADS;
    threadPool.reserve(numThreads);
    proxyThreadInstance = thread(&ContractServer::proxyThread, this);
    zmqPullPort = gArgs.GetArg("-zmqpullerport", 5560);
    zmqPushPort = gArgs.GetArg("-zmqpusherport", 5559);
}

ContractServer::~ContractServer()
{
    LogPrintf("contract server destroyed\n");
}

static void responseZmqMessage(zmq::socket_t& socket, const string& response)
{
    zmq::message_t message(response.size());
    memcpy(message.data(), response.c_str(), response.size());
    socket.send(message, zmq::send_flags::none);
}

static struct zmqMsg parseZmqMsg(const string& message)
{
    json j = json::parse(message);
    struct zmqMsg msg = {
        .address = j["address"],
        .parameters = j["parameters"],
        .isPure = j["isPure"],
        .preTxid = j["preTxid"],
    };
    return msg;
}

void ContractServer::workerThread()
{
    RenameThread("contract-worker");
    context_t context(1);
    zmq::socket_t puller(context, zmq::socket_type::rep);
    puller.connect(strprintf("tcp://127.0.0.1:%d", zmqPullPort));
    puller.set(zmq::sockopt::rcvtimeo, 2000);
    while (true) {
        zmq::message_t message;
        zmq::recv_result_t result = puller.recv(message, zmq::recv_flags::none);
        if (result) {
            std::string recv_msg(static_cast<char*>(message.data()), message.size());
            LogPrintf("Contract Received message: %s\n", recv_msg.c_str());
            // convert message to json to struct
            auto msg = parseZmqMsg(recv_msg);
            auto address = msg.address;
            auto isPure = msg.isPure;
            auto stateStack = new std::stack<ContractLocalState*>;
            auto mtx = new std::mutex;
            std::string state_buf = "{}";
            // init lamda function
            std::function<string()> readFunc = [stateStack]() {
                auto top = stateStack->top();
                return top->getState();
            };
            std::function<string()> readPreFunc = [stateStack]() {
                auto top = stateStack->top();
                return top->getPreState();
            };
            std::function<bool(string, string)> writeGeneralInterface = [stateStack](string name, string version) {
                json j = json::object();
                j["name"] = name;
                j["version"] = version;
                string state = j.dump();
                auto top = stateStack->top();
                top->setState(state);
                return true;
            };
            std::function<bool(string*)> writeFunc = [stateStack](string* state) {
                auto top = stateStack->top();
                top->setState(*state);
                return true;
            };
            std::function<void(string)> logFunc = [](string log) {
                LogPrintf("Contract log: %s\n", log.c_str());
            };
            std::function<bool(ContractArguments*)> callFunc = [&state_buf, isPure, stateStack, &address](ContractArguments* arg) {
                // read contract state in disk
                string state = "";
                if (!readContractCache(&state, &arg->address)) {
                    return false;
                }
                // push state to stack
                stateStack->push(new ContractLocalState(state));
                // call contract
                if (!callContract(&arg->address, arg)) {
                    return false;
                }
                // pop state from stack
                auto tmp = stateStack->top();
                std::string buf = tmp->getState();
                stateStack->pop();
                delete tmp;
                if (stateStack->size() == 0) {
                    // write state to DB or pure output
                    if (isPure) {
                        state_buf = buf;
                    } else {
                        if (buf == "null") {
                            buf = "{}";
                        }
                        if (!writeContractCache(&buf, &address)) {
                            return false;
                        }
                    }
                } else {
                    // write state to parent contract
                    auto top = stateStack->top();
                    top->setPreState(buf);
                }
                return true;
            };
            ContractArguments arg = {
                .api = {
                    .readContractState = readFunc,
                    .readPreContractState = readPreFunc,
                    .writeContractState = writeFunc,
                    .generalContractInterfaceOutput = writeGeneralInterface,
                    .contractLog = logFunc,
                    .recursiveCall = callFunc,
                },
                .address = address,
                .isPureCall = msg.isPure,
                .parameters = msg.parameters,
                .preTxid = msg.preTxid,
                .stateStack = stateStack,
                .mtx = mtx,
            };
            // execute contract
            if (!callFunc(&arg)) {
                responseZmqMessage(puller, "FAILED");
                delete stateStack;
                continue;
            }
            // send state
            responseZmqMessage(puller, state_buf);
            delete stateStack;
        } else {
            if (stopFlag.load()) {
                break;
            }
        }
    }
    puller.close();
}

void ContractServer::proxyThread()
{
    RenameThread("contract-proxy");
    context_t context(1);
    zmq::socket_t frontend(context, zmq::socket_type::router);
    zmq::socket_t backend(context, zmq::socket_type::dealer);
    frontend.bind(strprintf("tcp://*:%d", zmqPushPort));
    backend.bind(strprintf("tcp://*:%d", zmqPullPort));
    zmq::proxy(frontend, backend, nullptr);
    // close sockets
    frontend.close();
    backend.close();
}

bool ContractServer::start()
{
    stopFlag.store(false);
    for (int i = 0; i < numThreads; i++) {
        threadPool.emplace_back(&ContractServer::workerThread, this);
    }
    return true;
}

bool ContractServer::stop()
{
    for (auto& thread : threadPool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threadPool.clear();
    LogPrintf("contract server stopped\n");
    return true;
}

bool ContractServer::interrupt()
{
    // stop the worker threads
    stopFlag.store(true);
    return true;
}

int ContractServer::getZmqPullPort()
{
    return zmqPullPort;
}

int ContractServer::getZmqPushPort()
{
    return zmqPushPort;
}

bool contractServerInit()
{
    if (server != nullptr) {
        LogPrintf("contract server is running\n");
        return false;
    }
    LogPrintf("contract server initialized\n");
    server = new ContractServer();
    return true;
}

bool startContractServer()
{
    if (server == nullptr) {
        LogPrintf("contract server is not running\n");
        return false;
    }
    return server->start();
}

bool stopContractServer()
{
    if (stopFlag.load() == false && server != nullptr) {
        interruptContractServer();
    }
    if (!server->stop()) {
        LogPrintf("failed to stop contract server\n");
        return false;
    }
    // delete server;
    // server = nullptr;

    LogPrintf("contract server stopped\n");
    return true;
}

bool interruptContractServer()
{
    return server->interrupt();
}