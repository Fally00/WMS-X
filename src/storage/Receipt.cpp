//Most needed file inclusion
#include "storage/Receipt.h"

//needed libraries
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <ctime>

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
// Clear
// ─────────────────────────────────────────────
void Receipt::clear() {
    items.clear();
    customerName.clear();
    customerPhone.clear();
    customerEmail.clear();
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

string Receipt::getCustomerName() const {
    return customerName;
}

chrono::system_clock::time_point Receipt::getTimestamp() const {
    return timestamp;
}

// ─────────────────────────────────────────────
// Print
// ─────────────────────────────────────────────
void Receipt::print() const {
    cout << "\n=====================================\n";
    cout << "        WMS-X  RECEIPT\n";
    cout << "-------------------------------------\n";
    cout << "Receipt: " << receiptNumber << "\n";
    cout << "Date   : " << formatTime(timestamp) << "\n";
    if (!customerName.empty())
        cout << "Customer: " << customerName << "\n";

    cout << "-------------------------------------\n";
    cout << left << setw(5) << "ID"
         << setw(15) << "Name"
         << setw(6) << "Quantity"
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
// Save Receipt to SQLite
// ─────────────────────────────────────────────
void Receipt::saveToDB(SQLite::Database& db) const {
    SQLite::Transaction transaction(db);
    try {
        // Insert receipt header
        SQLite::Statement insertReceipt(db,
            "INSERT INTO receipts (receipt_number, timestamp, customer_name, "
            "customer_phone, customer_email, subtotal, tax, total) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

        insertReceipt.bind(1, receiptNumber);
        insertReceipt.bind(2, formatTime(timestamp));
        insertReceipt.bind(3, customerName);
        insertReceipt.bind(4, customerPhone);
        insertReceipt.bind(5, customerEmail);
        insertReceipt.bind(6, subtotal());
        insertReceipt.bind(7, tax());
        insertReceipt.bind(8, total());
        insertReceipt.exec();

        // Insert receipt line items
        for (const auto& item : items) {
            SQLite::Statement insertItem(db,
                "INSERT INTO receipt_items (receipt_number, item_id, name, location, "
                "quantity, unit_price, line_total) VALUES (?, ?, ?, ?, ?, ?, ?)");

            insertItem.bind(1, receiptNumber);
            insertItem.bind(2, item.id);
            insertItem.bind(3, item.name);
            insertItem.bind(4, item.location);
            insertItem.bind(5, item.quantity);
            insertItem.bind(6, item.unitPrice);
            insertItem.bind(7, item.lineTotal());
            insertItem.exec();
        }
        transaction.commit();
    } catch (const std::exception& e) {
        std::cerr << "[RECEIPT STORAGE] Save failed: " << e.what() << std::endl;
        throw; // Rethrow to inform the caller
    }
}

// ─────────────────────────────────────────────
// Parse a "%Y-%m-%d %H:%M:%S" string back into a time_point
// ─────────────────────────────────────────────
static chrono::system_clock::time_point parseTime(const string& str) {
    tm buf{};
    istringstream ss(str);
    ss >> get_time(&buf, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) return chrono::system_clock::now(); // fallback
    return chrono::system_clock::from_time_t(mktime(&buf));
}

// ─────────────────────────────────────────────
// Load Receipt History from SQLite
// ─────────────────────────────────────────────
vector<Receipt> Receipt::loadHistory(SQLite::Database& db) {
    vector<Receipt> receipts;

    SQLite::Statement query(db,
        "SELECT receipt_number, timestamp, customer_name, customer_phone, "
        "customer_email FROM receipts ORDER BY timestamp DESC");

    while (query.executeStep()) {
        Receipt receipt;
        receipt.receiptNumber = query.getColumn(0).getString();
        receipt.timestamp = parseTime(query.getColumn(1).getString());
        receipt.customerName = query.getColumn(2).getString();
        receipt.customerPhone = query.getColumn(3).getString();
        receipt.customerEmail = query.getColumn(4).getString();

        // Load line items for this receipt
        SQLite::Statement itemQuery(db,
            "SELECT item_id, name, location, quantity, unit_price "
            "FROM receipt_items WHERE receipt_number = ?");
        itemQuery.bind(1, receipt.receiptNumber);

        while (itemQuery.executeStep()) {
            ReceiptItem item;
            item.id = itemQuery.getColumn(0).getInt();
            item.name = itemQuery.getColumn(1).getString();
            item.location = itemQuery.getColumn(2).getString();
            item.quantity = itemQuery.getColumn(3).getInt();
            item.unitPrice = itemQuery.getColumn(4).getDouble();
            receipt.items.push_back(item);
        }

        receipts.push_back(receipt);
    }

    return receipts;
}

// ─────────────────────────────────────────────
// Delete a receipt from SQLite
// ─────────────────────────────────────────────
void Receipt::deleteFromDB(SQLite::Database& db, const string& receiptNumber) {
    SQLite::Statement delItems(db,
        "DELETE FROM receipt_items WHERE receipt_number = ?");
    delItems.bind(1, receiptNumber);
    delItems.exec();

    SQLite::Statement delReceipt(db,
        "DELETE FROM receipts WHERE receipt_number = ?");
    delReceipt.bind(1, receiptNumber);
    delReceipt.exec();
}
