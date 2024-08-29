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
      string state = api->readContractState();
      json j = json::parse(state);
      contractArg->mtx->lock();
      j.push_back("pure click: " + std::to_string((size_t)j.size()));
      contractArg->mtx->unlock();
      // pure output
      state = j.dump();
      api->writeContractState(&state);
      return 0;
    }
  }
  // non-pure call contract
  string state = api->readContractState();
  // deploy contract init call
  if (state == "null")
  {
    json j;
    j.push_back("baby cute");
    j.push_back(1);
    j.push_back(true);
    // write contract state
    state = j.dump();
    api->writeContractState(&state);
    return 0;
  }
  json j = json::parse(state);
  j.push_back("more click: " + std::to_string((size_t)j.size()));
  // write contract state
  state = j.dump();
  api->writeContractState(&state);
  // log pre txid
  api->contractLog("pre txid: " + contractArg->preTxid);
  return 0;
}
