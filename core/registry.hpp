#pragma once
#include <unordered_map>
#include <memory>
#include "core/command.hpp"

class CommandRegistry {
    std::unordered_map<std::string, std::unique_ptr<ICommand>> commands;

public:
    template<typename T>
    void registerCommand(const std::string& name) {
        commands[name] = std::make_unique<T>();
    }

    ICommand* get(const std::string& name) {
        auto it = commands.find(name);
        return (it != commands.end()) ? it->second.get() : nullptr;
    }
};