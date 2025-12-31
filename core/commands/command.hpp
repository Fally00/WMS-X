#pragma once
#include "core/WmsControllers.h"
#include "core/commands/results.hpp"
#include <vector>
#include <string>

struct CommandContext {
    WmsControllers& wms;
    bool autosave;
};

struct ICommand {
    virtual ~ICommand() = default;
    virtual Result<void> execute(CommandContext&, const std::vector<std::string>&) = 0;
};
