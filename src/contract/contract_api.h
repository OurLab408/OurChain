#ifndef  OURCONTRACT_API_H
#define OURCONTRACT_API_H

#include <vector>
#include <string>

extern "C" {
void print(const char *s);
bool call_contract(char *contract_id, char *func_name, const std::vector<std::string> &vec);
}

#endif //OURCONTRACT_API_H