#pragma once
#include "core/commands/command.hpp"
#include <unordered_map>
#include <memory>

class CommandRegistry {
    std::unordered_map<std::string, std::unique_ptr<ICommand>> cmds;
public:
    void add(const std::string& n, std::unique_ptr<ICommand> c) {
        cmds[n] = std::move(c);
    }
    ICommand* get(const std::string& n) {
        return cmds.count(n) ? cmds[n].get() : nullptr;
    }
};
