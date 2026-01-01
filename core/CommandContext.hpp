#pragma once
#include "core/WmsControllers.h"
#include <string>

struct CommandContext {
    WmsControllers& wms;
    bool autosave;
};