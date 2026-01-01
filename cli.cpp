#include "cli.h"
#include <algorithm>
#include <cctype>
#include <iostream>

// Helper function to convert string to lowercase
static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Constructor
CLIParser::CLIParser(int argc, char* argv[]) : argc_(argc), argv_(argv) {}

// Simple CLI parser implementation
CLIOptions CLIParser::parse() {
    CLIOptions opt;

    for (int i = 1; i < argc_; i++) {
        std::string arg = argv_[i];

        if (arg == "--help" || arg == "-h") {
            opt.showHelp = true;
            return opt;
        }

        if (arg == "--version" || arg == "-v") {
            opt.showVersion = true;
            return opt;
        }

        // --key=value
        if (arg.rfind("--", 0) == 0) {
            auto eq = arg.find('=');
            if (eq != std::string::npos) {
                std::string key = lower(arg.substr(2, eq - 2));
                std::string val = arg.substr(eq + 1);
                opt.namedArgs[key] = val;
            }
            else {
                opt.longFlags[lower(arg.substr(2))] = true;
            }
            continue;
        }

        // -abc  => -a -b -c
        if (arg.rfind("-", 0) == 0) {
            for (size_t j = 1; j < arg.size(); j++)
                opt.shortFlags[arg[j]] = true;
            continue;
        }

        if (opt.command.empty()) opt.command = lower(arg);
        else opt.positionalArgs.push_back(arg);
    }

    opt.interactive = opt.command.empty();
    return opt;
}
