//Included files
#include "models/Item.h"
#include "output/output.h"

//Needed libraries
#include <cctype>
#include <stdexcept>
#include <string>

// ─────────────────────────────────────────────
// Helpers / validation
// ─────────────────────────────────────────────

void Item::touch() {
    modifiedAt = std::time(nullptr);
}

bool Item::isValidLocation(const std::string& loc) {
    // Minimal validation for now
    return !loc.empty();
}

void Item::validate() const {
    if (id < 0) throw std::invalid_argument("Item id must be non-negative");
    if (quantity < 0) throw std::invalid_argument("Item quantity must be non-negative");
    if (name.empty()) throw std::invalid_argument("Item name must not be empty");
    if (!isValidLocation(location)) throw std::invalid_argument("Invalid location");
    if (!barcode.empty()) {
        for (unsigned char c : barcode) {
            if (std::isspace(c))
                throw std::invalid_argument("Barcode must not contain whitespace");
        }
    }
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
    const std::string& category,
    const std::string& barcodeValue)
    : id(id),
      name(name),
      quantity(qty),
      location(loc),
      price(price),
      barcode(barcodeValue),
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
const std::string& Item::getBarcode() const { return barcode; }
std::time_t Item::getCreatedAt() const { return createdAt; }
std::time_t Item::getModifiedAt() const { return modifiedAt; }

void Item::setLocation(const std::string& loc) {
    if (!isValidLocation(loc)) throw std::invalid_argument("Invalid location");
    auditLog.push_back("Location: " + location + " -> " + loc);
    location = loc;
    touch();
}

void Item::setName(const std::string& n) {
    if (n.empty()) throw std::invalid_argument("Name must not be empty");
    auditLog.push_back("Name: " + name + " -> " + n);
    name = n;
    touch();
}

void Item::setQuantity(int qty) {
    if (qty < 0) throw std::invalid_argument("Quantity must be non-negative");
    auditLog.push_back("Qty: " + std::to_string(quantity) + " -> " + std::to_string(qty));
    quantity = qty;
    touch();
}

void Item::setPrice(double p) {
    if (p < 0.0) throw std::invalid_argument("Price cannot be negative");
    auditLog.push_back("Price: " + std::to_string(price) + " -> " + std::to_string(p));
    price = p;
    touch();
}

void Item::setBarcode(const std::string& b) {
    auditLog.push_back("Barcode: " + barcode + " -> " + b);
    barcode = b;
    validate();
    touch();
}

// ─────────────────────────────────────────────
// Operators
// ─────────────────────────────────────────────

bool Item::operator==(const Item& o) const { return id == o.id; }
bool Item::operator<(const Item& o) const { return id < o.id; }

// Print Item Details (Improved)
void printItem(const Item& item) {
    std::vector<std::string> headers = {"ID", "Name", "Quantity", "Location", "Barcode", "Currency"};
    std::vector<std::vector<std::string>> rows = {{
        std::to_string(item.getId()),
        item.getName(),
        std::to_string(item.getQuantity()),
        item.getLocation(),
        item.getBarcode().empty() ? "—" : item.getBarcode(),
        item.getCurrency()
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

