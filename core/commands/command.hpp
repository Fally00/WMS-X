#pragma once
#include "core/WmsControllers.h"
#include "core/commands/results.hpp"
#include <vector>
#include <string>

// Context for command execution
struct CommandContext {
    WmsControllers& wms;
    bool autosave;
};

// Interface for commands
struct ICommand {
    virtual ~ICommand() = default;
    virtual Result<void> execute(CommandContext&, const std::vector<std::string>&) = 0;
};
