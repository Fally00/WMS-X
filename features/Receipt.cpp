#include "core/Receipt.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

// Add an item to the receipt
void Receipt::addItem(const Item& item, int quantity) {
    for (auto& receiptItem : items) {
        if (receiptItem.id == item.getId()) {
            receiptItem.quantity += quantity;
            return;
        }
    }
    items.push_back({item.getId(), item.getName(), quantity});      // Add new item
}



// Print the receipt to console
void Receipt::print() const {
    std::cout << "===== RECEIPT =====\n";
    for (const auto& receiptItem : items) {              // Print each item
        std::cout << "ID: " << receiptItem.id 
                  << " | Name: " << receiptItem.name
                  << " | Quantity: " << receiptItem.quantity << "\n";
    }
    std::cout << "===================\n";
}
void Receipt::clear() {   // Clear the receipt
    items.clear();
}


// Save receipt to a file in CSV format
void Receipt::saveToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Error opening file for writing: " << filename << "\n";
        return;
    }
    for (const auto& receiptItem : items) {              // Write each item as CSV
        outFile << receiptItem.id << ","
                << receiptItem.name << ","
                << receiptItem.quantity << "\n";
    }
    outFile.close();
}

