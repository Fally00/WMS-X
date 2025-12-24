#include "WmsControllers.h"
#include "Inventory.h"
#include "Storage.h"
#include "Item.h"
#include "Receipt.h"
#include "cli.h"
#include "output.h"
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
using namespace std;

/*==========================================
    CLI MAIN ENTRY POINT
============================================*/

int runInteractiveMode();

int main(int argc, char* argv[]) {
    CLIParser parser(argc, argv);
    CLIOptions options = parser.parse();

    if (options.showVersion) {
        OutputFormatter::printVersion();
        std::cout << "\nPress Enter to exit...";
        std::cin.get();
        return 0;
    }

    if (options.showHelp) {
        OutputFormatter::printHelp();
        return 0;
    }

    // Check for interactive mode (no arguments provided)
    if (options.interactive) {
        return runInteractiveMode();
    }

    OutputFormatter::printLogo();

    WmsControllers wms("inventory_data.csv");
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

    // Handle commands
    if (options.command == "add") {
        if (options.positionalArgs.size() < 4) {
            OutputFormatter::printError("Usage: wms add <id> <name> <quantity> <location>");
            return 1;
        }

        int id = stoi(options.positionalArgs[0]);
        string name = options.positionalArgs[1];
        int quantity = stoi(options.positionalArgs[2]);
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
        }
    }
    else if (options.command == "remove") {
        if (options.positionalArgs.size() < 1) {
            OutputFormatter::printError("Usage: wms remove <id>");
            return 1;
        }

        int id = stoi(options.positionalArgs[0]);
        if (wms.removeItem(id)) {
            OutputFormatter::printSuccess("Item removed successfully");
            if (autosave) wms.saveAll();
        } else {
            OutputFormatter::printError("Item not found");
        }
    }
    else if (options.command == "find") {
        if (options.positionalArgs.size() < 1) {
            OutputFormatter::printError("Usage: wms find <id>");
            return 1;
        }

        int id = stoi(options.positionalArgs[0]);
        Item* item = wms.searchItemInInventory(id);
        if (item) {
            OutputFormatter::printInfo("Item found:");
            printItem(*item);
        } else {
            OutputFormatter::printError("Item not found");
        }
    }
    else if (options.command == "list") {
        wms.listInventoryItems();
    }
    else if (options.command == "queue") {
        string operation = options.subcommand;
        if (operation.empty()) {
            if (options.positionalArgs.size() < 1) {
                OutputFormatter::printError("Usage: wms queue <operation> [args...]");
                return 1;
            }
            operation = options.positionalArgs[0];
        }

        if (operation == "add") {
            if (options.positionalArgs.size() < 4) {
                OutputFormatter::printError("Usage: wms queue add <id> <name> <quantity> <location>");
                return 1;
            }
            try {
                int id = stoi(options.positionalArgs[0]);
                string name = options.positionalArgs[1];
                int quantity = stoi(options.positionalArgs[2]);
                string location = options.positionalArgs[3];
                wms.enqueueAddTask(id, name, quantity, location);
                OutputFormatter::printSuccess("Add operation queued");
            } catch (const std::invalid_argument&) {
                OutputFormatter::printError("Invalid numeric input for ID or quantity");
                return 1;
            }
        }
        else if (operation == "remove") {
            if (options.positionalArgs.size() < 1) {
                OutputFormatter::printError("Usage: wms queue remove <id>");
                return 1;
            }
            try {
                int id = stoi(options.positionalArgs[0]);
                wms.enqueueRemoveTask(id);
                OutputFormatter::printSuccess("Remove operation queued");
            } catch (const std::invalid_argument&) {
                OutputFormatter::printError("Invalid numeric input for ID");
                return 1;
            }
        }
        else {
            OutputFormatter::printError("Invalid queue operation: " + operation);
        }
    }
    else if (options.command == "process") {
        wms.processTasks();
        OutputFormatter::printSuccess("Tasks processed");
        if (autosave) wms.saveAll();
    }
    else if (options.command == "save") {
        wms.saveAll();
        OutputFormatter::printSuccess("Data saved");
    }
    else {
        OutputFormatter::printError("Unknown command: " + options.command);
        OutputFormatter::printInfo("Use 'wms --help' for available commands");
        return 1;
    }

    return 0;
}

/*==========================================
    INTERACTIVE MODE FALLBACK
============================================*/

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

        // Simple command parsing
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
