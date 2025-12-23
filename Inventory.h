#pragma once
#include "Item.h"
#include <vector>
#include <string>

class Inventory {
private:
    std::vector<Item> items;
    std::string dataFilePath;

public:
    Inventory(const std::string &filePath);  // declare only
    void addItem(const Item &item);
    void removeItem(int itemId);
    Item* findItem(int itemId);
    void displayItems() const;
    std::string toCSV(const std::string& filePath) const;
    void fromCSV(const std::string &csvData);
    
    void sortByID();
    void sortByName();
    void sortByQuantity();
    void sortByLocation();
};
// Free function to print item details (solving inventory displaying issue)
void printItem(const Item& item);