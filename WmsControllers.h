#ifndef WMSCONTROLLERS_H
#define WMSCONTROLLERS_H
#include "Inventory.h"
#include "Storage.h"
#include <string>
#include <queue>

class WmsControllers {
private:
    Inventory inventory;
    Storage storage;
    std::queue<std::string> taskQueue;
public:
    WmsControllers(const std::string& storageFilePath)
        : inventory(storageFilePath), storage(storageFilePath) {}

    void enqueueTask(const std::string& task);
    void processTasks();

    bool initializeSystem();
    void saveAll();
    // returns false if an item with the same id already exists
    bool addNew(const std::string &id, const std::string &name, int quantity, const std::string &location);
    bool removeItem(int itemId);
    Item* searchItemInInventory(int itemId);
    void listInventoryItems();
};

#endif