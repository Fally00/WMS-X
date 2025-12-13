# Warehouse Management System (WMS) 👨‍💻

A console-based warehouse/inventory management system written in C++:
- Supports item add/remove/search/list
- Supports a task queue for batch operations (ADD, REMOVE, SEARCH, LIST)
- Data persistence via CSV (easy to switch to JSON if needed)
- Simple file-based storage
- Clean console interface with ASCII banner & color output
- Configurable Auto-Save and optional queue mode

## 🔧 Features

- Add / remove / search / list items  
- Queue mode: enqueue operations and process later (good for batch processing)  
- Auto-save toggle — automatic persistence after each operation  
- CSV-based storage  
- Simple, extendable codebase: Inventory, Item, Storage, Controller modular architecture  
- ANSI-colored console UI + ASCII banner for nicer user experience

## 🛠️ Getting Started

### Prerequisites

- A C++ compiler (g++ with C++17 support recommended)  
- On Windows: MSYS2 / MinGW or similar environment  
- On Unix-based OS: GCC / clang  

### Build & Run
the existing wms.exe is the main program that would work the command line below is for linking all files after making a change and still would make the program update and show any new details, but for making it running u will just use the .exe program 

```bash
g++ main.cpp WmsControllers.cpp Inventory.cpp Item.cpp Storage.cpp -o wms.exe
that command to link all files and get them into one wms.exe program which the one u will use to run and be able to run the program 

## Version
Current version: v1.0.0
