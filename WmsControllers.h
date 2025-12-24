#pragma once
#include "Inventory.h"
#include "Storage.h"
#include "Receipt.h"
#include <string>
#include <queue>
#include <functional>
#include <unordered_map>
#include <vector>

// Task structure for better type safety
struct Task {
    std::string command;
    std::vector<std::string> parameters;
    
    Task(const std::string& cmd, const std::vector<std::string>& params) 
        : command(cmd), parameters(params) {}
};

class WmsControllers {
private:
    Inventory inventory;
    Storage storage;
    std::queue<Task> taskQueue;
    
    // Command mapping for better extensibility
    std::unordered_map<std::string, std::function<bool(const std::vector<std::string>&)>> commandMap;

    // Helper methods
    std::vector<std::string> split(const std::string& s, char delim);
    bool validateNumeric(const std::string& str);
    
    // Command handlers
    bool handleAdd(const std::vector<std::string>& params);
    bool handleRemove(const std::vector<std::string>& params);
    bool handleList(const std::vector<std::string>& params);
    bool handleSearch(const std::vector<std::string>& params);

public:
    explicit WmsControllers(const std::string& storageFilePath);

    // Task queue management
    void enqueueTask(const std::string& task);
    void processTasks();
    
    // Direct command methods (for programmatic use)
    void enqueueAddTask(int id, const std::string& name, int quantity, const std::string& location, bool silent = false);
    void enqueueRemoveTask(int id, bool silent = false);
    void enqueueListTask(bool silent = false);
    void enqueueSearchTask(int id, bool silent = false);

    bool initializeSystem();
    void saveAll();

    // Direct methods for search and list
    Item* searchItemInInventory(int itemId);
    void listInventoryItems();

    // Direct methods for add/remove
    bool addItem(int id, const std::string& name, int quantity, const std::string& location);
    bool removeItem(int id);
};
