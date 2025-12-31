/**
 * @file Storage.h
 * @brief Header file for the Storage class, providing file storage operations.
 *
 * This class handles basic file operations such as reading, writing, appending,
 * and clearing file contents. It is designed for persistent data storage in the
 * Warehouse Management System.
 */

#pragma once
#include <string>
#include <vector>

/**
 * @class Storage
 * @brief Manages file-based storage operations for data persistence.
 *
 * The Storage class encapsulates file operations to ensure safe and consistent
 * access to data files. It supports reading, writing, appending, and clearing
 * file contents, with error handling for file access issues.
 */
class Storage {
private:
    std::string dataFilePath; /**< The file path for the storage file. */

public:
    /**
     * @brief Constructs a Storage object with the specified file path.
     * @param filePath The path to the file used for storage.
     */
    Storage(const std::string& filePath);

    /**
     * @brief Initializes the storage file if it does not exist.
     * @return True if initialization succeeds, false otherwise.
     */
    bool initializeStorage();

    /**
     * @brief Checks if the storage file exists.
     * @return True if the file exists, false otherwise.
     */
    bool fileExists() const;

    /**
     * @brief Retrieves the file path of the storage file.
     * @return The file path as a string.
     */
    std::string getFilePath() const;

    /**
     * @brief Reads the entire contents of the file into a string.
     * @return The file contents as a string, or empty string if file is missing or unreadable.
     */
    std::string readAll() const;

    /**
     * @brief Overwrites the entire file with the provided content.
     * @param content The content to write to the file.
     * @return True if the write operation succeeds, false otherwise.
     */
    bool writeAll(const std::string &content) const;

    /**
     * @brief Appends the provided content to the end of the file.
     * @param content The content to append to the file.
     * @return True if the append operation succeeds, false otherwise.
     */
    bool append(const std::string &content) const;

    /**
     * @brief Reads the file line-by-line into a vector of strings.
     * @return A vector containing each line of the file.
     */
    std::vector<std::string> readLines() const;

    /**
     * @brief Clears all content from the file.
     * @return True if the clear operation succeeds, false otherwise.
     */
    bool clear() const;
};
