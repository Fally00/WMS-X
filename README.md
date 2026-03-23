
#  WMS-X — Warehouse Management System

A **modular**, **high-performance** Warehouse Management System built in **modern C++17**, featuring both a **CLI** and a **Qt6 GUI**, backed by **SQLite** for persistent storage.

> **Dual Interface** — Full-featured CLI for power users and a Qt6 GUI for visual inventory management, receipts, and CSV export.

![WMS CLI Preview](assest/logo.png)
---

##  Features

### Core
| Feature | Description |
|---------|-------------|
| **Inventory Management** | Add, update, delete, search and list items with validation |
| **Persistent Storage** | SQLite-backed persistence with automatic load/save |
| **Receipt System** | Generate multi-item receipts, preview, and export to CSV |
| **Task Queue** | Queue batch commands (ADD/REMOVE/LIST/SEARCH) and process them |
| **Item Status** | Automatic stock status: Available, Low Stock, Out of Stock |
| **Command Architecture** | Extensible CLI command system via registration |
| **Cross-Platform** | Windows / Linux / macOS via GCC / Clang / MSVC |

### Qt6 GUI
| Feature | Description |
|---------|-------------|
| **Inventory Table** | Full inventory view with multi-select, color-coded status column |
| **CRUD Operations** | Add, update, delete items via dialog forms |
| **Multi-Item Receipts** | Select multiple items → set qty/price per item → generate one receipt |
| **Receipt Preview** | View receipt details (items, subtotal, tax, total) after creation |
| **Receipt History** | Browse all past receipts, click to preview any one |
| **CSV Export** | Export all receipts to a `.csv` file for Excel/printing |
| **Queue System** | Queue and run batch tasks from the GUI |

---

##  Architecture

```
CLI  ──▶  Command System  ──▶  Controllers  ──▶  Inventory Engine
                                 │
GUI  ──▶  Qt6 Widgets  ────────▶├── Storage (SQLite)
                                 ├── Receipt Engine
                                 └── Domain Models (Item, Inventory)
```

---

##  Project Structure

```
WMS-X/
├── src/
│   ├── main.cpp                       # CLI entry point & REPL loop
│   ├── models/
│   │   ├── Item.h / Item.cpp          # Domain model — item entity
│   │   └── Inventory.h / Inventory.cpp# In-memory inventory + SQLite sync
│   ├── storage/
│   │   ├── Storage.h / Storage.cpp    # SQLite database wrapper
│   │   └── Receipt.h / Receipt.cpp    # Receipt generation & persistence
│   ├── controllers/
│   │   └── WmsControllers.h / .cpp    # Business logic & task queue
│   ├── commands/
│   │   ├── command.hpp                # ICommand interface
│   │   ├── addComand.hpp              # All command implementations
│   │   ├── CommandContext.hpp         # Execution context struct
│   │   └── registry.hpp              # Command registry
│   ├── cli/
│   │   └── cli.h / cli.cpp           # CLI argument parser
│   ├── output/
│   │   └── output.h / output.cpp     # Formatted output & color support
│   └── utils/
│       ├── results.hpp                # Result<T> type
│       ├── safetyparse.hpp            # Safe integer parsing
│       └── parser.hpp                 # Input tokenizer
├── wms_gui/
│   ├── main.cpp                       # GUI entry point & all slots
│   ├── main.h                         # MainWindow class declaration
│   ├── main.ui                        # Qt Designer UI form
│   └── CMakeLists.txt                 # GUI build configuration
├── vendor/
│   └── SQLiteCpp/                     # SQLite C++ wrapper (vendored)
├── assest/                            # Screenshots & logo
├── CMakeLists.txt                     # CLI build configuration
└── README.md
```

---

##  Requirements

| Tool | Version |
|------|---------|
| C++ Compiler | GCC / Clang / MSVC (C++17+) |
| CMake | 3.16+ |
| Qt6 (GUI only) | 6.x with Widgets module |

---

##  Building

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

##  CLI Usage

| Command | Action |
|---------|--------|
| `add <id> <name> <qty> <loc>` | Add new item |
| `remove <id>` | Delete item |
| `update <id> [--name] [--qty] [--loc] [--price]` | Update item |
| `list [page] [pageSize]` | List items (paged) |
| `search <id>` or `search --name <query>` | Find items |
| `queue <COMMAND...>` | Queue a batch task |
| `runq [limit]` | Process queued tasks |
| `receipt <id qty price>... [customer]` | Generate receipt |
| `help` | Show commands |
| `exit` | Quit |

---

##  Roadmap

| Phase | Feature |
|-------|---------|
| Phase 1 | CLI Core ✅ |
| Phase 2 | SQLite Backend ✅ |
| Phase 3 | Qt6 GUI ✅ |
| Phase 4 | REST API (Crow / Pistache) |
| Phase 5 | Web Dashboard |

---

###  Author

**Rayan (Fally)**  
Cybersecurity Student • Systems Engineer • Builder of unnecessarily powerful tools
