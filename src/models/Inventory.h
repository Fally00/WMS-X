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

    // Persist single item changes to SQLite
    void saveItem(const Item& item);
    void updateItemInDB(const Item& item);

    // Batch operations
    void addMultiple(const std::vector<Item> &batch);
    void removeMultiple(const std::vector<int> &ids);

    // Display & Queries
    void displayItems(size_t page = 0, size_t pageSize = 10) const;
    std::vector<Item> filterByLocation(const std::string &loc) const;
    std::vector<Item> filterByQuantity(int minQty, int maxQty) const;
    std::vector<Item> searchByName(const std::string &query) const;

    // Sorting (return sorted copies)
    std::vector<Item> sortByID(bool ascending = true) const;
    std::vector<Item> sortByName(bool ascending = true) const;
    std::vector<Item> sortByQuantity(bool ascending = true) const;
    std::vector<Item> sortByLocation(bool ascending = true) const;

    // Stats
    size_t totalItems() const { return items.size(); }
    int totalQuantity() const;

    // Access raw items (for advanced use)
    std::vector<Item> getAllItems() const;
};
