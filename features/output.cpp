/**
 * @file output.cpp
 * @brief Implementation of the OutputFormatter class methods.
 *
 * This file contains the implementation of static methods for the OutputFormatter class,
 * providing functionality for formatted console output, including colored messages,
 * tables, logos, and help information.
 */

#include "output.h"
#include <algorithm>

/*==========================================
    OUTPUT FORMATTING UTILITIES
============================================*/

// ─────────────────────────────────────────────
// Print section header
// ─────────────────────────────────────────────
void OutputFormatter::printHeader(const std::string& title) {
    std::cout << "\n" << BLUE << BOLD << "=== " << title << " ===" << RESET << std::endl;
}

// ─────────────────────────────────────────────
// Print success message
// ─────────────────────────────────────────────
void OutputFormatter::printSuccess(const std::string& message) {
    std::cout << GREEN << "SUCCESS " << message << RESET << std::endl;
}

// ─────────────────────────────────────────────
// Print error message
// ─────────────────────────────────────────────
void OutputFormatter::printError(const std::string& message) {
    std::cout << RED << "ERROR [404] " << message << RESET << std::endl;
}

// ─────────────────────────────────────────────
// Print warning message
// ─────────────────────────────────────────────
void OutputFormatter::printWarning(const std::string& message) {
    std::cout << YELLOW << "WARNING " << message << RESET << std::endl;
}

// ─────────────────────────────────────────────
// Print info message
// ─────────────────────────────────────────────
void OutputFormatter::printInfo(const std::string& message) {
    std::cout << CYAN << "ℹ " << message << RESET << std::endl;
}

// ─────────────────────────────────────────────
// Print data table
// ─────────────────────────────────────────────
void OutputFormatter::printTable(const std::vector<std::string>& headers,
                                const std::vector<std::vector<std::string>>& rows) {
    if (headers.empty() && rows.empty()) return;

    // Calculate column widths
    std::vector<size_t> colWidths;
    if (!headers.empty()) {
        for (const auto& header : headers) {
            colWidths.push_back(header.length());
        }
    }

    // Update widths based on row data
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i < colWidths.size()) {
                colWidths[i] = std::max(colWidths[i], row[i].length());
            } else {
                colWidths.push_back(row[i].length());
            }
        }
    }

    // Ensure minimum width
    for (auto& width : colWidths) {
        width = std::max(width, size_t(8));
    }

    // Print top border
    std::cout << BLUE;
    for (size_t i = 0; i < colWidths.size(); ++i) {
        std::cout << "[-" << std::string(colWidths[i] + 2, '-') << "-]";
    }
    std::cout << RESET << std::endl;

    // Print headers
    if (!headers.empty()) {
        std::cout << BLUE << "|" << RESET;
        for (size_t i = 0; i < headers.size(); ++i) {
            std::cout << BOLD << CYAN << " " << headers[i]
                      << std::string(colWidths[i] - headers[i].length(), ' ') << " " << RESET << BLUE << "|" << RESET;
        }
        std::cout << std::endl;

        // Print separator
        std::cout << BLUE;
        for (size_t i = 0; i < colWidths.size(); ++i) {
            std::cout << "[-" << std::string(colWidths[i] + 2, '-') << "-]";
        }
        std::cout << RESET << std::endl;
    }

    // Print rows
    for (const auto& row : rows) {
        std::cout << BLUE << "|" << RESET;
        for (size_t i = 0; i < row.size(); ++i) {
            std::cout << " " << row[i]
                      << std::string(colWidths[i] - row[i].length(), ' ') << " " << BLUE << "|" << RESET;
        }
        std::cout << std::endl;
    }

    // Print bottom border
    std::cout << BLUE;
    for (size_t i = 0; i < colWidths.size(); ++i) {
        std::cout << "[" << std::string(colWidths[i] + 2, '-') << "]";
    }
    std::cout << RESET << std::endl;
}

// ─────────────────────────────────────────────
// Print application logo
// ─────────────────────────────────────────────
void OutputFormatter::printLogo() {
    const std::string logo = R"(
          _______  _______
|\     /|(       )(  ____ \     |\     /|
| )   ( || () () || (    \/     ( \   / )
| | _ | || || || || (_____  _____\ (_) /
| |( )| || |(_)| |(_____  )(_____)) _ (
| || || || |   | |      ) |      / ( ) \
| () () || )   ( |/\____) |     ( /   \ )
(_______)|/     \|\_______)     |/     \|

    )";
    std::cout << MAGENTA << logo << RESET;
    std::cout << WHITE << "      WAREHOUSE MANAGEMENT SYSTEM v1.3" << RESET << std::endl;
    std::cout << MAGENTA << "           Developed by Rayan Hisham & Abdulrahman Shaalan" << RESET << std::endl;
}

// ─────────────────────────────────────────────
// Print version information
// ─────────────────────────────────────────────
void OutputFormatter::printVersion() {
    printLogo();
    std::cout << "\nVersion: " << CYAN << "1.3.0" << RESET << std::endl;
    std::cout << "CLI Mode: " << GREEN << "Enabled" << RESET << std::endl;
}

// ─────────────────────────────────────────────
// Print help information
// ─────────────────────────────────────────────
void OutputFormatter::printHelp() {
    printLogo();
    std::cout << "\n" << BOLD << "USAGE:" << RESET << std::endl;
    std::cout << "  wms <command> [options] [arguments]\n" << std::endl;

    std::cout << BOLD << "COMMANDS:" << RESET << std::endl;
    std::cout << "  add <id> <name> <quantity> <location> Add item instantly" << std::endl;
    std::cout << "  remove <id>                          Remove item instantly" << std::endl;
    std::cout << "  find <id>                             Find item by ID" << std::endl;
    std::cout << "  list                                 List all items" << std::endl;
    std::cout << "  queue <operation> [args...]          Queue operations (add/remove)" << std::endl;
    std::cout << "  process                              Process queued tasks" << std::endl;
    std::cout << GREEN << "  save                                   Save all data" << std::endl;
    std::cout << GREEN << "  help                                   Show this help message" << std::endl;
    std::cout << GREEN << "  version                                Show version information" << std::endl;

    std::cout << "\n" << BOLD << "OPTIONS:" << RESET << std::endl;
    std::cout << "  -h, --help                             Show help message" << std::endl;
    std::cout << "  -v, --version                          Show version information" << std::endl;
    std::cout << "  --autosave                             Enable autosave after operations" << std::endl;

}