#include "Receipt.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <random>

using namespace std;

static constexpr double TAX_RATE = 0.14; // 14% VAT

// ─────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────
string Receipt::generateReceiptNumber() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(100000, 999999);
    return "RCPT-" + to_string(dis(gen));
}

string Receipt::formatTime(const chrono::system_clock::time_point& tp) {
    time_t t = chrono::system_clock::to_time_t(tp);
    tm buf{};
#ifdef _WIN32
    localtime_s(&buf, &t);
#else
    localtime_r(&t, &buf);
#endif
    stringstream ss;
    ss << put_time(&buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
Receipt::Receipt() {
    receiptNumber = generateReceiptNumber();
    timestamp = chrono::system_clock::now();
}

// ─────────────────────────────────────────────
// Customer
// ─────────────────────────────────────────────
void Receipt::setCustomer(const string& name, const string& phone, const string& email) {
    customerName = name;
    customerPhone = phone;
    customerEmail = email;
}

// ─────────────────────────────────────────────
// Add Item
// ─────────────────────────────────────────────
void Receipt::addItem(const Item& item, int quantity, double unitPrice) {
    if (quantity <= 0)
        throw invalid_argument("Quantity must be greater than zero");
    if (unitPrice < 0)
        throw invalid_argument("Price cannot be negative");

    for (auto& it : items) {
        if (it.id == item.getId()) {
            it.quantity += quantity;
            return;
        }
    }

    items.push_back({
        item.getId(),
        item.getName(),
        item.getLocation(),
        quantity,
        unitPrice
    });
}

// ─────────────────────────────────────────────
// Calculations
// ─────────────────────────────────────────────
double Receipt::subtotal() const {
    double sum = 0;
    for (const auto& i : items) sum += i.lineTotal();
    return sum;
}

double Receipt::tax() const {
    return subtotal() * TAX_RATE;
}

double Receipt::total() const {
    return subtotal() + tax();
}

string Receipt::getReceiptNumber() const {
    return receiptNumber;
}

// ─────────────────────────────────────────────
// Print
// ─────────────────────────────────────────────
void Receipt::print() const {
    cout << "\n=====================================\n";
    cout << "        ARC WAREHOUSE RECEIPT\n";
    cout << "-------------------------------------\n";
    cout << "Receipt: " << receiptNumber << "\n";
    cout << "Date   : " << formatTime(timestamp) << "\n";
    if (!customerName.empty())
        cout << "Customer: " << customerName << "\n";

    cout << "-------------------------------------\n";
    cout << left << setw(5) << "ID"
         << setw(15) << "Name"
         << setw(6) << "Qty"
         << setw(10) << "Price"
         << setw(10) << "Total" << "\n";

    for (const auto& i : items) {
        cout << left << setw(5) << i.id
             << setw(15) << i.name
             << setw(6) << i.quantity
             << setw(10) << fixed << setprecision(2) << i.unitPrice
             << setw(10) << i.lineTotal() << "\n";
    }

    cout << "-------------------------------------\n";
    cout << "Subtotal: " << subtotal() << "\n";
    cout << "Tax (14%): " << tax() << "\n";
    cout << "TOTAL   : " << total() << "\n";
    cout << "=====================================\n";
}

// ─────────────────────────────────────────────
// Save Receipt
// ─────────────────────────────────────────────
void Receipt::saveToFile(const string& directory) const {
    filesystem::create_directories(directory);
    string filename = directory + "/" + receiptNumber + ".csv";

    ofstream file(filename);
    if (!file)
        throw runtime_error("Failed to save receipt");

    file << "Receipt," << receiptNumber << "\n";
    file << "Date," << formatTime(timestamp) << "\n";
    file << "Customer," << customerName << "\n\n";
    file << "ID,Name,Location,Qty,UnitPrice,LineTotal\n";

    for (const auto& i : items) {
        file << i.id << "," << i.name << "," << i.location << ","
             << i.quantity << "," << i.unitPrice << "," << i.lineTotal() << "\n";
    }

    file << "\nSubtotal," << subtotal() << "\n";
    file << "Tax," << tax() << "\n";
    file << "Total," << total() << "\n";
}
