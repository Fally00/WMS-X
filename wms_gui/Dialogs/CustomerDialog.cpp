#include "CustomerDialog.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QDateTime>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>

namespace CustomerDialog {

void showManageDialog(QWidget* parent, WmsControllers& controller) {
    QDialog dialog(parent);
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
    refreshTable(controller.getAllCustomers());

    // Search
    QObject::connect(custSearchBtn, &QPushButton::clicked, [&]() {
        QString query = custSearchEdit->text().trimmed();
        if (query.isEmpty()) {
            refreshTable(controller.getAllCustomers());
            return;
        }

        bool isId;
        int id = query.toInt(&isId);
        if (isId) {
            auto cust = controller.getCustomer(id);
            if (cust.has_value()) {
                refreshTable({cust.value()});
            } else {
                table->setRowCount(0);
            }
        } else {
            refreshTable(controller.searchCustomerByName(query.toStdString()));
        }
    });

    QObject::connect(custSearchEdit, &QLineEdit::returnPressed, custSearchBtn, &QPushButton::click);

    // Show All
    QObject::connect(showAllBtn, &QPushButton::clicked, [&]() {
        custSearchEdit->clear();
        refreshTable(controller.getAllCustomers());
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

    QObject::connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    // ── Add Customer ──
    QObject::connect(addBtn, &QPushButton::clicked, [&]() {
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
        QObject::connect(&btns, &QDialogButtonBox::accepted, &addDlg, &QDialog::accept);
        QObject::connect(&btns, &QDialogButtonBox::rejected, &addDlg, &QDialog::reject);

        if (addDlg.exec() != QDialog::Accepted) return;

        if (nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Invalid Input", "Customer name is required.");
            return;
        }

        if (!controller.addCustomer(
                nameEdit->text().trimmed().toStdString(),
                phoneEdit->text().trimmed().toStdString(),
                addrEdit->text().trimmed().toStdString(),
                emailEdit->text().trimmed().toStdString())) {
            QMessageBox::warning(&dialog, "Failed", "Could not add customer.");
            return;
        }

        refreshTable(controller.getAllCustomers());
    });

    // ── Edit Customer ──
    QObject::connect(editBtn, &QPushButton::clicked, [&]() {
        int row = table->currentRow();
        if (row < 0) {
            QMessageBox::information(&dialog, "No Selection", "Please select a customer to edit.");
            return;
        }

        int custId = table->item(row, 0)->text().toInt();
        auto cust = controller.getCustomer(custId);
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
        QObject::connect(&btns, &QDialogButtonBox::accepted, &editDlg, &QDialog::accept);
        QObject::connect(&btns, &QDialogButtonBox::rejected, &editDlg, &QDialog::reject);

        if (editDlg.exec() != QDialog::Accepted) return;

        std::optional<std::string> name = nameEdit->text().trimmed().isEmpty()
            ? std::nullopt : std::optional<std::string>(nameEdit->text().trimmed().toStdString());
        std::optional<std::string> phone = std::optional<std::string>(phoneEdit->text().trimmed().toStdString());
        std::optional<std::string> addr = std::optional<std::string>(addrEdit->text().trimmed().toStdString());
        std::optional<std::string> email = std::optional<std::string>(emailEdit->text().trimmed().toStdString());

        if (!controller.updateCustomer(custId, name, phone, addr, email)) {
            QMessageBox::warning(&dialog, "Failed", "Could not update customer.");
            return;
        }

        refreshTable(controller.getAllCustomers());
    });

    // ── Delete Customer ──
    QObject::connect(deleteBtn, &QPushButton::clicked, [&]() {
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

        if (!controller.removeCustomer(custId)) {
            QMessageBox::warning(&dialog, "Failed", "Customer not found.");
            return;
        }

        refreshTable(controller.getAllCustomers());
    });

    // Double-click to edit
    QObject::connect(table, &QTableWidget::cellDoubleClicked, [&](int, int) {
        editBtn->click();
    });

    dialog.exec();
}

} // namespace CustomerDialog
