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

namespace ItemDialogs {

bool showAddDialog(QWidget* parent, WmsControllers& controller) {
    QDialog dialog(parent);
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
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return false;

    bool okId, okQty;
    int id  = idEdit->text().toInt(&okId);
    int qty = qtyEdit->text().toInt(&okQty);

    if (!okId || !okQty || nameEdit->text().isEmpty() || locEdit->text().isEmpty()) {
        QMessageBox::warning(parent, "Invalid Input", "Please fill all fields correctly.");
        return false;
    }

    if (!controller.addItem(id, nameEdit->text().toStdString(),
                               qty, locEdit->text().toStdString(),
                               barcodeEdit->text().trimmed().toStdString())) {
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

    int id = inventoryTable->item(selectedRow, 0)->text().toInt();

    QDialog dialog(parent);
    dialog.setWindowTitle(QString("Update Item #%1").arg(id));
    QFormLayout form(&dialog);

    QLineEdit *nameEdit = new QLineEdit(inventoryTable->item(selectedRow, 1)->text(), &dialog);
    QLineEdit *qtyEdit  = new QLineEdit(inventoryTable->item(selectedRow, 2)->text(), &dialog);
    QLineEdit *locEdit  = new QLineEdit(inventoryTable->item(selectedRow, 3)->text(), &dialog);
    QLineEdit *priceEdit = new QLineEdit(inventoryTable->item(selectedRow, 4)->text(), &dialog);
    QLineEdit *barcodeEdit = new QLineEdit(
        inventoryTable->item(selectedRow, 8) ? inventoryTable->item(selectedRow, 8)->text() : QString(),
        &dialog);
    barcodeEdit->setPlaceholderText("Optional");

    form.addRow("Name:",     nameEdit);
    form.addRow("Quantity:", qtyEdit);
    form.addRow("Location:", locEdit);
    form.addRow("Price:",    priceEdit);
    form.addRow("Barcode:",  barcodeEdit);

    auto* printBarcodeBtn = new QPushButton("Print label…", &dialog);
    form.addRow(printBarcodeBtn);
    QObject::connect(printBarcodeBtn, &QPushButton::clicked, &dialog, [&, id]() {
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
        QObject::connect(cl, &QPushButton::clicked, &printDlg, &QDialog::accept);
        QObject::connect(pb, &QPushButton::clicked, &printDlg, [text]() {
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
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return false;

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

    if (!controller.updateItem(id, name, qty, loc, price, bc)) {
        QMessageBox::warning(parent, "Failed", "Could not update item (duplicate barcode?).");
        return false;
    }

    return true;
}

bool showDeleteDialog(QWidget* parent, WmsControllers& controller, QTableWidget* inventoryTable) {
    int selectedRow = inventoryTable->currentRow();
    if (selectedRow < 0) {
        QMessageBox::information(parent, "No Selection", "Please select a row to delete.");
        return false;
    }

    int id = inventoryTable->item(selectedRow, 0)->text().toInt();
    QString name = inventoryTable->item(selectedRow, 1)->text();

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
