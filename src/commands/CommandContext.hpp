#pragma once

//Included Files
#include "controllers/WmsControllers.h"

//Needed libraries
#include <string>

struct CommandContext {
    WmsControllers& wms;
    bool autosave;
};