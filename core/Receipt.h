#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include "Item.h"

struct ReceiptItem {
    int id;
    std::string name;
    std::string location;
    int quantity;
    double unitPrice;
    double lineTotal() const { return quantity * unitPrice; }
};

class Receipt {
public:
    Receipt();

    void setCustomer(const std::string& name, const std::string& phone = "", const std::string& email = "");

    void addItem(const Item& item, int quantity, double unitPrice);
    void clear();

    double subtotal() const;
    double tax() const;
    double total() const;

    std::string getReceiptNumber() const;

    void print() const;
    void saveToFile(const std::string& directory = "receipts") const;
    static std::vector<Receipt> loadHistory(const std::string& directory = "receipts");

private:
    std::string receiptNumber;
    std::chrono::system_clock::time_point timestamp;

    std::string customerName;
    std::string customerPhone;
    std::string customerEmail;

    std::vector<ReceiptItem> items;

    static std::string generateReceiptNumber();
    static std::string formatTime(const std::chrono::system_clock::time_point& tp);
};
