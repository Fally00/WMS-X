#include "Receipt.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <random>
#include <cctype>
#include <algorithm>

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
    cout << "        WMS-X WAREHOUSE RECEIPT\n";
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
// JSON Helpers 
// ─────────────────────────────────────────────
static string escapeJSON(const string& str) {
    stringstream ss;
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
                    ss << "\\u" << hex << setw(4) << setfill('0') << (int)c;
                } else {
                    ss << c;
                }
                break;
        }
    }
    return ss.str();
}

static string unescapeJSON(const string& str) {
    string result;
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

// ─────────────────────────────────────────────
// JSON Serialization
// ─────────────────────────────────────────────
string Receipt::toJSON() const {
    stringstream ss;
    ss << "{\n";
    ss << "  \"receiptNumber\": \"" << escapeJSON(receiptNumber) << "\",\n";
    ss << "  \"timestamp\": \"" << escapeJSON(formatTime(timestamp)) << "\",\n";
    ss << "  \"customerName\": \"" << escapeJSON(customerName) << "\",\n";
    ss << "  \"customerPhone\": \"" << escapeJSON(customerPhone) << "\",\n";
    ss << "  \"customerEmail\": \"" << escapeJSON(customerEmail) << "\",\n";
    ss << "  \"items\": [\n";
    
    for (size_t i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        ss << "    {\n";
        ss << "      \"id\": " << item.id << ",\n";
        ss << "      \"name\": \"" << escapeJSON(item.name) << "\",\n";
        ss << "      \"location\": \"" << escapeJSON(item.location) << "\",\n";
        ss << "      \"quantity\": " << item.quantity << ",\n";
        ss << "      \"unitPrice\": " << fixed << setprecision(2) << item.unitPrice << ",\n";
        ss << "      \"lineTotal\": " << item.lineTotal() << "\n";
        ss << "    }";
        if (i < items.size() - 1) ss << ",";
        ss << "\n";
    }
    
    ss << "  ],\n";
    ss << "  \"subtotal\": " << fixed << setprecision(2) << subtotal() << ",\n";
    ss << "  \"tax\": " << tax() << ",\n";
    ss << "  \"total\": " << total() << "\n";
    ss << "}";
    return ss.str();
}

// ─────────────────────────────────────────────
// JSON Deserialization
// ─────────────────────────────────────────────
Receipt Receipt::fromJSON(const string& jsonStr) {
    Receipt receipt;
    
    // Simple JSON parser for receipt
    size_t pos = 0;
    
    // Helper to extract string value
    auto extractString = [&](const string& key) -> string {
        string searchKey = "\"" + key + "\"";
        size_t keyPos = jsonStr.find(searchKey, pos);
        if (keyPos == string::npos) return "";
        
        size_t colonPos = jsonStr.find(':', keyPos);
        if (colonPos == string::npos) return "";
        
        size_t quoteStart = jsonStr.find('"', colonPos);
        if (quoteStart == string::npos) return "";
        
        size_t quoteEnd = quoteStart + 1;
        bool escaped = false;
        while (quoteEnd < jsonStr.length()) {
            if (escaped) {
                escaped = false;
                quoteEnd++;
                continue;
            }
            if (jsonStr[quoteEnd] == '\\') {
                escaped = true;
                quoteEnd++;
                continue;
            }
            if (jsonStr[quoteEnd] == '"') break;
            quoteEnd++;
        }
        
        string value = jsonStr.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
        pos = quoteEnd + 1;
        return unescapeJSON(value);
    };
    
    // Extract receipt fields
    receipt.receiptNumber = extractString("receiptNumber");
    string timestampStr = extractString("timestamp");
    receipt.customerName = extractString("customerName");
    receipt.customerPhone = extractString("customerPhone");
    receipt.customerEmail = extractString("customerEmail");
    
    // Parse timestamp (simple format: "YYYY-MM-DD HH:MM:SS")
    // For now, we'll set it to current time since parsing time_t from string is complex
    receipt.timestamp = chrono::system_clock::now();
    
    // Extract items array
    size_t itemsStart = jsonStr.find("\"items\"", pos);
    if (itemsStart != string::npos) {
        size_t arrayStart = jsonStr.find('[', itemsStart);
        if (arrayStart != string::npos) {
            size_t arrayEnd = jsonStr.find(']', arrayStart);
            if (arrayEnd != string::npos) {
                string itemsStr = jsonStr.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                
                // Parse each item object
                size_t itemPos = 0;
                while (itemPos < itemsStr.length()) {
                    size_t objStart = itemsStr.find('{', itemPos);
                    if (objStart == string::npos) break;
                    
                    size_t objEnd = objStart;
                    int braceCount = 0;
                    for (size_t i = objStart; i < itemsStr.length(); i++) {
                        if (itemsStr[i] == '{') braceCount++;
                        else if (itemsStr[i] == '}') {
                            braceCount--;
                            if (braceCount == 0) {
                                objEnd = i;
                                break;
                            }
                        }
                    }
                    
                    if (braceCount == 0) {
                        string itemJson = itemsStr.substr(objStart, objEnd - objStart + 1);
                        
                        // Extract item fields
                        size_t itemJsonPos = 0;
                        auto getItemString = [&](const string& key) -> string {
                            string searchKey = "\"" + key + "\"";
                            size_t keyPos = itemJson.find(searchKey, itemJsonPos);
                            if (keyPos == string::npos) return "";
                            size_t colonPos = itemJson.find(':', keyPos);
                            if (colonPos == string::npos) return "";
                            size_t quoteStart = itemJson.find('"', colonPos);
                            if (quoteStart == string::npos) return "";
                            size_t quoteEnd = quoteStart + 1;
                            bool escaped = false;
                            while (quoteEnd < itemJson.length()) {
                                if (escaped) { escaped = false; quoteEnd++; continue; }
                                if (itemJson[quoteEnd] == '\\') { escaped = true; quoteEnd++; continue; }
                                if (itemJson[quoteEnd] == '"') break;
                                quoteEnd++;
                            }
                            string value = itemJson.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                            itemJsonPos = quoteEnd + 1;
                            return unescapeJSON(value);
                        };
                        
                        auto getItemNumber = [&](const string& key) -> double {
                            string searchKey = "\"" + key + "\"";
                            size_t keyPos = itemJson.find(searchKey, itemJsonPos);
                            if (keyPos == string::npos) return 0.0;
                            size_t colonPos = itemJson.find(':', keyPos);
                            if (colonPos == string::npos) return 0.0;
                            size_t numStart = colonPos + 1;
                            while (numStart < itemJson.length() && (itemJson[numStart] == ' ' || itemJson[numStart] == '\t')) {
                                numStart++;
                            }
                            size_t numEnd = numStart;
                            while (numEnd < itemJson.length() && 
                                   (isdigit(itemJson[numEnd]) || itemJson[numEnd] == '.' || 
                                    itemJson[numEnd] == '-' || itemJson[numEnd] == '+')) {
                                numEnd++;
                            }
                            string numStr = itemJson.substr(numStart, numEnd - numStart);
                            itemJsonPos = numEnd;
                            try {
                                return stod(numStr);
                            } catch (...) {
                                return 0.0;
                            }
                        };
                        
                        ReceiptItem item;
                        item.id = (int)getItemNumber("id");
                        item.name = getItemString("name");
                        item.location = getItemString("location");
                        item.quantity = (int)getItemNumber("quantity");
                        item.unitPrice = getItemNumber("unitPrice");
                        
                        receipt.items.push_back(item);
                    }
                    
                    itemPos = objEnd + 1;
                    while (itemPos < itemsStr.length() && 
                           (itemsStr[itemPos] == ',' || itemsStr[itemPos] == ' ' || 
                            itemsStr[itemPos] == '\t' || itemsStr[itemPos] == '\n' || itemsStr[itemPos] == '\r')) {
                        itemPos++;
                    }
                }
            }
        }
    }
    
    return receipt;
}
// ─────────────────────────────────────────────
// Save Receipt
// ─────────────────────────────────────────────
void Receipt::saveToFile(const string& directory) const {
    filesystem::create_directories(directory);
    string filename = directory + "/" + receiptNumber + ".json";

    ofstream file(filename);
    if (!file)
        throw runtime_error("Failed to save receipt");

    file << toJSON();
}

// ─────────────────────────────────────────────
// Load Receipt History
// ─────────────────────────────────────────────
vector<Receipt> Receipt::loadHistory(const string& directory) {
    vector<Receipt> receipts;
    
    if (!filesystem::exists(directory)) {
        return receipts;
    }
    
    try {
        for (const auto& entry : filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                ifstream file(entry.path());
                if (!file) continue;
                
                stringstream buffer;
                buffer << file.rdbuf();
                string jsonContent = buffer.str();
                
                if (!jsonContent.empty()) {
                    try {
                        Receipt receipt = fromJSON(jsonContent);
                        receipts.push_back(receipt);
                    } catch (...) {
                        // Skip invalid JSON files
                        continue;
                    }
                }
            }
        }
    } catch (const filesystem::filesystem_error&) {
        // Directory doesn't exist or can't be accessed
        return receipts;
    }
    
    return receipts;
}
