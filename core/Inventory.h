#pragma once
#include "core/Item.h"
#include <vector>
#include <string>

class Inventory {
private:
    std::vector<Item> items;
    std::string dataFilePath;

public:
    Inventory(const std::string &filePath);    // declare only
    void addItem(const Item &item);           //adding item to inventory
    void removeItem(int itemId);             //removing item from inventory
    Item* findItem(int itemId);             //finding item by ID
    void displayItems() const;             //display all items
    std::string toCSV(const std::string& filePath) const;
    void fromCSV(const std::string &csvData);
    
    void sortByID();               //sort items by ID
    void sortByName();            //sort items by Name
    void sortByQuantity();       //sort items by Quantity
    void sortByLocation();      //sort items by Location
};
// Free function to print item details (solving inventory displaying issue)
void printItem(const Item& item);