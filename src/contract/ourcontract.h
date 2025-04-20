#ifndef OURCONTRACT_H
#define OURCONTRACT_H

#include <functional>
#include <mutex>
#include <stack>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <stdexcept>
#include "util.h"

#define PUBLIC __attribute__((visibility("default")))

extern "C" {
    
PUBLIC bool call_contract(char *contract_id, char *func_name, const std::vector<std::string> &vec);

PUBLIC void print(const char *s);

}
#endif // OURCONTRACT_H