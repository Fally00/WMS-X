#pragma once // using pragma once to avoid multiple inclusions
#include <string>
#include <stdexcept>

class Item {
//defining class Item 
private:
    int id;
    std::string name;
    int quantity;
    std::string location;

public:
    // Constructor
    Item(int id, std::string name, int qty, std::string loc)
        : id(id), name(std::move(name)), location(std::move(loc)) {
        setQuantity(qty);
    }

    // Factory for CSV
    static Item fromCSV(const std::string& line);
    std::string toCSV() const;

    // Getters and Setters
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    
    // Modified method name
    void changeQuantity(int delta);
    int getQuantity() const { return quantity; }
    void setQuantity(int q) {
        if (q < 0) throw std::invalid_argument("Negative quantity");
        quantity = q;
    }
    // New getter and setter for location
    const std::string& getLocation() const { return location; }
    void setLocation(const std::string& loc) { location = loc; }
};
//Free function to print item details (solving inventory displaying issue)
void printItem(const Item& item);
