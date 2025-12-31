#pragma once
#include "core/commands/command.hpp"
#include "core/commands/safetyparse.hpp"

// Command to add an item
class AddCommand : public ICommand {
public:
    // Execute the add command
    Result<void> execute(CommandContext& ctx, const std::vector<std::string>& a) override {
        if (a.size() < 4)
            return Result<void>::fail("Usage: add <id> <name> <qty> <loc>");

        auto id = safetyparse(a[0]);
        auto qty = safetyparse(a[2]);
        if (!id.ok || !qty.ok)
            return Result<void>::fail(id.ok ? qty.error : id.error);

        if (!ctx.wms.addItem(id.value, a[1], qty.value, a[3]))
            return Result<void>::fail("Item exists");

        if (ctx.autosave) ctx.wms.saveAll();
        return Result<void>::success();
    }
};
