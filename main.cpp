#include "WmsControllers.h"
#include "Inventory.h"
#include "Storage.h"
#include "Item.h"
#include "Receipt.h"
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
    cout << MAGENTA << "           Devoloped by Rayan Hisham & Abdulrahman Shaalan" << RESET << endl;

    WmsControllers wms("inventory_data.csv");
    wms.initializeSystem();
    if (!wms.initializeSystem()) {
        cerr << RED << "[ERROR 404] System initialization failed." << endl;
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
            cout << RED << "\n[ERROR 303] Invalid number.\n";
            continue;
        }
        clearInput(); // reset input buffer for getline use

        cout << endl;

        /*==========================================
                DIRECT MODE (Enqueue embedded)
        ===========================================*/
        if (choice == 1) {
            string id, name, location;
            int qty;

            cout << "Enter ID: ";
            getline(cin, id);

            int numericId;
            try {
                numericId = stoi(id);
            } catch (...) {
                cout << RED << "Invalid ID. Must be a number.\n";
                break;
            }

            cout << "Enter Name: ";
            getline(cin, name);

            cout << "Enter Quantity: ";
            cin >> qty; clearInput();

            cout << "Enter Location: ";
            getline(cin, location);

            wms.enqueueAddTask(numericId, name, qty, location, true);
            wms.processTasks();
            if (autosave) wms.saveAll();
        }


        else if (choice == 2) {
            cout << "Enter ID: "; getline(cin, id);
            try {
                int itemId = stoi(id);
                wms.enqueueRemoveTask(itemId, true);
                wms.processTasks();
                if (autosave) wms.saveAll();
            } catch (...) {
                cout << RED << "Invalid ID.\n";
            }
        }

        else if (choice == 3) {
            cout << "Enter ID: "; getline(cin, id);
            try {
                int itemId = stoi(id);
                wms.enqueueSearchTask(itemId, true);
                wms.processTasks();
            } catch (...) {
                cout <<RED<< "Invalid ID.\n";
            }
        }

        else if (choice == 4) {
            wms.enqueueListTask(true);
            wms.processTasks();
        }



        /*==========================================
                        QUEUE MODE (ADVANCED)
        ==========================================*/

        else if (choice == 5) {
            cout << "ID: "; 
            getline(cin, id);
            
            cout << "Name: "; 
            getline(cin, name);
            
            cout << "Qty: "; 
            cin >> qty; 
            clearInput();
            
            cout << "Location: "; 
            getline(cin, location);

            // Use the new convenience method
            wms.enqueueAddTask(stoi(id), name, qty, location);
        }

        else if (choice == 6) {
            cout << "ID to remove: "; 
            getline(cin, id);
            
            try {
                int itemId = stoi(id);
                wms.enqueueRemoveTask(itemId);
            } catch (const std::exception& e) {
                cout << "[ERROR] Invalid ID format: " << e.what() << endl;
            }
        }

        else if (choice == 7) {
            cout << "ID to search: "; 
            getline(cin, id);
            
            try {
                int itemId = stoi(id);
                wms.enqueueSearchTask(itemId);
            } catch (const std::exception& e) {
                cout << "[ERROR] Invalid ID format: " << e.what() << endl;
            }
        }

        else if (choice == 8) {
            wms.enqueueListTask();
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
                cout << "Added to receipt.\n";     
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
            cout << "[AUTO-SAVE] : " << (autosave ? "Enabled" : "Disabled") << endl;
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
