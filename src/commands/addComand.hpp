#pragma once

//included files
#include "commands/CommandContext.hpp" // ← needed for CommandContext definition
#include "utils/safetyparse.hpp"
#include "commands/command.hpp"
#include "storage/Receipt.h"
#include "output/output.h"
#include "models/Item.h"
#include "models/Customer.h"

//needed libraries
#include <optional>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iomanip>


// Command to add an item
class AddCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        std::vector<std::string> args = a;
        std::string barcodeFlag;
        for (size_t i = 0; i + 1 < args.size(); ) {
            if (args[i] == "--barcode") {
                barcodeFlag = args[i + 1];
                args.erase(args.begin() + static_cast<decltype(args)::difference_type>(i),
                           args.begin() + static_cast<decltype(args)::difference_type>(i + 2));
                continue;
            }
            ++i;
        }

        if (args.size() < 4)
            return Result<void>::fail("Usage: add <id> <name> <qty> <loc> [--barcode <value>]");

        auto id = safetyparse(args[0]);
        auto qty = safetyparse(args[2]);
        if (!id.ok || !qty.ok)
            return Result<void>::fail(id.ok ? qty.error : id.error);

        if (!ctx.wms.addItem(id.value, args[1], qty.value, args[3], barcodeFlag))
            return Result<void>::fail("Item exists, invalid data, or duplicate barcode");

        if (ctx.autosave) ctx.wms.saveAll();
        return Result<void>::success();
    }
};

//Command to remove an item
class RemoveCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.size() != 1)
            return Result<void>::fail("Usage: remove <id>");

        auto id = safetyparse(a[0]);
        if (!id.ok) return Result<void>::fail(id.error);

        // Now expects Wmscootroller::removeItem(id)
        if (!ctx.wms.removeItem(id.value)) return Result<void>::fail("Item not found");

        if (ctx.autosave) ctx.wms.saveAll();
        return Result<void>::success();
    }
};

// Command to update an existing item's fields
// Usage: update <id> [--name <n>] [--qty <q>] [--loc <l>] [--price <p>] [--barcode <value>]
class UpdateCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.empty())
            return Result<void>::fail(
                "Usage: update <id> [--name <n>] [--qty <q>] [--loc <l>] [--price <p>] [--barcode <value>]");

        auto id = safetyparse(a[0]);
        if (!id.ok) return Result<void>::fail(id.error);

        std::optional<std::string> name, loc, barcode;
        std::optional<int>         qty;
        std::optional<double>      price;
        bool anyFlag = false;

        for (size_t i = 1; i + 1 < a.size(); i += 2) {
            const std::string& flag = a[i];
            const std::string& val  = a[i + 1];

            if (flag == "--name") {
                name = val;
                anyFlag = true;
            } else if (flag == "--qty") {
                auto q = safetyparse(val);
                if (!q.ok) return Result<void>::fail(q.error);
                if (q.value < 0) return Result<void>::fail("Quantity must be >= 0");
                qty = q.value;
                anyFlag = true;
            } else if (flag == "--loc") {
                loc = val;
                anyFlag = true;
            } else if (flag == "--price") {
                double p = 0.0;
                try { p = std::stod(val); }
                catch (...) { return Result<void>::fail("Price must be a number"); }
                if (p < 0.0) return Result<void>::fail("Price cannot be negative");
                price = p;
                anyFlag = true;
            } else if (flag == "--barcode") {
                barcode = val;
                anyFlag = true;
            } else {
                return Result<void>::fail("Unknown flag: " + flag);
            }
        }

        if (!anyFlag)
            return Result<void>::fail("No fields specified. Use --name, --qty, --loc, --price, --barcode");

        if (!ctx.wms.updateItem(id.value, name, qty, loc, price, barcode))
            return Result<void>::fail("Item not found, duplicate barcode, or update failed");

        if (ctx.autosave) ctx.wms.saveAll();
        return Result<void>::success();
    }
};

//Command to list the current stock of items
class ListCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        size_t page = 0, pageSize = 10;
        if (a.size() >= 1) {
            auto p = safetyparse(a[0]);
            if (!p.ok) return Result<void>::fail(p.error);
            page = static_cast<size_t>(std::max(0, p.value));
        }
        if (a.size() >= 2) {
            auto ps = safetyparse(a[1]);
            if (!ps.ok) return Result<void>::fail(ps.error);
            pageSize = static_cast<size_t>(std::max(1, ps.value));
        }

        ctx.wms.listItems(page, pageSize);
        return Result<void>::success();
    }
};

//Command to Search for a specific item — by ID or by name
// Usage: search <id>          → exact ID lookup
//        search --name <q>    → partial name search (case-sensitive)
class SearchCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.empty())
            return Result<void>::fail("Usage: search <id>  |  search --name <query>");

        // Name search path
        if (a[0] == "--name") {
            if (a.size() < 2) return Result<void>::fail("Usage: search --name <query>");
            std::string query = a[1];

            auto results = ctx.wms.searchByName(query);
            if (results.empty())
                return Result<void>::fail("No items found matching: " + query);

            for (const auto& item : results) printItem(item);
            return Result<void>::success();
        }

        // ID search path
        if (a.size() != 1)
            return Result<void>::fail("Usage: search <id>  |  search --name <query>");

        auto id = safetyparse(a[0]);
        if (!id.ok) return Result<void>::fail(id.error);

        auto item = ctx.wms.getItem(id.value);
        if (!item.has_value()) return Result<void>::fail("Item not found");

        printItem(item.value());
        return Result<void>::success();
    }
};

class ScanCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.empty())
            return Result<void>::fail("Usage: scan <barcode>");

        std::string barcode = a[0];
        for (size_t i = 1; i < a.size(); ++i) {
            barcode.push_back(' ');
            barcode += a[i];
        }

        auto item = ctx.wms.getItemByBarcode(barcode);
        if (!item.has_value())
            return Result<void>::fail("No item found for barcode: " + barcode);

        printItem(item.value());
        return Result<void>::success();
    }
};

// ─────────────────────────────────────────────
// Report Command (subcommand router)
// ─────────────────────────────────────────────
//   report summary
//   report topitems [limit]
//   report lowstock [threshold]
//   report customers [limit]
//   report daily <from> <to>   (YYYY-MM-DD)
//   report slowitems [limit]

namespace {

std::string fmtMoney(double v) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(2) << v;
    return o.str();
}

} // namespace

class ReportCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.empty())
            return Result<void>::fail(
                "Usage: report <summary|topitems|lowstock|customers|daily|slowitems> [args...]");

        std::string sub = a[0];
        std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);

        auto& eng = ctx.wms.getReportEngine();

        if (sub == "summary") {
            std::string from, to;
            if (a.size() >= 3) {
                from = a[1];
                to = a[2];
            }
            auto s = eng.getSalesSummary(from, to);
            std::vector<std::string> headers = {"Metric", "Value"};
            std::vector<std::vector<std::string>> rows = {
                {"Total Revenue (EGP)", fmtMoney(s.totalRevenue)},
                {"Total Tax (EGP)", fmtMoney(s.totalTax)},
                {"Total Receipts", std::to_string(s.totalReceipts)},
                {"Average Order Value (EGP)", fmtMoney(s.averageOrderValue)},
            };
            OutputFormatter::printTable(headers, rows);
            return Result<void>::success();
        }

        if (sub == "topitems") {
            int limit = 10;
            if (a.size() >= 2) {
                auto p = safetyparse(a[1]);
                if (!p.ok || p.value < 1) return Result<void>::fail("Limit must be a positive integer");
                limit = p.value;
            }
            auto rows = eng.getTopSellingItems(limit);
            if (rows.empty()) {
                OutputFormatter::printWarning("No sales data.");
                return Result<void>::success();
            }
            std::vector<std::string> headers = {"Item ID", "Name", "Qty Sold", "Revenue (EGP)"};
            std::vector<std::vector<std::string>> table;
            for (const auto& r : rows) {
                table.push_back({
                    std::to_string(r.itemId),
                    r.itemName,
                    std::to_string(r.totalQtySold),
                    fmtMoney(r.totalRevenue),
                });
            }
            OutputFormatter::printTable(headers, table);
            return Result<void>::success();
        }

        if (sub == "lowstock") {
            int threshold = 10;
            if (a.size() >= 2) {
                auto p = safetyparse(a[1]);
                if (!p.ok || p.value < 0) return Result<void>::fail("Threshold must be >= 0");
                threshold = p.value;
            }
            auto rows = eng.getLowStockItems(threshold);
            if (rows.empty()) {
                OutputFormatter::printWarning("No low-stock items.");
                return Result<void>::success();
            }
            std::vector<std::string> headers = {"Item ID", "Name", "Qty", "Location"};
            std::vector<std::vector<std::string>> table;
            for (const auto& r : rows) {
                table.push_back({
                    std::to_string(r.itemId),
                    r.itemName,
                    std::to_string(r.currentQty),
                    r.location,
                });
            }
            OutputFormatter::printTable(headers, table);
            return Result<void>::success();
        }

        if (sub == "customers") {
            int limit = 10;
            if (a.size() >= 2) {
                auto p = safetyparse(a[1]);
                if (!p.ok || p.value < 1) return Result<void>::fail("Limit must be a positive integer");
                limit = p.value;
            }
            auto rows = eng.getTopCustomers(limit);
            if (rows.empty()) {
                OutputFormatter::printWarning("No customer sales data.");
                return Result<void>::success();
            }
            std::vector<std::string> headers = {"Customer ID", "Name", "Receipts", "Total Spent (EGP)"};
            std::vector<std::vector<std::string>> table;
            for (const auto& r : rows) {
                table.push_back({
                    r.customerId < 0 ? "(manual)" : std::to_string(r.customerId),
                    r.customerName,
                    std::to_string(r.totalReceipts),
                    fmtMoney(r.totalSpent),
                });
            }
            OutputFormatter::printTable(headers, table);
            return Result<void>::success();
        }

        if (sub == "daily") {
            if (a.size() < 3)
                return Result<void>::fail("Usage: report daily <from> <to>  (YYYY-MM-DD)");
            const std::string& from = a[1];
            const std::string& to = a[2];
            auto rows = eng.getDailyRevenue(from, to);
            if (rows.empty()) {
                OutputFormatter::printWarning("No receipts in date range.");
                return Result<void>::success();
            }
            std::vector<std::string> headers = {"Date", "Revenue (EGP)", "Receipts"};
            std::vector<std::vector<std::string>> table;
            for (const auto& r : rows) {
                table.push_back({
                    r.date,
                    fmtMoney(r.revenue),
                    std::to_string(r.receiptCount),
                });
            }
            OutputFormatter::printTable(headers, table);
            return Result<void>::success();
        }

        if (sub == "slowitems") {
            int limit = 10;
            if (a.size() >= 2) {
                auto p = safetyparse(a[1]);
                if (!p.ok || p.value < 1) return Result<void>::fail("Limit must be a positive integer");
                limit = p.value;
            }
            auto rows = eng.getSlowMovingItems(limit);
            if (rows.empty()) {
                OutputFormatter::printWarning("No items.");
                return Result<void>::success();
            }
            std::vector<std::string> headers = {"Item ID", "Name", "Qty Sold", "Revenue (EGP)"};
            std::vector<std::vector<std::string>> table;
            for (const auto& r : rows) {
                table.push_back({
                    std::to_string(r.itemId),
                    r.itemName,
                    std::to_string(r.totalQtySold),
                    fmtMoney(r.totalRevenue),
                });
            }
            OutputFormatter::printTable(headers, table);
            return Result<void>::success();
        }

        return Result<void>::fail(
            "Unknown subcommand: '" + sub + "'. Use summary, topitems, lowstock, customers, daily, slowitems.");
    }
};

class QueueCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.empty()) return Result<void>::fail("Usage: queue <command...>");

        std::vector<std::string> parts = a;
        std::transform(parts[0].begin(), parts[0].end(), parts[0].begin(), ::toupper);

        std::ostringstream raw;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i) raw << ' ';
            raw << parts[i];
        }

        ctx.wms.enqueueTask(raw.str());
        return Result<void>::success();
    }
};

class ProcessQueueCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        size_t limit = 0;
        if (!a.empty()) {
            auto parsed = safetyparse(a[0]);
            if (!parsed.ok) return Result<void>::fail(parsed.error);
            if (parsed.value < 0) return Result<void>::fail("Limit must be >= 0");
            limit = static_cast<size_t>(parsed.value);
        }

        ctx.wms.processTasks(limit);
        return Result<void>::success();
    }
};

class ReceiptCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.size() < 3)
            return Result<void>::fail("Usage: receipt <id quantity price>... [customer] [--cid <customer_id>]");

        // Check for --cid flag
        int linkedCustomerId = -1;
        std::vector<std::string> args;
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] == "--cid") {
                if (i + 1 >= a.size())
                    return Result<void>::fail("--cid requires a customer ID");
                auto cidParsed = safetyparse(a[i + 1]);
                if (!cidParsed.ok)
                    return Result<void>::fail("Invalid customer ID: " + a[i + 1]);
                linkedCustomerId = cidParsed.value;
                ++i; // skip the ID value
            } else {
                args.push_back(a[i]);
            }
        }

        if (args.size() < 3)
            return Result<void>::fail("Usage: receipt <id quantity price>... [customer] [--cid <customer_id>]");

        size_t remainder = args.size() % 3;
        std::string customer;
        size_t itemsEnd = args.size();

        if (remainder == 1) {
            customer = args.back();
            itemsEnd -= 1;
        } else if (remainder != 0) {
            return Result<void>::fail("Usage: receipt <id quantity price>... [customer] [--cid <customer_id>]");
        }

        Receipt receipt;

        // If --cid is provided, look up customer from DB
        if (linkedCustomerId >= 0) {
            auto cust = ctx.wms.getCustomer(linkedCustomerId);
            if (!cust.has_value())
                return Result<void>::fail("Customer not found with ID: " + std::to_string(linkedCustomerId));
            receipt.setCustomer(cust->getName(), cust->getPhone(), cust->getEmail());
        } else if (!customer.empty()) {
            receipt.setCustomer(customer);
        }

        for (size_t i = 0; i < itemsEnd; i += 3) {
            auto id = safetyparse(args[i]);
            if (!id.ok) return Result<void>::fail(id.error);

            int qty = 0;
            double price = 0.0;
            try { qty = std::stoi(args[i + 1]); }
            catch (const std::exception&) { return Result<void>::fail("Quantity must be an integer"); }

            try { price = std::stod(args[i + 2]); }
            catch (const std::exception&) { return Result<void>::fail("Price must be a number"); }

            if (qty <= 0) return Result<void>::fail("Quantity must be > 0");
            if (price < 0) return Result<void>::fail("Price cannot be negative");

            auto item = ctx.wms.getItem(id.value);
            if (!item.has_value()) return Result<void>::fail("Item not found");

            try {
                receipt.addItem(item.value(), qty, price);
            } catch (const std::exception& e) {
                return Result<void>::fail(std::string("Failed to add item: ") + e.what());
            }
        }

        try {
            receipt.print();
            receipt.saveToDB(ctx.wms.getDB());

            // If linked to a customer, save customer_id on the receipt
            if (linkedCustomerId >= 0) {
                try {
                    SQLite::Statement upd(ctx.wms.getDB(),
                        "UPDATE receipts SET customer_id = ? WHERE receipt_number = ?");
                    upd.bind(1, linkedCustomerId);
                    upd.bind(2, receipt.getReceiptNumber());
                    upd.exec();
                } catch (const std::exception& e) {
                    OutputFormatter::printWarning(
                        "Warning: could not link customer to receipt: " + std::string(e.what()));
                }
            }
        } catch (const std::exception& e) {
            return Result<void>::fail(std::string("Failed to generate receipt: ") + e.what());
        }

        // Deduct sold quantities from inventory now that the receipt is committed
        for (size_t i = 0; i < itemsEnd; i += 3) {
            int itemId = std::stoi(args[i]);
            int qty    = std::stoi(args[i + 1]);
            if (!ctx.wms.adjustStock(itemId, -qty)) {
                OutputFormatter::printWarning(
                    "Warning: could not deduct stock for item " + args[i] +
                    " (receipt saved, inventory may be inconsistent)");
            }
        }

        if (ctx.autosave) ctx.wms.saveAll();
        return Result<void>::success();
    }
};

// ─────────────────────────────────────────────
// Customer Command (subcommand router)
// ─────────────────────────────────────────────
// Usage:
//   customer add <name> <phone> <address> [email]
//   customer remove <id>
//   customer list
//   customer search <id>
//   customer search --name <query>
//   customer update <id> [--name <n>] [--phone <p>] [--addr <a>] [--email <e>]

class CustomerCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.empty())
            return Result<void>::fail(
                "Usage: customer <add|remove|list|search|update> [args...]");

        std::string sub = a[0];
        std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);

        // ── customer add <name> <phone> <address> [email] ──
        if (sub == "add") {
            if (a.size() < 4)
                return Result<void>::fail("Usage: customer add <name> <phone> <address> [email]");

            std::string name  = a[1];
            std::string phone = a[2];
            std::string addr  = a[3];
            std::string email = (a.size() >= 5) ? a[4] : "";

            if (!ctx.wms.addCustomer(name, phone, addr, email))
                return Result<void>::fail("Failed to add customer");

            OutputFormatter::printInfo("Customer added successfully.");
            return Result<void>::success();
        }

        // ── customer remove <id> ──
        if (sub == "remove") {
            if (a.size() != 2)
                return Result<void>::fail("Usage: customer remove <id>");

            auto id = safetyparse(a[1]);
            if (!id.ok) return Result<void>::fail(id.error);

            if (!ctx.wms.removeCustomer(id.value))
                return Result<void>::fail("Customer not found");

            OutputFormatter::printInfo("Customer removed.");
            return Result<void>::success();
        }

        // ── customer list ──
        if (sub == "list") {
            auto customers = ctx.wms.getAllCustomers();
            if (customers.empty()) {
                OutputFormatter::printWarning("No customers found.");
                return Result<void>::success();
            }

            std::vector<std::string> headers = {"ID", "Name", "Phone", "Address", "Email"};
            std::vector<std::vector<std::string>> rows;
            for (const auto& c : customers) {
                rows.push_back({
                    std::to_string(c.getId()),
                    c.getName(),
                    c.getPhone(),
                    c.getAddress(),
                    c.getEmail()
                });
            }
            OutputFormatter::printTable(headers, rows);
            return Result<void>::success();
        }

        // ── customer search <id>  |  customer search --name <query> ──
        if (sub == "search") {
            if (a.size() < 2)
                return Result<void>::fail("Usage: customer search <id>  |  customer search --name <query>");

            if (a[1] == "--name") {
                if (a.size() < 3)
                    return Result<void>::fail("Usage: customer search --name <query>");

                auto results = ctx.wms.searchCustomerByName(a[2]);
                if (results.empty())
                    return Result<void>::fail("No customers found matching: " + a[2]);

                for (const auto& c : results) printCustomer(c);
                return Result<void>::success();
            }

            // Search by ID
            auto id = safetyparse(a[1]);
            if (!id.ok) return Result<void>::fail(id.error);

            auto customer = ctx.wms.getCustomer(id.value);
            if (!customer.has_value())
                return Result<void>::fail("Customer not found");

            printCustomer(customer.value());
            return Result<void>::success();
        }

        // ── customer update <id> [--name <n>] [--phone <p>] [--addr <a>] [--email <e>] ──
        if (sub == "update") {
            if (a.size() < 2)
                return Result<void>::fail(
                    "Usage: customer update <id> [--name <n>] [--phone <p>] [--addr <a>] [--email <e>]");

            auto id = safetyparse(a[1]);
            if (!id.ok) return Result<void>::fail(id.error);

            std::optional<std::string> name, phone, addr, email;
            bool anyFlag = false;

            for (size_t i = 2; i + 1 < a.size(); i += 2) {
                const std::string& flag = a[i];
                const std::string& val  = a[i + 1];

                if (flag == "--name") {
                    name = val;
                    anyFlag = true;
                } else if (flag == "--phone") {
                    phone = val;
                    anyFlag = true;
                } else if (flag == "--addr") {
                    addr = val;
                    anyFlag = true;
                } else if (flag == "--email") {
                    email = val;
                    anyFlag = true;
                } else {
                    return Result<void>::fail("Unknown flag: " + flag);
                }
            }

            if (!anyFlag)
                return Result<void>::fail("No fields specified. Use --name, --phone, --addr, --email");

            if (!ctx.wms.updateCustomer(id.value, name, phone, addr, email))
                return Result<void>::fail("Customer not found or update failed");

            OutputFormatter::printInfo("Customer updated.");
            return Result<void>::success();
        }

        return Result<void>::fail(
            "Unknown subcommand: '" + sub + "'. Use add, remove, list, search, or update.");
    }
};