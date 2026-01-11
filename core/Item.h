#pragma once

//needed libraries
#include <string>
#include <vector>
#include <ctime>

class Item {

// Data members
private:
    int id;
    std::string name;
    int quantity;
    std::string location;

    double price;
    std::string currency;
    std::string unit;
    std::string category;

    std::time_t createdAt;
    std::time_t modifiedAt;
    std::vector<std::string> auditLog;

    void touch();
    void validate() const;
    static bool isValidLocation(const std::string& loc);

// Public interface
public:
    // Constructors
    Item() = default;
    Item(int id,
         const std::string& name,
         int qty,
         const std::string& loc,
         double price = 0.0,
         const std::string& currency = "EGP",
         const std::string& unit = "pcs",
         const std::string& category = "general");
         
    // JSON Serialization / Deserialization
    static Item fromJSON(const std::string& jsonStr);
    std::string toJSON() const;

    // Getters and Setters for data members
    int getId() const;
    const std::string& getName() const;
    int getQuantity() const;
    const std::string& getLocation() const;
    double getPrice() const;
    const std::string& getCurrency() const;
    const std::string& getUnit() const;
    const std::string& getCategory() const;
    std::time_t getCreatedAt() const;
    std::time_t getModifiedAt() const;
    void changeQuantity(int delta);
    void setLocation(const std::string& loc);

    bool operator==(const Item& o) const;
    bool operator<(const Item& o) const;
};

// Print Item Details (Improved)
void printItem(const Item& item);
