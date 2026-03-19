#include "main.h"
#include "ui_main.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QString>
#include <QHeaderView>

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
}

Main::~Main()
{
    delete ui;
}

// ─── Load all items from the controller into the table ───────────────────────
void Main::loadInventory()
{
    populateTable(wmsController.getAllItems());
    ui->statusbar->showMessage(QString("Total items: %1").arg(
        wmsController.getAllItems().size()));
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
        table->setItem(row, 8,  new QTableWidgetItem(formatTime(item.getCreatedAt())));
        table->setItem(row, 9,  new QTableWidgetItem(formatTime(item.getModifiedAt())));
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
}
