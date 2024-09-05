# OurContract

OurContract 是在 OurChain 上的智能合约模塊。以下說明其概念和如何使用。

## 核心概念

### 狀態的讀取與存入

![image](https://github.com/leon123858/OurChain/assets/56253942/f29f5eef-297a-4574-8275-cc3c1ae455ef)

參考上圖, 合約和合約可以遞迴呼叫, 但是只有最外層的合約可以存入狀態或輸出狀態。每個合約被呼叫時都可以讀取屬於自己的當前狀態。

- 輸出狀態到資料庫被稱為 `callcontract` 操作, 這個操作會產生一筆交易, 並且會被區塊鏈記錄下來。
- 輸出狀態到用戶端被稱為 `dumpcontractmessage` 操作, 這個操作不會產生交易, 但是會讀取區塊鏈上的交易記錄。

### 合約狀態變化策略

描述合約模塊如何依據區塊鏈的變化改變狀態。合約模塊會比較當前鏈的狀態和合約當前維護狀態取用不同的更新策略。
詳見 [updateStrategy](../src/contract/updateStrategy.h)

## 合約撰寫

### 快速開始

請看 [sample.cpp](../sample.cpp)

### 所有方法

請看 [ourcontract.h](../src/contract/ourcontract.h)

### 實際rpc指令調用方法

請看 [mytest.sh](../mytest.sh)

### 線下合約編譯測試

須在已安裝, 且可順利運行 OurChain 的容器內, 假設測試合約名稱為 `aid.cpp`

```sh
g++ -fPIC -g -c -Wall -o "./aid.o" "./aid.cpp"
g++ -shared -Wl,-soname,"aid.so" -o "./aid.so" "./aid.o" -lssl -lcrypto
rm -f "./aid.o"
```

### 進階範例合約

以下為建議 OurContract 合約語法，利用以下語法可以組織出較為複雜的合約。更可以利用合約 API:`recursiveCall` 來拆分不同的邏輯到不同的合約中，並且同時呼叫。

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
