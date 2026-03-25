#pragma once

#include <string>
#include <vector>
#include <SQLiteCpp/Database.h>

struct SalesSummary {
    double totalRevenue = 0.0;
    double totalTax = 0.0;
    int    totalReceipts = 0;
    double averageOrderValue = 0.0;
};

struct ItemSalesRow {
    int    itemId = 0;
    std::string itemName;
    int    totalQtySold = 0;
    double totalRevenue = 0.0;
};

struct LowStockRow {
    int    itemId = 0;
    std::string itemName;
    int    currentQty = 0;
    std::string location;
};

struct CustomerSalesRow {
    int    customerId = -1;
    std::string customerName;
    int    totalReceipts = 0;
    double totalSpent = 0.0;
};

struct DailyRevenueRow {
    std::string date;
    double revenue = 0.0;
    int    receiptCount = 0;
};

class ReportEngine {
private:
    SQLite::Database& db;

public:
    explicit ReportEngine(SQLite::Database& database) : db(database) {}

    SalesSummary                 getSalesSummary(const std::string& from = "", const std::string& to = "");
    std::vector<ItemSalesRow>    getTopSellingItems(int limit = 10);
    std::vector<LowStockRow>     getLowStockItems(int threshold = 10);
    std::vector<CustomerSalesRow> getTopCustomers(int limit = 10);
    std::vector<DailyRevenueRow> getDailyRevenue(const std::string& from, const std::string& to);
    std::vector<ItemSalesRow>    getSlowMovingItems(int limit = 10);
};
