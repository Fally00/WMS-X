#pragma once
#include <string>
#include <vector>
#include <fstream>

class OutputFormatter {
public:
    static void initialize(bool enableColor = true, bool enableLogging = false, const std::string& logFile = "wms.log");

    static void printHeader(const std::string& title);
    static void printSuccess(const std::string& msg);
    static void printError(const std::string& msg);
    static void printWarning(const std::string& msg);
    static void printInfo(const std::string& msg);

    static void printTable(const std::vector<std::string>& headers,
                           const std::vector<std::vector<std::string>>& rows,
                           size_t pageSize = 15);

    static void printLogo(const std::string& path = "");
    static void printVersion(const std::string& version);
    static void printHelp(const std::vector<std::pair<std::string,std::string>>& commands);

private:
    static bool supportsColor();
    static size_t terminalWidth();
    static std::string wrap(const std::string& text, size_t width);

    static void write(const std::string& txt);

    static bool useColor;
    static bool logEnabled;
    static std::ofstream logFile;
};
