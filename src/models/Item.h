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
    std::string barcode;
    std::string currency;
    std::string unit;
    std::string category;

    std::time_t createdAt;
    std::time_t modifiedAt;


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
         const std::string& category = "general",
         const std::string& barcode = "");
         
    // Getters and Setters for data members
    int getId() const;
    const std::string& getName() const;
    int getQuantity() const;
    const std::string& getLocation() const;
    double getPrice() const;
    const std::string& getCurrency() const;
    const std::string& getUnit() const;
    const std::string& getCategory() const;
    const std::string& getBarcode() const;
    std::time_t getCreatedAt() const;
    std::time_t getModifiedAt() const;
    void changeQuantity(int delta);
    void setLocation(const std::string& loc);
    void setName(const std::string& n);
    void setQuantity(int qty);
    void setPrice(double p);
    void setBarcode(const std::string& b);

    bool operator==(const Item& o) const;
    bool operator<(const Item& o) const;
};

// Print Item Details (Improved)
void printItem(const Item& item);
