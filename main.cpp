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

// Fancy WMS banner
void printWelcome() {
    cout << BLUE << " .----------------.  .----------------.  .----------------.    " << RESET << endl;
    cout << BLUE << " | .--------------. || .--------------. || .--------------. |  " << RESET << endl;
    cout << BLUE << " | | _____  _____ | || | ____    ____ | || |    _______   | |  " << RESET << endl;
    cout << BLUE << " | ||_   _||_   _|| || ||_   |  /   _|| || |   /  ___  |  | |  " << RESET << endl;
    cout << BLUE << " | |  | | /| | |  | || |  |   | /   | | || |  |  (__   |  | |  " << RESET << endl;
    cout << BLUE << " | |  | |/  || |  | || |  | | |  /| | | || |   '.___`-.   | |  " << RESET << endl;
    cout << BLUE << " | |  |   / |  |  | || | _| |_|/_| |_ | || |  |` ___) |   | |  " << RESET << endl;
    cout << BLUE << " | |  |__/  |__|  | || ||_____||_____|| || |  |_______.'  | |  " << RESET << endl;
    cout << BLUE << " | |              | || |              | || |              | |  " << RESET << endl;
    cout << BLUE << " | '--------------' || '--------------' || '--------------' |  " << RESET << endl;
    cout << BLUE << "  '----------------'  '----------------'  '----------------'   " << RESET << endl;
    cout << MAGENTA << "      WAREHOUSE MANAGEMENT SYSTEM (WMS)                     " << RESET << endl;
    cout << MAGENTA << "            Developed by Rayan Hisham & Abdelrahman Hany    " << RESET << endl;
}


// Helper to flush bad input
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    WmsControllers wms("inventory_data.csv");

    if (!wms.initializeSystem()) {
        cerr << "[ERROR 404] System initialization failed." << endl;
        return 1;
    }

    printWelcome(); // Display welcome banner

    int choice;
    bool autosave = false;
    string id, name, location;
    int qty;

    while (true) {

        cout << "\n\n==============================";
        cout << BLUE << "\n < WAREHOUSE MANAGEMENT HUB >" << RESET;
        cout << "\n==============================";
        cout << GREEN << "\n[1] Add Item (Instant)";
        cout << GREEN << "\n[2] Remove Item (Instant)";
        cout << GREEN << "\n[3] Find Item";
        cout << GREEN << "\n[4] List All Items";
        cout << "\n------------------------------";
        cout << CYAN << "\n[5] Queue Add";
        cout << CYAN << "\n[6] Queue Remove";
        cout << CYAN << "\n[7] Queue Search";
        cout << CYAN << "\n[8] Queue List";
        cout << CYAN << "\n[9] Process Queue";
        cout << "\n------------------------------";
        cout <<RED<< "\n[10] Auto-Save  (" << (autosave ? "ON" : "OFF") << ")";
        cout <<RED<< "\n[11] Receipt Generation";
        cout <<RED<< "\n[12] Save & Exit";
        cout << "\n==============================";
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
                if (x) cout << "\n[FOUND]\n", x->displayItem();
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

                if (quantity > item->quantity) {
                    cout << "Not enough stock available.\n";
                    continue;
                }

                receipt.addItem(*item, quantity);
                item->updateQuantity(-quantity);
                cout << "Added to receipt.\n";
            }

            receipt.print();
            receipt.saveToFile("receipt.csv");
            cout << "Receipt saved to 'receipt.csv'.\n";
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
