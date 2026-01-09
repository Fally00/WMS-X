// main.cpp — entry point for WMS CLI
#include "WmsControllers.h"
#include "output.h"

// VISUAL: CLI argument parsing (help/version/no-color).
#include "cli.h"

#include "registry.hpp"        // CommandRegistry
#include "parser.hpp"          // tokenize()
#include "command.hpp"         // ICommand
#include "addComand.hpp"      // Command implementations (add/remove/etc)
#include "CommandContext.hpp"  // CommandContext

#include <vector>

#include <iostream>
#include <memory>
#include <string>
#include <algorithm>

/*==========================================
     = Main command processing loop  
============================================*/

int processLine(CommandRegistry& reg, CommandContext& ctx, const std::string& line) {
    auto args = tokenize(line);
    if (args.empty()) return 0;

    std::string cmd = args[0];         // Extract command
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
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

    OutputFormatter::printSuccess("DONE");
    return 0;
}

/*==========================================
        = Main function 
============================================*/

int main(int argc, char* argv[]) {
    // VISUAL: enable nicer output, but keep it ASCII-only.
    CLIParser parser(argc, argv);
    CLIOptions opt = parser.parse();

    const bool enableColor = !opt.longFlags["no-color"];
    const bool autosave = opt.shortFlags['a'] || opt.longFlags["autosave"];
    OutputFormatter::initialize(enableColor);

    OutputFormatter::printLogo();
    OutputFormatter::printInfo("Warehouse Management System (WMS-X) v1.4.1");
    OutputFormatter::printInfo("Type 'help' for commands, 'exit' to quit.");

    const std::vector<std::pair<std::string, std::string>> commands = {
        {"add <id> <name> <quantity> <location>", "                                             Add new item"},
        {"remove <id>", "                                                                  Remove item by id"},
        {"list [page] [pageSize]", "                                                      List items (paged)"},
        {"search <id>", "Find item by id"},
        {"queue <COMMAND...>", "                                       Queue a task (ADD/REMOVE/LIST/SEARCH)"},
        {"runq [limit]", "                                                              Process queued tasks"},
        {"receipt <id quantity price>... [customer]", "           Generate & save a receipt (multiple lines)"},
        {"help", "                                                                            Show this help"},
        {"exit", "                                                                                  Quit WMS"},
        {"version/-v/--version", "                                                              Show version"},
        {"--help/-h", "                                                                       Show CLI help "},
        {"--no-color", "                                                              Disable colored output"},
        {"-a/--autosave", "                                                          Save after each command"},
    };

    if (opt.showHelp) {
        OutputFormatter::printHelp(commands);
        return 0;
    }
    if (opt.showVersion) {
        OutputFormatter::printVersion("1.4.1");
        return 0;
    }

    // Initialize system
    WmsControllers wms("inventory_data.json");
    if (!wms.initializeSystem()) {
        OutputFormatter::printError("Failed to initialize WMS. Exiting.");
        return 1;
    }

    // Register commands
    CommandRegistry registry;
    registry.registerCommand<AddCommand>("add");
    registry.registerCommand<RemoveCommand>("remove");
    registry.registerCommand<ListCommand>("list");
    registry.registerCommand<SearchCommand>("search");
    registry.registerCommand<QueueCommand>("queue");
    registry.registerCommand<ProcessQueueCommand>("runq");
    registry.registerCommand<ReceiptCommand>("receipt");

    // Execution context
    CommandContext ctx{
        .wms = wms,
        .autosave = autosave
    };

    // Non-interactive CLI execution
    if (!opt.interactive) {
        std::string cmd = opt.command;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "version" || cmd == "-v" || cmd == "--version") {
            OutputFormatter::printVersion("1.4.1");
            return 0;
        }

        ICommand* command = registry.get(cmd);
        if (!command) {
            OutputFormatter::printError("Unknown command: '" + cmd + "'. Type 'help' for commands.");
            return 1;
        }

        auto result = command->execute(ctx, opt.positionalArgs);
        if (!result.ok) {
            OutputFormatter::printError("Error: " + result.error);
            return 1;
        }

        OutputFormatter::printSuccess("DONE");
        return 0;
    }

    // REPL loop
    std::string input;
    while (true) {
        OutputFormatter::printPrompt("input> ");
        if (!std::getline(std::cin, input)) break;  // EOF (e.g., Ctrl+Z/D)

        input.erase(0, input.find_first_not_of(" \t"));  // Trim leading whitespace
        if (input.empty()) continue;

        if (input == "exit") {
            // VISUAL: ASCII-only output.
            OutputFormatter::printInfo("Goodbye!");
            break;
        }

        if (input == "help") {
            OutputFormatter::printHelp(commands);
            continue;
        }

        if (input == "version" || input == "-v" || input == "--version") {
            OutputFormatter::printVersion("1.4.1");
            continue;
        }

        processLine(registry, ctx, input);
    }

    return 0;
}