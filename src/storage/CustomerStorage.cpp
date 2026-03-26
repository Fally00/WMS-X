//Included files
#include "storage/CustomerStorage.h"
#include "output/output.h"

//Needed libraries
#include <algorithm>
#include <iostream>

CustomerStorage::CustomerStorage(SQLite::Database& database)
    : db(database) {}

// ─────────────────────────────────────────────
// Load all customers from SQLite
// ─────────────────────────────────────────────
void CustomerStorage::loadAll() {
    customers.clear();
    try {
        SQLite::Statement query(db,
            "SELECT id, name, phone, address, email, created_at FROM customers");

        while (query.executeStep()) {
            Customer customer(
                query.getColumn(0).getInt(),       // id
                query.getColumn(1).getString(),    // name
                query.getColumn(2).getString(),    // phone
                query.getColumn(3).getString(),    // address
                query.getColumn(4).getString()     // email
            );
            customers[customer.getId()] = customer;
        }
    } catch (const std::exception& e) {
        std::cerr << "[CUSTOMER STORAGE] Load error: " << e.what() << std::endl;
    }
}

// ─────────────────────────────────────────────
// Add / Remove
// ─────────────────────────────────────────────
bool CustomerStorage::addCustomer(const Customer& customer) {
    if (customers.count(customer.getId()) > 0) return false;
    customers[customer.getId()] = customer;
    saveCustomer(customer);
    return true;
}

bool CustomerStorage::removeCustomer(int id) {
    if (customers.erase(id) > 0) {
        try {
            SQLite::Statement query(db, "DELETE FROM customers WHERE id = ?");
            query.bind(1, id);
            query.exec();
        } catch (const std::exception& e) {
            std::cerr << "[CUSTOMER STORAGE] Delete error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

// ─────────────────────────────────────────────
// Persist a single customer (INSERT OR REPLACE)
// ─────────────────────────────────────────────
void CustomerStorage::saveCustomer(const Customer& customer) {
    try {
        SQLite::Statement query(db,
            "INSERT OR REPLACE INTO customers "
            "(id, name, phone, address, email, created_at) "
            "VALUES (?, ?, ?, ?, ?, ?)");

        query.bind(1, customer.getId());
        query.bind(2, customer.getName());
        query.bind(3, customer.getPhone());
        query.bind(4, customer.getAddress());
        query.bind(5, customer.getEmail());
        query.bind(6, static_cast<int64_t>(customer.getCreatedAt()));
        query.exec();
    } catch (const std::exception& e) {
        std::cerr << "[CUSTOMER STORAGE] Save error: " << e.what() << std::endl;
    }
}

// ─────────────────────────────────────────────
// Search
// ─────────────────────────────────────────────
Customer* CustomerStorage::findCustomer(int id) {
    auto it = customers.find(id);
    if (it != customers.end()) return &it->second;
    return nullptr;
}

std::vector<Customer> CustomerStorage::searchByName(const std::string& query) const {
    std::vector<Customer> results;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    for (const auto& [id, customer] : customers) {
        std::string lowerName = customer.getName();
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (lowerName.find(lowerQuery) != std::string::npos) {
            results.push_back(customer);
        }
    }
    return results;
}

// ─────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────
std::vector<Customer> CustomerStorage::getAllCustomers() const {
    std::vector<Customer> all;
    for (const auto& [id, customer] : customers) all.push_back(customer);
    return all;
}

int CustomerStorage::getNextId() {
    try {
        SQLite::Statement query(db, "SELECT MAX(id) FROM customers");
        if (query.executeStep()) {
            if (query.getColumn(0).isNull()) return 1;
            return query.getColumn(0).getInt() + 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "[CUSTOMER STORAGE] getNextId error: " << e.what() << std::endl;
    }
    return 1;
}
