#include "ourcontract.h"
#include <functional>
#include <mutex>
#include <stack>
#include <dlfcn.h>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <chrono>
#include <ctime>
#include "../../util.h"
#include "../db/contractdb.h"

void print(const char *s) {
    std::stringstream ss;
    ss << "[CONTRACT] " << s;
    
    // Write to dedicated contract.log file
    static std::mutex log_mutex;
    std::lock_guard<std::mutex> lock(log_mutex);
    
    fs::path contract_log_path = GetDataDir() / "contracts" / "contract.log";
    std::ofstream log_file(contract_log_path, std::ios::app);
    
    if (log_file.is_open()) {
        // Add timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        char timestamp[64];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
        
        log_file << "[" << timestamp << "] " << ss.str() << std::endl;
        log_file.close();
    }
}

// Overload for std::string - just calls the const char* version
void print(const std::string& s) {
    print(s.c_str());
}

json& get_state(const std::string& contract_address) {
    if (contract_address.empty()) {
        throw std::runtime_error("get_state() called with empty contract address");
    }
    
    uint256 addr = uint256S(contract_address);
    auto& cache = ContractDB::getInstance();
    return cache.getState(addr);
}

