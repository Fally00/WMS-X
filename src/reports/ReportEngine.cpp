#include "ReportEngine.h"
#include <SQLiteCpp/Statement.h>
#include <algorithm>

SalesSummary ReportEngine::getSalesSummary(const std::string& from, const std::string& to) {
    SalesSummary summary;
    try {
        if (!from.empty() && !to.empty()) {
            SQLite::Statement query1(db,
                "SELECT SUM(total), SUM(tax), COUNT(*) FROM receipts "
                "WHERE timestamp >= ? AND timestamp <= ?;");
            query1.bind(1, from + " 00:00:00");
            query1.bind(2, to + " 23:59:59");
            if (query1.executeStep()) {
                summary.totalRevenue = query1.getColumn(0).getDouble();
                summary.totalTax = query1.getColumn(1).getDouble();
                summary.totalReceipts = query1.getColumn(2).getInt();
            }
        } else {
            SQLite::Statement query1(db,
                "SELECT SUM(total), SUM(tax), COUNT(*) FROM receipts;");
            if (query1.executeStep()) {
                summary.totalRevenue = query1.getColumn(0).getDouble();
                summary.totalTax = query1.getColumn(1).getDouble();
                summary.totalReceipts = query1.getColumn(2).getInt();
            }
        }
        if (summary.totalReceipts > 0)
            summary.averageOrderValue = summary.totalRevenue / summary.totalReceipts;
    } catch (const std::exception&) {}
    return summary;
}

std::vector<ItemSalesRow> ReportEngine::getTopSellingItems(int limit) {
    std::vector<ItemSalesRow> rows;
    try {
        SQLite::Statement query(db,
            "SELECT i.id, i.name, SUM(ri.quantity) as qty_sold, SUM(ri.line_total) as revenue "
            "FROM receipt_items ri "
            "JOIN items i ON ri.item_id = i.id "
            "GROUP BY i.id, i.name "
            "ORDER BY qty_sold DESC LIMIT ?;");
        query.bind(1, limit);

        while (query.executeStep()) {
            ItemSalesRow row;
            row.itemId = query.getColumn(0).getInt();
            row.itemName = query.getColumn(1).getString();
            row.totalQtySold = query.getColumn(2).getInt();
            row.totalRevenue = query.getColumn(3).getDouble();
            rows.push_back(row);
        }
    } catch (const std::exception&) {}
    return rows;
}

std::vector<LowStockRow> ReportEngine::getLowStockItems(int threshold) {
    std::vector<LowStockRow> rows;
    try {
        SQLite::Statement query(db,
            "SELECT id, name, quantity, location FROM items WHERE quantity <= ? ORDER BY quantity ASC;");
        query.bind(1, threshold);

        while (query.executeStep()) {
            LowStockRow row;
            row.itemId = query.getColumn(0).getInt();
            row.itemName = query.getColumn(1).getString();
            row.currentQty = query.getColumn(2).getInt();
            row.location = query.getColumn(3).getString();
            rows.push_back(row);
        }
    } catch (const std::exception&) {}
    return rows;
}

std::vector<CustomerSalesRow> ReportEngine::getTopCustomers(int limit) {
    std::vector<CustomerSalesRow> rows;
    try {
        SQLite::Statement q1(db,
            "SELECT c.id, c.name, COUNT(*), SUM(r.total) "
            "FROM receipts r "
            "INNER JOIN customers c ON r.customer_id = c.id "
            "GROUP BY c.id, c.name;");
        while (q1.executeStep()) {
            CustomerSalesRow row;
            row.customerId = q1.getColumn(0).getInt();
            row.customerName = q1.getColumn(1).getString();
            row.totalReceipts = q1.getColumn(2).getInt();
            row.totalSpent = q1.getColumn(3).getDouble();
            rows.push_back(row);
        }

        SQLite::Statement q2(db,
            "SELECT customer_name, COUNT(*), SUM(total) FROM receipts "
            "WHERE customer_id IS NULL AND TRIM(IFNULL(customer_name, '')) != '' "
            "GROUP BY customer_name;");
        while (q2.executeStep()) {
            CustomerSalesRow row;
            row.customerId = -1;
            row.customerName = q2.getColumn(0).getString();
            row.totalReceipts = q2.getColumn(1).getInt();
            row.totalSpent = q2.getColumn(2).getDouble();
            rows.push_back(row);
        }

        std::sort(rows.begin(), rows.end(),
            [](const CustomerSalesRow& a, const CustomerSalesRow& b) {
                return a.totalSpent > b.totalSpent;
            });
        if (static_cast<int>(rows.size()) > limit)
            rows.resize(static_cast<size_t>(limit));
    } catch (const std::exception&) {}
    return rows;
}

std::vector<DailyRevenueRow> ReportEngine::getDailyRevenue(const std::string& from, const std::string& to) {
    std::vector<DailyRevenueRow> rows;
    try {
        SQLite::Statement query(db,
            "SELECT DATE(timestamp) as date, SUM(total) as daily_revenue, COUNT(*) as count "
            "FROM receipts "
            "WHERE timestamp >= ? AND timestamp <= ? "
            "GROUP BY DATE(timestamp) "
            "ORDER BY date ASC;");
        query.bind(1, from + " 00:00:00");
        query.bind(2, to + " 23:59:59");

        while (query.executeStep()) {
            DailyRevenueRow row;
            row.date = query.getColumn(0).getString();
            row.revenue = query.getColumn(1).getDouble();
            row.receiptCount = query.getColumn(2).getInt();
            rows.push_back(row);
        }
    } catch (const std::exception&) {}
    return rows;
}

std::vector<ItemSalesRow> ReportEngine::getSlowMovingItems(int limit) {
    std::vector<ItemSalesRow> rows;
    try {
        SQLite::Statement query(db,
            "SELECT i.id, i.name, COALESCE(SUM(ri.quantity), 0) as qty_sold, "
            "COALESCE(SUM(ri.line_total), 0) as revenue "
            "FROM items i "
            "LEFT JOIN receipt_items ri ON i.id = ri.item_id "
            "GROUP BY i.id, i.name "
            "ORDER BY qty_sold ASC LIMIT ?;");
        query.bind(1, limit);

        while (query.executeStep()) {
            ItemSalesRow row;
            row.itemId = query.getColumn(0).getInt();
            row.itemName = query.getColumn(1).getString();
            row.totalQtySold = query.getColumn(2).getInt();
            row.totalRevenue = query.getColumn(3).getDouble();
            rows.push_back(row);
        }
    } catch (const std::exception&) {}
    return rows;
}
