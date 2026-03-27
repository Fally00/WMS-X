#pragma once

#include <QWidget>
#include "controllers/WmsControllers.h"
#include <functional>
#include <optional>

namespace ReportDialog {
    // Shows the overall KPI and Reports dialog.
    void showReportDialog(QWidget* parent, WmsControllers& controller, std::function<void()> reloadInventoryCb, std::function<void(std::optional<int>)> openReceiptHistoryCb);
}
