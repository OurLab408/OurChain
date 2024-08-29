#include <iostream>
#include <json.hpp>
#include "ourcontract.h"

using json = nlohmann::json;
using namespace std;

// contract main function
extern "C" int contract_main(void *arg)
{
  // cast argument
  ContractArguments *contractArg = (ContractArguments *)arg;
  ContractAPI *api = &contractArg->api;
  // pure call contract
  if (contractArg->isPureCall)
  {
    string command = contractArg->parameters[0];
    api->contractLog("command: " + command);
    if (command == "get")
    {
      api->generalContractInterfaceOutput("example", "0.1.0");
      return 0;
    }
    else
    {
      // pure operation
      return 0;
    }
  }
  // non-pure call contract
  string state = api->readContractState();
  // deploy contract init call
  if (state == "null")
  {
    json j = json::object();
    j["click"] = 0;
    // write contract state
    state = j.dump();
    api->writeContractState(&state);
    return 0;
  }
  json j = json::parse(state);
  j["click"] = j["click"].get<int>() + 1;
  state = j.dump();
  api->writeContractState(&state);
  return 0;
}
