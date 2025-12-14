# Warehouse Management System (WMS) – C++

A **robust, console-based Warehouse Management System** written in **modern C++**.  
This project focuses on clean architecture, modular design, and real-world inventory workflows.

Designed as a learning-driven yet scalable system, this WMS simulates how inventory, receipts, and batch operations are handled in real warehouse software.

---

## 🧠 Project Goals

- Practice **real system design**, not toy examples
- Apply **OOP principles** correctly in C++
- Build a foundation that can later evolve into:
  - GUI applications
  - Database-backed systems
  - Enterprise-level WMS software

This is **not** a finished product — it’s an evolving system with a strong architectural base.

---

## ⚙️ Core Features

### 📦 Inventory Management
- Add, remove, update, and search items
- Track item ID, name, quantity, and metadata
- Centralized inventory logic

### 🧾 Receipt System
- Generate receipts for transactions
- Queue-based receipt item handling
- Print and persist receipts
- Clear receipt state after completion

### 🔁 Batch Processing
- Queue system for executing multiple tasks sequentially
- Designed for automation and future scheduling

### 💾 Data Persistence
- CSV-based storage (configurable)
- Automatic save/load on runtime
- Easy to replace with a database later

### 🖥️ Console UI
- Clean, readable console output
- ANSI color support for better UX
- Menu-driven interaction

---
```
## 🗂️ Project Structure
WMS-Cpp/
├── src/
│ ├── main.cpp # Program entry point
│ ├── Inventory.cpp # Inventory logic
│ ├── Receipt.cpp # Receipt handling
│ ├── BatchQueue.cpp # Batch processing system
│ └── Utils.cpp # Helper utilities
│
├── include/
│ ├── Item.h
│ ├── Inventory.h
│ ├── Receipt.h
│ ├── BatchQueue.h
│ └── Utils.h
│
├── data/
│ └── inventory.csv # Persistent storage
│
├── README.md
└── build/ # the wms.exe program
```

*(Structure may evolve as the system grows)*

---

## 🧪 How It Works

1. User interacts with the **console menu**
2. Actions are routed to dedicated modules
3. Inventory updates are reflected immediately
4. Receipts are generated and stored
5. Data is persisted automatically

This separation keeps logic **testable, readable, and maintainable**.

---

## 🛠️ Build & Run

### Requirements
- C++17 or newer
- GCC / Clang / MSVC
- Terminal with ANSI color support (recommended)

### Compile
```bash

```

📈 Current Status
Component	Status
Inventory system	✅ Stable
Receipt generation	✅ Functional
Batch queue	🟡 Improving
CSV persistence	✅ Working
Error handling	🟡 Partial
Code refactoring	🟡 Ongoing
🚧 Planned Improvements

Input validation & exception safety

Unit tests

Better receipt formatting

Database support (SQLite / MySQL)

Role-based access (Admin / Worker)

GUI or Web interface
