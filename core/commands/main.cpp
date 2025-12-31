#include "core/WmsControllers.h"
#include "core/Inventory.h"
#include "core/Storage.h"
#include "core/Item.h"
#include "core/Receipt.h"
#include "core/cli.h"
#include "core/output.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
using namespace std;

/*==========================================
    CLI MAIN ENTRY POINT
============================================*/

// Forward declarations
int runInteractiveMode();
void printItem(const Item& item);

/**
 * @brief Prints detailed information about an item
 * @param item The item to display information for
 */
void printItem(const Item& item) {
    cout << "ID: " << item.getId() << ", Name: " << item.getName()
         << ", Quantity: " << item.getQuantity() << ", Location: " << item.getLocation() << endl;
}

/**
 * @brief Safely parses a string to integer with error handling
 * @param str The string to convert to integer
 * @param paramName The parameter name for error messages
 * @return The parsed integer value
 * @throws invalid_argument if string cannot be converted
 */
int parseInt(const string& str, const string& paramName) {
    try {
        return stoi(str);
    } catch (const invalid_argument&) {
        OutputFormatter::printError("Invalid numeric input for " + paramName);
        throw;
    }
}

/**
 * @brief Validates the number of arguments against expected count
 * @param options The CLI options containing arguments
 * @param expectedCount The minimum number of arguments required
 * @param usage The usage string to display if validation fails
 * @return True if validation passes, false otherwise
 */
bool validateArgCount(const CLIOptions& options, size_t expectedCount, const string& usage) {
    if (options.positionalArgs.size() < expectedCount) {
        OutputFormatter::printError("Usage: " + usage);
        return false;
    }
    return true;
}

/**
 * @brief Handles the 'add' command to add new items to inventory
 * @param wms Reference to the WMS controller
 * @param options The CLI options containing command arguments
 * @param autosave Flag indicating whether to save after operation
 * @return 0 on success, 1 on failure
 */
int handleAdd(WmsControllers& wms, const CLIOptions& options, bool autosave) {
    if (!validateArgCount(options, 4, "wms add <id> <name> <quantity> <location>")) {
        return 1;
    }

    try {
        int id = parseInt(options.positionalArgs[0], "ID");
        string name = options.positionalArgs[1];
        int quantity = parseInt(options.positionalArgs[2], "quantity");
        string location = options.positionalArgs[3];

        // Check if item already exists
        if (wms.searchItemInInventory(id)) {
            OutputFormatter::printError("Item with ID " + to_string(id) + " already exists");
            return 1;
        }

        if (wms.addItem(id, name, quantity, location)) {
            OutputFormatter::printSuccess("Item added successfully");
            if (autosave) wms.saveAll();
        } else {
            OutputFormatter::printError("Failed to add item");
            return 1;
        }
    } catch (const invalid_argument&) {
        return 1; // Error already printed by parseInt
    }
    return 0;
}

/**
 * @brief Handles the 'remove' command to remove items from inventory
 * @param wms Reference to the WMS controller
 * @param options The CLI options containing command arguments
 * @param autosave Flag indicating whether to save after operation
 * @return 0 on success, 1 on failure
 */
int handleRemove(WmsControllers& wms, const CLIOptions& options, bool autosave) {
    if (!validateArgCount(options, 1, "wms remove <id>")) {
        return 1;
    }

    try {
        int id = parseInt(options.positionalArgs[0], "ID");
        if (wms.removeItem(id)) {
            OutputFormatter::printSuccess("Item removed successfully");
            if (autosave) wms.saveAll();
        } else {
            OutputFormatter::printError("Item not found");
            return 1;
        }
    } catch (const invalid_argument&) {
        return 1;
    }
    return 0;
}

/**
 * @brief Handles the 'find' command to search for items in inventory
 * @param wms Reference to the WMS controller
 * @param options The CLI options containing command arguments
 * @return 0 on success, 1 on failure
 */
int handleFind(WmsControllers& wms, const CLIOptions& options) {
    if (!validateArgCount(options, 1, "wms find <id>")) {
        return 1;
    }

    try {
        int id = parseInt(options.positionalArgs[0], "ID");
        Item* item = wms.searchItemInInventory(id);
        if (item) {
            OutputFormatter::printInfo("Item found:");
            printItem(*item);
        } else {
            OutputFormatter::printError("Item not found");
            return 1;
        }
    } catch (const invalid_argument&) {
        return 1;
    }
    return 0;
}

/**
 * @brief Handles the 'queue' command to queue operations for later processing
 * @param wms Reference to the WMS controller
 * @param options The CLI options containing command arguments
 * @return 0 on success, 1 on failure
 */
int handleQueue(WmsControllers& wms, const CLIOptions& options) {
    if (options.positionalArgs.empty()) {
        OutputFormatter::printError("Usage: wms queue <operation> [args...]");
        return 1;
    }

    string operation = options.positionalArgs[0];

    if (operation == "add") {
        if (options.positionalArgs.size() < 5) {
            OutputFormatter::printError("Usage: wms queue add <id> <name> <quantity> <location>");
            return 1;
        }

        try {
            int id = parseInt(options.positionalArgs[1], "ID");
            string name = options.positionalArgs[2];
            int quantity = parseInt(options.positionalArgs[3], "quantity");
            string location = options.positionalArgs[4];
            wms.enqueueAddTask(id, name, quantity, location);
            OutputFormatter::printSuccess("Add operation queued");
        } catch (const std::invalid_argument&) {
            OutputFormatter::printError("Invalid numeric input for ID or quantity");
            return 1;
        }
    }
    else if (operation == "remove") {
        if (options.positionalArgs.size() < 2) {
            OutputFormatter::printError("Usage: wms queue remove <id>");
            return 1;
        }

        try {
            int id = parseInt(options.positionalArgs[1], "ID");
            wms.enqueueRemoveTask(id);
            OutputFormatter::printSuccess("Remove operation queued");
        } catch (const std::invalid_argument&) {
            OutputFormatter::printError("Invalid numeric input for ID");
            return 1;
        }
    }
    else {
        OutputFormatter::printError("Invalid queue operation: " + operation);
        return 1;
    }
    return 0;
}

/**
 * @brief Main command dispatch function that routes commands to appropriate handlers
 * @param wms Reference to the WMS controller
 * @param options The CLI options containing command information
 * @param autosave Flag indicating whether to save after operations
 * @return 0 on success, 1 on failure
 */
int mainCommandLoop(WmsControllers& wms, const CLIOptions& options, bool autosave) {
    if (options.command == "add") {
        return handleAdd(wms, options, autosave);
    }
    else if (options.command == "remove") {
        return handleRemove(wms, options, autosave);
    }
    else if (options.command == "find") {
        return handleFind(wms, options);
    }
    else if (options.command == "list") {
        wms.listInventoryItems();
        return 0;
    }
    else if (options.command == "queue") {
        return handleQueue(wms, options);
    }
    else if (options.command == "process") {
        wms.processTasks();
        OutputFormatter::printSuccess("Tasks processed");
        if (autosave) wms.saveAll();
        return 0;
    }
    else if (options.command == "save") {
        wms.saveAll();
        OutputFormatter::printSuccess("Data saved");
        return 0;
    }
    else {
        OutputFormatter::printError("Unknown command: " + options.command);
        OutputFormatter::printInfo("Use 'wms --help' for available commands");
        return 1;
    }
}

/**
 * @brief Main entry point of the CLI application
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return 0 on success, 1 on failure
 */
int main(int argc, char* argv[]) {
    CLIParser parser(argc, argv);          // Create parser instance
    CLIOptions options = parser.parse();  // Parse arguments

    if (options.showVersion) {           // Show version info
        OutputFormatter::printVersion();
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 0;
    }

    if (options.showHelp) {          // Show help info
        OutputFormatter::printHelp();
        return 0;
    }

    // Check for interactive mode (no arguments provided)
    if (options.interactive) {
        return runInteractiveMode();
    }

    OutputFormatter::printLogo();         // Print logo at start

    WmsControllers wms("inventory_data.csv");      // Initialize WMS controller
    if (!wms.initializeSystem()) {
        OutputFormatter::printError("System initialization failed.");
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 1;
    }

    OutputFormatter::printSuccess("System initialized successfully");

    bool autosave = options.flags.count("autosave") > 0;

    if (autosave) {
        OutputFormatter::printInfo("Autosave enabled");
    }

    return mainCommandLoop(wms, options, autosave);
}

/*==========================================
    INTERACTIVE MODE FALLBACK
============================================*/

/**
 * @brief Runs the interactive command-line interface mode
 * @return 0 on successful exit, 1 on failure
 */
int runInteractiveMode() {
    OutputFormatter::printLogo();

    WmsControllers wms("inventory_data.csv");
    if (!wms.initializeSystem()) {
        OutputFormatter::printError("System initialization failed.");
        return 1;
    }

    OutputFormatter::printSuccess("System initialized successfully");
    OutputFormatter::printInfo("Type 'help' for available commands or 'exit' to quit.");

    bool autosave = false;
    string input;

    while (true) {
        cout << "\nStart> ";
        getline(cin, input);

        if (input.empty()) continue;

        // Parse command and arguments
        vector<string> args;
        stringstream ss(input);
        string token;
        while (ss >> token) {
            args.push_back(token);
        }

        if (args.empty()) continue;

        string command = args[0];

        if (command == "exit" || command == "quit") {
            cout << "\nQueue may still contain pending tasks!\n";
            cout << "Process them first? (y/n): ";
            string response;
            getline(cin, response);
            if (response == "y" || response == "Y") wms.processTasks();
            wms.saveAll();
            cout << "\nSaved. Shutting down...\n";
            break;
        }
        else if (command == "help") {
            OutputFormatter::printHelp();
        }
        else if (command == "add") {
            if (args.size() < 5) {
                OutputFormatter::printError("Usage: add <id> <name> <quantity> <location>");
                continue;
            }
            try {
                int id = stoi(args[1]);
                string name = args[2];
                int quantity = stoi(args[3]);
                string location = args[4];
                if (wms.addItem(id, name, quantity, location)) {
                    OutputFormatter::printSuccess("Item added successfully");
                    if (autosave) wms.saveAll();
                } else {
                    OutputFormatter::printError("Failed to add item (ID may already exist)");
                }
            } catch (const invalid_argument&) {
                OutputFormatter::printError("Invalid numeric input for ID or quantity");
            }
        }
        else if (command == "remove") {
            if (args.size() < 2) {
                OutputFormatter::printError("Usage: remove <id>");
                continue;
            }
            try {
                int id = stoi(args[1]);
                if (wms.removeItem(id)) {
                    OutputFormatter::printSuccess("Item removed successfully");
                    if (autosave) wms.saveAll();
                } else {
                    OutputFormatter::printError("Item not found");
                }
            } catch (const invalid_argument&) {
                OutputFormatter::printError("Invalid numeric input for ID");
            }
        }
        else if (command == "find") {
            if (args.size() < 2) {
                OutputFormatter::printError("Usage: find <id>");
                continue;
            }
            try {
                int id = stoi(args[1]);
                Item* item = wms.searchItemInInventory(id);
                if (item) {
                    OutputFormatter::printInfo("Item found:");
                    printItem(*item);
                } else {
                    OutputFormatter::printError("Item not found");
                }
            } catch (const invalid_argument&) {
                OutputFormatter::printError("Invalid numeric input for ID");
            }
        }
        else if (command == "list") {
            wms.listInventoryItems();
        }
        else if (command == "queue") {
            if (args.size() < 2) {
                OutputFormatter::printError("Usage: queue <add|remove> [args...]");
                continue;
            }
            string op = args[1];
            if (op == "add") {
                if (args.size() < 6) {
                    OutputFormatter::printError("Usage: queue add <id> <name> <quantity> <location>");
                    continue;
                }
                try {
                    int id = stoi(args[2]);
                    string name = args[3];
                    int quantity = stoi(args[4]);
                    string location = args[5];
                    wms.enqueueAddTask(id, name, quantity, location);
                    OutputFormatter::printSuccess("Add operation queued");
                } catch (const invalid_argument&) {
                    OutputFormatter::printError("Invalid numeric input for ID or quantity");
                }
            }
            else if (op == "remove") {
                if (args.size() < 3) {
                    OutputFormatter::printError("Usage: queue remove <id>");
                    continue;
                }
                try {
                    int id = stoi(args[2]);
                    wms.enqueueRemoveTask(id);
                    OutputFormatter::printSuccess("Remove operation queued");
                } catch (const invalid_argument&) {
                    OutputFormatter::printError("Invalid numeric input for ID");
                }
            }
            else {
                OutputFormatter::printError("Invalid queue operation: " + op);
            }
        }
        else if (command == "process") {
            wms.processTasks();
            OutputFormatter::printSuccess("Tasks processed");
            if (autosave) wms.saveAll();
        }
        else if (command == "save") {
            wms.saveAll();
            OutputFormatter::printSuccess("Data saved");
        }
        else if (command == "autosave") {
            autosave = !autosave;
            cout << "[AUTO-SAVE] : " << (autosave ? "Enabled" : "Disabled") << endl;
        }
        else {
            OutputFormatter::printError("Unknown command: " + command);
            OutputFormatter::printInfo("Type 'help' for available commands");
        }
    }

    return 0;
}