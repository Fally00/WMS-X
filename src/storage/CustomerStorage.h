#pragma once

//Included file
#include "models/Customer.h"
#include "storage/Storage.h"

//Needed libraries
#include <unordered_map>
#include <string>
#include <vector>

class CustomerStorage {
private:
    std::unordered_map<int, Customer> customers;   // ID -> Customer for O(1) lookup
    SQLite::Database& db;

public:
    CustomerStorage(SQLite::Database& database);

    // Load all customers from SQLite into memory
    void loadAll();

    // CRUD
    bool addCustomer(const Customer& customer);      // returns false if duplicate
    bool removeCustomer(int id);                     // returns false if not found
    Customer* findCustomer(int id);                  // returns nullptr if not found

    // Persist single customer changes to SQLite
    void saveCustomer(const Customer& customer);

    // Search & Query
    std::vector<Customer> searchByName(const std::string& query) const;
    std::vector<Customer> getAllCustomers() const;

    // Auto-increment logic (query MAX(id)+1)
    int getNextId();
};
