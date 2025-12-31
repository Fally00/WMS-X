#include "Item.h"
#include "output.h"
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

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

// Modified method name
void Item::changeQuantity(int delta) {
    int newQty = quantity + delta;
    if (newQty < 0) {
        throw std::invalid_argument("Insufficient stock");
    }
    setQuantity(newQty);    /// Update quantity using setter
}

// CSV Serialization
std::string Item::toCSV() const {
    std::stringstream ss;
    ss << id << "," << name << "," << quantity << "," << location;
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