# WMS-X — Warehouse Management System

A **modular**, **high-performance** Warehouse Management System built in **modern C++17**, featuring both a **CLI** and a **Qt6 GUI**, backed by **SQLite** for persistent storage.

> **Dual Interface** — Full-featured CLI for power users and a Qt6 GUI for visual inventory management, receipts, reports, and exporting.

![WMS CLI Preview](assest/logo.png)
---

## Features

### Core
| Feature | Description |
|---------|-------------|
| **Inventory Management** | Add, update, delete, search and list items with validation (supports barcodes) |
| **Persistent Storage** | SQLite-backed persistence with automatic load/save |
| **Receipt System** | Generate multi-item receipts, preview, and export |
| **Customer Management** | Robust CRM directly integrated (add, update, search, link to receipts) |
| **Reports Engine** | Calculate daily revenue, top selling items, low stock warnings, and top customers |
| **Task Queue** | Queue batch commands (ADD/REMOVE/LIST/SEARCH) and process them |
| **Item Status** | Automatic stock status: Available, Low Stock, Out of Stock |
| **Cross-Platform** | Windows / Linux / macOS via GCC / Clang / MSVC |

### Qt6 GUI
| Feature | Description |
|---------|-------------|
| **Inventory Table** | Full inventory view with multi-select, color-coded status column |
| **CRM Windows** | Customer management dialogs for searching and editing customers |
| **KPI Dashboards** | Rich reports tab with sales charts, low stock alerts, and top items lists |
| **Multi-Item Receipts** | Select multiple items → set qty/price per item → generate one receipt |
| **Receipt History** | Browse past receipts, link to customers, export to formatted text/CSV |
| **Queue System** | Queue and run batch tasks from the GUI |

---

## Architecture

```
CLI  ──▶  Command System  ──▶  Controllers  ───▶  Inventory Engine
                                 │
GUI  ──▶  Qt6 Widgets  ─────────▶├── Storage (SQLite)
                                 ├── Receipt Engine
                                 ├── Reports Engine
                                 └── Domain Models (Item, Inventory, Customer)
```

---

## Project Structure

```
WMS-X/
├── src/
│   ├── main.cpp                       # CLI entry point & REPL loop
│   ├── models/
│   │   ├── Item.h / Item.cpp          # Domain model — item entity
│   │   ├── Customer.h / Customer.cpp  # Customer CRM models
│   │   └── Inventory.h / Inventory.cpp# In-memory inventory + SQLite sync
│   ├── storage/
│   │   ├── Storage.h / Storage.cpp    # SQLite database wrapper
│   │   ├── Receipt.h / Receipt.cpp    # Receipt generation & persistence
│   │   └── CustomerStorage.cpp        # Customer DB operations
│   ├── reports/
│   │   └── ReportEngine.h / .cpp      # Reports and KPI aggregations
│   ├── controllers/
│   │   └── WmsControllers.h / .cpp    # Business logic & task queue
│   ├── commands/
│   │   ├── command.hpp                # ICommand interface
│   │   ├── addComand.hpp              # All command implementations
│   │   ├── CommandContext.hpp         # Execution context struct
│   │   └── registry.hpp               # Command registry
│   ├── cli/
│   │   └── cli.h / cli.cpp            # CLI argument parser
│   ├── output/
│   │   └── output.h / output.cpp      # Formatted output & color support
│   └── utils/
│       └── ...
├── wms_gui/                           # Qt6 GUI Application Files
├── vendor/
│   └── SQLiteCpp/                     # SQLite C++ wrapper (vendored)
├── assest/                            # Screenshots & logo
├── CMakeLists.txt                     # CLI build configuration
└── README.md
```

---

## Requirements

| Tool | Version |
|------|---------|
| C++ Compiler | GCC / Clang / MSVC (C++17+) |
| CMake | 3.16+ |
| Qt6 (GUI only) | 6.x with Widgets, PrintSupport, and Charts modules |

---

## Building

### Set up Qt toolchain (Windows)

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;$env:PATH"
```

### Build CLI

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: `build/wms.exe`

### Build GUI

```powershell
cmake -S wms_gui -B wms_gui/build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/mingw_64" -DCMAKE_BUILD_TYPE=Release
cmake --build wms_gui/build
```

Output: `wms_gui/build/wms_gui.exe`

---

## CLI Usage

| Command | Action |
|---------|--------|
| `add <id> <name> <qty> <loc> [--barcode <val>]` | Add new item (with optional barcode) |
| `update <id> [--name] [--qty] [--loc] [--price] [--barcode]` | Update existing item fields |
| `remove <id>` | Delete an item |
| `search <id>` or `search --name <query>` | Find items |
| `scan <barcode>` | Scan and find item by barcode |
| `list [page] [pageSize]` | List items (paged) |
| `queue <COMMAND...>` | Queue a batch task |
| `runq [limit]` | Process queued tasks |
| `receipt <id qty price>... [cust] [--cid <id>]` | Generate receipt (optionally linked to customer ID) |
| `customer <add\|remove\|list\|search\|update>` | Manage CRM customers |
| `report <summary\|topitems\|lowstock\|customers\|daily\|slowitems>` | Display KPIs and analysis reports |
| `help` | Show commands |
| `exit` | Quit |

---

## Roadmap

| Phase | Feature |
|-------|---------|
| Phase 1 | CLI Core ✅ |
| Phase 2 | SQLite Backend ✅ |
| Phase 3 | Qt6 GUI ✅ |
| Phase 4 | Reporting Engine & CRM ✅ |
| Phase 5 | REST API (Crow / Pistache) |
| Phase 6 | Web Dashboard |

---

### Author

**Rayan (Fally)**  
Cybersecurity Student • Systems Engineer • Builder of unnecessarily powerful tools
