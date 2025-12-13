#pragma once // using pragma once to avoid multiple inclusions
#include <string>

class Item{

public:
    int id;
    std::string name;
    int quantity;
    std::string location;

    void displayItem() const;
    void updateQuantity(int qty);

    int getId() const { return id; }
    std::string getName() const { return name; }

    std::string toCSV() const;
    void fromCSV(const std::string &csvLine);

};
