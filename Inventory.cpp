#include "Inventory.h"
#include "Item.h"
#include "output.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
Inventory::Inventory(const std::string &filePath)
    : dataFilePath(filePath) {}

// ─────────────────────────────────────────────
// Load data from CSV string (Updated)
// ─────────────────────────────────────────────
void Inventory::fromCSV(const std::string &csvData) {
    std::stringstream ss(csvData);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        items.push_back(Item::fromCSV(line));
    }
}



// ─────────────────────────────────────────────
// Export inventory to CSV
// ─────────────────────────────────────────────
string Inventory::toCSV(const string& /*filePath*/) const {
    stringstream ss;
    for ( const auto &item : items ){
        ss << item.toCSV() << "\n";
    }
    return ss.str();
}


// ─────────────────────────────────────────────
// Display all items
// ─────────────────────────────────────────────
void Inventory::displayItems() const {
    if (items.empty()) {
        OutputFormatter::printWarning("No items in inventory");
        return;
    }

    std::vector<std::string> headers = {"ID", "Name", "Quantity", "Location"};
    std::vector<std::vector<std::string>> rows;

    for (const auto &item : items) {
        rows.push_back({
            std::to_string(item.getId()),
            item.getName(),
            std::to_string(item.getQuantity()),
            item.getLocation()
        });
    }

    OutputFormatter::printTable(headers, rows);
}



// ─────────────────────────────────────────────
// Add new item to inventory
// ─────────────────────────────────────────────
void Inventory::addItem(const Item &item) {
    items.push_back(item);
}


// ─────────────────────────────────────────────
// Find item by ID (updated linear search)
// ─────────────────────────────────────────────
Item* Inventory::findItem(int itemId) {
    for (auto &item : items) {
        if (item.getId() == itemId)
            return &item;
    }
    return nullptr;
}



// ─────────────────────────────────────────────
// Remove item by ID (updated linear search)
// ─────────────────────────────────────────────
void Inventory::removeItem(int itemId) {
   auto it = std::find_if(items.begin(), items.end(),
    [&](const Item &item) { return item.getId() == itemId; });

    if (it != items.end())
        items.erase(it);
}


// ─────────────────────────────────────────────
// Sort by ID
// ─────────────────────────────────────────────
void Inventory::sortByID() {
    std::sort(items.begin(), items.end(),
        [](const Item &a, const Item &b) {
            return a.getId() < b.getId();
        });
}

// ─────────────────────────────────────────────
// Sort by name
// ─────────────────────────────────────────────
void Inventory::sortByName() {
    std::sort(items.begin(), items.end(),
        [](const Item &a, const Item &b) {
            return a.getName() < b.getName();
        });
}



// ─────────────────────────────────────────────
// Sort by quantity
// ─────────────────────────────────────────────
void Inventory::sortByQuantity() {
    std::sort(items.begin(), items.end(),
        [](const Item &a, const Item &b) {
            return a.getQuantity() < b.getQuantity();
        });
}


// ─────────────────────────────────────────────
// Sort by location
// ─────────────────────────────────────────────
void Inventory::sortByLocation() {
    std::sort(items.begin(), items.end(),
        [](const Item &a, const Item &b) {
            return a.getLocation() < b.getLocation();
        });
}
