#include "cli.h"
#include <iostream>
#include <algorithm>

/*==========================================
    CLI ARGUMENT PARSER
============================================*/

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
CLIParser::CLIParser(int argc, char* argv[]) : argc_(argc), argv_(argv) {
    for (int i = 1; i < argc; ++i) {
        args_.push_back(argv[i]);
    }
}

// ─────────────────────────────────────────────
// Parse command line arguments
// ─────────────────────────────────────────────
CLIOptions CLIParser::parse() {
    CLIOptions options;

    // Check for help and version first
    for (const auto& arg : args_) {
        if (arg == "--help" || arg == "-h") {
            options.showHelp = true;
            return options;
        }
        if (arg == "--version" || arg == "-v") {
            options.showVersion = true;
            return options;
        }
    }

    if (args_.empty()) {
        options.interactive = true;
        return options;
    }

    // First argument is the command
    options.command = args_[0];

    // Check if command contains a subcommand (e.g., "queue-add")
    size_t dashPos = options.command.find('-');
    if (dashPos != std::string::npos) {
        options.subcommand = options.command.substr(dashPos + 1);
        options.command = options.command.substr(0, dashPos);
    }

    // Parse flags and positional arguments
    for (size_t i = 1; i < args_.size(); ++i) {
        const std::string& arg = args_[i];

        if (arg.substr(0, 2) == "--") {
            // Long flag
            size_t equalsPos = arg.find('=');
            if (equalsPos != std::string::npos) {
                std::string key = arg.substr(2, equalsPos - 2);
                std::string value = arg.substr(equalsPos + 1);
                options.flags[key] = value;
            } else {
                std::string key = arg.substr(2);
                if (i + 1 < args_.size() && args_[i + 1][0] != '-') {
                    options.flags[key] = args_[i + 1];
                    ++i;
                } else {
                    options.flags[key] = "true";
                }
            }
        } else if (arg[0] == '-') {
            // Short flag
            if (arg.size() == 2) {
                char flag = arg[1];
                if (i + 1 < args_.size() && args_[i + 1][0] != '-') {
                    options.flags[std::string(1, flag)] = args_[i + 1];
                    ++i;
                } else {
                    options.flags[std::string(1, flag)] = "true";
                }
            }
        } else {
            // Positional argument
            options.positionalArgs.push_back(arg);
        }
    }

    return options;
}