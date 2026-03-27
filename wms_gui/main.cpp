#include "main.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QTextDocument>
#include <QApplication>
#include <QInputDialog>
#include <QListWidget>
#include <QHeaderView>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDateTime>
#include <algorithm>
#include <cstdlib>
#include <QString>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QIcon>
#include <QSize>
#include <QMenu>
#include <QTableWidgetItem>
#include <QSpinBox>
#include <QVariant>
#include <QDialog>

#include "ui_main.h"

// New Dialog headers
#include "Dialogs/CustomerDialog.h"
#include "Dialogs/ReceiptDialog.h"
#include "Dialogs/ReportDialog.h"
#include "Dialogs/ItemDialogs.h"

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
    connect(ui->searchBtn,  &QPushButton::clicked, this, &Main::onSearch);
    connect(ui->searchLineEdit, &QLineEdit::returnPressed, this, &Main::onSearch);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &Main::onSearchTextChanged);
    connect(ui->queueTaskBtn,   &QPushButton::clicked, this, &Main::onQueueTask);
    connect(ui->runQueueBtn,    &QPushButton::clicked, this, &Main::onRunQueue);

    // Dialog Lambdas
    connect(ui->addBtn, &QPushButton::clicked, this, [this]() {
        if (ItemDialogs::showAddDialog(this, wmsController)) {
            loadInventory();
            ui->statusbar->showMessage("Item added successfully.", 3000);
        }
    });

    connect(ui->updateBtn, &QPushButton::clicked, this, [this]() {
        if (ItemDialogs::showUpdateDialog(this, wmsController, ui->inventoryTable)) {
            loadInventory();
            ui->statusbar->showMessage("Item updated successfully.", 3000);
        }
    });

    connect(ui->deleteBtn, &QPushButton::clicked, this, [this]() {
        if (ItemDialogs::showDeleteDialog(this, wmsController, ui->inventoryTable)) {
            loadInventory();
            ui->statusbar->showMessage("Item deleted successfully.", 3000);
        }
    });

    connect(ui->customersBtn, &QPushButton::clicked, this, [this]() {
        CustomerDialog::showManageDialog(this, wmsController);
    });

    connect(ui->receiptBtn, &QPushButton::clicked, this, [this]() {
        if (ReceiptDialog::showGenerateDialog(this, wmsController, ui->inventoryTable)) {
            loadInventory();
        }
    });

    connect(ui->receiptHistoryBtn, &QPushButton::clicked, this, [this]() {
        ReceiptDialog::showHistoryDialog(this, wmsController);
    });

    connect(ui->exportCsvBtn, &QPushButton::clicked, this, [this]() {
        ReceiptDialog::exportAllToCSV(this, wmsController);
    });

    connect(ui->reportsBtn, &QPushButton::clicked, this, [this]() {
        ReportDialog::showReportDialog(this, wmsController, 
            [this]() { loadInventory(); },
            [this](std::optional<int> cId) { ReceiptDialog::showHistoryDialog(this, wmsController, cId); }
        );
    });
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

        table->setItem(row, 10, new QTableWidgetItem(formatTime(item.getCreatedAt())));
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

    QDialog dialog(this);
    dialog.setWindowTitle("Queue Task — Command Help");
    dialog.setMinimumSize(560, 420);
    auto* layout = new QVBoxLayout(&dialog);

    layout->addWidget(new QLabel("<b>Select a command to queue:</b>", &dialog));

    auto* cmdList = new QListWidget(&dialog);
    for (const auto& cmd : commands) {
        cmdList->addItem(QString("%1  —  %2").arg(cmd.name, -14).arg(cmd.usage));
    }
    cmdList->setCurrentRow(0);
    layout->addWidget(cmdList);

    auto* descLabel = new QLabel(&dialog);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("QLabel { background: #2d2d2d; color: #e0e0e0; padding: 8px; border-radius: 4px; font-size: 13px; }");
    descLabel->setText(commands[0].description);
    layout->addWidget(descLabel);

    auto* argsLayout = new QHBoxLayout();
    argsLayout->addWidget(new QLabel("Arguments:", &dialog));
    auto* argsEdit = new QLineEdit(&dialog);
    argsEdit->setPlaceholderText("e.g. 101 \"Widget\" 5 A1");
    argsLayout->addWidget(argsEdit);
    layout->addLayout(argsLayout);

    auto* btnLayout = new QHBoxLayout();
    auto* queueBtn     = new QPushButton("Queue && Run", &dialog);
    auto* queueOnlyBtn = new QPushButton("Queue Only", &dialog);
    auto* cancelBtn    = new QPushButton("Cancel", &dialog);
    btnLayout->addWidget(queueBtn);
    btnLayout->addWidget(queueOnlyBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(cmdList, &QListWidget::currentRowChanged, [&](int row) {
        if (row >= 0 && row < commands.size())
            descLabel->setText(commands[row].description);
    });

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

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
    wmsController.reloadInventory();
    loadInventory();
    ui->statusbar->showMessage(
        QString("Processed %1 queued task(s).").arg(static_cast<qulonglong>(queued)),
        3000
    );
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Main window;
    window.show();
    return app.exec();
}
