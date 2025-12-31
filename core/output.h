#pragma once
#include <string>         // for std::string  it does string handling
#include <vector>        // for std::vector  it makes table rows
#include <iostream>     // for std::cout  it does output to console
#include <iomanip>     // for std::setw it does formatting 

// ANSI color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

class OutputFormatter {
public:
    static void printHeader(const std::string& title);          // Print section header
    static void printSuccess(const std::string& message);      // Print success message
    static void printError(const std::string& message);       // Print error message
    static void printWarning(const std::string& message);    // Print warning message
    static void printInfo(const std::string& message);      // Print info message

    // Print data table
    static void printTable(const std::vector<std::string>& headers,
                          const std::vector<std::vector<std::string>>& rows);

    static void printLogo();        // Print application logo 
    static void printVersion();    // Print application version information
    static void printHelp();      // Print help information 
};