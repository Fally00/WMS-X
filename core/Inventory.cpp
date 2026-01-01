#include "Inventory.h"
#include "output.h"
#include <algorithm>
#include <iostream>
#include <sstream>

Inventory::Inventory(const std::string &filePath)
    : dataFilePath(filePath) {}

// -----------------------------
// Add / Remove
// -----------------------------
bool Inventory::addItem(const Item &item) {
    if (items.count(item.getId()) > 0) return false;
    items[item.getId()] = item;
    return true;
}

bool Inventory::removeItem(int itemId) {
    return items.erase(itemId) > 0;
}

// Batch operations
void Inventory::addMultiple(const std::vector<Item> &batch) {
    for (const auto &item : batch) addItem(item);
}

void Inventory::removeMultiple(const std::vector<int> &ids) {
    for (int id : ids) removeItem(id);
}

// -----------------------------
// Lookup / Search / find 
// -----------------------------
Item* Inventory::findItem(int itemId) {
    auto it = items.find(itemId);
    if (it != items.end()) return &it->second;
    return nullptr;
}

std::vector<Item> Inventory::searchByName(const std::string &query) const {
    std::vector<Item> results;
    for (const auto &[id, item] : items) {
        if (item.getName().find(query) != std::string::npos) {
            results.push_back(item);
        }
    }
    return results;
}

// Filtering
std::vector<Item> Inventory::filterByLocation(const std::string &loc) const {
    std::vector<Item> results;
    for (const auto &[id, item] : items) {
        if (item.getLocation() == loc) results.push_back(item);
    }
    return results;
}

std::vector<Item> Inventory::filterByQuantity(int minQty, int maxQty) const {
    std::vector<Item> results;
    for (const auto &[id, item] : items) {
        int q = item.getQuantity();
        if (q >= minQty && q <= maxQty) results.push_back(item);
    }
    return results;
}

// -----------------------------
// Display all items
// -----------------------------
void Inventory::displayItems(size_t page, size_t pageSize) const {
    std::vector<Item> allItems;
    for (const auto &[id, item] : items) allItems.push_back(item);

    if (allItems.empty()) {
        OutputFormatter::printWarning("No items in inventory");
        return;
    }

    size_t start = page * pageSize;
    size_t end = std::min(start + pageSize, allItems.size());

    if (start >= allItems.size()) {
        OutputFormatter::printWarning("Page out of range");
        return;
    }

    std::vector<std::string> headers = {"ID", "Name", "Quantity", "Location"};
    std::vector<std::vector<std::string>> rows;

    for (size_t i = start; i < end; ++i) {
        const Item &item = allItems[i];
        rows.push_back({
            std::to_string(item.getId()),
            item.getName(),
            std::to_string(item.getQuantity()),
            item.getLocation()
        });
    }

    OutputFormatter::printTable(headers, rows);
}

// -----------------------------
// Sorting
// -----------------------------
std::vector<Item> Inventory::sortByID(bool ascending) const {
    std::vector<Item> sorted = getAllItems();
    std::sort(sorted.begin(), sorted.end(),
        [ascending](const Item &a, const Item &b) {
            return ascending ? a.getId() < b.getId() : a.getId() > b.getId();
        });
    return sorted;
}

std::vector<Item> Inventory::sortByName(bool ascending) const {
    std::vector<Item> sorted = getAllItems();
    std::sort(sorted.begin(), sorted.end(),
        [ascending](const Item &a, const Item &b) {
            return ascending ? a.getName() < b.getName() : a.getName() > b.getName();
        });
    return sorted;
}

std::vector<Item> Inventory::sortByQuantity(bool ascending) const {
    std::vector<Item> sorted = getAllItems();
    std::sort(sorted.begin(), sorted.end(),
        [ascending](const Item &a, const Item &b) {
            return ascending ? a.getQuantity() < b.getQuantity() : a.getQuantity() > b.getQuantity();
        });
    return sorted;
}

std::vector<Item> Inventory::sortByLocation(bool ascending) const {
    std::vector<Item> sorted = getAllItems();
    std::sort(sorted.begin(), sorted.end(),
        [ascending](const Item &a, const Item &b) {
            return ascending ? a.getLocation() < b.getLocation() : a.getLocation() > b.getLocation();
        });
    return sorted;
}

// -----------------------------
// Stats
// -----------------------------
int Inventory::totalQuantity() const {
    int total = 0;
    for (const auto &[id, item] : items) total += item.getQuantity();
    return total;
}

// -----------------------------
// CSV
// -----------------------------
void Inventory::fromCSV(const std::string &csvData) {
    std::stringstream ss(csvData);
    std::string line;
    bool firstLine = true;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        // Skip header
        if (firstLine && line.find("ID") != std::string::npos) {
            firstLine = false;
            continue;
        }
        firstLine = false;
        addItem(Item::fromCSV(line));
    }
}

std::string Inventory::toCSV() const {
    std::stringstream ss;
    ss << "ID,Name,Quantity,Location\n"; // header
    for (const auto &[id, item] : items) {
        ss << item.toCSV() << "\n";
    }
    return ss.str();
}

// -----------------------------
// Helpers
// -----------------------------
std::vector<Item> Inventory::getAllItems() const {
    std::vector<Item> all;
    for (const auto &[id, item] : items) all.push_back(item);
    return all;
}
