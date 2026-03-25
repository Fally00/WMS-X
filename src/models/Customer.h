#pragma once

//needed libraries
#include <string>
#include <ctime>
#include <vector>

class Customer {

// Data members
private:
    int id;
    std::string name;
    std::string phone;
    std::string address;
    std::string email;

    std::time_t createdAt;
    std::time_t modifiedAt;

    void touch();
    void validate() const;

// Public interface
public:
    // Constructors
    Customer() = default;
    Customer(int id,
             const std::string& name,
             const std::string& phone = "",
             const std::string& address = "",
             const std::string& email = "");

    // Getters
    int getId() const;
    const std::string& getName() const;
    const std::string& getPhone() const;
    const std::string& getAddress() const;
    const std::string& getEmail() const;
    std::time_t getCreatedAt() const;
    std::time_t getModifiedAt() const;

    // Setters
    void setName(const std::string& n);
    void setPhone(const std::string& p);
    void setAddress(const std::string& a);
    void setEmail(const std::string& e);

    bool operator==(const Customer& o) const;
    bool operator<(const Customer& o) const;
};

// Print Customer Details
void printCustomer(const Customer& customer);
