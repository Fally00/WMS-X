#pragma once
//needed file inclusion
#include "models/Inventory.h"
#include "storage/Storage.h"
#include "storage/Receipt.h"

//needed libraries
#include <unordered_map>
#include <functional>
#include <optional>
#include <chrono>
#include <queue>

// Task priorities
enum class TaskPriority { LOW = 0, NORMAL = 1, HIGH = 2 };

// Task metadata
struct Task {
    std::string id;
    std::string command;
    std::vector<std::string> params;
    TaskPriority priority;
    int retryCount = 0;
    std::chrono::system_clock::time_point created;

    bool operator<(const Task& other) const {
        return priority < other.priority; // for priority_queue
    }
};

class WmsControllers {
private:
    Storage storage;
    Inventory inventory;
    std::priority_queue<Task> taskQueue;

    std::unordered_map<std::string,
        std::function<bool(const Task&)>> commandRegistry;

    // Helpers
    std::string generateTaskId() const;
    std::vector<std::string> smartSplit(const std::string& input);
    bool isNumeric(const std::string& s);

    // Command handlers
    bool cmdAdd(const Task& t);
    bool cmdRemove(const Task& t);
    bool cmdList(const Task& t);
    bool cmdSearch(const Task& t);

public:
    explicit WmsControllers(const std::string& storagePath);

    bool initializeSystem();
    void saveAll();

    bool addItem(int id, const std::string& name, int qty, const std::string& loc);
    bool removeItem(int id);
    bool updateItem(int id,
                    const std::optional<std::string>& name,
                    const std::optional<int>& qty,
                    const std::optional<std::string>& loc,
                    const std::optional<double>& price);
    bool adjustStock(int id, int delta);
    void listItems(size_t page = 0, size_t pageSize = 10);
    std::optional<Item> getItem(int id);
    std::vector<Item> searchByName(const std::string& query);
    std::vector<Item> getAllItems();

    // Receipt support — expose DB for receipt operations
    SQLite::Database& getDB();

    void enqueueTask(const std::string& raw, TaskPriority prio = TaskPriority::NORMAL);
    void processTasks(size_t limit = 0); // limit=0 → all

    size_t queueSize() const;
};