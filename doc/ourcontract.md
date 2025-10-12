# OurContract

OurContract is the smart contract platform built on **OurChain**.
This documentation gives a brief overview on OurContract programming and execution.

## Installation

OurContract is built along with OurChain.
If you have already installed OurChain, everything is ready.

## OurContract Programming

OurContract uses C++ as its programming language, with some additional constraints.

### 1. The Contract Entrypoint

Every contract must define an entrypoint function with the following signature:
```cpp
extern "C" int contract_main(const std::vector<std::string>& args);
```

* `args` (`const std::vector<std::string>&`):
A vector of string arguments passed via the callcontract transaction.<br>
Example usage from CLI:
    ```sh
    ./src/bitcoin-cli callcontract "$contract_address" ARG1 ARG2 ...
    ```
    The contract address is automatically prepended as `args[0]`, so `ARG1` from CLI becomes `args[1]`, `ARG2` becomes `args[2]`, and so on. This follows C-style argument conventions.

* **State Access**:<br>
To access your contract's persistent state, use the `get_state()` API:
    ```cpp
    json& state = get_state(args[0]);  // args[0] is the contract address
    ```
    This returns a reference to the contract's JSON state object, which persists across executions.
    > **Note on JSON access:**<br>
    > The JSON object uses C++ **operator overloading** on the `[]` operator to provide access to its two underlying data structures: **JSON objects**, which are accessed with string keys (e.g., `my_json["key"]`), and **JSON arrays**, which are accessed with numerical indices (e.g., `my_json[0]`). For details on this functionality, see the [API documentation for `operator[]`](https://json.nlohmann.me/api/basic_json/operator%5B%5D/).

#### **Architecture Overview**

The contract execution system uses the following design to ensure thread safety and performance:

1. **In-Memory State Buffer**: Contract states are loaded into memory for fast access
2. **Lazy Loading**: States are loaded from the database only when first accessed
3. **Concurrent Execution**: Multiple contracts can execute simultaneously without blocking
4. **Batch Commits**: All state changes are committed to the database atomically after block processing

This design eliminates race conditions while maintaining high performance through parallel execution.

#### **Thread Safety Requirements**

While the system prevents database-level race conditions, **you must implement critical sections in your contract code** when multiple transactions might access the same contract simultaneously.

**Why mutexes are still needed:**
- Multiple transactions can call the same contract in parallel
- Without synchronization, concurrent reads/writes to the same state can cause lost updates
- The in-memory buffer only prevents database corruption, not logical race conditions

**Example 1: Simple mutex from [`test_contract.cpp`](/src/test/test_contract.cpp):**
```cpp
std::mutex contract_mutex;  // Global mutex for this contract

extern "C" int contract_main(const std::vector<std::string> args) {
    std::lock_guard<std::mutex> lock(contract_mutex);  // Lock critical section

    // Get operation type from args (C-style: args[0]=contract_name, args[1]=operation, args[2]=amount)
    std::string contract_name = args[0];
    std::string operation = (args.size() >= 2) ? args[1] : "increment";
    int amount = (args.size() >= 3) ? std::stoi(args[2]) : 1;

    // Get state reference - contract gets its own state buffer using contract address
    json& state_ref = get_state(contract_name);

    // Initialize state if empty
    if (state_ref.is_null() || state_ref.empty()) {
        state_ref = json::object();
        state_ref["counter"] = 0;
        state_ref["operations"] = json::array();
    }

    // Modify state by reference - clean and simple!
    int current_counter = state_ref["counter"];

    if (operation == "increment") {
        state_ref["counter"] = current_counter + amount;
        state_ref["operations"].push_back("increment+" + std::to_string(amount));
        print("Contract: " + contract_name + " Incremented counter by " + std::to_string(amount) + " to " + std::to_string(current_counter + amount));
    }

    // Unlock happens automatically when lock goes out of scope
    return 0;
}
```

**Example 2: Priority-based synchronization using `CMonitor`:**
```cpp
CMonitor<int> monitor;  // Priority-based monitor

extern "C" int contract_main(const std::vector<std::string>& args) {
    json& state = get_state(args[0]);

    monitor.acquire(std::stoi(args[1]));  // args[1] is the priority level

    // Critical section protected by priority-based locking
    int counter = state.value("counter", 0);
    state["counter"] = counter + 1;

    monitor.release();
    return 0;
}
```

**Key points:**
- Use `std::mutex` and `std::lock_guard` for simple automatic unlocking
- Use `CMonitor` for priority-based synchronization (first argument is priority level)
- Lock the entire `contract_main` function for simple contracts
- For complex contracts, lock only the critical sections that modify shared state

### 2. Writing a Basic Contract

You can use the C++ standard library and helper functions defined inside your own `code.cpp`. Here's a complete example from [`test_contract.cpp`](/src/test/test_contract.cpp):

```cpp
#include <string>
#include <mutex>
#include "json/json.hpp"
#include "contract/lib/ourcontract.h"

using json = nlohmann::json;

std::mutex contract_mutex;

extern "C" int contract_main(const std::vector<std::string> args) {
    std::lock_guard<std::mutex> lock(contract_mutex);

    // Get operation type from args (C-style: args[0]=contract_name, args[1]=operation, args[2]=amount)
    std::string contract_name = args[0];
    std::string operation = (args.size() >= 2) ? args[1] : "increment";
    int amount = (args.size() >= 3) ? std::stoi(args[2]) : 1;

    // Get state reference - contract gets its own state buffer using contract address
    json& state_ref = get_state(contract_name);

    // Initialize state if empty
    if (state_ref.is_null() || state_ref.empty()) {
        state_ref = json::object();
        state_ref["counter"] = 0;
        state_ref["operations"] = json::array();
    }

    // Modify state by reference - clean and simple!
    int current_counter = state_ref["counter"];

    if (operation == "increment") {
        state_ref["counter"] = current_counter + amount;
        state_ref["operations"].push_back("increment+" + std::to_string(amount));
        print("Contract: " + contract_name + " Incremented counter by " + std::to_string(amount) + " to " + std::to_string(current_counter + amount));
    } else if (operation == "decrement") {
        state_ref["counter"] = current_counter - amount;
        state_ref["operations"].push_back("decrement-" + std::to_string(amount));
        print("Contract: " + contract_name + " Decremented counter by " + std::to_string(amount) + " to " + std::to_string(current_counter - amount));
    } else if (operation == "set") {
        state_ref["counter"] = amount;
        state_ref["operations"].push_back("set=" + std::to_string(amount));
        print("Contract: " + contract_name + " Set counter to " + std::to_string(amount));
    }

    return 0;
}
```

### 3. Using functions from the node <br>
There are two cases when using functions defined in the OurChain node.
#### A. Functions Defined in Headers

If a function is fully defined in a header, you can simply `#include` the header in your contract.
The compiler will generate the code directly inside the contract `.so`.
No export from the node is required.

Example (header-only function):

A counter contract with synchronization using `CMonitor` from [`prio_monitor.h`](/src/contract/lib/prio_monitor.h):
```cpp
#include <string>
#include <vector>
#include "json/json.hpp"
#include "contract/lib/prio_monitor.h"
using json = nlohmann::json;

CMonitor<int> monitor;

extern "C" int contract_main(const std::vector<std::string>& args)
{
    // Get contract state using the contract address (args[0])
    json& state = get_state(args[0]);

    monitor.acquire(std::stoi(args[1]));  // args[1] is the priority level

    // read counter from persistent JSON state
    int counter = state.value("counter", 0);

    // update it
    counter += 1;

    // write back to JSON state
    state["counter"] = counter;

    monitor.release();
    return 0;
}
```
#### B. Functions Defined in `.cpp` Files (Node-Only Implementation)
If the function’s body lives in the node binary (for example LogPrintf, GetDataDir), contracts cannot access it directly.

To make such a function available to contracts:

1. **Declare** an API in `ourcontract.h` (marked with `PUBLIC`).

2. **Define** it in `libourcontract.cpp`, forwarding to the real node function.

3. **Recompile the node** so the symbol is exported in the dynamic symbol table.

Example wrapper for LogPrintf:
```cpp
// ourcontract.h
extern "C" {
    PUBLIC void print(const char* s);
}

extern "C++" {
    PUBLIC void print(const std::string& s);  // Overloaded version for std::string
}

// libourcontract.cpp
#include "ourcontract.h"
#include "util.h"   // contains LogPrintf
void print(const char* s) {
    // Write to dedicated contract.log file with timestamp
    std::stringstream ss;
    ss << "[CONTRACT] " << s;

    // Write to contract.log in contracts directory
    fs::path contract_log_path = GetDataDir() / "contracts" / "contract.log";
    std::ofstream log_file(contract_log_path, std::ios::app);
    if (log_file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);

        char timestamp[64];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);

        log_file << "[" << timestamp << "] " << ss.str() << std::endl;
        log_file.close();
    }

    // Contract logs are written only to contract.log
}

// Overload for std::string
void print(const std::string& s) {
    print(s.c_str());
}
```
Contract calling it:
```cpp
#include "contract/lib/ourcontract.h"
extern "C" int contract_main(const std::vector<std::string>& args) {
    print("hello from contract");
    print(std::string("also works with std::string"));
    return 0;
}
```
The output of `print()` is written to:
- **`contract.log`**: A dedicated log file with timestamps

### **Contract File Structure**

When contracts are deployed, they are organized in the following directory structure based on the network mode:

**Mainnet:**
```
~/.bitcoin/contracts/
├── contract.log          # All contract print() output with timestamps
└── [contract_address]/   # Each contract has its own directory
    ├── code.cpp          # Contract source code
    ├── code.so           # Compiled contract module
    └── err               # Compilation error messages (if any)
```

**Regtest:**
```
~/.bitcoin/regtest/contracts/
├── contract.log          # All contract print() output with timestamps
└── [contract_address]/   # Each contract has its own directory
    ├── code.cpp          # Contract source code
    ├── code.so           # Compiled contract module
    └── err               # Compilation error messages (if any)
```

**Testnet:**
```
~/.bitcoin/testnet3/contracts/
├── contract.log          # All contract print() output with timestamps
└── [contract_address]/   # Each contract has its own directory
    ├── code.cpp          # Contract source code
    ├── code.so           # Compiled contract module
    └── err               # Compilation error messages (if any)
```

- **`contract.log`**: Contains all `print()` output from all contracts with timestamps
- **`err`**: Contains compilation error messages when `ourcontract-mkdll` fails

### 4. Available API Functions

The following functions are available to contracts through the `ourcontract.h` header:

#### **State Management**
```cpp
json& get_state(const std::string& contract_address);
```
Returns a reference to the contract's persistent state. Use the contract address (typically `args[0]`) to access your contract's state.

#### **Logging**
```cpp
void print(const char* s);                    // C-style string
void print(const std::string& s);            // C++ std::string (overload)
```
Outputs messages to `contract.log` with timestamps.

#### **Contract Calls**
```cpp
bool call_contract(char* contract_id, char* func_name, const std::vector<std::string>& args);
```
Allows contracts to call other contracts (for advanced use cases).

For more details, see:
* Test contract: [`test_contract.cpp`](/src/test/test_contract.cpp)
* API header: [`ourcontract.h`](/src/contract/lib/ourcontract.h)
* API implementation: [`libourcontract.cpp`](/src/contract/lib/libourcontract.cpp)

## OurContract Execution

Contracts are deployed and executed through OurChain RPC commands.

* RPC Command to interact with contract: [`mytest.sh`](/mytest.sh)

### Deploy a Contract
```
bitcoin-cli deploycontract code.cpp
```

* Creates a transaction containing the source `code.cpp` and arguments.

* Broadcasts it to the network.

* Once the transaction is included in a block, nodes automatically run `ourcontract-mkdll` to compile code.cpp.

* The returned **txid** identifies the contract.

### Call a Contract
```
bitcoin-cli callcontract [txid] [ARG1] [ARG2] ...
```
* Creates a transaction referencing the deployed contract.

* Broadcasts it to the network.

* Once included in a block, nodes execute the `contract_main` function of the contract.

## Debugging and Troubleshooting

### **Viewing Contract Logs**
To see contract execution output (adjust path based on network mode):

**Mainnet:**
```sh
tail -f ~/.bitcoin/contracts/contract.log
```

**Regtest:**
```sh
tail -f ~/.bitcoin/regtest/contracts/contract.log
```

**Testnet:**
```sh
tail -f ~/.bitcoin/testnet3/contracts/contract.log
```

### **Checking Compilation Errors**
If contract deployment fails, check for compilation errors (adjust path based on network mode):

**Mainnet:**
```sh
cat ~/.bitcoin/contracts/[contract_address]/err
```

**Regtest:**
```sh
cat ~/.bitcoin/regtest/contracts/[contract_address]/err
```

**Testnet:**
```sh
cat ~/.bitcoin/testnet3/contracts/[contract_address]/err
```

### **Example Debugging Session (Regtest)**
```sh
# 1. Start regtest node
bitcoin-cli -regtest

# 2. Deploy a contract
bitcoin-cli -regtest deploycontract code.cpp

# 3. Call the contract
bitcoin-cli -regtest callcontract [txid] ARG1 ARG2

# 4. Check contract logs
tail -f ~/.bitcoin/regtest/contracts/contract.log

# 5. If deployment failed, check compilation errors
cat ~/.bitcoin/regtest/contracts/[contract_address]/err
```

## Development Progress

The following features are still in progress:

### OurCoin Transferring

A contract needs to be able to receive OurCoin and send OurCoin to some addresses. This is not trivial since a contract can not secretly store private keys to hold OurCoin. A possible approach is to design new OPs for both locking scripts and unlocking scripts in a way that only the contract can unlock its own transaction outputs.

### OurContract Code Security

Currently GCC is directly used to compile OurContract source files into dynamic linked libraries. This implementation exposes some issues due to the fact that the OurContract runtime do not have enough control to the behaviors of user-written contracts. For example, an attacker may write a contract that accesses invalid pointers or array elements with invalid indice to cause runtime errors, or executes system calls to control full-nodes. A possible approach is to create our own C++ compiler and linker based on GCC. Our compiler will generate runtime-check code to prevent invalid memory access, and our linker will block all calls to external functions not declared in "ourcontract.h".
