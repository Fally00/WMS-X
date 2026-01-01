#include "WmsControllers.h"
#include "Item.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <cctype>

using namespace std;

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
WmsControllers::WmsControllers(const string& storagePath)
    : inventory(storagePath), storage(storagePath) {

    commandRegistry["ADD"]    = [this](const Task& t){ return cmdAdd(t); };
    commandRegistry["REMOVE"] = [this](const Task& t){ return cmdRemove(t); };
    commandRegistry["LIST"]   = [this](const Task& t){ return cmdList(t); };
    commandRegistry["SEARCH"] = [this](const Task& t){ return cmdSearch(t); };
}

// ─────────────────────────────────────────────
// Init
// ─────────────────────────────────────────────
bool WmsControllers::initializeSystem() {
    if (auto err = storage.initializeStorage()) {
        cerr << "[STORAGE ERROR] " << err.value().message << endl;
        return false;
    }

    string e;
    auto data = storage.readAll(e);
    if (data) inventory.fromCSV(*data);

    return true;
}

void WmsControllers::saveAll() {
    storage.atomicWrite(inventory.toCSV());
}

bool WmsControllers::addItem(int id, const string& name, int qty, const string& loc) {
    if (qty < 0) return false;
    if (inventory.findItem(id)) return false;

    Item item(id, name, qty, loc);
    return inventory.addItem(item);
}

bool WmsControllers::removeItem(int id) {
    if (!inventory.findItem(id)) return false;
    return inventory.removeItem(id);
}

void WmsControllers::listItems(size_t page, size_t pageSize) {
    inventory.displayItems(page, pageSize);
}

std::optional<Item> WmsControllers::getItem(int id) {
    if (auto* item = inventory.findItem(id)) return *item;
    return std::nullopt;
}

// ─────────────────────────────────────────────
// Task ID generator
// ─────────────────────────────────────────────
string WmsControllers::generateTaskId() const {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(10000, 99999);
    return "TSK-" + to_string(dis(gen));
}

// ─────────────────────────────────────────────
// Smart split (quoted strings supported)
// ─────────────────────────────────────────────
vector<string> WmsControllers::smartSplit(const string& input) {
    vector<string> out;
    bool inQuotes = false;
    string temp;

    for (char c : input) {
        if (c == '"') inQuotes = !inQuotes;
        else if (isspace(c) && !inQuotes) {
            if (!temp.empty()) { out.push_back(temp); temp.clear(); }
        } else temp += c;
    }
    if (!temp.empty()) out.push_back(temp);
    return out;
}

bool WmsControllers::isNumeric(const string& s) {
    if (s.empty()) return false;
    size_t i = (s[0]=='+'||s[0]=='-')?1:0;
    for (; i<s.size(); ++i) if (!isdigit(s[i])) return false;
    return true;
}

// ─────────────────────────────────────────────
// Enqueue task
// ─────────────────────────────────────────────
void WmsControllers::enqueueTask(const string& raw, TaskPriority prio) {
    auto parts = smartSplit(raw);
    if (parts.empty()) return;

    Task t;
    t.id = generateTaskId();
    t.command = parts[0];
    t.params.assign(parts.begin()+1, parts.end());
    t.priority = prio;
    t.created = chrono::system_clock::now();

    taskQueue.push(t);
    cout << "[QUEUED] " << t.id << " :: " << raw << endl;
}

size_t WmsControllers::queueSize() const {
    return taskQueue.size();
}

// ─────────────────────────────────────────────
// Command handlers
// ─────────────────────────────────────────────
bool WmsControllers::cmdAdd(const Task& t) {
    if (t.params.size()!=4) return false;
    if (!isNumeric(t.params[0]) || !isNumeric(t.params[2])) return false;

    int id=stoi(t.params[0]), qty=stoi(t.params[2]);
    if (qty<0 || inventory.findItem(id)) return false;

    inventory.addItem({id,t.params[1],qty,t.params[3]});
    return true;
}

bool WmsControllers::cmdRemove(const Task& t) {
    if (t.params.size()!=1 || !isNumeric(t.params[0])) return false;
    int id=stoi(t.params[0]);
    if (!inventory.findItem(id)) return false;
    inventory.removeItem(id);
    return true;
}

bool WmsControllers::cmdList(const Task&) {
    inventory.displayItems();
    return true;
}

bool WmsControllers::cmdSearch(const Task& t) {
    if (t.params.size()!=1 || !isNumeric(t.params[0])) return false;
    if (auto* item=inventory.findItem(stoi(t.params[0])))
        printItem(*item);
    return true;
}

// ─────────────────────────────────────────────
// Process queue
// ─────────────────────────────────────────────
void WmsControllers::processTasks(size_t limit) {
    size_t done=0;
    while(!taskQueue.empty() && (limit==0 || done<limit)) {
        Task t=taskQueue.top(); taskQueue.pop();
        auto it=commandRegistry.find(t.command);
        if(it!=commandRegistry.end()) it->second(t);
        done++;
    }
}
