#pragma once
#include <string>
#include <vector>
#include <optional>

// Error representation for storage operations
class StorageError {
public:
    std::string message;
    explicit StorageError(const std::string& msg) : message(msg) {}
};

class Storage {
private:
    std::string dataFilePath;

    bool validatePath(std::string& err) const;         // Validate file path
    bool createBackup(std::string& err) const;        // Create backup of data file

public:
    
    explicit Storage(const std::string& filePath);

    std::optional<StorageError> initializeStorage() const;       // Create file if not exists
    std::optional<StorageError> atomicWrite(const std::string& content) const;    // Atomic write to prevent corruption
    std::optional<StorageError> append(const std::string& content, bool newline = true) const;  // Append data

    std::optional<std::string> readAll(std::string& err) const;   // Read entire file content
    std::vector<std::string> readLines(std::string& err) const;   // Read file line by line

    std::optional<StorageError> clear() const;    // Clear file content
    std::string getFilePath() const;
};
