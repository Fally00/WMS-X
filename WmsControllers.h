#pragma once
#include "Inventory.h"
#include "Storage.h"
#include "Receipt.h"
#include <string>
#include <queue>

class WmsControllers {
private:
    Inventory inventory;
    Storage storage;
    std::queue<std::string> taskQueue;

public:
    explicit WmsControllers(const std::string& storageFilePath);

    void enqueueTask(const std::string& task);
    void processTasks();

    bool initializeSystem();
    void saveAll();

    bool addNew(int id, const std::string& name, int quantity, const std::string& location);
    bool removeItem(int itemId);
    Item* searchItemInInventory(int itemId);
    void listInventoryItems();
};
