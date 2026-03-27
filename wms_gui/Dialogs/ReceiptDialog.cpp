#include "ReceiptDialog.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QListWidget>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QDateTime>
#include <QTextEdit>
#include <QFileDialog>
#include <QTextStream>
#include <SQLiteCpp/SQLiteCpp.h>

namespace ReceiptDialog {

static QString formatTime(std::time_t t) {
    if (t == 0) return "—";
    return QDateTime::fromSecsSinceEpoch(static_cast<qint64>(t))
               .toString("yyyy-MM-dd HH:mm");
}

void showPreviewDialog(QWidget* parent, WmsControllers& controller, const Receipt& receipt) {
    QDialog preview(parent);
    preview.setWindowTitle(QString("Receipt %1").arg(
        QString::fromStdString(receipt.getReceiptNumber())));
    preview.setMinimumSize(520, 460);

    auto* layout = new QVBoxLayout(&preview);

    QString text;
    text += "=====================================\n";
    text += "        WMS-X  RECEIPT\n";
    text += "-------------------------------------\n";
    text += QString("Receipt : %1\n").arg(QString::fromStdString(receipt.getReceiptNumber()));

    auto tp = receipt.getTimestamp();
    qint64 epoch = std::chrono::duration_cast<std::chrono::seconds>(
        tp.time_since_epoch()).count();
    text += QString("Date    : %1\n").arg(
        QDateTime::fromSecsSinceEpoch(epoch).toString("yyyy-MM-dd HH:mm:ss"));

    QString customer = QString::fromStdString(receipt.getCustomerName());
    if (!customer.isEmpty()) {
        text += QString("Customer: %1\n").arg(customer);
    }

    try {
        SQLite::Statement cq(controller.getDB(),
            "SELECT customer_id FROM receipts WHERE receipt_number = ?");
        cq.bind(1, receipt.getReceiptNumber());
        if (cq.executeStep() && !cq.getColumn(0).isNull()) {
            int custId = cq.getColumn(0).getInt();
            auto cust = controller.getCustomer(custId);
            if (cust.has_value()) {
                if (!cust->getPhone().empty())
                    text += QString("Phone   : %1\n").arg(QString::fromStdString(cust->getPhone()));
                if (!cust->getAddress().empty())
                    text += QString("Address : %1\n").arg(QString::fromStdString(cust->getAddress()));
            }
        }
    } catch (...) {}

    try {
        SQLite::Statement sq(controller.getDB(),
            "SELECT supplier_name FROM receipts WHERE receipt_number = ?");
        sq.bind(1, receipt.getReceiptNumber());
        if (sq.executeStep()) {
            QString supplier = QString::fromStdString(sq.getColumn(0).getString());
            if (!supplier.isEmpty())
                text += QString("Supplier: %1\n").arg(supplier);
        }
    } catch (...) {}

    text += "-------------------------------------\n";
    text += QString("%1%2%3%4%5\n")
        .arg("ID",    -6)
        .arg("Name",  -16)
        .arg("Qty",   -8)
        .arg("Price", -10)
        .arg("Total", -10);

    try {
        SQLite::Statement itemQuery(controller.getDB(),
            "SELECT item_id, name, quantity, unit_price, line_total "
            "FROM receipt_items WHERE receipt_number = ?");
        itemQuery.bind(1, receipt.getReceiptNumber());

        while (itemQuery.executeStep()) {
            text += QString("%1%2%3%4%5\n")
                .arg(itemQuery.getColumn(0).getInt(),  -6)
                .arg(QString::fromStdString(itemQuery.getColumn(1).getString()).left(15), -16)
                .arg(itemQuery.getColumn(2).getInt(),  -8)
                .arg(QString::number(itemQuery.getColumn(3).getDouble(), 'f', 2), -10)
                .arg(QString::number(itemQuery.getColumn(4).getDouble(), 'f', 2), -10);
        }
    } catch (...) {}

    text += "-------------------------------------\n";
    text += QString("Subtotal: %1\n").arg(QString::number(receipt.subtotal(), 'f', 2));
    text += QString("Tax (14%%): %1\n").arg(QString::number(receipt.tax(), 'f', 2));
    text += QString("TOTAL   : %1\n").arg(QString::number(receipt.total(), 'f', 2));
    text += "=====================================\n";

    auto* textEdit = new QTextEdit(&preview);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(text);
    textEdit->setStyleSheet(
        "QTextEdit { font-family: 'Consolas', 'Courier New', monospace; "
        "font-size: 13px; background: #1e1e1e; color: #d4d4d4; "
        "padding: 12px; border-radius: 4px; }");
    layout->addWidget(textEdit);

    auto* closeBtn = new QPushButton("Close", &preview);
    QObject::connect(closeBtn, &QPushButton::clicked, &preview, &QDialog::accept);
    layout->addWidget(closeBtn);

    preview.exec();
}

void exportToCSV(QWidget* parent, WmsControllers& controller, const std::vector<Receipt>& receipts) {
    if (receipts.empty()) {
        QMessageBox::information(parent, "No Receipts", "Nothing to export.");
        return;
    }

    QString defaultName = (receipts.size() == 1)
        ? QString::fromStdString(receipts[0].getReceiptNumber()) + ".txt"
        : "receipts.txt";

    QString filePath = QFileDialog::getSaveFileName(
        parent, "Export Receipt", defaultName,
        "Text Files (*.txt);;All Files (*)");

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(parent, "File Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    for (size_t idx = 0; idx < receipts.size(); ++idx) {
        const auto& r = receipts[idx];

        qint64 epoch = std::chrono::duration_cast<std::chrono::seconds>(
            r.getTimestamp().time_since_epoch()).count();
        QString dateStr = QDateTime::fromSecsSinceEpoch(epoch)
            .toString("yyyy-MM-dd HH:mm:ss");

        out << "=====================================\n";
        out << "        WMS-X  RECEIPT\n";
        out << "-------------------------------------\n";
        out << QString("Receipt : %1\n").arg(QString::fromStdString(r.getReceiptNumber()));
        out << QString("Date    : %1\n").arg(dateStr);

        QString customer = QString::fromStdString(r.getCustomerName());
        if (!customer.isEmpty())
            out << QString("Customer: %1\n").arg(customer);

        try {
            SQLite::Statement cq(controller.getDB(),
                "SELECT customer_id FROM receipts WHERE receipt_number = ?");
            cq.bind(1, r.getReceiptNumber());
            if (cq.executeStep() && !cq.getColumn(0).isNull()) {
                int custId = cq.getColumn(0).getInt();
                auto cust = controller.getCustomer(custId);
                if (cust.has_value()) {
                    if (!cust->getPhone().empty())
                        out << QString("Phone   : %1\n").arg(QString::fromStdString(cust->getPhone()));
                    if (!cust->getAddress().empty())
                        out << QString("Address : %1\n").arg(QString::fromStdString(cust->getAddress()));
                }
            }
        } catch (...) {}

        try {
            SQLite::Statement sq(controller.getDB(),
                "SELECT supplier_name FROM receipts WHERE receipt_number = ?");
            sq.bind(1, r.getReceiptNumber());
            if (sq.executeStep()) {
                QString supplier = QString::fromStdString(sq.getColumn(0).getString());
                if (!supplier.isEmpty())
                    out << QString("Supplier: %1\n").arg(supplier);
            }
        } catch (...) {}

        out << "-------------------------------------\n";
        out << QString("%1%2%3%4%5\n")
            .arg("ID",    -6)
            .arg("Name",  -16)
            .arg("Qty",   -8)
            .arg("Price", -10)
            .arg("Total", -10);

        try {
            SQLite::Statement iq(controller.getDB(),
                "SELECT item_id, name, quantity, unit_price, line_total "
                "FROM receipt_items WHERE receipt_number = ?");
            iq.bind(1, r.getReceiptNumber());

            while (iq.executeStep()) {
                out << QString("%1%2%3%4%5\n")
                    .arg(iq.getColumn(0).getInt(), -6)
                    .arg(QString::fromStdString(iq.getColumn(1).getString()).left(15), -16)
                    .arg(iq.getColumn(2).getInt(), -8)
                    .arg(QString::number(iq.getColumn(3).getDouble(), 'f', 2), -10)
                    .arg(QString::number(iq.getColumn(4).getDouble(), 'f', 2), -10);
            }
        } catch (...) {}

        out << "-------------------------------------\n";
        out << QString("Subtotal : %1\n").arg(QString::number(r.subtotal(), 'f', 2));
        out << QString("Tax (14%%): %1\n").arg(QString::number(r.tax(), 'f', 2));
        out << QString("TOTAL    : %1\n").arg(QString::number(r.total(), 'f', 2));
        out << "=====================================\n";

        if (idx + 1 < receipts.size())
            out << "\n\n";
    }

    file.close();
    QMessageBox::information(parent, "Export Complete", QString("Receipt exported to:\n%1").arg(filePath));
}

void exportAllToCSV(QWidget* parent, WmsControllers& controller) {
    std::vector<Receipt> receipts;
    try {
        receipts = Receipt::loadHistory(controller.getDB());
    } catch (const std::exception& e) {
        QMessageBox::warning(parent, "Error", QString("Could not load receipts: %1").arg(e.what()));
        return;
    }
    exportToCSV(parent, controller, receipts);
}

void showHistoryDialog(QWidget* parent, WmsControllers& controller, std::optional<int> preselectCustomerId) {
    std::vector<Receipt> receipts;
    try {
        receipts = Receipt::loadHistory(controller.getDB());
    } catch (const std::exception& e) {
        QMessageBox::warning(parent, "Error", QString("Could not load receipts: %1").arg(e.what()));
        return;
    }

    if (receipts.empty()) {
        QMessageBox::information(parent, "No Receipts", "No receipts have been generated yet.");
        return;
    }

    QDialog dialog(parent);
    dialog.setWindowTitle("Receipt History");
    dialog.setMinimumSize(750, 480);
    auto* layout = new QVBoxLayout(&dialog);

    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Filter by Customer:", &dialog));
    auto* custFilterCombo = new QComboBox(&dialog);
    custFilterCombo->addItem("All Customers", -1);

    auto allCustomers = controller.getAllCustomers();
    for (const auto& c : allCustomers) {
        custFilterCombo->addItem(
            QString("#%1 — %2").arg(c.getId()).arg(QString::fromStdString(c.getName())),
            c.getId());
    }
    if (preselectCustomerId.has_value()) {
        const int want = *preselectCustomerId;
        for (int i = 0; i < custFilterCombo->count(); ++i) {
            if (custFilterCombo->itemData(i).toInt() == want) {
                custFilterCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    filterLayout->addWidget(custFilterCombo);
    filterLayout->addStretch();
    layout->addLayout(filterLayout);

    auto* table = new QTableWidget(static_cast<int>(receipts.size()), 4, &dialog);
    table->setHorizontalHeaderLabels({"Receipt #", "Customer", "Date", "Total"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    auto populateReceiptTable = [&](int filterCustId) {
        table->setRowCount(0);
        for (int i = 0; i < static_cast<int>(receipts.size()); ++i) {
            const auto& r = receipts[static_cast<size_t>(i)];

            if (filterCustId >= 0) {
                try {
                    SQLite::Statement cq(controller.getDB(),
                        "SELECT customer_id FROM receipts WHERE receipt_number = ?");
                    cq.bind(1, r.getReceiptNumber());
                    if (cq.executeStep()) {
                        if (cq.getColumn(0).isNull() || cq.getColumn(0).getInt() != filterCustId)
                            continue;
                    } else continue;
                } catch (...) { continue; }
            }

            int row = table->rowCount();
            table->insertRow(row);

            table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(r.getReceiptNumber())));
            table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(r.getCustomerName())));
            qint64 epoch = std::chrono::duration_cast<std::chrono::seconds>(
                r.getTimestamp().time_since_epoch()).count();
            table->setItem(row, 2, new QTableWidgetItem(
                QDateTime::fromSecsSinceEpoch(epoch).toString("yyyy-MM-dd HH:mm")));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(r.total(), 'f', 2)));

            table->item(row, 0)->setData(Qt::UserRole, i);
        }
    };

    populateReceiptTable(custFilterCombo->currentData().toInt());

    QObject::connect(custFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int idx) {
        int custId = custFilterCombo->itemData(idx).toInt();
        populateReceiptTable(custId);
    });

    layout->addWidget(table);

    auto* btnLayout = new QHBoxLayout();
    auto* viewBtn   = new QPushButton("View Receipt", &dialog);
    auto* exportBtn = new QPushButton("Export Selected", &dialog);
    auto* deleteBtn = new QPushButton("Delete Receipt", &dialog);
    auto* closeBtn  = new QPushButton("Close", &dialog);
    deleteBtn->setStyleSheet("QPushButton { color: #cc3333; }");
    btnLayout->addWidget(viewBtn);
    btnLayout->addWidget(exportBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    QObject::connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    auto getReceiptIndex = [&](int row) -> int {
        if (row < 0 || !table->item(row, 0)) return -1;
        return table->item(row, 0)->data(Qt::UserRole).toInt();
    };

    QObject::connect(viewBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        int idx = getReceiptIndex(row);
        if (idx < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a receipt.");
            return;
        }
        showPreviewDialog(&dialog, controller, receipts[static_cast<size_t>(idx)]);
    });

    QObject::connect(exportBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        int idx = getReceiptIndex(row);
        if (idx < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a receipt to export.");
            return;
        }
        exportToCSV(&dialog, controller, { receipts[static_cast<size_t>(idx)] });
    });

    QObject::connect(deleteBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        int idx = getReceiptIndex(row);
        if (idx < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a receipt to delete.");
            return;
        }
        const auto& r = receipts[static_cast<size_t>(idx)];
        auto confirm = QMessageBox::question(&dialog, "Confirm Delete",
            QString("Delete receipt %1?\nThis cannot be undone.")
                .arg(QString::fromStdString(r.getReceiptNumber())));
        if (confirm != QMessageBox::Yes) return;

        try {
            Receipt::deleteFromDB(controller.getDB(), r.getReceiptNumber());
            receipts.erase(receipts.begin() + idx);
            int custId = custFilterCombo->currentData().toInt();
            populateReceiptTable(custId);
        } catch (const std::exception& e) {
            QMessageBox::warning(&dialog, "Error", QString("Could not delete receipt: %1").arg(e.what()));
        }
    });

    QObject::connect(table, &QTableWidget::cellDoubleClicked, [&](int row, int) {
        int idx = getReceiptIndex(row);
        if (idx >= 0 && idx < static_cast<int>(receipts.size())) {
            showPreviewDialog(&dialog, controller, receipts[static_cast<size_t>(idx)]);
        }
    });

    dialog.exec();
}

bool showGenerateDialog(QWidget* parent, WmsControllers& controller, QTableWidget* inventoryTable) {
    const auto normalizeScannedBarcode = [](const QString& raw) {
        QString cleaned;
        cleaned.reserve(raw.size());
        for (const QChar ch : raw) {
            if (ch.isPrint()) cleaned.append(ch);
        }
        return cleaned.trimmed();
    };

    QList<int> selectedRows;
    const auto selectedItems = inventoryTable->selectionModel()->selectedRows();
    for (const auto& idx : selectedItems) {
        selectedRows.append(idx.row());
    }

    struct SelectedItem {
        int id;
        QString name;
        int maxQty;
        double price;
    };
    QList<SelectedItem> itemList;

    for (int row : selectedRows) {
        SelectedItem si;
        si.id     = inventoryTable->item(row, 0)->text().toInt();
        si.name   = inventoryTable->item(row, 1)->text();
        si.maxQty = inventoryTable->item(row, 2)->text().toInt();
        si.price  = inventoryTable->item(row, 4)->text().toDouble();
        if (si.maxQty <= 0) continue;
        itemList.append(si);
    }

    if (!selectedRows.isEmpty() && itemList.isEmpty()) {
        QMessageBox::warning(parent, "Out of Stock", "All selected items are out of stock.");
        return false;
    }

    QDialog dialog(parent);
    dialog.setWindowTitle("Generate Receipt");
    dialog.setMinimumWidth(500);
    auto* mainLayout = new QVBoxLayout(&dialog);

    auto* scanForm = new QFormLayout();
    auto* scanRow = new QHBoxLayout();
    auto* scanEdit = new QLineEdit(&dialog);
    scanEdit->setPlaceholderText("Scan or enter barcode");
    
    auto* scanQtySpin = new QSpinBox(&dialog);
    scanQtySpin->setRange(1, 9999);
    scanQtySpin->setValue(1);
    scanQtySpin->setPrefix("Qty: ");
    
    auto* addScanBtn = new QPushButton("Add", &dialog);
    scanRow->addWidget(scanEdit, 1);
    scanRow->addWidget(scanQtySpin);
    scanRow->addWidget(addScanBtn);
    auto* scanWarn = new QLabel(&dialog);
    scanWarn->setStyleSheet("QLabel { color: #c04040; }");
    scanForm->addRow("Add by barcode:", scanRow);
    scanForm->addRow(scanWarn);
    mainLayout->addLayout(scanForm);

    auto* custGroup = new QFormLayout();

    auto* manualCheck = new QCheckBox("Manual entry (type customer name)", &dialog);
    manualCheck->setChecked(false);
    custGroup->addRow(manualCheck);

    auto* lookupLayout = new QHBoxLayout();
    auto* custSearchEdit = new QLineEdit(&dialog);
    custSearchEdit->setPlaceholderText("Search customer by name or ID...");
    auto* lookupBtn = new QPushButton("Lookup", &dialog);
    lookupLayout->addWidget(custSearchEdit);
    lookupLayout->addWidget(lookupBtn);
    custGroup->addRow("Customer:", lookupLayout);

    auto* custResultList = new QListWidget(&dialog);
    custResultList->setMaximumHeight(100);
    custResultList->setVisible(false);
    custGroup->addRow("", custResultList);

    auto* custInfoLabel = new QLabel("No customer selected", &dialog);
    custInfoLabel->setStyleSheet("QLabel { background: #2d2d2d; color: #b0b0b0; padding: 6px; border-radius: 4px; }");
    custGroup->addRow("Selected:", custInfoLabel);

    auto* customerEdit = new QLineEdit(&dialog);
    customerEdit->setPlaceholderText("Customer name (required)");
    customerEdit->setVisible(false);
    custGroup->addRow("Name:", customerEdit);

    auto* supplierEdit = new QLineEdit(&dialog);
    supplierEdit->setPlaceholderText("Supplier / company name (optional)");
    custGroup->addRow("Supplier:", supplierEdit);

    mainLayout->addLayout(custGroup);

    int selectedCustomerId = -1;

    QObject::connect(manualCheck, &QCheckBox::toggled, [&](bool manual) {
        custSearchEdit->setVisible(!manual);
        lookupBtn->setVisible(!manual);
        custResultList->setVisible(false);
        custInfoLabel->setVisible(!manual);
        customerEdit->setVisible(manual);
        if (manual) {
            selectedCustomerId = -1;
            custInfoLabel->setText("No customer selected");
        }
    });

    QObject::connect(lookupBtn, &QPushButton::clicked, [&]() {
        QString query = custSearchEdit->text().trimmed();
        if (query.isEmpty()) return;

        custResultList->clear();
        custResultList->setVisible(true);

        bool isId;
        int searchId = query.toInt(&isId);
        if (isId) {
            auto cust = controller.getCustomer(searchId);
            if (cust.has_value()) {
                custResultList->addItem(QString("#%1 — %2 (%3)")
                    .arg(cust->getId())
                    .arg(QString::fromStdString(cust->getName()))
                    .arg(QString::fromStdString(cust->getPhone())));
                custResultList->item(0)->setData(Qt::UserRole, cust->getId());
            } else {
                custResultList->addItem("No customer found with that ID.");
            }
        } else {
            auto results = controller.searchCustomerByName(query.toStdString());
            if (results.empty()) {
                custResultList->addItem("No customers found matching: " + query);
            } else {
                for (const auto& c : results) {
                    auto* item = new QListWidgetItem(QString("#%1 — %2 (%3)")
                        .arg(c.getId())
                        .arg(QString::fromStdString(c.getName()))
                        .arg(QString::fromStdString(c.getPhone())));
                    item->setData(Qt::UserRole, c.getId());
                    custResultList->addItem(item);
                }
            }
        }
    });

    QObject::connect(custResultList, &QListWidget::itemClicked, [&](QListWidgetItem* item) {
        QVariant data = item->data(Qt::UserRole);
        if (!data.isValid()) return;

        selectedCustomerId = data.toInt();
        auto cust = controller.getCustomer(selectedCustomerId);
        if (cust.has_value()) {
            custInfoLabel->setText(QString("%1 | %2 | %3")
                .arg(QString::fromStdString(cust->getName()))
                .arg(QString::fromStdString(cust->getPhone()))
                .arg(QString::fromStdString(cust->getAddress())));
            custInfoLabel->setStyleSheet(
                "QLabel { background: #1a3a1a; color: #70d070; padding: 6px; border-radius: 4px; }");
        }
        custResultList->setVisible(false);
    });

    auto* itemTable = new QTableWidget(0, 4, &dialog);
    itemTable->setHorizontalHeaderLabels({"Item", "Max Qty", "Quantity", "Unit Price"});
    itemTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QList<QSpinBox*> qtySpins;
    QList<QDoubleSpinBox*> priceSpins;

    const auto rebuildItemTable = [&]() {
        itemTable->setRowCount(0);
        qtySpins.clear();
        priceSpins.clear();
        for (int i = 0; i < itemList.size(); ++i) {
            const auto& si = itemList[i];
            int r = itemTable->rowCount();
            itemTable->insertRow(r);
            itemTable->setItem(r, 0, new QTableWidgetItem(
                QString("#%1 %2").arg(si.id).arg(si.name)));
            itemTable->setItem(r, 1, new QTableWidgetItem(QString::number(si.maxQty)));

            auto* qtySpin = new QSpinBox(&dialog);
            qtySpin->setRange(1, std::max(1, si.maxQty));
            qtySpin->setValue(1);
            itemTable->setCellWidget(r, 2, qtySpin);
            qtySpins.append(qtySpin);

            auto* priceSpin = new QDoubleSpinBox(&dialog);
            priceSpin->setDecimals(2);
            priceSpin->setRange(0.0, 1000000000.0);
            priceSpin->setValue(si.price);
            itemTable->setCellWidget(r, 3, priceSpin);
            priceSpins.append(priceSpin);
        }
    };

    rebuildItemTable();
    scanEdit->setFocus();

    auto processScan = [&]() {
        QString bc = normalizeScannedBarcode(scanEdit->text());
        scanEdit->clear();
        scanWarn->clear();
        if (bc.isEmpty()) {
            scanEdit->setFocus();
            return;
        }

        auto inv = controller.getItemByBarcode(bc.toStdString());
        if (!inv.has_value()) {
            bool isNumeric;
            int id = bc.toInt(&isNumeric);
            if (isNumeric) {
                inv = controller.getItem(id);
            }
        }

        if (!inv.has_value()) {
            scanWarn->setText("Item not found");
            scanEdit->setFocus();
            return;
        }
        if (inv->getQuantity() <= 0) {
            scanWarn->setText("Out of stock");
            scanEdit->setFocus();
            return;
        }

        int inputQty = scanQtySpin->value();

        int mergeIdx = -1;
        for (int j = 0; j < itemList.size(); ++j) {
            if (itemList[j].id == inv->getId()) {
                mergeIdx = j;
                break;
            }
        }

        if (mergeIdx >= 0) {
            auto fresh = controller.getItem(inv->getId());
            if (!fresh.has_value()) return;
            itemList[mergeIdx].maxQty = fresh->getQuantity();
            itemTable->item(mergeIdx, 1)->setText(QString::number(itemList[mergeIdx].maxQty));
            QSpinBox* sp = qtySpins[mergeIdx];
            sp->setMaximum(std::max(1, itemList[mergeIdx].maxQty));
            
            int newQty = sp->value() + inputQty;
            if (newQty > sp->maximum()) newQty = sp->maximum();
            sp->setValue(newQty);
        } else {
            SelectedItem si;
            si.id = inv->getId();
            si.name = QString::fromStdString(inv->getName());
            si.maxQty = inv->getQuantity();
            si.price = inv->getPrice();
            itemList.append(si);
            
            int r = itemTable->rowCount();
            itemTable->insertRow(r);
            itemTable->setItem(r, 0, new QTableWidgetItem(QString("#%1 %2").arg(si.id).arg(si.name)));
            itemTable->setItem(r, 1, new QTableWidgetItem(QString::number(si.maxQty)));
            
            auto* qtySpin = new QSpinBox(&dialog);
            qtySpin->setRange(1, std::max(1, si.maxQty));
            qtySpin->setValue(std::min(inputQty, std::max(1, si.maxQty)));
            itemTable->setCellWidget(r, 2, qtySpin);
            qtySpins.append(qtySpin);
            
            auto* priceSpin = new QDoubleSpinBox(&dialog);
            priceSpin->setDecimals(2);
            priceSpin->setRange(0.0, 1000000000.0);
            priceSpin->setValue(si.price);
            itemTable->setCellWidget(r, 3, priceSpin);
            priceSpins.append(priceSpin);
        }
        
        scanQtySpin->setValue(1);
        scanEdit->setFocus();
    };

    QObject::connect(scanEdit, &QLineEdit::returnPressed, &dialog, processScan);
    QObject::connect(addScanBtn, &QPushButton::clicked, &dialog, processScan);

    mainLayout->addWidget(itemTable);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, &dialog);
    mainLayout->addWidget(&buttons);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (itemList.isEmpty()) {
            QMessageBox::warning(&dialog, "Receipt",
                "Add at least one item (select rows in the table or scan a barcode).");
            return;
        }
        dialog.accept();
    });

    if (dialog.exec() != QDialog::Accepted) return false;

    bool isManual = manualCheck->isChecked();
    if (isManual && customerEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(parent, "Missing Customer", "Please enter a customer name.");
        return false;
    }
    if (!isManual && selectedCustomerId < 0) {
        QMessageBox::warning(parent, "No Customer Selected", "Please select a customer, or use manual entry.");
        return false;
    }

    Receipt receipt;

    if (isManual) {
        receipt.setCustomer(customerEdit->text().trimmed().toStdString());
    } else {
        auto cust = controller.getCustomer(selectedCustomerId);
        if (cust.has_value()) {
            receipt.setCustomer(cust->getName(), cust->getPhone(), cust->getEmail());
        }
    }

    QString supplierName = supplierEdit->text().trimmed();

    try {
        SQLite::Transaction transaction(controller.getDB());
        for (int i = 0; i < itemList.size(); ++i) {
            auto item = controller.getItem(itemList[i].id);
            if (!item) continue;
            int qty = qtySpins[i]->value();
            double price = priceSpins[i]->value();
            receipt.addItem(*item, qty, price);
        }

        receipt.saveToDB(controller.getDB());

        if (!isManual && selectedCustomerId >= 0) {
            try {
                SQLite::Statement upd(controller.getDB(),
                    "UPDATE receipts SET customer_id = ? WHERE receipt_number = ?");
                upd.bind(1, selectedCustomerId);
                upd.bind(2, receipt.getReceiptNumber());
                upd.exec();
            } catch (...) {}
        }

        if (!supplierName.isEmpty()) {
            try {
                controller.getDB().exec("ALTER TABLE receipts ADD COLUMN supplier_name TEXT DEFAULT ''");
            } catch (...) {}
            SQLite::Statement upd(controller.getDB(),
                "UPDATE receipts SET supplier_name = ? WHERE receipt_number = ?");
            upd.bind(1, supplierName.toStdString());
            upd.bind(2, receipt.getReceiptNumber());
            upd.exec();
        }

        for (int i = 0; i < itemList.size(); ++i) {
            int qty = qtySpins[i]->value();
            controller.adjustStock(itemList[i].id, -qty); // Ignoring warning logic for simplicity
        }

        transaction.commit();

        showPreviewDialog(parent, controller, receipt);
        return true;
    } catch (const std::exception& e) {
        QMessageBox::warning(parent, "Receipt Failed", QString("Could not save receipt: %1").arg(e.what()));
        return false;
    }
}

} // namespace ReceiptDialog
