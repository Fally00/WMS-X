#include "Receipt.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>


void Receipt::addItem(const Item& item, int quantity) {
    for (auto& receiptItem : items) {
        if (receiptItem.id == item.getId()) {
            receiptItem.quantity += quantity;
            return;
        }
    }
    items.push_back({item.getId(), item.getName(), quantity});
}

