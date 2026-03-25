#pragma once

//Included file
#include "models/Item.h"
#include "storage/Storage.h"

//Needed libraries 
#include <unordered_map>
#include <string>
#include <vector>

class Inventory {
private:
    std::unordered_map<int, Item> items;   // ID -> Item for O(1) lookup
    SQLite::Database& db;

public:
    Inventory(SQLite::Database& database);

    // Load all items from SQLite into memory
    void loadAll();

    // CRUD
    bool addItem(const Item &item);               // returns false if duplicate
    bool removeItem(int itemId);                  // returns false if not found
    Item* findItem(int itemId);                   // returns nullptr if not found
    Item* findByBarcode(const std::string& barcode); // nullptr if empty or not found

    // Persist single item changes to SQLite
    void saveItem(const Item& item);
    void updateItemInDB(const Item& item);

    // Display & Queries
    void displayItems(size_t page = 0, size_t pageSize = 10) const;
    std::vector<Item> searchByName(const std::string &query) const;

    // Access all items
    std::vector<Item> getAllItems() const;
};
