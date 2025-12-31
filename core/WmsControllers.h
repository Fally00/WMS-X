#pragma once
#include "core/Inventory.h"
#include "core/Storage.h"
#include "core/Receipt.h"
#include <queue>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

// Task structure for better type safety
struct Task {
    std::string command;
    std::vector<std::string> parameters;
    
    Task(const std::string& cmd, const std::vector<std::string>& params) 
        : command(cmd), parameters(params) {}
};

/**
 * @class WmsControllers
 * @brief Main controller class for the Warehouse Management System.
 *
 * The WmsControllers class orchestrates all warehouse operations, including
 * inventory management, task queuing, and data persistence. It provides both
 * direct method calls and queued task execution for flexible operation handling.
 */
class WmsControllers {
private:
    Inventory inventory; /**< The inventory management component. */
    Storage storage; /**< The data storage component for persistence. */
    std::queue<Task> taskQueue; /**< Queue for storing pending tasks. */

    // Command mapping for better extensibility
    std::unordered_map<std::string, std::function<bool(const std::vector<std::string>&)>> commandMap; /**< Maps command strings to handler functions. */

    // Helper methods
    /**
     * @brief Splits a string by a delimiter.
     * @param s The string to split.
     * @param delim The delimiter character.
     * @return A vector of split strings.
     */
    std::vector<std::string> split(const std::string& s, char delim);

    /**
     * @brief Validates if a string represents a numeric value.
     * @param str The string to validate.
     * @return True if the string is numeric, false otherwise.
     */
    bool validateNumeric(const std::string& str);

    // Command handlers
    /**
     * @brief Handles the add item command.
     * @param params The parameters for the add operation.
     * @return True if successful, false otherwise.
     */
    bool handleAdd(const std::vector<std::string>& params);

    /**
     * @brief Handles the remove item command.
     * @param params The parameters for the remove operation.
     * @return True if successful, false otherwise.
     */
    bool handleRemove(const std::vector<std::string>& params);

    /**
     * @brief Handles the list items command.
     * @param params The parameters for the list operation.
     * @return True if successful, false otherwise.
     */
    bool handleList(const std::vector<std::string>& params);

    /**
     * @brief Handles the search item command.
     * @param params The parameters for the search operation.
     * @return True if successful, false otherwise.
     */
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
