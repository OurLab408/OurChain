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
extern "C" int contract_main(const std::vector<std::string>& args, json& state);
```

* `args` (`const std::vector<std::string>&`):
A vector of string arguments passed via the callcontract transaction.<br>
Example usage from CLI:
    ```sh
    ./src/bitcoin-cli callcontract "$contract_address" arg0 arg1 ...
    ```
    The first argument on the command line (`arg0`) is accessible as `args[0]` in the contract, the second argument (`arg1`) as `args[1]`, and so on.

* `state` (`json&`):<br>
A JSON object that holds the contract’s persistent state.
You can read from and write to it to maintain state across executions.
> **Note on JSON access:**<br>
> The JSON object uses C++ **operator overloading** on the `[]` operator to provide access to its two underlying data structures: **JSON objects**, which are accessed with string keys (e.g., `my_json["key"]`), and **JSON arrays**, which are accessed with numerical indices (e.g., `my_json[0]`). For details on this functionality, see the [API documentation for `operator[]`](https://json.nlohmann.me/api/basic_json/operator%5B%5D/).

**Warning**: Contract transactions are executed in parallel on multiple threads, and access to shared objects within a contract is not thread-safe. If multiple transactions call the same contract simultaneously, you must implement a locking mechanism (like a mutex) to prevent race conditions.

### 2. Writing a Basic Contract

You can use the C++ standard library and helper functions defined inside your own `code.cpp`.

```cpp
#include <string>
#include <vector>
#include "json/json.hpp"
using json = nlohmann::json;

extern "C" int contract_main(const std::vector<std::string>& args, json& state) {
    int counter = state.value("counter", 0);
    state["counter"] = counter + 1;

    if (!args.empty()) {
        state["last_arg"] = args[0];
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

A counter contract with synchronization using `CMonitor` from "[prio_monitor.h](/src/contract/libourcontract/prio_monitor.h)".
```cpp
#include <string>
#include <vector>
#include "json/json.hpp"
#include "contract/libourcontract/prio_monitor.h"
using json = nlohmann::json;

CMonitor<int> monitor;

extern "C" int contract_main(const std::vector<std::string>& args, json& state) 
{
    monitor.acquire(std::stoi(args[0]));

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

// libourcontract.cpp
#include "ourcontract.h"
#include "util.h"   // contains LogPrintf
void print(const char* s) {
    LogPrintf("Contract log: %s\n", s);
}
```
Contract calling it:
```cpp
#include "contract/libourcontract/ourcontract.h"
extern "C" int contract_main(const std::vector<std::string>&, nlohmann::json&) {
    print("hello from contract");
    return 0;
}
```
The output of `print()` is written to `debug.log` when running with `-daemon`,
or to standard output (`stdout`) otherwise.

For more details, see:
* Example contract: [`sample.cpp`](/sample.cpp)
* API header: [`ourcontract.h`](/src/contract/libourcontract/ourcontract.h)
* API implementation: [`libourcontract.cpp`](/src/contract/libourcontract/libourcontract.cpp).

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
bitcoin-cli callcontract [txid] [ARG 0] [ARG 1] ...
```
* Creates a transaction referencing the deployed contract.

* Broadcasts it to the network.

* Once included in a block, nodes execute the `contract_main` function of the contract.

## Development Progress

The following features are still in progress:

### OurCoin Transferring

A contract needs to be able to receive OurCoin and send OurCoin to some addresses. This is not trivial since a contract can not secretly store private keys to hold OurCoin. A possible approach is to design new OPs for both locking scripts and unlocking scripts in a way that only the contract can unlock its own transaction outputs.

### OurContract Code Security

Currently GCC is directly used to compile OurContract source files into dynamic linked libraries. This implementation exposes some issues due to the fact that the OurContract runtime do not have enough control to the behaviors of user-written contracts. For example, an attacker may write a contract that accesses invalid pointers or array elements with invalid indice to cause runtime errors, or executes system calls to control full-nodes. A possible approach is to create our own C++ compiler and linker based on GCC. Our compiler will generate runtime-check code to prevent invalid memory access, and our linker will block all calls to external functions not declared in "ourcontract.h".
