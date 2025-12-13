#include "Inventory.h"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
using namespace std;

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
Inventory::Inventory(const std::string &filePath) : dataFilePath(filePath) {}



// ─────────────────────────────────────────────
// Load data from CSV string
// ─────────────────────────────────────────────
void Inventory::fromCSV(const std::string &csvData) {
    stringstream ss(csvData);
    string line;
    while (getline(ss, line)) {
        Item item;
        item.fromCSV(line);
        items.push_back(item);
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
void Inventory::displayAllItems() const {
    for (const auto &item : items) {
        item.displayItem();
        cout << "------------------" << endl;
    }
}


// ─────────────────────────────────────────────
// Add new item to inventory
// ─────────────────────────────────────────────
void Inventory::addItem(const Item &item) {
    items.push_back(item);
}


// ─────────────────────────────────────────────
// Find item by ID (Linear Search)
// ─────────────────────────────────────────────
Item* Inventory::findItem(int itemId) {
    for (auto &item : items) {
        if (item.id == itemId)
            return &item;
    }
    return nullptr;
}



// ─────────────────────────────────────────────
// Remove item by ID
// ─────────────────────────────────────────────
void Inventory::removeItem(int itemId) {
    auto it = std::find_if(items.begin(), items.end(), 
        [&](const Item &item) { return item.id == itemId; });

    if (it != items.end())
        items.erase(it);
}


// ─────────────────────────────────────────────
// Sort by ID
// ─────────────────────────────────────────────
void Inventory::sortByID() {
    sort(items.begin(), items.end(),
         [](const Item &a, const Item &b) { return a.id < b.id; });
}


// ─────────────────────────────────────────────
// Sort by name
// ─────────────────────────────────────────────
void Inventory::sortByName() {
    sort(items.begin(), items.end(),
         [](const Item &a, const Item &b) { return a.name < b.name; });
}


// ─────────────────────────────────────────────
// Sort by quantity
// ─────────────────────────────────────────────
void Inventory::sortByQuantity() {
    sort(items.begin(), items.end(),
         [](const Item &a, const Item &b) { return a.quantity < b.quantity; });
}


// ─────────────────────────────────────────────
// Sort by location
// ─────────────────────────────────────────────
void Inventory::sortByLocation() {
    sort(items.begin(), items.end(),
         [](const Item &a, const Item &b) { return a.location < b.location; });
}
