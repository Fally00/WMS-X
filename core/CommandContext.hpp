#pragma once
#include "WmsControllers.h"
#include <string>

struct CommandContext {
    WmsControllers& wms;
    bool autosave;
};