#pragma once
#include "core/results.hpp"
#include <vector>
#include <string>

struct CommandContext; // Forward declare to avoid circularity if needed

// Interface for commands
struct ICommand {
    virtual ~ICommand() = default;
    virtual Result<void> execute(CommandContext&, const std::vector<std::string>&) = 0;
};