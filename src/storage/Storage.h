#pragma once
#include <string>
#include <SQLiteCpp/SQLiteCpp.h>

class Storage {
private:
    std::string dbFilePath;
    SQLite::Database db;

public:
    explicit Storage(const std::string& filePath);

    void initializeStorage();          // Create tables if they don't exist
    SQLite::Database& getDB();         // Access the underlying database
    std::string getFilePath() const;
};
