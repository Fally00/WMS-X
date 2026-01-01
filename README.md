#  WMS-X — Warehouse Management System (C++ CLI Edition)

A **modular**, **high-performance** Warehouse Management System built in **modern C++17**, designed with clean architecture, real-world logistics workflows, and future scalability in mind.

>  **Production-Ready Core** — Engineered to serve as a backend foundation for **CLI**, **GUI**, **REST API**, and enterprise integrations — *without refactoring business logic*.

![WMS CLI Preview](assets/logo.png)
---

##  Key Features

| Category | Description |
|--------|-------------|
| - **Inventory Management** | Add, update, delete, search and list items with validation
| - - -![WMS CLI Preview](assets/search.png)                                                                             |
| - **Persistent Storage** | CSV-backed persistence (`inventory_data.csv`) with automatic load/save
| - - -![WMS CLI Preview](assets/list.png)                                                                               | 
| - **Receipt System** | Generate timestamped transaction receipts (e.g., for audits)
| - - -![WMS CLI Preview](assets/receipt.png)                                                                            |
| - **Command Architecture** | Extensible CLI command system — plug in new commands via registration
| - - -![WMS CLI Preview](assets/help.png)                                                                               |
| - **Modular Core** | Strict separation: domain models → business logic → persistence → interface                       |
| - **CMake Build** | Cross-platform (Windows/macOS/Linux) with one-command builds                                       |
| - **Future-Proof** | Core designed for seamless integration with Qt GUI, REST APIs (Crow/Pistache), and web dashboards |

---

##  Architecture Overview
```
CLI  ──▶  Command System  ──▶  Controllers  ──▶  Inventory Engine
                                 │
                                 ├── Storage (CSV Persistence)
                                 ├── Receipt Engine
                                 └── Domain Models (Item, Inventory)

```

 **Separation of Concerns**:  
- **Domain** (`Item`, `Inventory`) encapsulates business rules  
- **Storage** handles data persistence *without* leaking format details upward  
- **Controllers** orchestrate workflows — ideal for testing & reuse  

---

##  Project Structure

```bash
WMS-Cpp/
│
├── core/
│   ├── Inventory.h / .cpp        # Inventory container & logic
│   ├── Item.h / .cpp             # Inventory item model
│   ├── Storage.h / .cpp          # CSV persistence engine
│   ├── Receipt.h / .cpp          # Transaction receipts
│   ├── WmsControllers.h / .cpp   # Business logic controllers
│   ├── command.hpp               # Extensible command system
│   └── main.cpp                  # Core application entry
│
├── cli.cpp                       # CLI interface layer
├── inventory_data.csv            # Persistent inventory file
└── README.md
```

---

## Requirements
```
   Tool	                              Version
C++ Compiler	               GCC / Clang / MSVC (C++17+)
CMake	                       3.10+
Git	                           Latest
```
---

## CLI Usage
```
Command	        Action
add	Add     new inventory item
list	    Display all items
remove	    Delete item
update	    Modify item details
receipt	    Generate transaction receipt
exit	    Close application
```
## Building process using MSYS
```
g++ -std=c++17 -O0 -g -Wall -Wextra -Icore core\\*.cpp cli.cpp -o wms.exe
```
**same command will work if the same structure is followed as above**
---

## Roadmap
```
Phase	        Feature
Phase 1	    CLI Core (Current)
Phase 2	    JSON persistence + SQLite backend
Phase 3	    Qt GUI Front-End
Phase 4	    REST API (C++ Crow / Pistache)
Phase 5	    Web Dashboard
```

---

### Author
**Rayan (Fally)**
**Cybersecurity Student • Systems Engineer • Builder of unnecessarily powerful tools**

---
