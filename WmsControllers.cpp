#include "WmsControllers.h"
#include "Inventory.h"
#include "Storage.h"
#include "Receipt.h"
#include "Item.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;


/*==========================================
    INITIALIZATION + SAVE SYSTEM
============================================*/

bool WmsControllers::initializeSystem() {
    if (!storage.intializeStorage()) {
        cerr << "[ERROR] Could not initialize storage file!" << endl;
        return false;
    }

    const string csvData = storage.readAll();

    if (!csvData.empty()) {
        inventory.fromCSV(csvData);
        cout << "[SYSTEM READY] Inventory loaded from storage." << endl;
    } else {
        cout << "[SYSTEM READY] Storage empty. Fresh inventory initialized." << endl;
    }

    return true;
}
void WmsControllers::saveAll() {
    const string csvData = inventory.toCSV(storage.getFilePath());
    if (storage.writeAll(csvData)) {
        cout << "[SAVED] Inventory data saved to storage." << endl;
    } else {
        cerr << "[ERROR] Failed to save inventory data!" << endl;
    }
}

Item* WmsControllers::searchItemInInventory(int itemId) {
    return inventory.findItem(itemId);
}

void WmsControllers::listInventoryItems() {
    inventory.displayItems();
}

bool WmsControllers::addItem(int id, const std::string& name, int quantity, const std::string& location) {
    // Check if item already exists
    if (inventory.findItem(id)) {
        return false;
    }

    if (quantity < 0) {
        return false;
    }

    inventory.addItem({id, name, quantity, location});
    return true;
}

bool WmsControllers::removeItem(int id) {
    if (!inventory.findItem(id)) {
        return false;
    }

    inventory.removeItem(id);
    return true;
}



/*==========================================
        ENHANCED QUEUE SYSTEM 2.1
==========================================*/

WmsControllers::WmsControllers(const std::string& storageFilePath) 
    : inventory(storageFilePath), storage(storageFilePath) {
    // Initialize command mapping
    commandMap["ADD"] = [this](const std::vector<std::string>& params) { 
        return this->handleAdd(params); 
    };
    commandMap["REMOVE"] = [this](const std::vector<std::string>& params) { 
        return this->handleRemove(params); 
    };
    commandMap["LIST"] = [this](const std::vector<std::string>& params) { 
        return this->handleList(params); 
    };
    commandMap["SEARCH"] = [this](const std::vector<std::string>& params) { 
        return this->handleSearch(params); 
    };
}

void WmsControllers::enqueueTask(const std::string& task) {
    std::vector<std::string> tokens = split(task, ' ');
    if (!tokens.empty()) {
        std::string command = tokens[0];
        std::vector<std::string> params(tokens.begin() + 1, tokens.end());
        taskQueue.push(Task(command, params));
        std::cout << "[QUEUED] " << task << std::endl;
    }
}

// Convenience methods for specific tasks
void WmsControllers::enqueueAddTask(int id, const std::string& name, 
                                   int quantity, const std::string& location, bool silent) {
    std::vector<std::string> params = {std::to_string(id), name, 
                                      std::to_string(quantity), location};
    taskQueue.push(Task("ADD", params));
    if (!silent) std::cout << "[QUEUED] ADD " << id << " " << name << " " << quantity << " " << location << std::endl;
}

void WmsControllers::enqueueRemoveTask(int id, bool silent) {
    std::vector<std::string> params = {std::to_string(id)};
    taskQueue.push(Task("REMOVE", params));
    if (!silent) std::cout << "[QUEUED] REMOVE " << id << std::endl;
}

void WmsControllers::enqueueListTask(bool silent) {
    taskQueue.push(Task("LIST", {}));
    if (!silent) std::cout << "[QUEUED] LIST" << std::endl;
}

void WmsControllers::enqueueSearchTask(int id, bool silent) {
    std::vector<std::string> params = {std::to_string(id)};
    taskQueue.push(Task("SEARCH", params));
    if (!silent) std::cout << "[QUEUED] SEARCH " << id << std::endl;
}

// ------------------------------------------------------------
// Helper: split string by delimiter (improved)
// ------------------------------------------------------------
std::vector<std::string> WmsControllers::split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::string temp;
    std::stringstream ss(s);

    while (std::getline(ss, temp, delim)) {
        if (!temp.empty()) {
            // Trim whitespace
            size_t start = temp.find_first_not_of(" \t");
            size_t end = temp.find_last_not_of(" \t");
            if (start != std::string::npos) {
                parts.push_back(temp.substr(start, end - start + 1));
            } else {
                parts.push_back(temp);
            }
        }
    }
    return parts;
}

// ------------------------------------------------------------
// Validation helper
// ------------------------------------------------------------
bool WmsControllers::validateNumeric(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    
    for (size_t i = start; i < str.length(); ++i) {
        if (!std::isdigit(str[i])) return false;
    }
    return true;
}

// ------------------------------------------------------------
// Command handlers
// ------------------------------------------------------------
bool WmsControllers::handleAdd(const std::vector<std::string>& params) {
    if (params.size() != 4) {
        std::cout << "[FAILED] Usage: ADD <id> <name> <qty> <location>\n";
        return false;
    }

    if (!validateNumeric(params[0]) || !validateNumeric(params[2])) {
        std::cout << "[FAILED] Invalid numeric value\n";
        return false;
    }

    try {
        int id = std::stoi(params[0]);
        int quantity = std::stoi(params[2]);
        
        if (quantity < 0) {
            std::cout << "[FAILED] Quantity cannot be negative\n";
            return false;
        }

        std::string name = params[1];
        std::string location = params[3];

        // Check if item already exists
        if (inventory.findItem(id)) {
            std::cout << "[FAILED] Item with ID " << id << " already exists\n";
            return false;
        }

        inventory.addItem({id, name, quantity, location});
        std::cout << "[DONE] Added : " << id << " (" << name << ")\n";
        return true;
        
    } catch (const std::exception& e) {
        std::cout << "[FAILED] Invalid numeric value: " << e.what() << "\n";
        return false;
    }
}

bool WmsControllers::handleRemove(const std::vector<std::string>& params) {
    if (params.size() != 1) {
        std::cout << "[FAILED] Usage: REMOVE <id>\n";
        return false;
    }

    if (!validateNumeric(params[0])) {
        std::cout << "[FAILED] Invalid ID\n";
        return false;
    }

    try {
        int id = std::stoi(params[0]);
        
        if (inventory.findItem(id)) {
            inventory.removeItem(id);
            std::cout << "[DONE] Removed :" << id << "\n";
            return true;
        } else {
            std::cout << "[FAILED] Item not found: " << id << "\n";
            return false;
        }
    } catch (const std::exception& e) {
        std::cout << "[FAILED] Invalid ID: " << e.what() << "\n";
        return false;
    }
}

bool WmsControllers::handleList(const std::vector<std::string>& params) {
    if (!params.empty()) {
        std::cout << "[FAILED] Usage: LIST (no parameters needed)\n";
        return false;
    }
    
    inventory.displayItems();
    return true;
}

bool WmsControllers::handleSearch(const std::vector<std::string>& params) {
    if (params.size() != 1) {
        std::cout << "[FAILED] Usage: SEARCH <id>\n";
        return false;
    }

    if (!validateNumeric(params[0])) {
        std::cout << "[FAILED] Invalid ID\n";
        return false;
    }

    try {
        int id = std::stoi(params[0]);
        Item* item = inventory.findItem(id);
        
        if (item) {
            std::cout << "\n[FOUND]\n";
            printItem(*item);
            return true;
        } else {
            std::cout << "[NOT FOUND] Item " << id << "\n";
            return false;
        }
    } catch (const std::exception& e) {
        std::cout << "[FAILED] Invalid ID: " << e.what() << "\n";
        return false;
    }
}

// ------------------------------------------------------------
// Process all tasks in queue
// ------------------------------------------------------------
void WmsControllers::processTasks() {
    std::cout << "\n========== PROCESSING TASK QUEUE ==========\n";
    
    if (taskQueue.empty()) {
        std::cout << "[INFO] No tasks to process\n";
        return;
    }

    int processedCount = 0;
    int failedCount = 0;

    while (!taskQueue.empty()) {
        Task task = taskQueue.front();
        taskQueue.pop();

        std::cout << ">> " << task.command;
        for (const auto& param : task.parameters) {
            std::cout << " " << param;
        }
        std::cout << std::endl;

        auto it = commandMap.find(task.command);
        if (it != commandMap.end()) {
            if (it->second(task.parameters)) {
                processedCount++;
            } else {
                failedCount++;
            }
        } else {
            std::cout << "[FAILED] Unknown command: " << task.command << "\n";
            failedCount++;
        }
    }

    std::cout << "\n========== TASK PROCESSING COMPLETE ==========\n";
    std::cout << "Processed: " << processedCount << " | Failed: " << failedCount << std::endl;
}