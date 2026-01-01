#include "core/Storage.h"
#include <openssl/sha.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;
namespace fs = std::filesystem;

// ─────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────
Storage::Storage(const string& filePath) : dataFilePath(filePath) {}

// ─────────────────────────────────────────────
// Path validation
// ─────────────────────────────────────────────
bool Storage::validatePath(string& err) const {
    if (dataFilePath.empty()) {
        err = "File path is empty";
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────
// Backup creator
// ─────────────────────────────────────────────
bool Storage::createBackup(string& err) const {
    try {
        if (!fs::exists(dataFilePath)) return true;
        fs::copy_file(dataFilePath, dataFilePath + ".bak", fs::copy_options::overwrite_existing);
        return true;
    } catch (...) {
        err = "Failed to create backup file";
        return false;
    }
}

// ─────────────────────────────────────────────
// SHA256 checksum
// ─────────────────────────────────────────────
string Storage::computeChecksum(const string& data) const {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), hash);

    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

// ─────────────────────────────────────────────
// Initialize storage
// ─────────────────────────────────────────────
optional<StorageError> Storage::initializeStorage() const {
    string err;
    if (!validatePath(err))
        return StorageError(err);

    if (!fs::exists(dataFilePath)) {
        ofstream out(dataFilePath);
        if (!out) return StorageError("Failed to create storage file");
    }
    return nullopt;
}

// ─────────────────────────────────────────────
// Atomic write
// ─────────────────────────────────────────────
optional<StorageError> Storage::atomicWrite(const string& content) const {
    string err;
    if (!validatePath(err))
        return StorageError(err);

    if (!createBackup(err))
        return StorageError(err);

    string tempFile = dataFilePath + ".tmp";

    ofstream out(tempFile);
    if (!out) return StorageError("Failed to open temp file");

    out << content;
    out.close();

    try {
        fs::rename(tempFile, dataFilePath);
    } catch (...) {
        return StorageError("Atomic rename failed");
    }

    return nullopt;
}

// ─────────────────────────────────────────────
// Append
// ─────────────────────────────────────────────
optional<StorageError> Storage::append(const string& content, bool newline) const {
    ofstream out(dataFilePath, ios::app);
    if (!out) return StorageError("Append failed");
    out << content;
    if (newline) out << "\n";
    return nullopt;
}

// ─────────────────────────────────────────────
// Read all
// ─────────────────────────────────────────────
optional<string> Storage::readAll(string& err) const {
    ifstream in(dataFilePath);
    if (!in) {
        err = "Failed to open file for reading";
        return nullopt;
    }
    stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// ─────────────────────────────────────────────
// Read lines
// ─────────────────────────────────────────────
vector<string> Storage::readLines(string& err) const {
    vector<string> lines;
    ifstream in(dataFilePath);
    if (!in) {
        err = "Failed to open file for reading";
        return {};
    }
    string line;
    while (getline(in, line)) lines.push_back(line);
    return lines;
}

// ─────────────────────────────────────────────
// Clear
// ─────────────────────────────────────────────
optional<StorageError> Storage::clear() const {
    ofstream out(dataFilePath, ios::trunc);
    if (!out) return StorageError("Failed to clear file");
    return nullopt;
}

// ─────────────────────────────────────────────
// Get path
// ─────────────────────────────────────────────
string Storage::getFilePath() const {
    return dataFilePath;
}
