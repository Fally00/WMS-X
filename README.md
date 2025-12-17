##WMS‑X — Warehouse Management System in Modern C++##
What is WMS‑Cpp?

WMS‑Cpp is a console‑based Warehouse Management System implemented in robust modern C++. It models real warehouse workflows like inventory tracking, receipt handling, batch tasks, and data persistence, all with a focus on clean architecture, modularity, and real‑world usability. 
GitHub

This isn’t just toy code — it’s a system redesigned from the ground up with performance, readability, and maintainability in mind.

## What Changed in v1.2.0 ##
```
poured actual craft into this release — and it shows! This version includes:
```
##Major Improvements##

-- Redesigned architecture to make modules more independent and testable

-- Refactored codebase — clearer logic, better function boundaries, safer memory use

-- Comprehensive bug fixes across core features

-- Optimizations for performance and user responsiveness

-- More readable console UI & workflow feedback

-- Better layering of logic (UI ↔ core ↔ data)

-- Path paved for future GUI, DB, or web interface support

This is not your average “hello world” example — it’s a serious inventory system with a foundation for expansion.

##Core Features##

##Inventory Management##

--Track items, quantities, metadata and search inventory efficiently. 


##Receipt System##
--Generate, queue, and print receipts for transactions — stateful and persistent. 


## Batch Processing##
--Queue up multiple operations to run sequentially — simulating real warehouse tasks. 


##Data Persistence##
--CSV‑based storage that loads and saves inventory automatically. 


##Console UI##
--ANSI‑colored, menu‑driven UX for exploring features without digging into code. 


---
```
## 🗂️ Project Structure
WMS‑Cpp/
├── src/                  # Implementation files
│   ├── main.cpp
│   ├── Inventory.cpp
│   ├── Receipt.cpp
│   ├── Storage.cpp
│   ├── WmsControllers.cpp
│   └── … 
├── include/              # Public interface headers
│   ├── Item.h
│   ├── Inventory.h
│   ├── Receipt.h
│   ├── Storage.h
│   └── …
├── data/
│   └── inventory_data.csv
├── README.md
└── build/                # Compiled output
```

*(Structure may evolve as the system grows)*

---

## 🧪 How It Works

How It Works

1- Interactive Menu — The user selects actions from a menu.

2- Controller Dispatch — Controller modules route input to appropriate logic.

3- Logic Modules — Inventory, receipts, and batch queues process actions.

4- Persistence — CSV files auto‑save changes between runs.

--> This separation keeps code **testable, readable, and maintainable**.

---

## 🛠️ Build & Run

### Requirements
- C++17 or newer
- GCC / Clang / MSVC
- Terminal with ANSI color support (recommended)

### Compile
```bash
g++ main.cpp WmsControllers.cpp Inventory.cpp Item.cpp Storage.cpp Receipt.cpp -o wms.exe
```

**Current Status**
```
Component	Status
Inventory system	  -->   ✅ Stable
Receipt generation  -->	  🟡 Functional
Batch queue	        -->   🟠 Improving
CSV persistence	    -->   🟡 Working
Input validation	  -->   🛠️ Ongoing
Unit tests	        -->   🧪 Planned
Database support    --> 	🔜 Future
GUI / Web interface --> 	🔜 Future
```
