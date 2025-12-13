Warehouse Management System (WMS) 👨‍💻
A robust, console-based warehouse and inventory management system written in C++. Designed for efficiency and extensibility, this project demonstrates clean architecture and modern C++ practices.

Manage Inventory: Support for adding, removing, searching, and listing items.

Receipt System: Automatically generates receipts for transactions.

Batch Processing: Task queue for executing multiple operations in sequence.

Data Persistence: Automatic CSV-based storage (configurable).

Console UI: Clean interface with ANSI color support and ASCII banners.

🔧 Features
📦 Core Inventory Management: Add, remove, search, and list warehouse items effortlessly.

🧾 Digital Receipt System: [New] Generate and display detailed receipts for inventory transactions.

⚡ Optimized Logic: [New] Enhanced ID management algorithms and improved object-passing efficiency for better performance.

🔄 Queue Mode: Enqueue operations to be processed in batches (ideal for bulk updates).

💾 Auto-Save: Toggleable automatic persistence to CSV after every operation.

🏗️ Modular Architecture: Clean separation of concerns using Inventory, Item, Storage, Receipt, and Controller modules.

🎨 UI/UX: User-friendly console interface with color-coded feedback and ASCII art.

🛠️ Getting Started
Prerequisites
Compiler: A C++ compiler with C++17 support (e.g., g++).

OS: Windows (MSYS2/MinGW) or Unix-based systems (Linux/macOS).

📂 Project Structure
main.cpp: Entry point of the application.

WmsControllers: Handles user input and coordinates between modules.

Inventory: Manages the collection of items.

Storage: Handles CSV file I/O and persistence.

Receipt: Manages transaction logging and receipt generation.

Item: Defines the data structure for warehouse items.

🪄Build & Run
You can run the existing wms.exe directly. However, if you have modified the code or want to build from the source, follow the steps below.

To compile the project manually:

```bash

g++ main.cpp WmsControllers.cpp Inventory.cpp Item.cpp Storage.cpp -o wms.exe

that command to link all files and get them into one wms.exe program which the one u will use to run and be able to run the program 
(Make sure to include Receipt.cpp if you have created a source file for the new receipt system!)

To run the application:

./wms.exe


📌 Version
Current Version: v1.1.0 (Bumped from 1.0.0 to reflect the new features)
