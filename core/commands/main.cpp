#include "core/WmsControllers.h"
#include "core/output.h"

#include "core/commands/registry.hpp"
#include "core/commands/parser.hpp"
#include "core/commands/addComand.hpp"

#include <iostream>

/*==========================================
     = Main command processing loop  
============================================*/

int processLine(CommandRegistry& reg, CommandContext& ctx, const std::string& line) {
    auto args = tokenize(line);
    if (args.empty()) return 0;

    std::string cmd = args[0];         // Extract command
    args.erase(args.begin());         // Remove command from args
 
    auto* c = reg.get(cmd);
    if (!c) {                        // Command not found
        OutputFormatter::printError("Unknown command: " + cmd);
        return 1;
    }

    auto res = c->execute(ctx, args);
    if (!res.ok) {                 // Command execution failed  
        OutputFormatter::printError(res.error);
        return 1;
    }
    return 0;
}

/*==========================================
        = Main function 
============================================*/

int main() {
    WmsControllers wms("inventory_data.csv");
    if (!wms.initializeSystem()) return 1;

    CommandRegistry reg;                       // Command registry
    reg.add("add", std::make_unique<AddCommand>());

    CommandContext ctx{wms, false};         // autosave disabled for demo

    std::string line;
    while (true) {
        std::cout << "WMS> ";
        std::getline(std::cin, line);
        if (line == "exit") break;
        processLine(reg, ctx, line);
    }
}
