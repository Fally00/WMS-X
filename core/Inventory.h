#pragma once
#include "Item.h"
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

class Inventory {
private:
    std::unordered_map<int, Item> items;   // ID -> Item for O(1) lookup
    std::string dataFilePath;

public:
    Inventory(const std::string &filePath);

    // CRUD
    bool addItem(const Item &item);               // returns false if duplicate
    bool removeItem(int itemId);                  // returns false if not found
    Item* findItem(int itemId);                   // returns nullptr if not found

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

    // CSV
    void fromCSV(const std::string &csvData);
    std::string toCSV() const;

    // Stats
    size_t totalItems() const { return items.size(); }
    int totalQuantity() const;

    // Access raw items (for advanced use)
    std::vector<Item> getAllItems() const;
};
