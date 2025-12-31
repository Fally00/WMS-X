#pragma once
#include "core/commands/command.hpp"
#include <unordered_map>
#include <memory>

// Registry for commands
class CommandRegistry {
    // Map of command names to command instances
    std::unordered_map<std::string, std::unique_ptr<ICommand>> cmds;
public:
    // Add a command to the registry
    void add(const std::string& n, std::unique_ptr<ICommand> c) {
        cmds[n] = std::move(c);
    }

    // Retrieve command by name
    ICommand* get(const std::string& n) {
        return cmds.count(n) ? cmds[n].get() : nullptr;
    }
};
