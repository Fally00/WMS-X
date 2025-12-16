#include "WmsControllers.h"
#include <iostream>
#include <string>
#include <limits>
using namespace std;

// ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define WHITE   "\033[37m"


// Helper to flush bad input
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
}

int main() {

    const std::string logo = R"(          _______  _______               
|\     /|(       )(  ____ \     |\     /|
| )   ( || () () || (    \/     ( \   / )
| | _ | || || || || (_____  _____\ (_) / 
| |( )| || |(_)| |(_____  )(_____)) _ (  
| || || || |   | |      ) |      / ( ) \ 
| () () || )   ( |/\____) |     ( /   \ )
(_______)|/     \|\_______)     |/     \|
                                        )";
    cout<< MAGENTA <<logo << std::endl;
    cout << WHITE << "      WAREHOUSE MANAGEMENT SYSTEM v1.2" << RESET << endl;

    WmsControllers wms("inventory_data.csv");

    if (!wms.initializeSystem()) {
        cerr << "[ERROR 404] System initialization failed." << endl;
        return 1;
    }

    int choice;
    bool autosave = false;
    string id, name, location;
    int qty;

    while (true) {

        cout << "\n\n==============================";
        cout << MAGENTA << "\n < WAREHOUSE MANAGEMENT HUB >" << RESET;
        cout << "\n==============================";
        cout << "\n[1] Add Item (Instant)";
        cout << "\n[2] Remove Item (Instant)";
        cout << "\n[3] Find Item";
        cout << "\n[4] List All Items";
        cout << "\n------------------------------";
        cout << "\n[5] Queue Add";
        cout << "\n[6] Queue Remove";
        cout << "\n[7] Queue Search";
        cout << "\n[8] Queue List";
        cout << "\n[9] Process Queue";
        cout << "\n------------------------------";
        cout <<GREEN<< "\n[10] Auto-Save  (" << (autosave ? "ON" : "OFF") << ")";
        cout <<GREEN<< "\n[11] Receipt Generation";
        cout <<GREEN<< "\n[12] Save & Exit";
        cout <<WHITE<< "\n==============================";
        cout <<WHITE<< "\nSelect: ";

        if (!(cin >> choice)) {
            clearInput();
            cout << "\n[ERROR 303] Invalid number.\n";
            continue;
        }
        clearInput(); // reset input buffer for getline use

        cout << endl;

        /*==========================================
                        DIRECT MODE (EASY WAY)
        ===========================================*/
        if (choice == 1) {
            cout << "Enter ID: ";
            getline(cin, id);
            cout << "Enter Name: ";
            getline(cin, name);
            cout << "Enter Quantity: ";
            cin >> qty; clearInput();
            cout << "Enter Location: ";
            getline(cin, location);

            wms.addNew(id, name, qty, location);
            if (autosave) wms.saveAll();
        }

        else if (choice == 2) {
            cout << "Enter ID: "; getline(cin, id);
            try {
                int itemId = stoi(id);
                wms.removeItem(itemId);
                if (autosave) wms.saveAll();
            } catch (...) {
                cout << "Invalid ID.\n";
            }
        }

        else if (choice == 3) {
            cout << "Enter ID: "; getline(cin, id);
            try {
                int itemId = stoi(id);
                auto *x = wms.searchItemInInventory(itemId);
                if (x) cout << "\n[FOUND]\n", printItem(const_cast<const Item&>(*x));
                else   cout << "\n[NOT FOUND]\n";
            } catch (...) {
                cout << "Invalid ID.\n";
            }
        }

        else if (choice == 4) wms.listInventoryItems();



        /*==========================================
                        QUEUE MODE (ADVANCED)
        ===========================================*/
        else if (choice == 5) {
            cout << "ID: "; getline(cin, id);
            cout << "Name: "; getline(cin, name);
            cout << "Qty: "; cin >> qty; clearInput();
            cout << "Location: "; getline(cin, location);

            wms.enqueueTask("ADD " + id + " " + name + " " + to_string(qty) + " " + location);
        }

        else if (choice == 6) {
            cout << "ID to remove: "; getline(cin, id);
            wms.enqueueTask("REMOVE " + id);
        }

        else if (choice == 7) {
            cout << "ID to search: "; getline(cin, id);
            wms.enqueueTask("SEARCH " + id);
        }

        else if (choice == 8) {
            wms.enqueueTask("LIST");
        }

        else if (choice == 9) {
            wms.processTasks();
            if (autosave) wms.saveAll();
        }

        /*==========================================
                    RECEIPT GENERATION
        ===========================================*/
        else if (choice == 11) {
            Receipt receipt;
            while (true) {
                cout << "Enter Item ID to add to receipt (or 'done' to finish): ";
                string input;
                getline(cin, input);
                if (input == "done") break;

                int itemId;
                try {
                    itemId = stoi(input);
                } catch (...) {
                    cout << "Invalid ID.\n";
                    continue;
                }

                Item* item = wms.searchItemInInventory(itemId);
                if (!item) {
                    cout << "Item not found.\n";
                    continue;
                }

                cout << "Enter quantity to add: ";
                int quantity;
                cin >> quantity; clearInput();

                if (quantity > item->getQuantity()) {
                    cout << "Not enough stock available.\n";
                    break;
                }

                receipt.addItem(*item, quantity);
                item->changeQuantity(-quantity);
                cout << "Added to receipt.\n";     // there is error in the non stopping loop i need to check it later 
            }
 
            receipt.print();
            receipt.saveToFile("receipt.csv");
            cout << "Receipt saved to 'receipt.csv'.\n";
            break;
        }


        /*==========================================
                     SYSTEM CONTROL
        ===========================================*/
        else if (choice == 10) {
            autosave = !autosave;
            cout << "[AUTO-SAVE] → " << (autosave ? "Enabled" : "Disabled") << endl;
        }

        else if (choice == 11) {
            cout << "\nQueue may still contain pending tasks!\n";
            cout << "Process them first? (y/n): ";

            char c; cin >> c;
            if (c == 'y' || c == 'Y') wms.processTasks();

            wms.saveAll();
            cout << "\nSaved. Shutting down...\n";
            break;
        }

        else cout << "\n[ERROR] Invalid selection.\n";
    }

    return 0;
}
