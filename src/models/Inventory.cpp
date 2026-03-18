//Included files
#include "models/Inventory.h"
#include "output/output.h"

//Needed libraries 
#include <algorithm>
#include <iostream>

Inventory::Inventory(SQLite::Database& database)
    : db(database) {}

// -----------------------------
// Load all items from SQLite
// -----------------------------
void Inventory::loadAll() {
    items.clear();
    SQLite::Statement query(db,
        "SELECT id, name, quantity, location, price, currency, unit, category, "
        "created_at, modified_at FROM items");

    while (query.executeStep()) {
        Item item(
            query.getColumn(0).getInt(),       // id
            query.getColumn(1).getString(),    // name
            query.getColumn(2).getInt(),       // quantity
            query.getColumn(3).getString(),    // location
            query.getColumn(4).getDouble(),    // price
            query.getColumn(5).getString(),    // currency
            query.getColumn(6).getString(),    // unit
            query.getColumn(7).getString()     // category
        );
        items[item.getId()] = item;
    }
}

// -----------------------------
// Add / Remove
// -----------------------------
bool Inventory::addItem(const Item &item) {
    if (items.count(item.getId()) > 0) return false;
    items[item.getId()] = item;
    saveItem(item);
    return true;
}

bool Inventory::removeItem(int itemId) {
    if (items.erase(itemId) > 0) {
        SQLite::Statement query(db, "DELETE FROM items WHERE id = ?");
        query.bind(1, itemId);
        query.exec();
        return true;
    }
    return false;
}

// Persist a single item (INSERT)
void Inventory::saveItem(const Item& item) {
    SQLite::Statement query(db,
        "INSERT OR REPLACE INTO items "
        "(id, name, quantity, location, price, currency, unit, category, created_at, modified_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    query.bind(1, item.getId());
    query.bind(2, item.getName());
    query.bind(3, item.getQuantity());
    query.bind(4, item.getLocation());
    query.bind(5, item.getPrice());
    query.bind(6, item.getCurrency());
    query.bind(7, item.getUnit());
    query.bind(8, item.getCategory());
    query.bind(9, static_cast<int64_t>(item.getCreatedAt()));
    query.bind(10, static_cast<int64_t>(item.getModifiedAt()));
    query.exec();
}

// Update an existing item in the database
void Inventory::updateItemInDB(const Item& item) {
    saveItem(item);  // INSERT OR REPLACE handles both
}

// Batch operations
void Inventory::addMultiple(const std::vector<Item> &batch) {
    for (const auto &item : batch) addItem(item);
}

void Inventory::removeMultiple(const std::vector<int> &ids) {
    for (int id : ids) removeItem(id);
}

// -----------------------------
// Search 
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
// Helpers
// -----------------------------
std::vector<Item> Inventory::getAllItems() const {
    std::vector<Item> all;
    for (const auto &[id, item] : items) all.push_back(item);
    return all;
}
