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

    if (argc_ < 2) {
        opt.interactive = true;
        return opt;
    }
    
    // First argument is the command
    opt.command = lower(argv_[1]);

    for (int i = 2; i < argc_; i++) {
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
        }

        // -abc  => -a -b -c
        else if (arg.rfind("-", 0) == 0) {
            for (size_t j = 1; j < arg.size(); j++)
                opt.shortFlags[arg[j]] = true;
        }

        else {
            opt.positionalArgs.push_back(arg);
        }
    }

    return opt;
}
