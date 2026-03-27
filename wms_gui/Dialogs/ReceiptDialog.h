#pragma once

#include <QWidget>
#include <QTableWidget>
#include <optional>
#include "controllers/WmsControllers.h"
#include "storage/Receipt.h"

namespace ReceiptDialog {

    // Shows the Generate Receipt dialog (multi-item with customer lookup)
    bool showGenerateDialog(QWidget* parent, WmsControllers& controller, QTableWidget* inventoryTable);

    // Shows the Receipt History dialog
    void showHistoryDialog(QWidget* parent, WmsControllers& controller, std::optional<int> preselectCustomerId = std::nullopt);

    // Shows the Receipt Preview dialog
    void showPreviewDialog(QWidget* parent, WmsControllers& controller, const Receipt& receipt);

    // Export receipts to CSV
    void exportToCSV(QWidget* parent, WmsControllers& controller, const std::vector<Receipt>& receipts);
    
    // Quick action for exporting all receipts
    void exportAllToCSV(QWidget* parent, WmsControllers& controller);

} // namespace ReceiptDialog
