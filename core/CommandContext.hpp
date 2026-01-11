#pragma once

//Included Files
#include "WmsControllers.h"

//Needed libraries
#include <string>

struct CommandContext {
    WmsControllers& wms;
    bool autosave;
};