#include "Item.h"
#include "output.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cctype>
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


// JSON helper: escape string for JSON
static std::string escapeJSON(const std::string& str) {
    std::stringstream ss;
    for (char c : str) {
        switch (c) {
            case '"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default:
                if (c < 0x20) {
                    ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    ss << c;
                }
                break;
        }
    }
    return ss.str();
}

// JSON helper: unescape JSON string
static std::string unescapeJSON(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': result += '"'; i++; break;
                case '\\': result += '\\'; i++; break;
                case '/': result += '/'; i++; break;
                case 'b': result += '\b'; i++; break;
                case 'f': result += '\f'; i++; break;
                case 'n': result += '\n'; i++; break;
                case 'r': result += '\r'; i++; break;
                case 't': result += '\t'; i++; break;
                case 'u':
                    // Simple unicode escape (basic support)
                    if (i + 5 < str.length()) {
                        result += '?'; // Placeholder for unicode
                        i += 5;
                    }
                    break;
                default: result += str[i]; break;
            }
        } else {
            result += str[i];
        }
    }
    return result;
}

// JSON Serialization
std::string Item::toJSON() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":" << id << ","
       << "\"name\":\"" << escapeJSON(name) << "\","
       << "\"quantity\":" << quantity << ","
       << "\"location\":\"" << escapeJSON(location) << "\""
       << "}";
    return ss.str();
}

// JSON Deserialization
Item Item::fromJSON(const std::string& jsonStr) {
    int id = 0;
    std::string name;
    int quantity = 0;
    std::string location;

    // Simple JSON parser - find key-value pairs
    size_t pos = 0;
    while (pos < jsonStr.length()) {
        // Find key
        size_t keyStart = jsonStr.find('"', pos);
        if (keyStart == std::string::npos) break;
        size_t keyEnd = jsonStr.find('"', keyStart + 1);
        if (keyEnd == std::string::npos) break;
        std::string key = jsonStr.substr(keyStart + 1, keyEnd - keyStart - 1);

        // Find colon
        size_t colonPos = jsonStr.find(':', keyEnd);
        if (colonPos == std::string::npos) break;

        // Find value
        size_t valueStart = colonPos + 1;
        while (valueStart < jsonStr.length() && (jsonStr[valueStart] == ' ' || jsonStr[valueStart] == '\t')) {
            valueStart++;
        }

        if (valueStart >= jsonStr.length()) break;

        if (jsonStr[valueStart] == '"') {
            // String value
            size_t valueEnd = valueStart + 1;
            bool escaped = false;
            while (valueEnd < jsonStr.length()) {
                if (escaped) {
                    escaped = false;
                    valueEnd++;
                    continue;
                }
                if (jsonStr[valueEnd] == '\\') {
                    escaped = true;
                    valueEnd++;
                    continue;
                }
                if (jsonStr[valueEnd] == '"') {
                    break;
                }
                valueEnd++;
            }
            std::string value = unescapeJSON(jsonStr.substr(valueStart + 1, valueEnd - valueStart - 1));
            
            if (key == "name") name = value;
            else if (key == "location") location = value;
            
            pos = valueEnd + 1;
        } else {
            // Numeric value
            size_t valueEnd = valueStart;
            while (valueEnd < jsonStr.length() && 
                   (isdigit(jsonStr[valueEnd]) || jsonStr[valueEnd] == '-' || jsonStr[valueEnd] == '+')) {
                valueEnd++;
            }
            int value = std::stoi(jsonStr.substr(valueStart, valueEnd - valueStart));
            
            if (key == "id") id = value;
            else if (key == "quantity") quantity = value;
            
            pos = valueEnd;
        }

        // Skip comma and whitespace
        while (pos < jsonStr.length() && (jsonStr[pos] == ',' || jsonStr[pos] == ' ' || jsonStr[pos] == '\t')) {
            pos++;
        }
    }

    return Item(id, name, quantity, location);
}