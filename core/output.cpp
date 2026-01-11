#include "output.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <windows.h>

bool OutputFormatter::useColor = true;
bool OutputFormatter::logEnabled = false;
std::ofstream OutputFormatter::logFile;

// We use Win32 console attributes instead when a console is available.
static HANDLE gConsole = INVALID_HANDLE_VALUE;
static WORD gDefaultAttrs = 0;
static bool gHasConsole = false;

static WORD colorAttrs(WORD base, WORD fg) {
    // Preserve background bits.
    return static_cast<WORD>((base & 0xFFF0) | fg);
}

static void setColor(WORD fg) {
    if (!gHasConsole || !OutputFormatter::isColorEnabled()) return;
    SetConsoleTextAttribute(gConsole, colorAttrs(gDefaultAttrs, fg));
}

static void resetColor() {
    if (!gHasConsole || !OutputFormatter::isColorEnabled()) return;
    SetConsoleTextAttribute(gConsole, gDefaultAttrs);
}

bool OutputFormatter::supportsColor() {
    gConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (gConsole == INVALID_HANDLE_VALUE || gConsole == nullptr) return false;

    DWORD mode;
    if (!GetConsoleMode(gConsole, &mode)) return false;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(gConsole, &csbi)) return false;

    gDefaultAttrs = csbi.wAttributes;
    gHasConsole = true;
    return true;
}

size_t OutputFormatter::terminalWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

void OutputFormatter::initialize(bool enableColor, bool enableLogging, const std::string& file) {
    useColor = enableColor && supportsColor();
    logEnabled = enableLogging;
    if(logEnabled) logFile.open(file,std::ios::app);
}

bool OutputFormatter::isColorEnabled() {
    return useColor;
}

void OutputFormatter::write(const std::string& t) {
    std::cout<<t;
    if(logEnabled && logFile.is_open()) logFile<<t;
}

void OutputFormatter::printBanner() {
    // VISUAL: ASCII-only banner (no emojis / no box-drawing unicode).
    const char* banner =
        " _    ____  ___ _____                        \n"
        "| |  | |  \\//  |/  ___|                     \n"
        "| |  | | .  . |\\ `--. __________            \n"
        "| |/\\| | |\\/| | `--. \\______\ \\/ /      \n"
        "\\  /\\  / |  | |/\\__/ /        > <        \n"
        " \\/  \\/\\_|  |_/\\____/      /_/\\_\     \n";

    setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    write(std::string("\n") + banner + "\n");
    resetColor();
}

void OutputFormatter::printPrompt(const std::string& prompt) {
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    write(prompt);
    resetColor();
}

void OutputFormatter::printHeader(const std::string& t){
    write("\n");
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    write("== ");
    setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    write(t);
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    write(" ==\n");
    resetColor();
}
void OutputFormatter::printSuccess(const std::string& m){
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    write("[OK] ");
    resetColor();
    write(m+"\n");
}
void OutputFormatter::printError(const std::string& m){
    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    write("[ERR] ");
    resetColor();
    write(m+"\n");
}
void OutputFormatter::printWarning(const std::string& m){
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    write("[WARN] ");
    resetColor();
    write(m+"\n");
}
void OutputFormatter::printInfo(const std::string& m){
    setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    write("[INFO] ");
    resetColor();
    write(m+"\n");
}

std::string OutputFormatter::wrap(const std::string& txt,size_t w){
    std::stringstream ss; size_t c=0;
    for(char ch:txt){
        if(c++>=w){ ss<<"\n"; c=0; }
        ss<<ch;
    }
    return ss.str();
}

void OutputFormatter::printTable(const std::vector<std::string>& h,
const std::vector<std::vector<std::string>>& r,size_t page){
    if (h.empty()) return;

    const size_t cols = h.size();
    const size_t termW = (terminalWidth() > 4) ? (terminalWidth() - 4) : 80;
    const size_t colW = std::max<size_t>(8, termW / cols);

    auto fit = [&](const std::string& s) {
        if (s.size() <= colW) return s;
        if (colW <= 3) return s.substr(0, colW);
        return s.substr(0, colW - 3) + "...";
    };

    auto printDivider = [&]() {
        write("+");
        for (size_t i = 0; i < cols; i++) {
            write(std::string(colW + 2, '-') + "+");
        }
        write("\n");
    };

    auto printRow = [&](const std::vector<std::string>& row, bool isHeader) {
        write("|");
        for (size_t i = 0; i < cols; i++) {
            const std::string cell = (i < row.size()) ? fit(row[i]) : std::string();
            if (isHeader) setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            write(" " + cell + std::string(colW - cell.size(), ' ') + " |");
            if (isHeader) resetColor();
        }
        write("\n");
    };

    for (size_t p = 0; p < r.size(); p += page) {
        printDivider();
        printRow(h, true);
        printDivider();

        for (size_t i = p; i < std::min(p + page, r.size()); i++) {
            printRow(r[i], false);
        }

        printDivider();

        if (p + page < r.size()) {
            write("-- More -- press Enter to continue --");
            std::string tmp;
            std::getline(std::cin, tmp);
        }
    }
}

void OutputFormatter::printLogo(const std::string&){
    printBanner();
    setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    write("WMS-X 1.4.1\n");
    resetColor();
}

void OutputFormatter::printVersion(const std::string& v){
    printLogo();
    write("Version: "+v+"\n");
}

void OutputFormatter::printHelp(const std::vector<std::pair<std::string,std::string>>& cmds){
    printHeader("HELP");
    for(auto& c:cmds)
        write(c.first+" -> "+c.second+"\n");
}
