#include "ItemDialogs.h"

#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QDoubleValidator>
#include <QIntValidator>

namespace ItemDialogs {

bool showAddDialog(QWidget* parent, WmsControllers& controller) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Add New Item");
    dialog.setMinimumWidth(380);
    QFormLayout form(&dialog);

    auto* idEdit       = new QLineEdit(&dialog);
    auto* nameEdit     = new QLineEdit(&dialog);
    auto* qtyEdit      = new QLineEdit(&dialog);
    auto* locEdit      = new QLineEdit(&dialog);
    auto* priceEdit    = new QLineEdit(&dialog);
    auto* currencyEdit = new QLineEdit(&dialog);
    auto* unitEdit     = new QLineEdit(&dialog);
    auto* categoryEdit = new QLineEdit(&dialog);
    auto* barcodeEdit  = new QLineEdit(&dialog);

    idEdit->setPlaceholderText("Unique integer ID");
    qtyEdit->setPlaceholderText("e.g. 100");
    locEdit->setPlaceholderText("e.g. A1, B2");
    priceEdit->setPlaceholderText("e.g. 49.99");
    currencyEdit->setText("EGP");
    unitEdit->setText("pcs");
    categoryEdit->setText("general");
    barcodeEdit->setPlaceholderText("Optional");

    idEdit->setValidator(new QIntValidator(0, 2'000'000'000, &dialog));
    qtyEdit->setValidator(new QIntValidator(0, 2'000'000'000, &dialog));
    priceEdit->setValidator(new QDoubleValidator(0.0, 1e9, 2, &dialog));

    form.addRow("ID:",        idEdit);
    form.addRow("Name:",      nameEdit);
    form.addRow("Quantity:",  qtyEdit);
    form.addRow("Location:",  locEdit);
    form.addRow("Price:",     priceEdit);
    form.addRow("Currency:",  currencyEdit);
    form.addRow("Unit:",      unitEdit);
    form.addRow("Category:",  categoryEdit);
    form.addRow("Barcode:",   barcodeEdit);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, &dialog);
    form.addRow(&buttons);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return false;

    bool okId, okQty;
    int id  = idEdit->text().toInt(&okId);
    int qty = qtyEdit->text().toInt(&okQty);

    if (!okId || !okQty || nameEdit->text().trimmed().isEmpty() || locEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(parent, "Invalid Input", "ID, Name, Quantity and Location are required and must be valid.");
        return false;
    }

    bool okPrice;
    double price = priceEdit->text().isEmpty() ? 0.0 : priceEdit->text().toDouble(&okPrice);
    if (!priceEdit->text().isEmpty() && !okPrice) {
        QMessageBox::warning(parent, "Invalid Input", "Price must be a valid number.");
        return false;
    }

    QString currency = currencyEdit->text().trimmed().isEmpty() ? "EGP" : currencyEdit->text().trimmed();
    QString unit     = unitEdit->text().trimmed().isEmpty()     ? "pcs" : unitEdit->text().trimmed();
    QString category = categoryEdit->text().trimmed().isEmpty() ? "general" : categoryEdit->text().trimmed();

    if (!controller.addItem(
            id,
            nameEdit->text().trimmed().toStdString(),
            qty,
            locEdit->text().trimmed().toStdString(),
            barcodeEdit->text().trimmed().toStdString(),
            price,
            currency.toStdString(),
            unit.toStdString(),
            category.toStdString())) {
        QMessageBox::warning(parent, "Failed", "Item already exists, invalid data, or duplicate barcode.");
        return false;
    }

    return true;
}

bool showUpdateDialog(QWidget* parent, WmsControllers& controller, QTableWidget* inventoryTable) {
    int selectedRow = inventoryTable->currentRow();
    if (selectedRow < 0) {
        QMessageBox::information(parent, "No Selection", "Please select a row to update.");
        return false;
    }

    // Safety: make sure the table cell exists
    if (!inventoryTable->item(selectedRow, 0)) {
        QMessageBox::information(parent, "No Selection", "Please select a valid row to update.");
        return false;
    }

    int id = inventoryTable->item(selectedRow, 0)->text().toInt();

    QDialog dialog(parent);
    dialog.setWindowTitle(QString("Update Item #%1").arg(id));
    dialog.setMinimumWidth(380);
    QFormLayout form(&dialog);

    auto colText = [&](int col) -> QString {
        auto* it = inventoryTable->item(selectedRow, col);
        return it ? it->text() : QString();
    };

    auto* nameEdit     = new QLineEdit(colText(1), &dialog);
    auto* qtyEdit      = new QLineEdit(colText(2), &dialog);
    auto* locEdit      = new QLineEdit(colText(3), &dialog);
    auto* priceEdit    = new QLineEdit(colText(4), &dialog);
    auto* currencyEdit = new QLineEdit(colText(5), &dialog);
    auto* unitEdit     = new QLineEdit(colText(6), &dialog);
    auto* categoryEdit = new QLineEdit(colText(7), &dialog);
    auto* barcodeEdit  = new QLineEdit(colText(8), &dialog);

    barcodeEdit->setPlaceholderText("Optional");
    qtyEdit->setValidator(new QIntValidator(0, 2'000'000'000, &dialog));
    priceEdit->setValidator(new QDoubleValidator(0.0, 1e9, 2, &dialog));

    form.addRow("Name:",      nameEdit);
    form.addRow("Quantity:",  qtyEdit);
    form.addRow("Location:",  locEdit);
    form.addRow("Price:",     priceEdit);
    form.addRow("Currency:",  currencyEdit);
    form.addRow("Unit:",      unitEdit);
    form.addRow("Category:",  categoryEdit);
    form.addRow("Barcode:",   barcodeEdit);

    // ── Print barcode label ──────────────────────────────────────
    auto* printBarcodeBtn = new QPushButton("Print label…", &dialog);
    form.addRow(printBarcodeBtn);
    QObject::connect(printBarcodeBtn, &QPushButton::clicked, &dialog, [&, id]() {
        QString text = barcodeEdit->text().trimmed();
        if (text.isEmpty()) text = QString("ID %1").arg(id);
        QDialog printDlg(&dialog);
        printDlg.setWindowTitle("Barcode label");
        auto* vl  = new QVBoxLayout(&printDlg);
        auto* lab = new QLabel(text, &printDlg);
        lab->setStyleSheet("QLabel { font-family: Consolas, monospace; font-size: 28px; padding: 24px; }");
        lab->setAlignment(Qt::AlignCenter);
        vl->addWidget(lab);
        auto* h  = new QHBoxLayout();
        auto* pb = new QPushButton("Print…", &printDlg);
        auto* cl = new QPushButton("Close",  &printDlg);
        h->addStretch();
        h->addWidget(pb);
        h->addWidget(cl);
        vl->addLayout(h);
        QObject::connect(cl, &QPushButton::clicked, &printDlg, &QDialog::accept);
        QObject::connect(pb, &QPushButton::clicked, &printDlg, [text]() {
            QPrinter printer(QPrinter::HighResolution);
            QPrintDialog pd(&printer);
            if (pd.exec() != QDialog::Accepted) return;
            QTextDocument doc;
            doc.setDefaultFont(QFont("Consolas", 22));
            doc.setPlainText(text);
            doc.print(&printer);
        });
        printDlg.exec();
    });

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                             Qt::Horizontal, &dialog);
    form.addRow(&buttons);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return false;

    bool okQty, okPrice;

    std::optional<std::string> name = nameEdit->text().trimmed().isEmpty()
        ? std::nullopt
        : std::optional<std::string>(nameEdit->text().trimmed().toStdString());

    std::optional<int> qty = qtyEdit->text().isEmpty()
        ? std::nullopt
        : std::optional<int>(qtyEdit->text().toInt(&okQty));
    if (!qtyEdit->text().isEmpty() && !okQty) {
        QMessageBox::warning(parent, "Invalid Input", "Quantity must be a valid integer.");
        return false;
    }

    std::optional<std::string> loc = locEdit->text().trimmed().isEmpty()
        ? std::nullopt
        : std::optional<std::string>(locEdit->text().trimmed().toStdString());

    std::optional<double> price = priceEdit->text().isEmpty()
        ? std::nullopt
        : std::optional<double>(priceEdit->text().toDouble(&okPrice));
    if (!priceEdit->text().isEmpty() && !okPrice) {
        QMessageBox::warning(parent, "Invalid Input", "Price must be a valid number.");
        return false;
    }

    std::optional<std::string> bc = barcodeEdit->text().trimmed().isEmpty()
        ? std::nullopt
        : std::optional<std::string>(barcodeEdit->text().trimmed().toStdString());

    if (!controller.updateItem(id, name, qty, loc, price, bc)) {
        QMessageBox::warning(parent, "Failed", "Could not update item (duplicate barcode or invalid data).");
        return false;
    }

    // Also update unit/currency/category via a second updateItem-style call if they changed.
    // Since WmsControllers::updateItem doesn't yet expose these, we call it with only what matters.
    // Unit / currency / category are handled through extended updateItem if they differ:
    QString newCurrency = currencyEdit->text().trimmed();
    QString newUnit     = unitEdit->text().trimmed();
    QString newCategory = categoryEdit->text().trimmed();

    bool changedExtra = (newCurrency != colText(5) || newUnit != colText(6) || newCategory != colText(7));
    if (changedExtra) {
        // Apply via SQL directly through the controller's DB reference.
        // We do this safely in a try/catch.
        try {
            SQLite::Statement upd(controller.getDB(),
                "UPDATE items SET currency=?, unit=?, category=? WHERE id=?");
            upd.bind(1, newCurrency.toStdString());
            upd.bind(2, newUnit.toStdString());
            upd.bind(3, newCategory.toStdString());
            upd.bind(4, id);
            upd.exec();
            // Re-sync the in-memory inventory by reloading from DB
            controller.reloadInventory();
        } catch (const std::exception& e) {
            QMessageBox::warning(parent, "Partial Update",
                QString("Item fields updated, but extra fields (unit/currency/category) failed:\n%1").arg(e.what()));
        }
    }

    return true;
}

bool showDeleteDialog(QWidget* parent, WmsControllers& controller, QTableWidget* inventoryTable) {
    int selectedRow = inventoryTable->currentRow();
    if (selectedRow < 0) {
        QMessageBox::information(parent, "No Selection", "Please select a row to delete.");
        return false;
    }

    if (!inventoryTable->item(selectedRow, 0)) {
        QMessageBox::information(parent, "No Selection", "Please select a valid row to delete.");
        return false;
    }

    int id = inventoryTable->item(selectedRow, 0)->text().toInt();
    QString name = inventoryTable->item(selectedRow, 1)
                   ? inventoryTable->item(selectedRow, 1)->text()
                   : QString();

    auto confirm = QMessageBox::question(parent, "Confirm Delete",
        QString("Delete item #%1 (%2)?").arg(id).arg(name));
    if (confirm != QMessageBox::Yes) return false;

    if (!controller.removeItem(id)) {
        QMessageBox::warning(parent, "Failed", "Item not found.");
        return false;
    }

    return true;
}

} // namespace ItemDialogs
