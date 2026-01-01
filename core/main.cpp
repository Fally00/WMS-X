// main.cpp — entry point for WMS CLI
#include "WmsControllers.h"
#include "output.h"

#include "registry.hpp"        // CommandRegistry
#include "parser.hpp"          // tokenize()
#include "command.hpp"         // ICommand
#include "addComand.hpp"      // AddCommand (note spelling!)
#include "commandcontext.hpp"  // CommandContext

#include <iostream>
#include <memory>
#include <string>

/*==========================================
     = Main command processing loop  
============================================*/

int processLine(CommandRegistry& reg, CommandContext& ctx, const std::string& line) {
    auto args = tokenize(line);
    if (args.empty()) return 0;

    std::string cmd = args[0];         // Extract command
    args.erase(args.begin());          // Remove command name

    ICommand* command = reg.get(cmd);
    if (!command) {
        OutputFormatter::printError("Unknown command: '" + cmd + "'. Type 'help' for commands.");
        return 1;
    }

    auto result = command->execute(ctx, args);
    if (!result.ok) {
        OutputFormatter::printError("Error: " + result.error);
        return 1;
    }

    OutputFormatter::printSuccess("OK");
    return 0;
}

/*==========================================
        = Main function 
============================================*/

int main() {
    std::cout << " Warehouse Management System (WMS-X) v1.4.1\n";
    std::cout << "Type 'help' for available commands, 'exit' to quit.\n\n";

    // Initialize system
    WmsControllers wms("inventory_data.csv");
    if (!wms.initializeSystem()) {
        OutputFormatter::printError("Failed to initialize WMS. Exiting.");
        return 1;
    }

    // Register commands
    CommandRegistry registry;
    registry.registerCommand<AddCommand>("add");

    // Execution context
    CommandContext ctx{
        .wms = wms,
        .autosave = false   // Set to true if you want auto-persistence
    };

    // REPL loop
    std::string input;
    while (true) {
        std::cout << "WMS> ";
        if (!std::getline(std::cin, input)) break;  // EOF (e.g., Ctrl+Z/D)

        input.erase(0, input.find_first_not_of(" \t"));  // Trim leading whitespace
        if (input.empty()) continue;

        if (input == "exit") {
            std::cout << "👋 Goodbye!\n";
            break;
        }

        if (input == "help") {
            std::cout << "Available commands:\n"
                      << "  add <id> <name> <qty> <loc>   – Add new item\n"
                      << "  exit                          – Quit WMS\n"
                      << "  help                          – Show this help\n";
            continue;
        }

        processLine(registry, ctx, input);
    }

    return 0;
}