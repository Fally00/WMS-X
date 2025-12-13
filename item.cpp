#include "Item.h"
#include <iostream>
#include <sstream>
using namespace std;

void Item::displayItem() const {
    cout << "Item ID: " << id << endl;
    cout << "Item Name: " << name << endl;
    cout << "Quantity: " << quantity << endl;
    cout << "Location: " << location << endl;

}

void Item::updateQuantity(int qty){
    quantity += qty;

}

string Item::toCSV() const {
    stringstream ss;
    ss << id << "," << name << "," << quantity << "," << location;
    return ss.str();
}

void Item::fromCSV(const string &csvLine) {
    stringstream ss(csvLine);
    string token;
    // Expecting format: id,name,quantity,location
    if (!getline(ss, token, ',')) return;
    try {
        id = stoi(token);
    } catch (...) {
        id = 0;
    }
    if (!getline(ss, token, ',')) return;
    name = token;
    if (!getline(ss, token, ',')) return;
    try {
        quantity = stoi(token);
    } catch (...) {
        quantity = 0;
    }
    if (!getline(ss, token, ',')) token = "";
    location = token;
}