                         📦 Warehouse Management System (WMS) — C++


A modular, console-based Warehouse Management System written in modern C++ (C++17).
Built to be efficient, extensible, and readable, this project demonstrates solid architecture, clean separation of concerns, and practical systems programming concepts.

Think real-world inventory logic, minus enterprise bloat.

--> ✨ Key Highlights

1- 🧠 Clean Architecture — Clear separation between logic, storage, UI, and control flow

2- ⚡ Efficient Core Logic — Optimized ID handling & reduced unnecessary object copies

3- 🧾 Receipt System — Automatic transaction receipts (because accountability matters)

4- 🔄 Batch / Queue Mode — Execute multiple operations sequentially

5- 💾 CSV Persistence — Lightweight, configurable data storage

6- 🎨 Console UI — ANSI colors + ASCII banners for extra drip

--> 🔧 Features

1-📦 Inventory Management

Add, remove, search, and list warehouse items

Clean item abstraction via the Item module

2- 🧾 Receipt System (New)

Automatically generates detailed receipts for inventory transactions

Clear logging of item changes per operation

3-⚡ Performance Improvements (New)

Smarter ID assignment

Improved object passing (less copying, more sanity)

4- 🔄 Queue / Batch Processing

Enqueue multiple operations

Execute them sequentially (ideal for bulk updates)

5- 💾 Auto-Save System

Toggleable CSV persistence

Automatically saves after operations when enabled

6- 🏗️ Modular Design

Each responsibility lives where it should:

Inventory → Item collection & logic

Item → Item data structure

Storage → CSV I/O & persistence

Receipt → Transaction logging

WmsControllers → User input & coordination

--> 🛠️ Getting Started
📌 Prerequisites

Compiler: Any C++ compiler with C++17 support (e.g. g++)

OS:

Windows (MinGW / MSYS2)

Linux

```
--> 📂 Project Structure
.
├── main.cpp              # Application entry point
├── WmsControllers.*      # User input & flow control
├── Inventory.*           # Inventory management logic
├── Item.*                # Item data structure
├── Storage.*             # CSV persistence
├── Receipt.*             # Receipt & transaction logging

🪄 Build & Run
▶️ Quick Run (Windows)

```bash
If you just want to run it:

wms.exe

🔨 Build From Source

If you’ve modified the code or want a clean build:

g++ main.cpp WmsControllers.cpp Inventory.cpp Item.cpp Storage.cpp Receipt.cpp -o wms.exe


⚠️ Important:
If you’re using the receipt system, make sure Receipt.cpp is included or the linker will choose violence.

▶️ Run
./wms.exe

📌 Versioning

Current Version: v1.1.0

Added receipt system

Performance optimizations

Improved internal logic
