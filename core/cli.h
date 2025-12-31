#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct CLIOptions {
    std::string command;
    std::string subcommand;
    std::unordered_map<std::string, std::string> flags;
    std::vector<std::string> positionalArgs;
    bool showHelp = false;
    bool showVersion = false;
     bool interactive = false;
};

class CLIParser {
public:
    CLIParser(int argc, char* argv[]);
    CLIOptions parse();

private:
    int argc_;
    char** argv_;
    std::vector<std::string> args_;
};