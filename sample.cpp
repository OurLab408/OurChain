#include <string>
#include "json/json.hpp"
#include "contract/libourcontract/ourcontract.h"

using json = nlohmann::json;

// contract main function
extern "C" int contract_main(const std::vector<std::string> args, json state)
{
    print("Hello, World");
    return 0;
}