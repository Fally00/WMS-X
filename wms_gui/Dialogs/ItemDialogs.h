#pragma once

#include "controllers/WmsControllers.h"
#include <QTableWidget>
#include <QWidget>


namespace ItemDialogs {
// Shows the Add Item dialog. Returns true if an item was added.
bool showAddDialog(QWidget *parent, WmsControllers &controller);

// Shows the Update Item dialog. Returns true if the item was updated.
bool showUpdateDialog(QWidget *parent, WmsControllers &controller,
                      QTableWidget *inventoryTable);

// Prompts and deletes the selected item. Returns true if deleted.
bool showDeleteDialog(QWidget *parent, WmsControllers &controller,
                      QTableWidget *inventoryTable);
} // namespace ItemDialogs
