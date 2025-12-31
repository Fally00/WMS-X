#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// Structure to hold parsed CLI options
struct CLIOptions {
    std::string command;
    std::string subcommand;

    std::unordered_map<std::string, std::string> namedArgs;   // --id=5
    std::unordered_map<char, bool> shortFlags;                // -abc
    std::unordered_map<std::string, bool> longFlags;          // --autosave

    std::vector<std::string> positionalArgs;

    bool showHelp = false;
    bool showVersion = false;
    bool interactive = false;
};

// CLIParser is responsible for parsing command-line arguments
class CLIParser {
public:
    CLIParser(int argc, char* argv[]);
    CLIOptions parse();

private:
    int argc_;
    char** argv_;
};