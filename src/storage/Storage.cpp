#include "storage/Storage.h"

// ─────────────────────────────────────────────
// Constructor — opens (or creates) the database
// ─────────────────────────────────────────────
Storage::Storage(const std::string& filePath)
    : dbFilePath(filePath),
      db(filePath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {}

// ─────────────────────────────────────────────
// Create tables if they don't exist
// ─────────────────────────────────────────────
void Storage::initializeStorage() {
    db.exec(
        "CREATE TABLE IF NOT EXISTS items ("
        "  id          INTEGER PRIMARY KEY,"
        "  name        TEXT    NOT NULL,"
        "  quantity    INTEGER NOT NULL DEFAULT 0,"
        "  location    TEXT    NOT NULL,"
        "  price       REAL    NOT NULL DEFAULT 0.0,"
        "  currency    TEXT    NOT NULL DEFAULT 'EGP',"
        "  unit        TEXT    NOT NULL DEFAULT 'pcs',"
        "  category    TEXT    NOT NULL DEFAULT 'general',"
        "  created_at  INTEGER NOT NULL,"
        "  modified_at INTEGER NOT NULL"
        ");"
    );

    db.exec(
        "CREATE TABLE IF NOT EXISTS receipts ("
        "  receipt_number TEXT PRIMARY KEY,"
        "  timestamp      TEXT NOT NULL,"
        "  customer_name  TEXT DEFAULT '',"
        "  customer_phone TEXT DEFAULT '',"
        "  customer_email TEXT DEFAULT '',"
        "  subtotal       REAL NOT NULL,"
        "  tax            REAL NOT NULL,"
        "  total          REAL NOT NULL"
        ");"
    );

    db.exec(
        "CREATE TABLE IF NOT EXISTS receipt_items ("
        "  id             INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  receipt_number TEXT NOT NULL REFERENCES receipts(receipt_number),"
        "  item_id        INTEGER NOT NULL,"
        "  name           TEXT    NOT NULL,"
        "  location       TEXT    NOT NULL,"
        "  quantity       INTEGER NOT NULL,"
        "  unit_price     REAL    NOT NULL,"
        "  line_total     REAL    NOT NULL"
        ");"
    );

    // ─────────────────────────────────────────────
    // Customer table
    // ─────────────────────────────────────────────
    db.exec(
        "CREATE TABLE IF NOT EXISTS customers ("
        "  id         INTEGER PRIMARY KEY,"
        "  name       TEXT    NOT NULL,"
        "  phone      TEXT    DEFAULT '',"
        "  address    TEXT    DEFAULT '',"
        "  email      TEXT    DEFAULT '',"
        "  created_at INTEGER NOT NULL"
        ");"
    );

    // Add customer_id column to receipts (safe for upgrade — column may already exist)
    try {
        db.exec("ALTER TABLE receipts ADD COLUMN customer_id INTEGER DEFAULT NULL");
    } catch (...) {
        // Column already exists — safe to ignore
    }
}

// ─────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────
SQLite::Database& Storage::getDB() {
    return db;
}

std::string Storage::getFilePath() const {
    return dbFilePath;
}
