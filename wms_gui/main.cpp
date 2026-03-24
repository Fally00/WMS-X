#include "main.h"
#include "ui_main.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QInputDialog>
#include <QListWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTextStream>
#include <algorithm>
#include <QDateTime>
#include <QSpinBox>
#include <QDialog>
#include <QString>
#include <QLabel>
#include <QFile>

// ─── Constructor ─────────────────────────────────────────────────────────────
Main::Main(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Main)
    , wmsController("../inventory.db")
{
    ui->setupUi(this);
    setWindowTitle("WMS-X — Warehouse Management System");

    // Stretch table columns to fill width
    ui->inventoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Initialize the backend (connects to DB, loads items into memory)
    if (!wmsController.initializeSystem()) {
        QMessageBox::critical(this, "DB Error",
            "Failed to connect to inventory.db.\nMake sure the database file exists.");
        return;
    }

    // Populate the table with DB data
    loadInventory();

    // Connect signals → slots
    connect(ui->addBtn,     &QPushButton::clicked, this, &Main::onAddItem);
    connect(ui->updateBtn,  &QPushButton::clicked, this, &Main::onUpdateItem);
    connect(ui->deleteBtn,  &QPushButton::clicked, this, &Main::onDeleteItem);
    connect(ui->searchBtn,  &QPushButton::clicked, this, &Main::onSearch);
    connect(ui->searchLineEdit, &QLineEdit::returnPressed, this, &Main::onSearch);
    connect(ui->queueTaskBtn,      &QPushButton::clicked, this, &Main::onQueueTask);
    connect(ui->runQueueBtn,       &QPushButton::clicked, this, &Main::onRunQueue);
    connect(ui->receiptBtn,        &QPushButton::clicked, this, &Main::onGenerateReceipt);
    connect(ui->receiptHistoryBtn, &QPushButton::clicked, this, &Main::onReceiptHistory);
    connect(ui->exportCsvBtn,      &QPushButton::clicked, this, &Main::onExportCSV);
}

Main::~Main()
{
    delete ui;
}

// ─── Helper: item status based on quantity ───────────────────────────────────
QString Main::itemStatus(int quantity) const
{
    if (quantity <= 0)  return "Out of Stock";
    if (quantity <= 10) return "Low Stock";
    return "Available";
}

// ─── Load all items from the controller into the table ───────────────────────
void Main::loadInventory()
{
    auto items = wmsController.getAllItems();
    ui->statusbar->showMessage(QString("Total items: %1").arg(items.size()));
    populateTable(items);
}

// ─── Fill the QTableWidget rows ──────────────────────────────────────────────
void Main::populateTable(const std::vector<Item>& items)
{
    QTableWidget* table = ui->inventoryTable;
    table->setRowCount(0); // clear

    for (const auto& item : items) {
        int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0,  new QTableWidgetItem(QString::number(item.getId())));
        table->setItem(row, 1,  new QTableWidgetItem(QString::fromStdString(item.getName())));
        table->setItem(row, 2,  new QTableWidgetItem(QString::number(item.getQuantity())));
        table->setItem(row, 3,  new QTableWidgetItem(QString::fromStdString(item.getLocation())));
        table->setItem(row, 4,  new QTableWidgetItem(QString::number(item.getPrice(), 'f', 2)));
        table->setItem(row, 5,  new QTableWidgetItem(QString::fromStdString(item.getCurrency())));
        table->setItem(row, 6,  new QTableWidgetItem(QString::fromStdString(item.getUnit())));
        table->setItem(row, 7,  new QTableWidgetItem(QString::fromStdString(item.getCategory())));

        // Status column based on quantity
        const QString status = itemStatus(item.getQuantity());
        auto* statusItem = new QTableWidgetItem(status);
        if (status == "Out of Stock") {
            statusItem->setForeground(QBrush(QColor(220, 50, 50)));
        } else if (status == "Low Stock") {
            statusItem->setForeground(QBrush(QColor(230, 160, 0)));
        } else {
            statusItem->setForeground(QBrush(QColor(40, 180, 60)));
        }
        table->setItem(row, 8, statusItem);

        table->setItem(row, 9,  new QTableWidgetItem(formatTime(item.getCreatedAt())));
        table->setItem(row, 10, new QTableWidgetItem(formatTime(item.getModifiedAt())));
    }
}

// ─── Helper: format a time_t to human-readable string ───────────────────────
QString Main::formatTime(std::time_t t) const
{
    if (t == 0) return "—";
    return QDateTime::fromSecsSinceEpoch(static_cast<qint64>(t))
               .toString("yyyy-MM-dd HH:mm");
}

// ─── Slot: Add Item ──────────────────────────────────────────────────────────
void Main::onAddItem()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Add New Item");
    QFormLayout form(&dialog);

    QLineEdit *idEdit   = new QLineEdit(&dialog);
    QLineEdit *nameEdit = new QLineEdit(&dialog);
    QLineEdit *qtyEdit  = new QLineEdit(&dialog);
    QLineEdit *locEdit  = new QLineEdit(&dialog);

    form.addRow("ID:",       idEdit);
    form.addRow("Name:",     nameEdit);
    form.addRow("Quantity:", qtyEdit);
    form.addRow("Location:", locEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    bool okId, okQty;
    int id  = idEdit->text().toInt(&okId);
    int qty = qtyEdit->text().toInt(&okQty);

    if (!okId || !okQty || nameEdit->text().isEmpty() || locEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please fill all fields correctly.");
        return;
    }

    if (!wmsController.addItem(id, nameEdit->text().toStdString(),
                               qty, locEdit->text().toStdString())) {
        QMessageBox::warning(this, "Failed", "Item already exists or invalid data.");
        return;
    }

    loadInventory();
    ui->statusbar->showMessage("Item added successfully.", 3000);
}

// ─── Slot: Update Item ───────────────────────────────────────────────────────
void Main::onUpdateItem()
{
    int selectedRow = ui->inventoryTable->currentRow();
    if (selectedRow < 0) {
        QMessageBox::information(this, "No Selection", "Please select a row to update.");
        return;
    }

    int id = ui->inventoryTable->item(selectedRow, 0)->text().toInt();

    QDialog dialog(this);
    dialog.setWindowTitle(QString("Update Item #%1").arg(id));
    QFormLayout form(&dialog);

    QLineEdit *nameEdit = new QLineEdit(ui->inventoryTable->item(selectedRow, 1)->text(), &dialog);
    QLineEdit *qtyEdit  = new QLineEdit(ui->inventoryTable->item(selectedRow, 2)->text(), &dialog);
    QLineEdit *locEdit  = new QLineEdit(ui->inventoryTable->item(selectedRow, 3)->text(), &dialog);
    QLineEdit *priceEdit = new QLineEdit(ui->inventoryTable->item(selectedRow, 4)->text(), &dialog);

    form.addRow("Name:",     nameEdit);
    form.addRow("Quantity:", qtyEdit);
    form.addRow("Location:", locEdit);
    form.addRow("Price:",    priceEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, &dialog);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    bool okQty, okPrice;
    std::optional<std::string>  name  = nameEdit->text().isEmpty()
                                        ? std::nullopt
                                        : std::optional<std::string>(nameEdit->text().toStdString());
    std::optional<int>          qty   = qtyEdit->text().isEmpty()
                                        ? std::nullopt
                                        : std::optional<int>(qtyEdit->text().toInt(&okQty));
    std::optional<std::string>  loc   = locEdit->text().isEmpty()
                                        ? std::nullopt
                                        : std::optional<std::string>(locEdit->text().toStdString());
    std::optional<double>       price = priceEdit->text().isEmpty()
                                        ? std::nullopt
                                        : std::optional<double>(priceEdit->text().toDouble(&okPrice));

    if (!wmsController.updateItem(id, name, qty, loc, price)) {
        QMessageBox::warning(this, "Failed", "Could not update item.");
        return;
    }

    loadInventory();
    ui->statusbar->showMessage("Item updated.", 3000);
}

// ─── Slot: Delete Item ───────────────────────────────────────────────────────
void Main::onDeleteItem()
{
    int selectedRow = ui->inventoryTable->currentRow();
    if (selectedRow < 0) {
        QMessageBox::information(this, "No Selection", "Please select a row to delete.");
        return;
    }

    int id = ui->inventoryTable->item(selectedRow, 0)->text().toInt();
    QString name = ui->inventoryTable->item(selectedRow, 1)->text();

    auto confirm = QMessageBox::question(this, "Confirm Delete",
        QString("Delete item #%1 (%2)?").arg(id).arg(name));
    if (confirm != QMessageBox::Yes) return;

    if (!wmsController.removeItem(id)) {
        QMessageBox::warning(this, "Failed", "Item not found.");
        return;
    }

    loadInventory();
    ui->statusbar->showMessage("Item deleted.", 3000);
}

// ─── Slot: Search ────────────────────────────────────────────────────────────
void Main::onSearch()
{
    QString query = ui->searchLineEdit->text().trimmed();
    if (query.isEmpty()) {
        loadInventory();
        return;
    }

    // Try numeric → search by ID first, then fall back to name
    bool isId;
    int id = query.toInt(&isId);

    if (isId) {
        auto result = wmsController.getItem(id);
        if (result) {
            populateTable({*result});
            ui->statusbar->showMessage("1 result found.", 3000);
        } else {
            populateTable({});
            ui->statusbar->showMessage("No item found with that ID.", 3000);
        }
    } else {
        auto results = wmsController.searchByName(query.toStdString());
        populateTable(results);
        ui->statusbar->showMessage(
            QString("%1 result(s) found.").arg(results.size()), 3000);
    }

    // Clear selection after search
    ui->inventoryTable->clearSelection();
}

// ─── Slot: Queue Task — Help Menu ────────────────────────────────────────────
void Main::onQueueTask()
{
    // Command definitions: name, usage, description
    struct CmdInfo {
        QString name;
        QString usage;
        QString description;
    };

    const QList<CmdInfo> commands = {
        { "ADD",
          "ADD <id> <name> <qty> <location>",
          "Add a new item to inventory.\nResult: new row appears in the table." },
        { "REMOVE",
          "REMOVE <id>",
          "Remove an item by its ID.\nResult: row is deleted from the table." },
        { "UPDATE",
          "UPDATE <id> --name <n> --qty <q> --loc <l> --price <p>",
          "Update one or more fields of an existing item.\nResult: updated values appear in the table." },
        { "LIST",
          "LIST [page] [pageSize]",
          "List inventory items (paginated, CLI output).\nResult: prints to console; table unchanged." },
        { "SEARCH",
          "SEARCH <id>  or  SEARCH --name <query>",
          "Search for an item by ID or name.\nResult: prints to console; table unchanged." },
        { "RECEIPT",
          "RECEIPT <id qty price>... [customer]",
          "Generate a receipt for one or more items.\nResult: receipt saved to DB, stock deducted." },
        { "QUEUE",
          "QUEUE <command...>",
          "Queue another command for deferred processing." },
        { "PROCESSQUEUE",
          "PROCESSQUEUE [limit]",
          "Process all (or N) queued tasks.\nResult: queued operations executed; table refreshed." },
    };

    // Build dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Queue Task — Command Help");
    dialog.setMinimumSize(560, 420);
    auto* layout = new QVBoxLayout(&dialog);

    layout->addWidget(new QLabel("<b>Select a command to queue:</b>", &dialog));

    // Command list
    auto* cmdList = new QListWidget(&dialog);
    for (const auto& cmd : commands) {
        cmdList->addItem(QString("%1  —  %2").arg(cmd.name, -14).arg(cmd.usage));
    }
    cmdList->setCurrentRow(0);
    layout->addWidget(cmdList);

    // Description label (updates on selection)
    auto* descLabel = new QLabel(&dialog);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { background: #f0f0f0; padding: 6px; border-radius: 4px; }");
    descLabel->setText(commands[0].description);
    layout->addWidget(descLabel);

    // Arguments input
    auto* argsLayout = new QHBoxLayout();
    argsLayout->addWidget(new QLabel("Arguments:", &dialog));
    auto* argsEdit = new QLineEdit(&dialog);
    argsEdit->setPlaceholderText("e.g. 101 \"Widget\" 5 A1");
    argsLayout->addWidget(argsEdit);
    layout->addLayout(argsLayout);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    auto* queueBtn     = new QPushButton("Queue && Run", &dialog);
    auto* queueOnlyBtn = new QPushButton("Queue Only", &dialog);
    auto* cancelBtn    = new QPushButton("Cancel", &dialog);
    btnLayout->addWidget(queueBtn);
    btnLayout->addWidget(queueOnlyBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // Update description when selection changes
    connect(cmdList, &QListWidget::currentRowChanged, [&](int row) {
        if (row >= 0 && row < commands.size())
            descLabel->setText(commands[row].description);
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    // Queue & Run: enqueue, process immediately, refresh table
    connect(queueBtn, &QPushButton::clicked, [&]() {
        int row = cmdList->currentRow();
        if (row < 0) return;
        QString raw = commands[row].name;
        QString args = argsEdit->text().trimmed();
        if (!args.isEmpty()) raw += " " + args;

        wmsController.enqueueTask(raw.toStdString());
        wmsController.processTasks(0);
        wmsController.reloadInventory();
        loadInventory();

        ui->statusbar->showMessage(
            QString("Executed: %1").arg(raw), 4000);
        dialog.accept();
    });

    // Queue Only: just enqueue, don't process yet
    connect(queueOnlyBtn, &QPushButton::clicked, [&]() {
        int row = cmdList->currentRow();
        if (row < 0) return;
        QString raw = commands[row].name;
        QString args = argsEdit->text().trimmed();
        if (!args.isEmpty()) raw += " " + args;

        wmsController.enqueueTask(raw.toStdString());
        ui->statusbar->showMessage(
            QString("Task queued (%1). Queue size: %2")
                .arg(raw)
                .arg(wmsController.queueSize()), 4000);
        dialog.accept();
    });

    dialog.exec();
}

// ─── Slot: Run Queue ─────────────────────────────────────────────────────────
void Main::onRunQueue()
{
    const size_t queued = wmsController.queueSize();
    if (queued == 0) {
        ui->statusbar->showMessage("Queue is empty.", 3000);
        return;
    }

    wmsController.processTasks(0);
    wmsController.reloadInventory();   // Force DB → memory re-sync
    loadInventory();
    ui->statusbar->showMessage(
        QString("Processed %1 queued task(s).").arg(static_cast<qulonglong>(queued)),
        3000
    );
}

// ─── Receipt preview dialog ─────────────────────────────────────────────────
void Main::showReceiptPreview(const Receipt& receipt)
{
    QDialog preview(this);
    preview.setWindowTitle(QString("Receipt %1").arg(
        QString::fromStdString(receipt.getReceiptNumber())));
    preview.setMinimumSize(500, 400);

    auto* layout = new QVBoxLayout(&preview);

    // Header info
    auto* headerLabel = new QLabel(
        QString("<b>Receipt:</b> %1<br>"
                "<b>Subtotal:</b> %2<br>"
                "<b>Tax (14%):</b> %3<br>"
                "<b>Total:</b> %4")
            .arg(QString::fromStdString(receipt.getReceiptNumber()))
            .arg(receipt.subtotal(), 0, 'f', 2)
            .arg(receipt.tax(), 0, 'f', 2)
            .arg(receipt.total(), 0, 'f', 2),
        &preview);
    layout->addWidget(headerLabel);

    // Items table — we'll load from DB for this receipt
    auto* table = new QTableWidget(&preview);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Item ID", "Name", "Qty", "Unit Price", "Line Total"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Load items for this receipt from DB
    try {
        SQLite::Statement itemQuery(wmsController.getDB(),
            "SELECT item_id, name, quantity, unit_price, line_total "
            "FROM receipt_items WHERE receipt_number = ?");
        itemQuery.bind(1, receipt.getReceiptNumber());

        while (itemQuery.executeStep()) {
            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(itemQuery.getColumn(0).getInt())));
            table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(itemQuery.getColumn(1).getString())));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(itemQuery.getColumn(2).getInt())));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(itemQuery.getColumn(3).getDouble(), 'f', 2)));
            table->setItem(row, 4, new QTableWidgetItem(QString::number(itemQuery.getColumn(4).getDouble(), 'f', 2)));
        }
    } catch (...) {
        // Table stays empty if query fails
    }

    layout->addWidget(table);

    // Close button
    auto* closeBtn = new QPushButton("Close", &preview);
    connect(closeBtn, &QPushButton::clicked, &preview, &QDialog::accept);
    layout->addWidget(closeBtn);

    preview.exec();
}

// ─── Slot: Generate Receipt (multi-item) ─────────────────────────────────────
void Main::onGenerateReceipt()
{
    // Collect all selected rows
    QList<int> selectedRows;
    const auto selectedItems = ui->inventoryTable->selectionModel()->selectedRows();
    for (const auto& idx : selectedItems) {
        selectedRows.append(idx.row());
    }

    if (selectedRows.isEmpty()) {
        QMessageBox::information(this, "No Selection",
            "Please select one or more rows to generate a receipt.");
        return;
    }

    // Gather items from the selected rows
    struct SelectedItem {
        int id;
        QString name;
        int maxQty;
        double price;
    };
    QList<SelectedItem> itemList;

    for (int row : selectedRows) {
        SelectedItem si;
        si.id     = ui->inventoryTable->item(row, 0)->text().toInt();
        si.name   = ui->inventoryTable->item(row, 1)->text();
        si.maxQty = ui->inventoryTable->item(row, 2)->text().toInt();
        si.price  = ui->inventoryTable->item(row, 4)->text().toDouble();
        if (si.maxQty <= 0) continue; // skip out-of-stock items
        itemList.append(si);
    }

    if (itemList.isEmpty()) {
        QMessageBox::warning(this, "Out of Stock",
            "All selected items are out of stock.");
        return;
    }

    // Dialog: let user set qty/price per item + customer
    QDialog dialog(this);
    dialog.setWindowTitle("Generate Receipt");
    dialog.setMinimumWidth(450);
    auto* mainLayout = new QVBoxLayout(&dialog);

    // Customer name
    auto* customerLayout = new QHBoxLayout();
    customerLayout->addWidget(new QLabel("Customer:", &dialog));
    auto* customerEdit = new QLineEdit(&dialog);
    customerLayout->addWidget(customerEdit);
    mainLayout->addLayout(customerLayout);

    // Items table with editable qty/price
    auto* itemTable = new QTableWidget(itemList.size(), 4, &dialog);
    itemTable->setHorizontalHeaderLabels({"Item", "Max Qty", "Quantity", "Unit Price"});
    itemTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QList<QSpinBox*> qtySpins;
    QList<QDoubleSpinBox*> priceSpins;

    for (int i = 0; i < itemList.size(); ++i) {
        const auto& si = itemList[i];
        itemTable->setItem(i, 0, new QTableWidgetItem(
            QString("#%1 %2").arg(si.id).arg(si.name)));
        itemTable->setItem(i, 1, new QTableWidgetItem(QString::number(si.maxQty)));

        auto* qtySpin = new QSpinBox(&dialog);
        qtySpin->setRange(1, si.maxQty);
        qtySpin->setValue(1);
        itemTable->setCellWidget(i, 2, qtySpin);
        qtySpins.append(qtySpin);

        auto* priceSpin = new QDoubleSpinBox(&dialog);
        priceSpin->setDecimals(2);
        priceSpin->setRange(0.0, 1000000000.0);
        priceSpin->setValue(si.price);
        itemTable->setCellWidget(i, 3, priceSpin);
        priceSpins.append(priceSpin);
    }

    mainLayout->addWidget(itemTable);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, &dialog);
    mainLayout->addWidget(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    // Build the receipt
    Receipt receipt;
    if (!customerEdit->text().trimmed().isEmpty()) {
        receipt.setCustomer(customerEdit->text().trimmed().toStdString());
    }

    try {
        for (int i = 0; i < itemList.size(); ++i) {
            auto item = wmsController.getItem(itemList[i].id);
            if (!item) continue;

            int qty = qtySpins[i]->value();
            double price = priceSpins[i]->value();
            receipt.addItem(*item, qty, price);
        }

        receipt.saveToDB(wmsController.getDB());
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Receipt Failed",
            QString("Could not save receipt: %1").arg(e.what()));
        return;
    }

    // Deduct stock for each item
    bool stockWarning = false;
    for (int i = 0; i < itemList.size(); ++i) {
        int qty = qtySpins[i]->value();
        if (!wmsController.adjustStock(itemList[i].id, -qty)) {
            stockWarning = true;
        }
    }

    loadInventory();

    if (stockWarning) {
        QMessageBox::warning(this, "Stock Warning",
            "Receipt saved, but stock could not be deducted for some items.");
    }

    ui->statusbar->showMessage(
        QString("Receipt %1 saved.").arg(
            QString::fromStdString(receipt.getReceiptNumber())), 4000);

    // Show the receipt preview
    showReceiptPreview(receipt);
}

// ─── Slot: Receipt History ───────────────────────────────────────────────────
void Main::onReceiptHistory()
{
    std::vector<Receipt> receipts;
    try {
        receipts = Receipt::loadHistory(wmsController.getDB());
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error",
            QString("Could not load receipts: %1").arg(e.what()));
        return;
    }

    if (receipts.empty()) {
        QMessageBox::information(this, "No Receipts", "No receipts have been generated yet.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Receipt History");
    dialog.setMinimumSize(600, 400);
    auto* layout = new QVBoxLayout(&dialog);

    auto* table = new QTableWidget(static_cast<int>(receipts.size()), 3, &dialog);
    table->setHorizontalHeaderLabels({"Receipt #", "Customer", "Total"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    for (int i = 0; i < static_cast<int>(receipts.size()); ++i) {
        const auto& r = receipts[static_cast<size_t>(i)];
        table->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(r.getReceiptNumber())));
        table->setItem(i, 1, new QTableWidgetItem(
            QString::fromStdString(r.getCustomerName())));
        table->setItem(i, 2, new QTableWidgetItem(
            QString::number(r.total(), 'f', 2)));
    }

    layout->addWidget(table);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    auto* viewBtn  = new QPushButton("View Receipt", &dialog);
    auto* closeBtn = new QPushButton("Close", &dialog);
    btnLayout->addWidget(viewBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    // View selected receipt
    connect(viewBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a receipt.");
            return;
        }
        showReceiptPreview(receipts[static_cast<size_t>(row)]);
    });

    // Double-click to view
    connect(table, &QTableWidget::cellDoubleClicked, [&](int row, int) {
        if (row >= 0 && row < static_cast<int>(receipts.size())) {
            showReceiptPreview(receipts[static_cast<size_t>(row)]);
        }
    });

    dialog.exec();
}

// ─── Slot: Export CSV ────────────────────────────────────────────────────────
void Main::onExportCSV()
{
    std::vector<Receipt> receipts;
    try {
        receipts = Receipt::loadHistory(wmsController.getDB());
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error",
            QString("Could not load receipts: %1").arg(e.what()));
        return;
    }

    if (receipts.empty()) {
        QMessageBox::information(this, "No Receipts",
            "No receipts to export.");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Receipts to CSV", "receipts.csv",
        "CSV Files (*.csv);;All Files (*)");

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error",
            "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    // Header
    out << "Receipt Number,Customer,Item ID,Item Name,Location,"
           "Quantity,Unit Price,Line Total,Subtotal,Tax,Total\n";

    // For each receipt, load its items and write rows
    for (const auto& r : receipts) {
        const QString rcptNum = QString::fromStdString(r.getReceiptNumber());
        QString customer = "";
        double subtotal = 0, tax = 0, total = 0;

        // Get receipt header from DB
        try {
            SQLite::Statement hq(wmsController.getDB(),
                "SELECT customer_name, subtotal, tax, total "
                "FROM receipts WHERE receipt_number = ?");
            hq.bind(1, r.getReceiptNumber());
            if (hq.executeStep()) {
                customer = QString::fromStdString(hq.getColumn(0).getString());
                subtotal = hq.getColumn(1).getDouble();
                tax      = hq.getColumn(2).getDouble();
                total    = hq.getColumn(3).getDouble();
            }
        } catch (...) {}

        // Get line items
        try {
            SQLite::Statement iq(wmsController.getDB(),
                "SELECT item_id, name, location, quantity, unit_price, line_total "
                "FROM receipt_items WHERE receipt_number = ?");
            iq.bind(1, r.getReceiptNumber());

            bool firstLine = true;
            while (iq.executeStep()) {
                // Escape customer name for CSV (wrap in quotes if it contains commas)
                QString safeCustomer = customer;
                if (safeCustomer.contains(',') || safeCustomer.contains('"')) {
                    safeCustomer = "\"" + safeCustomer.replace("\"", "\"\"") + "\"";
                }

                out << rcptNum << ","
                    << safeCustomer << ","
                    << iq.getColumn(0).getInt() << ","
                    << QString::fromStdString(iq.getColumn(1).getString()).replace(",", " ") << ","
                    << QString::fromStdString(iq.getColumn(2).getString()).replace(",", " ") << ","
                    << iq.getColumn(3).getInt() << ","
                    << QString::number(iq.getColumn(4).getDouble(), 'f', 2) << ","
                    << QString::number(iq.getColumn(5).getDouble(), 'f', 2) << ",";

                if (firstLine) {
                    out << QString::number(subtotal, 'f', 2) << ","
                        << QString::number(tax, 'f', 2) << ","
                        << QString::number(total, 'f', 2);
                    firstLine = false;
                } else {
                    out << ",,";
                }
                out << "\n";
            }

            // Receipt with no items (edge case)
            if (firstLine) {
                out << rcptNum << "," << customer << ",,,,,,,"
                    << QString::number(subtotal, 'f', 2) << ","
                    << QString::number(tax, 'f', 2) << ","
                    << QString::number(total, 'f', 2) << "\n";
            }
        } catch (...) {}
    }

    file.close();
    ui->statusbar->showMessage(
        QString("Exported %1 receipt(s) to %2").arg(receipts.size()).arg(filePath), 4000);
    QMessageBox::information(this, "Export Complete",
        QString("Receipts exported to:\n%1").arg(filePath));
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Main window;
    window.show();
    return app.exec();
}