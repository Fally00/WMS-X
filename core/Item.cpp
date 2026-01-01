#include "Item.h"
#include "output.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
using namespace std;

// ─────────────────────────────────────────────
// Helpers / validation
// ─────────────────────────────────────────────

void Item::touch() {
    modifiedAt = std::time(nullptr);
}

bool Item::isValidLocation(const std::string& loc) {
    // Minimal validation for now; can be tightened later.
    return !loc.empty();
}

void Item::validate() const {
    if (id < 0) throw std::invalid_argument("Item id must be non-negative");
    if (quantity < 0) throw std::invalid_argument("Item quantity must be non-negative");
    if (name.empty()) throw std::invalid_argument("Item name must not be empty");
    if (!isValidLocation(location)) throw std::invalid_argument("Invalid location");
}

// ─────────────────────────────────────────────
// Constructors
// ─────────────────────────────────────────────

Item::Item(
    int id,
    const std::string& name,
    int qty,
    const std::string& loc,
    double price,
    const std::string& currency,
    const std::string& unit,
    const std::string& category)
    : id(id),
      name(name),
      quantity(qty),
      location(loc),
      price(price),
      currency(currency),
      unit(unit),
      category(category) {
    createdAt = modifiedAt = std::time(nullptr);
    validate();
}

// ─────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────

int Item::getId() const { return id; }
const std::string& Item::getName() const { return name; }
int Item::getQuantity() const { return quantity; }
const std::string& Item::getLocation() const { return location; }
double Item::getPrice() const { return price; }
const std::string& Item::getCurrency() const { return currency; }
const std::string& Item::getUnit() const { return unit; }
const std::string& Item::getCategory() const { return category; }
std::time_t Item::getCreatedAt() const { return createdAt; }
std::time_t Item::getModifiedAt() const { return modifiedAt; }

void Item::setLocation(const std::string& loc) {
    if (!isValidLocation(loc)) throw std::invalid_argument("Invalid location");
    location = loc;
    auditLog.push_back("Location -> " + location);
    touch();
}

// ─────────────────────────────────────────────
// Operators
// ─────────────────────────────────────────────

bool Item::operator==(const Item& o) const { return id == o.id; }
bool Item::operator<(const Item& o) const { return id < o.id; }

// Print Item Details (Improved)
void printItem(const Item& item) {
    std::vector<std::string> headers = {"ID", "Name", "Quantity", "Location"};
    std::vector<std::vector<std::string>> rows = {{
        std::to_string(item.getId()),
        item.getName(),
        std::to_string(item.getQuantity()),
        item.getLocation()
    }};
    OutputFormatter::printTable(headers, rows);
}

// Constructor for item class quantity validation
void Item::changeQuantity(int delta) {
    int old = quantity;
    quantity += delta;
    if (quantity < 0) throw std::invalid_argument("Insufficient stock");
    auditLog.push_back("Qty " + std::to_string(old) + " -> " + std::to_string(quantity));
    touch();
}


// CSV Serialization
std::string Item::toCSV() const {
    std::stringstream ss;
    ss << id << "," << name << "," << quantity << "," << location;
    return ss.str();
}

std::string Item::toJSON() const {
    // Minimal JSON; no escaping for now (intended for debugging).
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id << ","
       << "\"name\":\"" << name << "\","
       << "\"quantity\":" << quantity << ","
       << "\"location\":\"" << location << "\""
       << "}";
    return ss.str();
}

// CSV Deserialization
Item Item::fromCSV(const std::string& csvLine) {
    std::stringstream ss(csvLine);        // String stream for parsing
    std::string token;                   // Temporary string to hold each token

    int id = 0;
    std::string name;
    int quantity = 0;
    std::string location;

    if (getline(ss, token, ',')) id = stoi(token);
    if (getline(ss, token, ',')) name = token;
    if (getline(ss, token, ',')) quantity = stoi(token);
    if (getline(ss, token, ',')) location = token;

    return Item(id, name, quantity, location);
}