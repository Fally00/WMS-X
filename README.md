📦 Warehouse Management System (WMS) — C++

A modular, console-based Warehouse Management System written in modern C++ (C++17).
Built to be efficient, extensible, and readable, this project demonstrates solid architecture, clean separation of concerns, and practical systems programming concepts.

Think real-world inventory logic, minus enterprise bloat.

✨ Key Highlights

🧠 Clean Architecture — Clear separation between logic, storage, UI, and control flow

⚡ Efficient Core Logic — Optimized ID handling & reduced unnecessary object copies

🧾 Receipt System — Automatic transaction receipts (because accountability matters)

🔄 Batch / Queue Mode — Execute multiple operations sequentially

💾 CSV Persistence — Lightweight, configurable data storage

🎨 Console UI — ANSI colors + ASCII banners for extra drip

🔧 Features
📦 Inventory Management

Add, remove, search, and list warehouse items

Clean item abstraction via the Item module

🧾 Receipt System (New)

Automatically generates detailed receipts for inventory transactions

Clear logging of item changes per operation

⚡ Performance Improvements (New)

Smarter ID assignment

Improved object passing (less copying, more sanity)

🔄 Queue / Batch Processing

Enqueue multiple operations

Execute them sequentially (ideal for bulk updates)

💾 Auto-Save System

Toggleable CSV persistence

Automatically saves after operations when enabled

🏗️ Modular Design

Each responsibility lives where it should:

Inventory → Item collection & logic

Item → Item data structure

Storage → CSV I/O & persistence

Receipt → Transaction logging

WmsControllers → User input & coordination

🛠️ Getting Started
📌 Prerequisites

Compiler: Any C++ compiler with C++17 support (e.g. g++)

OS:

Windows (MinGW / MSYS2)

Linux


📂 Project Structure
.
├── main.cpp              # Application entry point
├── WmsControllers.*      # User input & flow control
├── Inventory.*           # Inventory management logic
├── Item.*                # Item data structure
├── Storage.*             # CSV persistence
├── Receipt.*             # Receipt & transaction logging

🪄 Build & Run
▶️ Quick Run (Windows)

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

🚀 Future Improvements (Planned / Ideas)

File format abstraction (CSV → JSON / SQLite)

Undo / rollback system

Unit tests (yes, real ones)

CMake support (to end manual compile suffering)

Cross-platform release builds
