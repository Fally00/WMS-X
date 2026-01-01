
#  WMS-X — Warehouse Management System (C++ CLI Edition)

A **modular**, **high-performance** Warehouse Management System built in **modern C++17**, designed with clean architecture, real-world logistics workflows, and future scalability in mind.

> **Production-Ready Core** — Engineered to serve as a backend foundation for **CLI**, **GUI**, **REST API**, and enterprise integrations — *without refactoring business logic*.

![WMS CLI Preview](assest/logo.png)
---

##  Key Features

| Feature | Preview |
|--------|---------|
| **Inventory Management** — Add, update, delete, search and list items with validation | <img src="https://raw.githubusercontent.com/Fally00/WMS-X/main/assest/search.png" width="300"> |
| **Persistent Storage** — CSV-backed persistence (`inventory_data.csv`) with automatic load/save | <img src="https://raw.githubusercontent.com/Fally00/WMS-X/main/assest/list.png" width="300"> |
| **Receipt System** — Generate timestamped transaction receipts (e.g., for audits) | <img src="https://raw.githubusercontent.com/Fally00/WMS-X/main/assest/receipt.png" width="300"> |
| **Command Architecture** — Extensible CLI command system via registration | <img src="https://raw.githubusercontent.com/Fally00/WMS-X/main/assest/help.png" width="300"> |
| **Modular Core** — Strict separation: domain → controllers → storage → interface |  |
| **CMake Build** — Cross-platform Windows / Linux / macOS |  |
| **Future-Proof** — Qt GUI, REST API, Web Dashboard ready |  |

---

##  Architecture Overview

```
CLI  ──▶  Command System  ──▶  Controllers  ──▶  Inventory Engine
                                 │
                                 ├── Storage (CSV Persistence)
                                 ├── Receipt Engine
                                 └── Domain Models (Item, Inventory)
```

---

##  Requirements

| Tool | Version |
|------|--------|
| C++ Compiler | GCC / Clang / MSVC (C++17+) |
| CMake | 3.10+ |
| Git | Latest |

---

##  CLI Usage

| Command | Action |
|--------|--------|
| `add` | Add new inventory item |
| `list` | Display all items |
| `remove` | Delete item |
| `update` | Modify item details |
| `receipt` | Generate transaction receipt |
| `exit` | Close application |

---

##  Building process (MSYS / MinGW)

```bash
g++ -std=c++17 -O0 -g -Wall -Wextra -Icore core/*.cpp cli.cpp -o wms.exe
```

---

##  Roadmap

| Phase | Feature |
|------|--------|
| Phase 1 | CLI Core (Current) |
| Phase 2 | JSON persistence + SQLite backend |
| Phase 3 | Qt GUI Front-End |
| Phase 4 | REST API (Crow / Pistache) |
| Phase 5 | Web Dashboard |

---

###  Author

**Rayan (Fally)**  
Cybersecurity Student • Systems Engineer • Builder of unnecessarily powerful tools
