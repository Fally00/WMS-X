#include "ReportDialog.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QDateEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QSpinBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QTextStream>
#include <QEvent>

#include "reports/ReportEngine.h"

#ifdef WMS_GUI_HAS_CHARTS
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QPainter>
#endif

namespace ReportDialog {

void showReportDialog(QWidget* parent, WmsControllers& controller, std::function<void()> reloadInventoryCb, std::function<void(std::optional<int>)> openReceiptHistoryCb) {
    auto* dlg = new QDialog(parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle("WMS-X — Reports");
    dlg->resize(920, 620);

    ReportEngine eng(controller.getDB());

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
    QObject::connect(applyRangeBtn, &QPushButton::clicked, dlg, applySalesRange);
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
    QObject::connect(topRefreshBtn, &QPushButton::clicked, dlg, fillTopItems);
    QObject::connect(topLimitCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), dlg, fillTopItems);
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
    QObject::connect(lowRefreshBtn, &QPushButton::clicked, dlg, fillLowStock);
    QObject::connect(lowThreshSpin, QOverload<int>::of(&QSpinBox::valueChanged), dlg, fillLowStock);
    QObject::connect(restockBtn, &QPushButton::clicked, dlg, [&]() {
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
        QObject::connect(&ubtn, &QDialogButtonBox::accepted, &udlg, &QDialog::accept);
        QObject::connect(&ubtn, &QDialogButtonBox::rejected, &udlg, &QDialog::reject);
        if (udlg.exec() != QDialog::Accepted) return;
        bool ok = false;
        int nq = qtyE->text().toInt(&ok);
        if (!ok || nq < 0) {
            QMessageBox::warning(dlg, "Reports", "Invalid quantity.");
            return;
        }
        if (!controller.updateItem(id, std::nullopt, nq, std::nullopt, std::nullopt, std::nullopt)) {
            QMessageBox::warning(dlg, "Reports", "Update failed.");
            return;
        }
        if (reloadInventoryCb) reloadInventoryCb();
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
    QObject::connect(viewRcptBtn, &QPushButton::clicked, dlg, [&]() {
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
        if (openReceiptHistoryCb) openReceiptHistoryCb(v.toInt());
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
    QObject::connect(exportRepBtn, &QPushButton::clicked, dlg, [&]() {
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

    dlg->show();
}

} // namespace ReportDialog
