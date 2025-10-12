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

#include "json/json.hpp"
#include <string>
#include <vector>

using json = nlohmann::json;

#define PUBLIC __attribute__((visibility("default")))

extern "C" {
PUBLIC void print(const char* s);
PUBLIC json& get_state(const std::string& contract_address);
}

// C++ linkage functions (outside extern "C")
extern "C++" {
PUBLIC void print(const std::string& s);
}
#endif // OURCONTRACT_H
