/**
 * MODULE:      ourcontract.h
 * COMPONENT:   API/Core
 * PROJECT:     OurChain
 * DESCRIPTION:
 * Defines the public C-style API that smart contracts can use to
 * interact with the underlying OurChain node.
 *
 * These functions are exported from the node's symbol table and are
 * the only way for a sandboxed contract to call back into the node
 * for services like logging or inter-contract communication.
 *
 * Notes:
 * - All functions are marked with `PUBLIC` to ensure they are dynamically
 * exported for use by contract shared libraries (.so files).
 * - The `extern "C"` block prevents C++ name mangling, providing a
 * stable Application Binary Interface (ABI).
 */
#ifndef OURCONTRACT_H
#define OURCONTRACT_H

#include <string>
#include <vector>

#define PUBLIC __attribute__((visibility("default")))

extern "C" {
    
PUBLIC bool call_contract(char *contract_id, char *func_name, const std::vector<std::string> &vec);

PUBLIC void print(const char *s);

}
#endif // OURCONTRACT_H