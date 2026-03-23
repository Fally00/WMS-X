#ifndef MAIN_H
#define MAIN_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QStatusBar>

#include "controllers/WmsControllers.h"
#include "storage/Receipt.h"

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

private:
    Ui::Main *ui;
    WmsControllers wmsController;

    void loadInventory();
    void populateTable(const std::vector<Item>& items);
    QString formatTime(std::time_t t) const;
    QString itemStatus(int quantity) const;
    void showReceiptPreview(const Receipt& receipt);
};

#endif // MAIN_H
