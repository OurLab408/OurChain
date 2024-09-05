Ourcontract
===========

Ourcontract is the smart contract platform built on Ourcoin. This documentation gives a brief overview on Ourcontract programming, testing, and execution.

Installation
------------

Ourcontract is built along with Ourcoin. If you have already finished installing Ourcoin, everything should be ready.

Here is an additional note you may find useful. If you choose to install Ourcoin into a custom path (assume OURCOIN_DIR), be sure to set the following environment variables:

```
export PATH=$PATH:$OURCOIN_DIR/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OURCOIN_DIR/lib
export LIBRARY_PATH=$LIBRARY_PATH:$OURCOIN_DIR/lib
export CPATH=$CPATH:$OURCOIN_DIR/include
```

This is because to compile and execute a user-written Ourcontract source file, "ourcontract.h" in "$OURCOIN_DIR/include" is needed at compile-time, and "libourcontract.so" in "$OURCOIN_DIR/lib" is needed at both compile-time and run-time.

Ourcontract Programming
-----------------------

Ourcontract uses C as its programming language, with some additional constrants:

1. Entry point should be contract_main(int argc, char **argv).

2. External functions are not allowed, except for those declared in "ourcontract.h"

For more details, see the example contract "[vote.c](/example/vote.c)" and the API header "[ourcontract.h](/src/contract/ourcontract.h)"

Off-chain Testing
-----------------

There are two utilities in "$OURCOIN_DIR/bin" that help you compile and execute your contract. "ourcontract-mkdll" takes a given contract source file and compiles it into a dynamic linked library. "ourcontract-rt" executes the contract_main function of a given contract dynamic linked library.

To perform off-chain testing, you need to invoke these two utilities by yourself, so it is necessary to follow some directory and filename conventions. Assume you have two contract source files, "hello.c" and "vote.c". Run the following commands to organize the directory structure:

```
$ mkdir contracts
$ mkdir contracts/hello
$ mkdir contracts/vote
$ mv hello.c contracts/hello/code.cpp
$ mv vote.c contracts/vote/code.cpp
```

The directory structure will be like this:

```
contracts
|
+---hello
|   |
|   +---code.cpp
|
+---vote
|   |
|   +---code.cpp
|
|---(another contract)
.
.
.
```

Assume the directory "contracts" is in your current directory, and you want to test the contract "vote", the following commands show how to compile and run it.

```
$ ourcontract-mkdll contracts vote
$ ourcontract-rt contracts vote [ARG 1] [ARG 2] ...
```

ARG x will be fed into argv[]. In the above example, argv\[0\] will be "vote", argv\[1\] will be \[ARG 1\], and so on.

Contract output message will be written to "contracts/vote/out". Debug message will be written to stderr.

Running on the Blockchain
-------------------------

In this case, you don't touch the details described in the last section. Instead, just use Ourcoin RPCs to submit "vote.c", call it, and examine its output message.

### Deploy the contract
```
bitcoin-cli deploycontract vote.c [ARG 1] [ARG 2] ...
```

This RPC creates a transaction, with the content of "vote.c" and all ARG x in it, and broadcast it. Once the transaction is included in the blockchain, full-nodes will automatically invoke "ourcontract-mkdll" to compile "vote.c" and "ourcontract-rt" to execute its contract_main for the first time immediately.

The hash returned by this RPC is the txid of this transaction. This txid is needed to reference the contract in the future.

### Call the contract
```
bitcoin-cli callcontract [txid] [ARG 1] [ARG 2] ...
```

This RPC creates a transaction, with the txid of the called contract and all ARG x in it, and broadcast it. Once the transaction is included in the blockchain, full-nodes will automatically invoke "ourcontract-rt" to execute the contract_main of the called contract.

### Dump the output message of the contract
```
bitcoin-cli dumpcontractmessage [txid]
```

This RPC is an off-chain operation. It just returns the content of "contracts/\[txid\]/out".

Development Progress
--------------------

The following features are still in progress:

### Blockchain Reorganization Handling

Each block should record the hash of the states of all contracts (i.e. all directories and files in "contracts") at that time. When a shorter chain outpaces the original longest chain, the states of all contracts needs to be reverted to the states at the fork point block, and then re-transition to the tip block of the new longest chain.

### Ourcoin Transferring

A contract needs to be able to receive Ourcoin and send Ourcoin to some addresses. This is not trivial since a contract can not secretly store private keys to hold Ourcoin. A possible approach is to design new OPs for both locking scripts and unlocking scripts in a way that only the contract can unlock its own transaction outputs.

### Ourcontract Code Security

Currently GCC is directly used to compile Ourcontract source files into dynamic linked libraries. This implementation exposes some issues due to the fact that the Ourcontract runtime (i.e. ourcontract-rt) do not have enough control to the behaviors of user-written contracts. For example, an attacker may write a contract that accesses invalid pointers or array elements with invalid indice to cause runtime errors, or executes system calls to control full-nodes. A possible approach is to create our own C compiler and linker based on GCC. Our compiler will generate runtime-check code to prevent invalid memory access, and our linker will block all calls to external functions not declared in "ourcontract.h".
