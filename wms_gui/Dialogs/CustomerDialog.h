#pragma once

#include <QWidget>
#include "controllers/WmsControllers.h"

namespace CustomerDialog {
    // Shows the Customer Management dialog
    void showManageDialog(QWidget* parent, WmsControllers& controller);
}
