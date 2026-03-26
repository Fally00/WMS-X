#include "main.h"
#include "ui_main.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QTextDocument>
#include <QApplication>
#include <QInputDialog>
#include <QPrintDialog>
#include <QListWidget>
#include <QFileDialog>
#include <QHeaderView>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTextStream>
#include <QTabWidget>
#include <QDateTime>
#include <QTextEdit>
#include <QDateEdit>
#include <QCheckBox>
#include <QComboBox>
#include <algorithm>
#include <QSpinBox>
#include <QPrinter>
#include <optional>
#include <iomanip>
#include <sstream>
#include <QDialog>
#include <QString>
#include <QLabel>
#include <QEvent>
#include <QFile>
#include <QFont>
#include <cstdlib>

#ifdef WMS_GUI_HAS_CHARTS
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QPainter>
#endif

// ─── Constructor ─────────────────────────────────────────────────────────────
Main::Main(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Main)
    , wmsController(std::getenv("WMS_DB_PATH") ? std::getenv("WMS_DB_PATH") : "inventory.db")
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
    connect(ui->customersBtn,      &QPushButton::clicked, this, &Main::onManageCustomers);
    connect(ui->reportsBtn,       &QPushButton::clicked, this, &Main::onOpenReports);
    connect(ui->searchLineEdit,   &QLineEdit::textChanged, this, &Main::onSearchTextChanged);
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
        table->setItem(row, 8,  new QTableWidgetItem(QString::fromStdString(item.getBarcode())));

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
        table->setItem(row, 9, statusItem);

        table->setItem(row, 10,  new QTableWidgetItem(formatTime(item.getCreatedAt())));
        table->setItem(row, 11, new QTableWidgetItem(formatTime(item.getModifiedAt())));
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
    QLineEdit *priceEdit = new QLineEdit(&dialog);
    QLineEdit *unitEdit = new QLineEdit(&dialog);
    QLineEdit *categoryEdit = new QLineEdit(&dialog);
    QLineEdit *barcodeEdit = new QLineEdit(&dialog);
    barcodeEdit->setPlaceholderText("Optional");

    form.addRow("ID:",       idEdit);
    form.addRow("Name:",     nameEdit);
    form.addRow("Quantity:", qtyEdit);
    form.addRow("Location:", locEdit);
    form.addRow("Price:",    priceEdit);
    form.addRow("Unit:",     unitEdit);
    form.addRow("Category:", categoryEdit);
    form.addRow("Barcode:",  barcodeEdit);

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
                               qty, locEdit->text().toStdString(),
                               barcodeEdit->text().trimmed().toStdString())) {
        QMessageBox::warning(this, "Failed", "Item already exists, invalid data, or duplicate barcode.");
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
    QLineEdit *barcodeEdit = new QLineEdit(
        ui->inventoryTable->item(selectedRow, 8) ? ui->inventoryTable->item(selectedRow, 8)->text() : QString(),
        &dialog);
    barcodeEdit->setPlaceholderText("Optional");

    form.addRow("Name:",     nameEdit);
    form.addRow("Quantity:", qtyEdit);
    form.addRow("Location:", locEdit);
    form.addRow("Price:",    priceEdit);
    form.addRow("Barcode:",  barcodeEdit);

    auto* printBarcodeBtn = new QPushButton("Print label…", &dialog);
    form.addRow(printBarcodeBtn);
    connect(printBarcodeBtn, &QPushButton::clicked, &dialog, [&, id]() {
        QString text = barcodeEdit->text().trimmed();
        if (text.isEmpty())
            text = QString("ID %1").arg(id);
        QDialog printDlg(&dialog);
        printDlg.setWindowTitle("Barcode label");
        auto* vl = new QVBoxLayout(&printDlg);
        auto* lab = new QLabel(text, &printDlg);
        lab->setStyleSheet("QLabel { font-family: Consolas, monospace; font-size: 28px; padding: 24px; }");
        lab->setAlignment(Qt::AlignCenter);
        vl->addWidget(lab);
        auto* h = new QHBoxLayout();
        auto* pb = new QPushButton("Print…", &printDlg);
        auto* cl = new QPushButton("Close", &printDlg);
        h->addStretch();
        h->addWidget(pb);
        h->addWidget(cl);
        vl->addLayout(h);
        connect(cl, &QPushButton::clicked, &printDlg, &QDialog::accept);
        connect(pb, &QPushButton::clicked, &printDlg, [text]() {
            QPrinter printer(QPrinter::HighResolution);
            QPrintDialog pd(&printer);
            if (pd.exec() != QDialog::Accepted) return;
            QTextDocument doc;
            QFont f("Consolas", 22);
            doc.setDefaultFont(f);
            doc.setPlainText(text);
            doc.print(&printer);
        });
        printDlg.exec();
    });

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
    std::optional<std::string>  bc    = barcodeEdit->text().trimmed().isEmpty()
                                        ? std::nullopt
                                        : std::optional<std::string>(barcodeEdit->text().trimmed().toStdString());

    if (!wmsController.updateItem(id, name, qty, loc, price, bc)) {
        QMessageBox::warning(this, "Failed", "Could not update item (duplicate barcode?).");
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

void Main::onSearchTextChanged(const QString& text)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (text.isEmpty()) {
        searchBurstStartMs = -1;
        return;
    }
    if (searchBurstStartMs < 0 || (now - searchLastKeyMs) > 100)
        searchBurstStartMs = now;
    searchLastKeyMs = now;
}

// ─── Slot: Search ────────────────────────────────────────────────────────────
void Main::onSearch()
{
    QString query = ui->searchLineEdit->text().trimmed();
    if (query.isEmpty()) {
        loadInventory();
        searchBurstStartMs = -1;
        return;
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    const bool scanLike = (searchBurstStartMs >= 0
        && (now - searchBurstStartMs) <= 100
        && !query.isEmpty());

    if (scanLike) {
        auto item = wmsController.getItemByBarcode(query.toStdString());
        if (item.has_value()) {
            populateTable({*item});
            ui->inventoryTable->selectRow(0);
            ui->inventoryTable->setFocus();
            ui->statusbar->showMessage(
                QString("Barcode match: %1").arg(QString::fromStdString(item->getName())), 5000);
        } else {
            populateTable({});
            ui->statusbar->showMessage(
                QString("No item found for barcode: %1").arg(query), 5000);
        }
        searchBurstStartMs = -1;
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
    descLabel->setStyleSheet("QLabel { background: #2d2d2d; color: #e0e0e0; padding: 8px; border-radius: 4px; font-size: 13px; }");
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

// ─── Receipt preview dialog (monospaced text format) ────────────────────────
void Main::showReceiptPreview(const Receipt& receipt)
{
    QDialog preview(this);
    preview.setWindowTitle(QString("Receipt %1").arg(
        QString::fromStdString(receipt.getReceiptNumber())));
    preview.setMinimumSize(520, 460);

    auto* layout = new QVBoxLayout(&preview);

    // Build the receipt text in CLI-style format
    QString text;
    text += "=====================================\n";
    text += "        WMS-X  RECEIPT\n";
    text += "-------------------------------------\n";
    text += QString("Receipt : %1\n").arg(QString::fromStdString(receipt.getReceiptNumber()));

    // Timestamp
    auto tp = receipt.getTimestamp();
    qint64 epoch = std::chrono::duration_cast<std::chrono::seconds>(
        tp.time_since_epoch()).count();
    text += QString("Date    : %1\n").arg(
        QDateTime::fromSecsSinceEpoch(epoch).toString("yyyy-MM-dd HH:mm:ss"));

    // Customer / Supplier name
    QString customer = QString::fromStdString(receipt.getCustomerName());
    if (!customer.isEmpty()) {
        text += QString("Customer: %1\n").arg(customer);
    }

    // Try to load linked customer details from DB
    try {
        SQLite::Statement cq(wmsController.getDB(),
            "SELECT customer_id FROM receipts WHERE receipt_number = ?");
        cq.bind(1, receipt.getReceiptNumber());
        if (cq.executeStep() && !cq.getColumn(0).isNull()) {
            int custId = cq.getColumn(0).getInt();
            auto cust = wmsController.getCustomer(custId);
            if (cust.has_value()) {
                if (!cust->getPhone().empty())
                    text += QString("Phone   : %1\n").arg(QString::fromStdString(cust->getPhone()));
                if (!cust->getAddress().empty())
                    text += QString("Address : %1\n").arg(QString::fromStdString(cust->getAddress()));
            }
        }
    } catch (...) {}

    // Try to load supplier from DB
    try {
        SQLite::Statement sq(wmsController.getDB(),
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

    // Load items from DB
    try {
        SQLite::Statement itemQuery(wmsController.getDB(),
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

    // Display in a monospaced QTextEdit (read-only)
    auto* textEdit = new QTextEdit(&preview);
    textEdit->setReadOnly(true);
    textEdit->setPlainText(text);
    textEdit->setStyleSheet(
        "QTextEdit { font-family: 'Consolas', 'Courier New', monospace; "
        "font-size: 13px; background: #1e1e1e; color: #d4d4d4; "
        "padding: 12px; border-radius: 4px; }");
    layout->addWidget(textEdit);

    // Close button
    auto* closeBtn = new QPushButton("Close", &preview);
    connect(closeBtn, &QPushButton::clicked, &preview, &QDialog::accept);
    layout->addWidget(closeBtn);

    preview.exec();
}

// ─── Slot: Generate Receipt (multi-item with customer lookup) ────────────────
void Main::onGenerateReceipt()
{
    const auto normalizeScannedBarcode = [](const QString& raw) {
        QString cleaned;
        cleaned.reserve(raw.size());
        for (const QChar ch : raw) {
            // Keep printable characters, ignore hidden scanner suffixes like CR/LF/TAB.
            if (ch.isPrint()) cleaned.append(ch);
        }
        return cleaned.trimmed();
    };

    QList<int> selectedRows;
    const auto selectedItems = ui->inventoryTable->selectionModel()->selectedRows();
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
        si.id     = ui->inventoryTable->item(row, 0)->text().toInt();
        si.name   = ui->inventoryTable->item(row, 1)->text();
        si.maxQty = ui->inventoryTable->item(row, 2)->text().toInt();
        si.price  = ui->inventoryTable->item(row, 4)->text().toDouble();
        if (si.maxQty <= 0) continue;
        itemList.append(si);
    }

    if (!selectedRows.isEmpty() && itemList.isEmpty()) {
        QMessageBox::warning(this, "Out of Stock",
            "All selected items are out of stock.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Generate Receipt");
    dialog.setMinimumWidth(500);
    auto* mainLayout = new QVBoxLayout(&dialog);

    auto* scanForm = new QFormLayout();
    auto* scanRow = new QHBoxLayout();
    auto* scanEdit = new QLineEdit(&dialog);
    scanEdit->setPlaceholderText("Scan or enter barcode");
    auto* addScanBtn = new QPushButton("Add", &dialog);
    scanRow->addWidget(scanEdit, 1);
    scanRow->addWidget(addScanBtn);
    auto* scanWarn = new QLabel(&dialog);
    scanWarn->setStyleSheet("QLabel { color: #c04040; }");
    scanForm->addRow("Add by barcode:", scanRow);
    scanForm->addRow(scanWarn);
    mainLayout->addLayout(scanForm);

    // ── Customer section with lookup/manual toggle ──
    auto* custGroup = new QFormLayout();

    // Manual entry toggle
    auto* manualCheck = new QCheckBox("Manual entry (type customer name)", &dialog);
    manualCheck->setChecked(false);
    custGroup->addRow(manualCheck);

    // Customer lookup controls
    auto* lookupLayout = new QHBoxLayout();
    auto* custSearchEdit = new QLineEdit(&dialog);
    custSearchEdit->setPlaceholderText("Search customer by name or ID...");
    auto* lookupBtn = new QPushButton("Lookup", &dialog);
    lookupLayout->addWidget(custSearchEdit);
    lookupLayout->addWidget(lookupBtn);
    custGroup->addRow("Customer:", lookupLayout);

    // Customer results list
    auto* custResultList = new QListWidget(&dialog);
    custResultList->setMaximumHeight(100);
    custResultList->setVisible(false);
    custGroup->addRow("", custResultList);

    // Selected customer display (read-only)
    auto* custInfoLabel = new QLabel("No customer selected", &dialog);
    custInfoLabel->setStyleSheet("QLabel { background: #2d2d2d; color: #b0b0b0; padding: 6px; border-radius: 4px; }");
    custGroup->addRow("Selected:", custInfoLabel);

    // Manual name input (hidden by default)
    auto* customerEdit = new QLineEdit(&dialog);
    customerEdit->setPlaceholderText("Customer name (required)");
    customerEdit->setVisible(false);
    custGroup->addRow("Name:", customerEdit);

    // Supplier field
    auto* supplierEdit = new QLineEdit(&dialog);
    supplierEdit->setPlaceholderText("Supplier / company name (optional)");
    custGroup->addRow("Supplier:", supplierEdit);

    mainLayout->addLayout(custGroup);

    // Track selected customer ID
    int selectedCustomerId = -1;

    // Toggle manual/lookup mode
    connect(manualCheck, &QCheckBox::toggled, [&](bool manual) {
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

    // Lookup button
    connect(lookupBtn, &QPushButton::clicked, [&]() {
        QString query = custSearchEdit->text().trimmed();
        if (query.isEmpty()) return;

        custResultList->clear();
        custResultList->setVisible(true);

        // Try ID first
        bool isId;
        int searchId = query.toInt(&isId);
        if (isId) {
            auto cust = wmsController.getCustomer(searchId);
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
            auto results = wmsController.searchCustomerByName(query.toStdString());
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

    // Select customer from results
    connect(custResultList, &QListWidget::itemClicked, [&](QListWidgetItem* item) {
        QVariant data = item->data(Qt::UserRole);
        if (!data.isValid()) return;

        selectedCustomerId = data.toInt();
        auto cust = wmsController.getCustomer(selectedCustomerId);
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

    const auto addItemFromScan = [&]() {
        QString bc = normalizeScannedBarcode(scanEdit->text());
        scanEdit->clear();
        scanWarn->clear();
        if (bc.isEmpty()) {
            scanEdit->setFocus();
            return;
        }

        auto inv = wmsController.getItemByBarcode(bc.toStdString());
        // Fallback: if no barcode match and scanned text is numeric, treat it as item ID.
        if (!inv.has_value()) {
            bool okId = false;
            int id = bc.toInt(&okId);
            if (okId) inv = wmsController.getItem(id);
        }
        if (!inv.has_value()) {
            scanWarn->setText("Item not found (barcode/ID)");
            scanEdit->setFocus();
            return;
        }
        if (inv->getQuantity() <= 0) {
            scanWarn->setText("Out of stock");
            scanEdit->setFocus();
            return;
        }

        int mergeIdx = -1;
        for (int j = 0; j < itemList.size(); ++j) {
            if (itemList[j].id == inv->getId()) {
                mergeIdx = j;
                break;
            }
        }

        if (mergeIdx >= 0) {
            auto fresh = wmsController.getItem(inv->getId());
            if (!fresh.has_value()) return;
            itemList[mergeIdx].maxQty = fresh->getQuantity();
            itemTable->item(mergeIdx, 1)->setText(QString::number(itemList[mergeIdx].maxQty));
            QSpinBox* sp = qtySpins[mergeIdx];
            sp->setMaximum(std::max(1, itemList[mergeIdx].maxQty));
            if (sp->value() < sp->maximum())
                sp->setValue(sp->value() + 1);
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
        scanEdit->setFocus();
    };

    connect(scanEdit, &QLineEdit::returnPressed, &dialog, addItemFromScan);
    connect(addScanBtn, &QPushButton::clicked, &dialog, addItemFromScan);

    mainLayout->addWidget(itemTable);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, &dialog);
    mainLayout->addWidget(&buttons);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (itemList.isEmpty()) {
            QMessageBox::warning(&dialog, "Receipt",
                "Add at least one item (select rows in the table or scan a barcode).");
            return;
        }
        dialog.accept();
    });

    if (dialog.exec() != QDialog::Accepted) return;

    // Validate customer input
    bool isManual = manualCheck->isChecked();
    if (isManual && customerEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Customer",
            "Please enter a customer name.");
        return;
    }
    if (!isManual && selectedCustomerId < 0) {
        QMessageBox::warning(this, "No Customer Selected",
            "Please look up and select a customer, or switch to manual entry.");
        return;
    }

    // Build the receipt
    Receipt receipt;

    if (isManual) {
        receipt.setCustomer(customerEdit->text().trimmed().toStdString());
    } else {
        auto cust = wmsController.getCustomer(selectedCustomerId);
        if (cust.has_value()) {
            receipt.setCustomer(cust->getName(), cust->getPhone(), cust->getEmail());
        }
    }

    // Store supplier name to save after receipt is created
    QString supplierName = supplierEdit->text().trimmed();

    bool stockWarning = false;
    try {
        SQLite::Transaction transaction(wmsController.getDB());
        for (int i = 0; i < itemList.size(); ++i) {
            auto item = wmsController.getItem(itemList[i].id);
            if (!item) continue;

            int qty = qtySpins[i]->value();
            double price = priceSpins[i]->value();
            receipt.addItem(*item, qty, price);
        }

        receipt.saveToDB(wmsController.getDB());

        // Save customer_id if linked
        if (!isManual && selectedCustomerId >= 0) {
            try {
                SQLite::Statement upd(wmsController.getDB(),
                    "UPDATE receipts SET customer_id = ? WHERE receipt_number = ?");
                upd.bind(1, selectedCustomerId);
                upd.bind(2, receipt.getReceiptNumber());
                upd.exec();
            } catch (...) {}
        }

        // Save supplier name if provided
        if (!supplierName.isEmpty()) {
            try {
                // Add supplier_name column if it doesn't exist yet
                wmsController.getDB().exec(
                    "ALTER TABLE receipts ADD COLUMN supplier_name TEXT DEFAULT ''");
            } catch (...) {} // column already exists
            SQLite::Statement upd(wmsController.getDB(),
                "UPDATE receipts SET supplier_name = ? WHERE receipt_number = ?");
            upd.bind(1, supplierName.toStdString());
            upd.bind(2, receipt.getReceiptNumber());
            upd.exec();
        }

        // Deduct stock for each item
        for (int i = 0; i < itemList.size(); ++i) {
            int qty = qtySpins[i]->value();
            if (!wmsController.adjustStock(itemList[i].id, -qty)) {
                stockWarning = true;
            }
        }

        transaction.commit();
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Receipt Failed",
            QString("Could not save receipt: %1").arg(e.what()));
        return;
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

void Main::onReceiptHistory()
{
    openReceiptHistoryDialog(std::nullopt);
}

// ─── Receipt History (with customer filter) ────────────────────────────────
void Main::openReceiptHistoryDialog(std::optional<int> preselectCustomerId)
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
    dialog.setMinimumSize(750, 480);
    auto* layout = new QVBoxLayout(&dialog);

    // ── Customer filter ──
    auto* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Filter by Customer:", &dialog));
    auto* custFilterCombo = new QComboBox(&dialog);
    custFilterCombo->addItem("All Customers", -1);

    // Populate combo with customers from DB
    auto allCustomers = wmsController.getAllCustomers();
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

    // Lambda to populate the receipt table
    auto populateReceiptTable = [&](int filterCustId) {
        table->setRowCount(0);
        for (int i = 0; i < static_cast<int>(receipts.size()); ++i) {
            const auto& r = receipts[static_cast<size_t>(i)];

            // Filter by customer_id if requested
            if (filterCustId >= 0) {
                try {
                    SQLite::Statement cq(wmsController.getDB(),
                        "SELECT customer_id FROM receipts WHERE receipt_number = ?");
                    cq.bind(1, r.getReceiptNumber());
                    if (cq.executeStep()) {
                        if (cq.getColumn(0).isNull() || cq.getColumn(0).getInt() != filterCustId)
                            continue;
                    } else {
                        continue;
                    }
                } catch (...) { continue; }
            }

            int row = table->rowCount();
            table->insertRow(row);

            table->setItem(row, 0, new QTableWidgetItem(
                QString::fromStdString(r.getReceiptNumber())));
            table->setItem(row, 1, new QTableWidgetItem(
                QString::fromStdString(r.getCustomerName())));
            qint64 epoch = std::chrono::duration_cast<std::chrono::seconds>(
                r.getTimestamp().time_since_epoch()).count();
            table->setItem(row, 2, new QTableWidgetItem(
                QDateTime::fromSecsSinceEpoch(epoch).toString("yyyy-MM-dd HH:mm")));
            table->setItem(row, 3, new QTableWidgetItem(
                QString::number(r.total(), 'f', 2)));

            // Store the original index so we can reference the correct receipt
            table->item(row, 0)->setData(Qt::UserRole, i);
        }
    };

    populateReceiptTable(custFilterCombo->currentData().toInt());

    // Filter on combo change
    connect(custFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int idx) {
        int custId = custFilterCombo->itemData(idx).toInt();
        populateReceiptTable(custId);
    });

    layout->addWidget(table);

    // Buttons
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

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    // Helper to get original receipt index from table row
    auto getReceiptIndex = [&](int row) -> int {
        if (row < 0 || !table->item(row, 0)) return -1;
        return table->item(row, 0)->data(Qt::UserRole).toInt();
    };

    // View selected receipt
    connect(viewBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        int idx = getReceiptIndex(row);
        if (idx < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a receipt.");
            return;
        }
        showReceiptPreview(receipts[static_cast<size_t>(idx)]);
    });

    // Export selected receipt
    connect(exportBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        int idx = getReceiptIndex(row);
        if (idx < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a receipt to export.");
            return;
        }
        exportReceiptsToCSV({ receipts[static_cast<size_t>(idx)] });
    });

    // Delete selected receipt
    connect(deleteBtn, &QPushButton::clicked, [&]() {
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
            Receipt::deleteFromDB(wmsController.getDB(), r.getReceiptNumber());
            receipts.erase(receipts.begin() + idx);
            int custId = custFilterCombo->currentData().toInt();
            populateReceiptTable(custId);
            ui->statusbar->showMessage("Receipt deleted.", 3000);
        } catch (const std::exception& e) {
            QMessageBox::warning(&dialog, "Error",
                QString("Could not delete receipt: %1").arg(e.what()));
        }
    });

    // Double-click to view
    connect(table, &QTableWidget::cellDoubleClicked, [&](int row, int) {
        int idx = getReceiptIndex(row);
        if (idx >= 0 && idx < static_cast<int>(receipts.size())) {
            showReceiptPreview(receipts[static_cast<size_t>(idx)]);
        }
    });

    dialog.exec();
}

// ─── Slot: Export CSV (exports ALL receipts) ─────────────────────────────────
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
        QMessageBox::information(this, "No Receipts", "No receipts to export.");
        return;
    }

    exportReceiptsToCSV(receipts);
}

// ─── Helper: Export receipts as printable formatted text ─────────────────────
void Main::exportReceiptsToCSV(const std::vector<Receipt>& receipts)
{
    if (receipts.empty()) {
        QMessageBox::information(this, "No Receipts", "Nothing to export.");
        return;
    }

    // Default filename
    QString defaultName = (receipts.size() == 1)
        ? QString::fromStdString(receipts[0].getReceiptNumber()) + ".txt"
        : "receipts.txt";

    QString filePath = QFileDialog::getSaveFileName(
        this, "Export Receipt", defaultName,
        "Text Files (*.txt);;All Files (*)");

    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "File Error",
            "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    for (size_t idx = 0; idx < receipts.size(); ++idx) {
        const auto& r = receipts[idx];

        // Timestamp
        qint64 epoch = std::chrono::duration_cast<std::chrono::seconds>(
            r.getTimestamp().time_since_epoch()).count();
        QString dateStr = QDateTime::fromSecsSinceEpoch(epoch)
            .toString("yyyy-MM-dd HH:mm:ss");

        out << "=====================================\n";
        out << "        WMS-X  RECEIPT\n";
        out << "-------------------------------------\n";
        out << QString("Receipt : %1\n").arg(
            QString::fromStdString(r.getReceiptNumber()));
        out << QString("Date    : %1\n").arg(dateStr);

        QString customer = QString::fromStdString(r.getCustomerName());
        if (!customer.isEmpty())
            out << QString("Customer: %1\n").arg(customer);

        // Include linked customer phone & address if available
        try {
            SQLite::Statement cq(wmsController.getDB(),
                "SELECT customer_id FROM receipts WHERE receipt_number = ?");
            cq.bind(1, r.getReceiptNumber());
            if (cq.executeStep() && !cq.getColumn(0).isNull()) {
                int custId = cq.getColumn(0).getInt();
                auto cust = wmsController.getCustomer(custId);
                if (cust.has_value()) {
                    if (!cust->getPhone().empty())
                        out << QString("Phone   : %1\n").arg(QString::fromStdString(cust->getPhone()));
                    if (!cust->getAddress().empty())
                        out << QString("Address : %1\n").arg(QString::fromStdString(cust->getAddress()));
                }
            }
        } catch (...) {}

        // Try to load supplier from DB
        try {
            SQLite::Statement sq(wmsController.getDB(),
                "SELECT supplier_name FROM receipts WHERE receipt_number = ?");
            sq.bind(1, r.getReceiptNumber());
            if (sq.executeStep()) {
                QString supplier = QString::fromStdString(
                    sq.getColumn(0).getString());
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

        // Load items from DB
        try {
            SQLite::Statement iq(wmsController.getDB(),
                "SELECT item_id, name, quantity, unit_price, line_total "
                "FROM receipt_items WHERE receipt_number = ?");
            iq.bind(1, r.getReceiptNumber());

            while (iq.executeStep()) {
                out << QString("%1%2%3%4%5\n")
                    .arg(iq.getColumn(0).getInt(), -6)
                    .arg(QString::fromStdString(
                        iq.getColumn(1).getString()).left(15), -16)
                    .arg(iq.getColumn(2).getInt(), -8)
                    .arg(QString::number(
                        iq.getColumn(3).getDouble(), 'f', 2), -10)
                    .arg(QString::number(
                        iq.getColumn(4).getDouble(), 'f', 2), -10);
            }
        } catch (...) {}

        out << "-------------------------------------\n";
        out << QString("Subtotal : %1\n").arg(
            QString::number(r.subtotal(), 'f', 2));
        out << QString("Tax (14%%): %1\n").arg(
            QString::number(r.tax(), 'f', 2));
        out << QString("TOTAL    : %1\n").arg(
            QString::number(r.total(), 'f', 2));
        out << "=====================================\n";

        // Separator between receipts
        if (idx + 1 < receipts.size())
            out << "\n\n";
    }

    file.close();
    ui->statusbar->showMessage(
        QString("Exported %1 receipt(s) to %2")
            .arg(static_cast<qulonglong>(receipts.size())).arg(filePath), 4000);
    QMessageBox::information(this, "Export Complete",
        QString("Receipt exported to:\n%1").arg(filePath));
}

// ─── Slot: Manage Customers ──────────────────────────────────────────────────
void Main::onManageCustomers()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Customer Management");
    dialog.setMinimumSize(750, 480);
    auto* layout = new QVBoxLayout(&dialog);

    // Search bar
    auto* searchLayout = new QHBoxLayout();
    auto* custSearchEdit = new QLineEdit(&dialog);
    custSearchEdit->setPlaceholderText("Search by name or ID...");
    auto* custSearchBtn = new QPushButton("Search", &dialog);
    auto* showAllBtn = new QPushButton("Show All", &dialog);
    searchLayout->addWidget(custSearchEdit);
    searchLayout->addWidget(custSearchBtn);
    searchLayout->addWidget(showAllBtn);
    layout->addLayout(searchLayout);

    // Customer table
    auto* table = new QTableWidget(0, 6, &dialog);
    table->setHorizontalHeaderLabels({"ID", "Name", "Phone", "Address", "Email", "Created At"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(table);

    // Lambda to refresh customer table
    auto refreshTable = [&](const std::vector<Customer>& customers) {
        table->setRowCount(0);
        for (const auto& c : customers) {
            int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(c.getId())));
            table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(c.getName())));
            table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(c.getPhone())));
            table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(c.getAddress())));
            table->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(c.getEmail())));
            table->setItem(row, 5, new QTableWidgetItem(
                QDateTime::fromSecsSinceEpoch(static_cast<qint64>(c.getCreatedAt()))
                    .toString("yyyy-MM-dd HH:mm")));
        }
    };

    // Load all customers initially
    refreshTable(wmsController.getAllCustomers());

    // Search
    connect(custSearchBtn, &QPushButton::clicked, [&]() {
        QString query = custSearchEdit->text().trimmed();
        if (query.isEmpty()) {
            refreshTable(wmsController.getAllCustomers());
            return;
        }

        bool isId;
        int id = query.toInt(&isId);
        if (isId) {
            auto cust = wmsController.getCustomer(id);
            if (cust.has_value()) {
                refreshTable({cust.value()});
            } else {
                table->setRowCount(0);
            }
        } else {
            refreshTable(wmsController.searchCustomerByName(query.toStdString()));
        }
    });

    connect(custSearchEdit, &QLineEdit::returnPressed, custSearchBtn, &QPushButton::click);

    // Show All
    connect(showAllBtn, &QPushButton::clicked, [&]() {
        custSearchEdit->clear();
        refreshTable(wmsController.getAllCustomers());
    });

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    auto* addBtn    = new QPushButton("Add Customer", &dialog);
    auto* editBtn   = new QPushButton("Edit Customer", &dialog);
    auto* deleteBtn = new QPushButton("Delete Customer", &dialog);
    auto* closeBtn  = new QPushButton("Close", &dialog);
    deleteBtn->setStyleSheet("QPushButton { color: #cc3333; }");
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);

    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    // ── Add Customer ──
    connect(addBtn, &QPushButton::clicked, [&]() {
        QDialog addDlg(&dialog);
        addDlg.setWindowTitle("Add New Customer");
        QFormLayout form(&addDlg);

        auto* nameEdit  = new QLineEdit(&addDlg);
        auto* phoneEdit = new QLineEdit(&addDlg);
        auto* addrEdit  = new QLineEdit(&addDlg);
        auto* emailEdit = new QLineEdit(&addDlg);

        nameEdit->setPlaceholderText("Required");
        phoneEdit->setPlaceholderText("Optional");
        addrEdit->setPlaceholderText("Optional");
        emailEdit->setPlaceholderText("Optional");

        form.addRow("Name:",    nameEdit);
        form.addRow("Phone:",   phoneEdit);
        form.addRow("Address:", addrEdit);
        form.addRow("Email:",   emailEdit);

        QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                              Qt::Horizontal, &addDlg);
        form.addRow(&btns);
        connect(&btns, &QDialogButtonBox::accepted, &addDlg, &QDialog::accept);
        connect(&btns, &QDialogButtonBox::rejected, &addDlg, &QDialog::reject);

        if (addDlg.exec() != QDialog::Accepted) return;

        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Invalid Input", "Customer name is required.");
            return;
        }

        if (!wmsController.addCustomer(
                nameEdit->text().trimmed().toStdString(),
                phoneEdit->text().trimmed().toStdString(),
                addrEdit->text().trimmed().toStdString(),
                emailEdit->text().trimmed().toStdString())) {
            QMessageBox::warning(&dialog, "Failed", "Could not add customer.");
            return;
        }

        refreshTable(wmsController.getAllCustomers());
        ui->statusbar->showMessage("Customer added.", 3000);
    });

    // ── Edit Customer ──
    connect(editBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a customer to edit.");
            return;
        }

        int custId = table->item(row, 0)->text().toInt();
        auto cust = wmsController.getCustomer(custId);
        if (!cust.has_value()) {
            QMessageBox::warning(&dialog, "Error", "Customer not found.");
            return;
        }

        QDialog editDlg(&dialog);
        editDlg.setWindowTitle(QString("Edit Customer #%1").arg(custId));
        QFormLayout form(&editDlg);

        auto* nameEdit  = new QLineEdit(QString::fromStdString(cust->getName()), &editDlg);
        auto* phoneEdit = new QLineEdit(QString::fromStdString(cust->getPhone()), &editDlg);
        auto* addrEdit  = new QLineEdit(QString::fromStdString(cust->getAddress()), &editDlg);
        auto* emailEdit = new QLineEdit(QString::fromStdString(cust->getEmail()), &editDlg);

        form.addRow("Name:",    nameEdit);
        form.addRow("Phone:",   phoneEdit);
        form.addRow("Address:", addrEdit);
        form.addRow("Email:",   emailEdit);

        QDialogButtonBox btns(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                              Qt::Horizontal, &editDlg);
        form.addRow(&btns);
        connect(&btns, &QDialogButtonBox::accepted, &editDlg, &QDialog::accept);
        connect(&btns, &QDialogButtonBox::rejected, &editDlg, &QDialog::reject);

        if (editDlg.exec() != QDialog::Accepted) return;

        std::optional<std::string> name = nameEdit->text().trimmed().isEmpty()
            ? std::nullopt : std::optional<std::string>(nameEdit->text().trimmed().toStdString());
        std::optional<std::string> phone = std::optional<std::string>(phoneEdit->text().trimmed().toStdString());
        std::optional<std::string> addr = std::optional<std::string>(addrEdit->text().trimmed().toStdString());
        std::optional<std::string> email = std::optional<std::string>(emailEdit->text().trimmed().toStdString());

        if (!wmsController.updateCustomer(custId, name, phone, addr, email)) {
            QMessageBox::warning(&dialog, "Failed", "Could not update customer.");
            return;
        }

        refreshTable(wmsController.getAllCustomers());
        ui->statusbar->showMessage("Customer updated.", 3000);
    });

    // ── Delete Customer ──
    connect(deleteBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a customer to delete.");
            return;
        }

        int custId = table->item(row, 0)->text().toInt();
        QString custName = table->item(row, 1)->text();

        auto confirm = QMessageBox::question(&dialog, "Confirm Delete",
            QString("Delete customer #%1 (%2)?\nThis cannot be undone.")
                .arg(custId).arg(custName));
        if (confirm != QMessageBox::Yes) return;

        if (!wmsController.removeCustomer(custId)) {
            QMessageBox::warning(&dialog, "Failed", "Customer not found.");
            return;
        }

        refreshTable(wmsController.getAllCustomers());
        ui->statusbar->showMessage("Customer deleted.", 3000);
    });

    // Double-click to edit
    connect(table, &QTableWidget::cellDoubleClicked, [&](int, int) {
        editBtn->click();
    });

    dialog.exec();
}

// ─── Reports & KPI dialog ────────────────────────────────────────────────────
void Main::onOpenReports()
{
    auto* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle("WMS-X — Reports");
    dlg->resize(920, 620);

    ReportEngine eng(wmsController.getDB());

    auto* root = new QVBoxLayout(dlg);

    auto* cardRow = new QHBoxLayout();
    auto* cardRev = new QLabel("—");
    auto* cardRcpt = new QLabel("—");
    auto* cardAov = new QLabel("—");
    auto* cardLow = new QLabel("—");
    for (auto* lb : {cardRev, cardRcpt, cardAov, cardLow}) {
        lb->setWordWrap(true);
        lb->setMinimumWidth(160);
        lb->setStyleSheet(
            "QLabel { background: #2a2a2a; color: #e8e8e8; padding: 12px; "
            "border-radius: 6px; font-size: 13px; }");
    }
    auto mkCard = [&](const QString& title, QLabel* body) {
        auto* box = new QVBoxLayout();
        auto* t = new QLabel(QString("<b>%1</b>").arg(title), dlg);
        t->setStyleSheet("QLabel { color: #aaa; }");
        box->addWidget(t);
        box->addWidget(body);
        auto* w = new QWidget(dlg);
        w->setLayout(box);
        cardRow->addWidget(w);
    };
    mkCard("Total Revenue (EGP)", cardRev);
    mkCard("Total Receipts", cardRcpt);
    mkCard("Avg Order (EGP)", cardAov);
    mkCard("Low-stock items (≤10)", cardLow);
    root->addLayout(cardRow);

    int lowStockThreshold = 10;
    std::string reportPeriodFrom;
    std::string reportPeriodTo;

    auto refreshSummaryCards = [&]() {
        SalesSummary s = eng.getSalesSummary(reportPeriodFrom, reportPeriodTo);
        cardRev->setText(QString::number(s.totalRevenue, 'f', 2));
        cardRcpt->setText(QString::number(s.totalReceipts));
        cardAov->setText(QString::number(s.averageOrderValue, 'f', 2));
        auto lowRows = eng.getLowStockItems(lowStockThreshold);
        cardLow->setText(QString::number(static_cast<int>(lowRows.size())));
    };
    refreshSummaryCards();

    auto* tabs = new QTabWidget(dlg);
    root->addWidget(tabs, 1);

    // ── Tab: Sales overview ──
    auto* salesPage = new QWidget(dlg);
    auto* salesLay = new QVBoxLayout(salesPage);
    auto* rangeLay = new QHBoxLayout();
    auto* fromDate = new QDateEdit(dlg);
    auto* toDate = new QDateEdit(dlg);
    fromDate->setCalendarPopup(true);
    toDate->setCalendarPopup(true);
    fromDate->setDate(QDate::currentDate().addMonths(-1));
    toDate->setDate(QDate::currentDate());
    auto* applyRangeBtn = new QPushButton("Apply range", dlg);
    rangeLay->addWidget(new QLabel("From:", dlg));
    rangeLay->addWidget(fromDate);
    rangeLay->addWidget(new QLabel("To:", dlg));
    rangeLay->addWidget(toDate);
    rangeLay->addWidget(applyRangeBtn);
    rangeLay->addStretch();
    salesLay->addLayout(rangeLay);

#ifdef WMS_GUI_HAS_CHARTS
    auto* chartView = new QChartView(dlg);
    chartView->setRenderHint(QPainter::Antialiasing);
    salesLay->addWidget(chartView, 1);
#else
    auto* dailyFallback = new QTableWidget(0, 3, dlg);
    dailyFallback->setHorizontalHeaderLabels({"Date", "Revenue (EGP)", "Receipts"});
    dailyFallback->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    salesLay->addWidget(dailyFallback, 1);
#endif

    auto applySalesRange = [&]() {
        reportPeriodFrom = fromDate->date().toString("yyyy-MM-dd").toStdString();
        reportPeriodTo = toDate->date().toString("yyyy-MM-dd").toStdString();
        refreshSummaryCards();
        auto daily = eng.getDailyRevenue(reportPeriodFrom, reportPeriodTo);
#ifdef WMS_GUI_HAS_CHARTS
        auto* chart = new QChart();
        chart->setTitle("Daily revenue");
        auto* series = new QBarSeries();
        auto* set = new QBarSet("EGP");
        QStringList cats;
        for (const auto& row : daily) {
            *set << row.revenue;
            cats << QString::fromStdString(row.date);
        }
        series->append(set);
        chart->addSeries(series);
        auto* axisX = new QBarCategoryAxis();
        axisX->append(cats);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);
        auto* axisY = new QValueAxis();
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
        chart->legend()->setVisible(false);
        chartView->setChart(chart);
#else
        dailyFallback->setRowCount(0);
        for (const auto& row : daily) {
            int r = dailyFallback->rowCount();
            dailyFallback->insertRow(r);
            dailyFallback->setItem(r, 0, new QTableWidgetItem(QString::fromStdString(row.date)));
            dailyFallback->setItem(r, 1, new QTableWidgetItem(QString::number(row.revenue, 'f', 2)));
            dailyFallback->setItem(r, 2, new QTableWidgetItem(QString::number(row.receiptCount)));
        }
#endif
    };
    connect(applyRangeBtn, &QPushButton::clicked, dlg, applySalesRange);
    applySalesRange();

    tabs->addTab(salesPage, "Sales overview");

    // ── Tab: Top items ──
    auto* topPage = new QWidget(dlg);
    auto* topLay = new QVBoxLayout(topPage);
    auto* topCtrl = new QHBoxLayout();
    auto* topLimitCombo = new QComboBox(dlg);
    topLimitCombo->addItem("Top 10", 10);
    topLimitCombo->addItem("Top 20", 20);
    topLimitCombo->addItem("Top 50", 50);
    auto* topRefreshBtn = new QPushButton("Refresh", dlg);
    topCtrl->addWidget(new QLabel("List:", dlg));
    topCtrl->addWidget(topLimitCombo);
    topCtrl->addWidget(topRefreshBtn);
    topCtrl->addStretch();
    topLay->addLayout(topCtrl);
    auto* topTable = new QTableWidget(0, 5, dlg);
    topTable->setHorizontalHeaderLabels({"Rank", "Item ID", "Name", "Qty sold", "Revenue (EGP)"});
    topTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    topLay->addWidget(topTable);

    auto fillTopItems = [&]() {
        int lim = topLimitCombo->currentData().toInt();
        auto rows = eng.getTopSellingItems(lim);
        topTable->setRowCount(0);
        int rank = 1;
        for (const auto& r : rows) {
            int row = topTable->rowCount();
            topTable->insertRow(row);
            topTable->setItem(row, 0, new QTableWidgetItem(QString::number(rank++)));
            topTable->setItem(row, 1, new QTableWidgetItem(QString::number(r.itemId)));
            topTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(r.itemName)));
            topTable->setItem(row, 3, new QTableWidgetItem(QString::number(r.totalQtySold)));
            topTable->setItem(row, 4, new QTableWidgetItem(QString::number(r.totalRevenue, 'f', 2)));
        }
    };
    connect(topRefreshBtn, &QPushButton::clicked, dlg, fillTopItems);
    connect(topLimitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), dlg, fillTopItems);
    fillTopItems();
    tabs->addTab(topPage, "Top items");

    // ── Tab: Low stock ──
    auto* lowPage = new QWidget(dlg);
    auto* lowLay = new QVBoxLayout(lowPage);
    auto* lowCtrl = new QHBoxLayout();
    auto* lowThreshSpin = new QSpinBox(dlg);
    lowThreshSpin->setRange(0, 1000000);
    lowThreshSpin->setValue(10);
    auto* lowRefreshBtn = new QPushButton("Refresh", dlg);
    auto* restockBtn = new QPushButton("Restock selected…", dlg);
    lowCtrl->addWidget(new QLabel("Threshold:", dlg));
    lowCtrl->addWidget(lowThreshSpin);
    lowCtrl->addWidget(lowRefreshBtn);
    lowCtrl->addWidget(restockBtn);
    lowCtrl->addStretch();
    lowLay->addLayout(lowCtrl);
    auto* lowTable = new QTableWidget(0, 4, dlg);
    lowTable->setHorizontalHeaderLabels({"ID", "Name", "Qty", "Location"});
    lowTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    lowTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    lowLay->addWidget(lowTable);

    auto fillLowStock = [&]() {
        lowStockThreshold = lowThreshSpin->value();
        refreshSummaryCards();
        auto rows = eng.getLowStockItems(lowStockThreshold);
        lowTable->setRowCount(0);
        for (const auto& r : rows) {
            int row = lowTable->rowCount();
            lowTable->insertRow(row);
            lowTable->setItem(row, 0, new QTableWidgetItem(QString::number(r.itemId)));
            lowTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(r.itemName)));
            lowTable->setItem(row, 2, new QTableWidgetItem(QString::number(r.currentQty)));
            lowTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(r.location)));
            QColor bg;
            bool setBg = false;
            if (r.currentQty <= 0) {
                bg = QColor(255, 200, 200);
                setBg = true;
            } else if (r.currentQty <= lowStockThreshold) {
                bg = QColor(255, 240, 180);
                setBg = true;
            }
            for (int c = 0; c < 4; ++c) {
                if (setBg) {
                    lowTable->item(row, c)->setBackground(bg);
                    lowTable->item(row, c)->setForeground(QColor("#1a1a1a"));
                }
            }
        }
    };
    connect(lowRefreshBtn, &QPushButton::clicked, dlg, fillLowStock);
    connect(lowThreshSpin, QOverload<int>::of(&QSpinBox::valueChanged), dlg, fillLowStock);
    connect(restockBtn, &QPushButton::clicked, dlg, [&]() {
        int row = lowTable->currentRow();
        if (row < 0) {
            QMessageBox::information(dlg, "Reports", "Select a low-stock item.");
            return;
        }
        int id = lowTable->item(row, 0)->text().toInt();
        QDialog udlg(dlg);
        udlg.setWindowTitle(QString("Update quantity — item %1").arg(id));
        QFormLayout form(&udlg);
        auto* qtyE = new QLineEdit(lowTable->item(row, 2)->text(), &udlg);
        form.addRow("New quantity:", qtyE);
        QDialogButtonBox ubtn(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &udlg);
        form.addRow(&ubtn);
        connect(&ubtn, &QDialogButtonBox::accepted, &udlg, &QDialog::accept);
        connect(&ubtn, &QDialogButtonBox::rejected, &udlg, &QDialog::reject);
        if (udlg.exec() != QDialog::Accepted) return;
        bool ok = false;
        int nq = qtyE->text().toInt(&ok);
        if (!ok || nq < 0) {
            QMessageBox::warning(dlg, "Reports", "Invalid quantity.");
            return;
        }
        if (!wmsController.updateItem(id, std::nullopt, nq, std::nullopt, std::nullopt, std::nullopt)) {
            QMessageBox::warning(dlg, "Reports", "Update failed.");
            return;
        }
        loadInventory();
        fillLowStock();
    });
    fillLowStock();
    tabs->addTab(lowPage, "Low stock");

    // ── Tab: Customers ──
    auto* custPage = new QWidget(dlg);
    auto* custLay = new QVBoxLayout(custPage);
    auto* custTable = new QTableWidget(0, 5, dlg);
    custTable->setHorizontalHeaderLabels({"Rank", "Customer ID", "Name", "Receipts", "Total spent (EGP)"});
    custTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    custTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    custLay->addWidget(custTable);
    auto* viewRcptBtn = new QPushButton("View receipts for selected customer", dlg);
    custLay->addWidget(viewRcptBtn);

    auto fillCustomers = [&]() {
        auto rows = eng.getTopCustomers(50);
        custTable->setRowCount(0);
        int rank = 1;
        for (const auto& r : rows) {
            int row = custTable->rowCount();
            custTable->insertRow(row);
            custTable->setItem(row, 0, new QTableWidgetItem(QString::number(rank++)));
            QString idStr = r.customerId < 0 ? "—" : QString::number(r.customerId);
            auto* idIt = new QTableWidgetItem(idStr);
            if (r.customerId >= 0)
                idIt->setData(Qt::UserRole, r.customerId);
            custTable->setItem(row, 1, idIt);
            custTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(r.customerName)));
            custTable->setItem(row, 3, new QTableWidgetItem(QString::number(r.totalReceipts)));
            custTable->setItem(row, 4, new QTableWidgetItem(QString::number(r.totalSpent, 'f', 2)));
        }
    };
    connect(viewRcptBtn, &QPushButton::clicked, dlg, [&]() {
        int row = custTable->currentRow();
        if (row < 0) {
            QMessageBox::information(dlg, "Reports", "Select a customer row.");
            return;
        }
        QVariant v = custTable->item(row, 1)->data(Qt::UserRole);
        if (!v.isValid()) {
            QMessageBox::information(dlg, "Reports", "Receipt filter applies to registered customers only.");
            return;
        }
        openReceiptHistoryDialog(v.toInt());
    });
    fillCustomers();
    tabs->addTab(custPage, "Customers");

    cardLow->setCursor(Qt::PointingHandCursor);
    class LowStockCardFilter : public QObject {
    public:
        QTabWidget* tabWidget = nullptr;
        explicit LowStockCardFilter(QTabWidget* tw, QObject* parent)
            : QObject(parent), tabWidget(tw) {}
        bool eventFilter(QObject*, QEvent* e) override {
            if (e->type() == QEvent::MouseButtonRelease)
                tabWidget->setCurrentIndex(2);
            return false;
        }
    };
    auto* lowStockCardFilter = new LowStockCardFilter(tabs, dlg);
    cardLow->installEventFilter(lowStockCardFilter);

    auto* exportRepBtn = new QPushButton("Export current tab as text…", dlg);
    root->addWidget(exportRepBtn);
    connect(exportRepBtn, &QPushButton::clicked, dlg, [&]() {
        QString path = QFileDialog::getSaveFileName(dlg, "Export report", "wms_report.txt",
            "Text (*.txt);;All Files (*)");
        if (path.isEmpty()) return;
        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(dlg, "Export", "Could not write file.");
            return;
        }
        QTextStream out(&f);
        const QDateTime gen = QDateTime::currentDateTime();
        out << "=====================================\n";
        out << "        WMS-X  REPORT\n";
        out << "-------------------------------------\n";
        out << "Generated: " << gen.toString("yyyy-MM-dd HH:mm:ss") << "\n";
        out << "-------------------------------------\n";

        const int ti = tabs->currentIndex();
        if (ti == 0) {
            out << "Report  : Sales overview\n";
            out << "Period  : " << fromDate->date().toString("yyyy-MM-dd") << " -> "
                << toDate->date().toString("yyyy-MM-dd") << "\n";
            auto dr = eng.getDailyRevenue(reportPeriodFrom, reportPeriodTo);
            for (const auto& r : dr)
                out << QString::fromStdString(r.date) << '\t' << r.revenue << '\t' << r.receiptCount << "\n";
        } else if (ti == 1) {
            out << "Report  : Top selling items\n";
            for (int r = 0; r < topTable->rowCount(); ++r) {
                for (int c = 0; c < topTable->columnCount(); ++c) {
                    if (c) out << '\t';
                    auto* it = topTable->item(r, c);
                    out << (it ? it->text() : QString());
                }
                out << "\n";
            }
        } else if (ti == 2) {
            out << "Report  : Low stock (threshold " << lowThreshSpin->value() << ")\n";
            for (int r = 0; r < lowTable->rowCount(); ++r) {
                for (int c = 0; c < lowTable->columnCount(); ++c) {
                    if (c) out << '\t';
                    auto* it = lowTable->item(r, c);
                    out << (it ? it->text() : QString());
                }
                out << "\n";
            }
        } else if (ti == 3) {
            out << "Report  : Top customers\n";
            for (int r = 0; r < custTable->rowCount(); ++r) {
                for (int c = 0; c < custTable->columnCount(); ++c) {
                    if (c) out << '\t';
                    auto* it = custTable->item(r, c);
                    out << (it ? it->text() : QString());
                }
                out << "\n";
            }
        }
        out << "=====================================\n";
        QMessageBox::information(dlg, "Export", "Report saved.");
    });

    dlg->exec();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Main window;
    window.show();
    return app.exec();
}
