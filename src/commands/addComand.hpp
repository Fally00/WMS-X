#pragma once

//included files
#include "commands/CommandContext.hpp" // ← needed for CommandContext definition
#include "utils/safetyparse.hpp"
#include "commands/command.hpp"
#include "storage/Receipt.h"
#include "output/output.h"
#include "models/Item.h"

//needed libraries
#include <optional>
#include <sstream>
#include <algorithm>
#include <stdexcept>


// Command to add an item
class AddCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.size() < 4)
            return Result<void>::fail("Usage: add <id> <name> <qty> <loc>");

        auto id = safetyparse(a[0]);
        auto qty = safetyparse(a[2]);
        if (!id.ok || !qty.ok)
            return Result<void>::fail(id.ok ? qty.error : id.error);

        //  Now expects WmsControllers::addItem(int, string, int, string)
        if (!ctx.wms.addItem(id.value, a[1], qty.value, a[3]))
            return Result<void>::fail("Item exists");

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
// Usage: update <id> [--name <n>] [--qty <q>] [--loc <l>] [--price <p>]
class UpdateCommand : public ICommand {
public:
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.empty())
            return Result<void>::fail("Usage: update <id> [--name <n>] [--qty <q>] [--loc <l>] [--price <p>]");

        auto id = safetyparse(a[0]);
        if (!id.ok) return Result<void>::fail(id.error);

        std::optional<std::string> name, loc;
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
            } else {
                return Result<void>::fail("Unknown flag: " + flag);
            }
        }

        if (!anyFlag)
            return Result<void>::fail("No fields specified. Use --name, --qty, --loc, --price");

        if (!ctx.wms.updateItem(id.value, name, qty, loc, price))
            return Result<void>::fail("Item not found or update failed");

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
            return Result<void>::fail("Usage: receipt <id quantity price>... [customer]");

        size_t remainder = a.size() % 3;
        std::string customer;
        size_t itemsEnd = a.size();

        if (remainder == 1) {
            customer = a.back();
            itemsEnd -= 1;
        } else if (remainder != 0) {
            return Result<void>::fail("Usage: receipt <id quantity price>... [customer]");
        }

        Receipt receipt;
        if (!customer.empty()) receipt.setCustomer(customer);

        for (size_t i = 0; i < itemsEnd; i += 3) {
            auto id = safetyparse(a[i]);
            if (!id.ok) return Result<void>::fail(id.error);

            int qty = 0;
            double price = 0.0;
            try { qty = std::stoi(a[i + 1]); }
            catch (const std::exception&) { return Result<void>::fail("Quantity must be an integer"); }

            try { price = std::stod(a[i + 2]); }
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
        } catch (const std::exception& e) {
            return Result<void>::fail(std::string("Failed to generate receipt: ") + e.what());
        }

        // Deduct sold quantities from inventory now that the receipt is committed
        for (size_t i = 0; i < itemsEnd; i += 3) {
            int itemId = std::stoi(a[i]);
            int qty    = std::stoi(a[i + 1]);
            if (!ctx.wms.adjustStock(itemId, -qty)) {
                OutputFormatter::printWarning(
                    "Warning: could not deduct stock for item " + a[i] +
                    " (receipt saved, inventory may be inconsistent)");
            }
        }

        if (ctx.autosave) ctx.wms.saveAll();
        return Result<void>::success();
    }
};