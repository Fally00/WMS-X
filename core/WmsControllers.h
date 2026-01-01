#pragma once
#include "Inventory.h"
#include "Storage.h"
#include "Receipt.h"
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
    Inventory inventory;
    Storage storage;
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

    void enqueueTask(const std::string& raw, TaskPriority prio = TaskPriority::NORMAL);
    void processTasks(size_t limit = 0); // limit=0 → all

    size_t queueSize() const;
};
