#include <string>
#include <mutex>
#include "json/json.hpp"
#include "contract/lib/ourcontract.h"

using json = nlohmann::json;

std::mutex contract_mutex;

// Perfect approach: Contract manages its own state reference
extern "C" int contract_main(const std::vector<std::string> args)
{
    std::lock_guard<std::mutex> lock(contract_mutex);
    
    // 2. Get operation type from args (C-style: args[0]=contract_name, args[1]=operation, args[2]=amount)
    std::string contract_name = args[0];
    std::string operation = (args.size() >= 2) ? args[1] : "increment";
    int amount = (args.size() >= 3) ? std::stoi(args[2]) : 1;
    
    // 3. Get state reference - contract gets its own state buffer using contract address
    json& state_ref = get_state(contract_name);
    
    // 4. Initialize state if empty
    if (state_ref.is_null() || state_ref.empty()) {
        state_ref = json::object();
        state_ref["counter"] = 0;
        state_ref["operations"] = json::array();
    }
    
    // 5. Modify state by reference - clean and simple!
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
    
    // 6. Unlock - automatic when lock goes out of scope
    return 0;
}