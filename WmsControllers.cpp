#include "WmsControllers.h"
#include "Inventory.h"
#include "Storage.h"
#include "Receipt.h"
#include "Item.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;


/*==========================================
    INITIALIZATION + SAVE SYSTEM
============================================*/

WmsControllers::WmsControllers(const std::string& storageFilePath)
    : inventory(storageFilePath),storage(storageFilePath){}

bool WmsControllers::initializeSystem() {
    if (!storage.intializeStorage()) {
        cerr << "[ERROR] Could not initialize storage file!" << endl;
        return false;
    }

    const string csvData = storage.readAll();

    if (!csvData.empty()) {
        inventory.fromCSV(csvData);
        cout << "[SYSTEM READY] Inventory loaded from storage." << endl;
    } else {
        cout << "[SYSTEM READY] Storage empty. Fresh inventory initialized." << endl;
    }

    return true;
}
void WmsControllers::saveAll() {
    const string csvData = inventory.toCSV(storage.getFilePath());
    if (storage.writeAll(csvData)) {
        cout << "[SAVED] Inventory data saved to storage." << endl;
    } else {
        cerr << "[ERROR] Failed to save inventory data!" << endl;
    }
}

/*==========================================
    DIRECT INSTANT MODE (No Queue)
============================================*/
bool WmsControllers::addNew(int id, const std::string& name, int quantity, const std::string& location) {
    int itemId;
    try {
        itemId = (id);
    } catch (...) {
        cout << "[FAILED] Invalid ID." << endl;
        return false;
    }
    if (inventory.findItem(itemId)) {
        cout << "[FAILED] Item with ID already exists." << endl;
        return false;
    }
    inventory.addItem({itemId, name, quantity, location});
    cout << "[ADDED] " << itemId << " (" << name << ")" << endl;
    return true;
}

bool WmsControllers::removeItem(int itemId) {
    if (!inventory.findItem(itemId)) {
        cout << "[FAILED] Item not found." << endl;
        return false;
    }
    inventory.removeItem(itemId);
    cout << "[REMOVED] " << itemId << endl;
    return true;
}

Item* WmsControllers::searchItemInInventory(int itemId) {
    return inventory.findItem(itemId);
}

void WmsControllers::listInventoryItems() {
    inventory.displayItems();
}



/*==========================================
                 QUEUE SYSTEM 2.0
==========================================*/

void WmsControllers::enqueueTask(const string& task) {
    taskQueue.push(task);
    cout << "[QUEUED] " << task << endl;
}

// ------------------------------------------------------------
// Helper: split string by delimiter
// ------------------------------------------------------------
static vector<string> split(const string& s, char delim) {
    vector<string> parts;
    string temp;
    stringstream ss(s);

    while (getline(ss, temp, delim)) {
        if (!temp.empty())   
            parts.push_back(temp);
    }
    return parts;
}

//Process all tasks in the queue
void WmsControllers::processTasks() {
    cout << "\n========== PROCESSING TASK QUEUE ==========\n";

    while (!taskQueue.empty()) {
        string task = taskQueue.front();
        taskQueue.pop();

        cout << ">> " << task << endl;

        vector<string> tokens = split(task, ' ');

        if (tokens.empty()) {
            cout << "[FAILED] Empty command\n";
            continue;
        }

        // ======================================================= ADD
        if (tokens[0] == "ADD") {
            if (tokens.size() != 5) {
                cout << "[FAILED] Usage: ADD <id> <name> <qty> <location>\n";
                continue;
            }

            int id, quantity;
            try {
                id       = stoi(tokens[1]);
                quantity = stoi(tokens[3]);
            } catch (...) {
                cout << "[FAILED] Invalid numeric value\n";
                continue;
            }

            string name     = tokens[2];
            string location = tokens[4];

            inventory.addItem({id, name, quantity, location});
            cout << "[DONE] Added → " << id << "\n";
        }

        // ======================================================= REMOVE
        else if (tokens[0] == "REMOVE") {
            if (tokens.size() != 2) {
                cout << "[FAILED] Usage: REMOVE <id>\n";
                continue;
            }

            int id;
            try {
                id = stoi(tokens[1]);
            } catch (...) {
                cout << "[FAILED] Invalid ID\n";
                continue;
            }

            if (inventory.findItem(id)) {
                inventory.removeItem(id);
                cout << "[DONE] Removed → " << id << "\n";
            } else {
                cout << "[FAILED] Item not found\n";
            }
        }

        // ======================================================= LIST
        else if (tokens[0] == "LIST") {
            inventory.displayItems();
        }

        // ======================================================= SEARCH
        else if (tokens[0] == "SEARCH") {
            if (tokens.size() != 2) {
                cout << "[FAILED] Usage: SEARCH <id>\n";
                continue;
            }

            int id;
            try {
                id = stoi(tokens[1]);
            } catch (...) {
                cout << "[FAILED] Invalid ID\n";
                continue;
            }

            Item* item = inventory.findItem(id);
            if (item) {
                cout << "\n[FOUND]\n";
                printItem(*item);
            } else {
                cout << "[NOT FOUND]\n";
            }
        }

        // ======================================================= UNKNOWN Command
        else {
            cout << "[FAILED] Unknown command\n";
        }
    }
}
