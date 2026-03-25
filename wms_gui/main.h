#ifndef MAIN_H
#define MAIN_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QStatusBar>

#include <optional>

#include "controllers/WmsControllers.h"
#include "storage/Receipt.h"
#include "models/Customer.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Main;
}
QT_END_NAMESPACE

class Main : public QMainWindow
{
    Q_OBJECT

public:
    explicit Main(QWidget *parent = nullptr);
    ~Main() override;

private slots:
    void onAddItem();
    void onUpdateItem();
    void onDeleteItem();
    void onSearch();
    void onQueueTask();
    void onRunQueue();
    void onGenerateReceipt();
    void onReceiptHistory();
    void onExportCSV();
    void onManageCustomers();
    void onOpenReports();
    void onSearchTextChanged(const QString& text);

private:
    Ui::Main *ui;
    WmsControllers wmsController;

    qint64 searchBurstStartMs = -1;
    qint64 searchLastKeyMs = 0;

    void loadInventory();
    void populateTable(const std::vector<Item>& items);
    QString formatTime(std::time_t t) const;
    QString itemStatus(int quantity) const;
    void showReceiptPreview(const Receipt& receipt);
    void exportReceiptsToCSV(const std::vector<Receipt>& receipts);
    void openReceiptHistoryDialog(std::optional<int> preselectCustomerId = std::nullopt);
};

#endif // MAIN_H
