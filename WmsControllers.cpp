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
bool WmsControllers::initializeSystem() {
    if (!storage.intializeStorage()) {
        cerr << "[ERROR] Could not initialize storage file!" << endl;
        return false;
    }

    string data = storage.readAll();
    if (!data.empty()) inventory.fromCSV(data);

    cout << "[SYSTEM READY] Inventory loaded successfully." << endl;
    return true;
}

void WmsControllers::saveAll() {
    string csv = inventory.toCSV(storage.getFilePath());
    storage.writeAll(csv);
    cout << "[DATA SAVED] Inventory has been written to file." << endl;
}



/*==========================================
    DIRECT INSTANT MODE (No Queue)
============================================*/
bool WmsControllers::addNew(const string &id, const string &name, int quantity, const string &location) {
    int itemId;
    try {
        itemId = stoi(id);
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

void WmsControllers::enqueueTask(const string &task) {
    taskQueue.push(task);
    cout << "[QUEUED] " << task << endl;
}

static vector<string> split(const string &s, char delim) {
    vector<string> parts;
    string temp; stringstream ss(s);
    while (getline(ss, temp, delim)) parts.push_back(temp);
    return parts;
}

void WmsControllers::processTasks() {
    cout << "\n========== PROCESSING TASK QUEUE ==========\n";

    while (!taskQueue.empty()) {
        string task = taskQueue.front();
        taskQueue.pop();

        cout << ">> " << task << endl;

        vector<string> tokens = split(task, ' ');

        //======================================================= ADD
        if (tokens[0] == "ADD" && tokens.size() >= 5) {
            int id;
            try {
                id = stoi(tokens[1]);
            } catch (...) {
                cout << "[FAILED] Invalid ID\n";
                continue;
            }
            string name     = tokens[2];
            int quantity    = stoi(tokens[3]);
            string location = tokens[4];

            inventory.addItem({id, name, quantity, location});
            cout << "[DONE] Added → " << id << "\n";
        }

        //======================================================= REMOVE
        else if (tokens[0] == "REMOVE" && tokens.size() >= 2) {
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
            } else cout << "[FAILED] Not found\n";
        }

        //======================================================= LIST
        else if (tokens[0] == "LIST") {
            inventory.displayItems();
        }

        //======================================================= SEARCH
        else if (tokens[0] == "SEARCH" && tokens.size() >= 2) {
            int id;
            try {
                id = stoi(tokens[1]);
            } catch (...) {
                cout << "[FAILED] Invalid ID\n";
                continue;
            }
            Item* i = inventory.findItem(id);
            if (i) { cout << "\n[FOUND]\n"; printItem(const_cast<const Item&>(*i)); }
            else cout << "[NOT FOUND]\n";
        }

        //======================================================= SAVE
        else if (tokens[0] == "SAVE") {
            saveAll();
        }

        //======================================================= Unknown
        else {
            cout << "[ERROR] Unsupported or malformed task.\n";
        }
    }

    cout << "============================================\n";
}
