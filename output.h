#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

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
    static void printHeader(const std::string& title);
    static void printSuccess(const std::string& message);
    static void printError(const std::string& message);
    static void printWarning(const std::string& message);
    static void printInfo(const std::string& message);

    static void printTable(const std::vector<std::string>& headers,
                          const std::vector<std::vector<std::string>>& rows);

    static void printLogo();
    static void printVersion();
    static void printHelp();
};