# OurContract

OurContract is a smart contract module on OurChain. The following explains its concept and how to use it.

## Core Concepts

### State Read and Write

![image](https://github.com/leon123858/OurChain/assets/56253942/f29f5eef-297a-4574-8275-cc3c1ae455ef)

Referring to the figure above, contracts and contracts can make recursive calls, but only the outermost contract can store state or output state. Each contract can read its own current state when called.

- Outputting status to the database is called the `callcontract` operation. This operation will generate a transaction and will be recorded by the blockchain.
- Outputting status to the client is called the `dumpcontractmessage` operation. This operation will not generate a transaction, but will read the transaction records on the blockchain. This kind of operation is called a "pure call".

## How to write a contract

### quick start

See details [sample.cpp](../sample.cpp)

### all available Contract API

See details [ourcontract.h](../src/contract/ourcontract.h)

### how to use rpc command interact with contract

See details [mytest.sh](../mytest.sh)

### compile contract without running OurChain node

It must be installed in a container that can run OurChain smoothly. Assume that the test contract name is `aid.cpp`

```sh
g++ -fPIC -g -c -Wall -o "./aid.o" "./aid.cpp"
g++ -shared -Wl,-soname,"aid.so" -o "./aid.so" "./aid.o" -lssl -lcrypto
rm -f "./aid.o"
```

### Advanced Sample Contract

The following is the suggested OurContract contract syntax. More complex contracts can be organized using the following syntax. 

You can also use the contract API: `recursiveCall` to split different logic into different contracts and call them at the same time.

```cpp
#include <ourcontract.h>
#include <json.hpp>
#include <string>
#include <vector>
#include <map>

using json = nlohmann::json;

enum Command
{
  get,
  registerUser,
  addTask,
  completeTask,
  getTasks
};

static std::unordered_map<std::string, Command> const string2Command = {
    {"get", Command::get},
    {"registerUser", Command::registerUser},
    {"addTask", Command::addTask},
    {"completeTask", Command::completeTask},
    {"getTasks", Command::getTasks}};

/**
 * data structures
 */

struct Task
{
  std::string description;
  bool completed;
};

struct User
{
  std::vector<Task> tasks;
};

struct TodoList
{
  std::map<std::string, User> users;
};

void to_json(json &j, const Task &t)
{
  j = json{{"description", t.description}, {"completed", t.completed}};
}

void from_json(const json &j, Task &t)
{
  j.at("description").get_to(t.description);
  j.at("completed").get_to(t.completed);
}

void to_json(json &j, const User &u)
{
  j = json{{"tasks", u.tasks}};
}

void from_json(const json &j, User &u)
{
  j.at("tasks").get_to(u.tasks);
}

void to_json(json &j, const TodoList &tl)
{
  j = json{{"users", tl.users}};
}

void from_json(const json &j, TodoList &tl)
{
  j.at("users").get_to(tl.users);
}

/**
 * Main
 */
extern "C" int contract_main(void *arg)
{
  ContractArguments *contractArg = (ContractArguments *)arg;
  ContractAPI *api = &contractArg->api;

  // Initialize state if it's empty
  if (api->readContractState() == "null")
  {
    TodoList tl;
    std::string state = json(tl).dump();
    api->writeContractState(&state);
    return 0;
  }

  // Execute command
  if (contractArg->parameters.size() < 1)
  {
    api->contractLog("Argument count error");
    return 0;
  }

  std::string command = contractArg->parameters[0];
  auto eCommand = string2Command.find(command);
  if (eCommand == string2Command.end())
  {
    api->contractLog("Command error");
    return 0;
  }

  switch (eCommand->second)
  {
  case Command::get:
    if (!contractArg->isPureCall)
    {
      api->contractLog("get must be a pure call");
      return 0;
    }
    {
      api->generalContractInterfaceOutput("TodoList", "1.0.0");
      return 0;
    }

  case Command::registerUser:
    if (contractArg->isPureCall)
    {
      api->contractLog("registerUser cannot be a pure call");
      return 0;
    }
    {
      // 1: userId
      TodoList tl = json::parse(api->readContractState());
      if (tl.users.find(contractArg->parameters[1]) != tl.users.end())
      {
        api->contractLog("User already exists");
        return 0;
      }
      User newUser;
      tl.users[contractArg->parameters[1]] = newUser;
      std::string state = json(tl).dump();
      api->writeContractState(&state);
      return 0;
    }

  case Command::addTask:
    if (contractArg->isPureCall)
    {
      api->contractLog("addTask cannot be a pure call");
      return 0;
    }
    {
      // 1: userId, 2: taskDescription
      TodoList tl = json::parse(api->readContractState());
      if (tl.users.find(contractArg->parameters[1]) == tl.users.end())
      {
        api->contractLog("User not found");
        return 0;
      }
      Task newTask{contractArg->parameters[2], false};
      tl.users[contractArg->parameters[1]].tasks.push_back(newTask);
      std::string state = json(tl).dump();
      api->writeContractState(&state);
      return 0;
    }

  case Command::completeTask:
    if (contractArg->isPureCall)
    {
      api->contractLog("completeTask cannot be a pure call");
      return 0;
    }
    {
      // 1: userId, 2: taskIndex
      TodoList tl = json::parse(api->readContractState());
      if (tl.users.find(contractArg->parameters[1]) == tl.users.end())
      {
        api->contractLog("User not found");
        return 0;
      }
      int taskIndex = std::stoi(contractArg->parameters[2]);
      if (taskIndex < 0 || taskIndex >= tl.users[contractArg->parameters[1]].tasks.size())
      {
        api->contractLog("Invalid task index");
        return 0;
      }
      tl.users[contractArg->parameters[1]].tasks[taskIndex].completed = true;
      std::string state = json(tl).dump();
      api->writeContractState(&state);
      return 0;
    }

  case Command::getTasks:
    if (!contractArg->isPureCall)
    {
      api->contractLog("getTasks must be a pure call");
      return 0;
    }
    {
      // 1: userId
      TodoList tl = json::parse(api->readContractState());
      if (tl.users.find(contractArg->parameters[1]) == tl.users.end())
      {
        api->contractLog("User not found");
        return 0;
      }
      User user = tl.users[contractArg->parameters[1]];
      std::string tasks = json(user.tasks).dump();
      api->writeContractState(&tasks);
      return 0;
    }

  default:
    return 0;
  }
  return 0;
}
```
