/**
 * @file Storage.cpp
 * @brief Implementation of the Storage class for file operations.
 *
 * This file contains the implementation of methods for the Storage class,
 * providing functionality for initializing, reading, writing, and managing
 * file-based storage in the Warehouse Management System.
 */

#include "core/Storage.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <vector>
using namespace std;

/**
 * @brief Constructs a Storage object with the specified file path.
 * @param filePath The path to the file used for storage.
 */
Storage::Storage(const std::string& filePath) : dataFilePath(filePath) {}


// ─────────────────────────────────────────────
// Create storage file if not found
// ─────────────────────────────────────────────
bool Storage::initializeStorage() {
    if (!fileExists()) {
        ofstream outFile(dataFilePath);
        if (!outFile) return false;
    }
    return true;
}


// ─────────────────────────────────────────────
// Check file existence
// ─────────────────────────────────────────────
bool Storage::fileExists() const {
    return filesystem::exists(dataFilePath);
}


// ─────────────────────────────────────────────
// Get full path to file
// ─────────────────────────────────────────────
string Storage::getFilePath() const {
    return dataFilePath;
}


// ─────────────────────────────────────────────
// Read entire file to a single string
// ─────────────────────────────────────────────
string Storage::readAll() const {
    ifstream in(dataFilePath);
    if (!in) return {};
    stringstream ss; ss << in.rdbuf();
    return ss.str();
}


// ─────────────────────────────────────────────
// Overwrite entire file content
// ─────────────────────────────────────────────
bool Storage::writeAll(const string &content) const {
    ofstream out(dataFilePath, ios::trunc);
    if (!out) return false;
    out << content;
    return out.good();
}


// ─────────────────────────────────────────────
// Append data at end of file
// ─────────────────────────────────────────────
bool Storage::append(const string &content) const {
    ofstream out(dataFilePath, ios::app);
    if (!out) return false;
    out << content << "\n";
    return out.good();
}


// ─────────────────────────────────────────────
// Load file line-by-line into vector<string>
// ─────────────────────────────────────────────
vector<string> Storage::readLines() const {
    vector<string> lines;
    ifstream in(dataFilePath);
    string line;

    while (getline(in, line))
        lines.push_back(line);

    return lines;
}


// ─────────────────────────────────────────────
// Erase all content (clean wipe)
// ─────────────────────────────────────────────
bool Storage::clear() const {
    ofstream out(dataFilePath, ios::trunc);
    return out.good();
}
