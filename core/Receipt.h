#pragma once
#include <string>
#include <vector>
#include "core/Item.h"


// Structure to hold item details in the receipt
struct ReceiptItem {
    int id;
    std::string name;
    int quantity;
};

// Receipt class to manage purchased items
class Receipt {
    public:
        void addItem(const Item& item, int quantity);           //add an item to the receipt
        void print() const;                                    //print the receipt to console
        void clear();                                         //clear the receipt
        void saveToFile(const std::string& filename) const;  //save receipt to a file in CSV format
    private:
        std::vector<ReceiptItem> items;        //make a table with items in the receipt
    };

    