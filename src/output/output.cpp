#include "output/output.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

// ─────────────────────────────────────────────
// Platform-specific color support
// ─────────────────────────────────────────────
#ifdef _WIN32
  #include <windows.h>

  static HANDLE gConsole      = INVALID_HANDLE_VALUE;
  static WORD   gDefaultAttrs = 0;
  static bool   gHasConsole   = false;

  enum class Color { Reset, Green, Red, Yellow, Cyan, White };

  static WORD colorAttrs(WORD base, WORD fg) {
      return static_cast<WORD>((base & 0xFFF0) | fg);
  }

  static void setColor(Color c) {
      if (!gHasConsole || !OutputFormatter::isColorEnabled()) return;
      WORD fg = gDefaultAttrs & 0x0F;
      switch (c) {
          case Color::Green:  fg = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
          case Color::Red:    fg = FOREGROUND_RED   | FOREGROUND_INTENSITY; break;
          case Color::Yellow: fg = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
          case Color::Cyan:   fg = FOREGROUND_BLUE  | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
          case Color::White:  fg = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
          default: fg = gDefaultAttrs & 0x0F; break;
      }
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
      gHasConsole   = true;
      return true;
  }

  size_t OutputFormatter::terminalWidth() {
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
          return static_cast<size_t>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
      return 80;
  }

#else
  // ── POSIX / ANSI fallback ────────────────────
  #include <unistd.h>
  #include <sys/ioctl.h>

  enum class Color { Reset, Green, Red, Yellow, Cyan, White };

  static const char* ansiCode(Color c) {
      switch (c) {
          case Color::Green:  return "\033[32;1m";
          case Color::Red:    return "\033[31;1m";
          case Color::Yellow: return "\033[33;1m";
          case Color::Cyan:   return "\033[36;1m";
          case Color::White:  return "\033[37;1m";
          default:            return "\033[0m";
      }
  }

  static void setColor(Color c) {
      if (!OutputFormatter::isColorEnabled()) return;
      std::cout << ansiCode(c);
  }

  static void resetColor() {
      if (!OutputFormatter::isColorEnabled()) return;
      std::cout << "\033[0m";
  }

  bool OutputFormatter::supportsColor() {
      return isatty(STDOUT_FILENO) == 1;
  }

  size_t OutputFormatter::terminalWidth() {
      struct winsize ws{};
      if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
          return ws.ws_col;
      return 80;
  }

#endif  // _WIN32

// ─────────────────────────────────────────────
// Static member definitions
// ─────────────────────────────────────────────
bool          OutputFormatter::useColor   = true;
bool          OutputFormatter::logEnabled = false;
std::ofstream OutputFormatter::logFile;

// ─────────────────────────────────────────────
// Core I/O
// ─────────────────────────────────────────────
void OutputFormatter::write(const std::string& t) {
    std::cout << t;
    if (logEnabled && logFile.is_open()) logFile << t;
}

void OutputFormatter::initialize(bool enableColor, bool enableLogging, const std::string& file) {
    useColor   = enableColor && supportsColor();
    logEnabled = enableLogging;
    if (logEnabled) logFile.open(file, std::ios::app);
}

bool OutputFormatter::isColorEnabled() { return useColor; }

// ─────────────────────────────────────────────
// Banner / Logo
// ─────────────────────────────────────────────
void OutputFormatter::printBanner() {
    const char* banner =
        " _    _  __  __  ___      _  _ \n"
        "( \\/\\/ )(  \\/  )/ __) ___( \\/ )\n"
        " )    (  )    ( \\__ \\(___))  ( \n"
        "(__/\\__)(_/\\/\\_)(___/    (_/\\_)\n";

    setColor(Color::Cyan);
    write(std::string("\n") + banner + "\n");
    resetColor();
}

void OutputFormatter::printLogo(const std::string&) {
    printBanner();
    setColor(Color::Cyan);
    write("WMS-X 1.5.0 Stable DB Edition\n");
    resetColor();
}

// ─────────────────────────────────────────────
// Formatted output helpers
// ─────────────────────────────────────────────
void OutputFormatter::printPrompt(const std::string& prompt) {
    setColor(Color::Green);
    write(prompt);
    resetColor();
}

void OutputFormatter::printHeader(const std::string& t) {
    write("\n");
    setColor(Color::Cyan);
    write("== " + t + " ==\n");
    resetColor();
}

void OutputFormatter::printSuccess(const std::string& m) {
    setColor(Color::Green);
    write("[OK] ");
    resetColor();
    write(m + "\n");
}

void OutputFormatter::printError(const std::string& m) {
    setColor(Color::Red);
    write("[ERR] ");
    resetColor();
    write(m + "\n");
}

void OutputFormatter::printWarning(const std::string& m) {
    setColor(Color::Yellow);
    write("[WARN] ");
    resetColor();
    write(m + "\n");
}

void OutputFormatter::printInfo(const std::string& m) {
    setColor(Color::Cyan);
    write("[INFO] ");
    resetColor();
    write(m + "\n");
}

void OutputFormatter::printVersion(const std::string& v) {
    printLogo();
    write("Version: " + v + "\n");
}

void OutputFormatter::printHelp(const std::vector<std::pair<std::string, std::string>>& cmds) {
    printHeader("HELP");
    for (const auto& c : cmds)
        write("  " + c.first + " -> " + c.second + "\n");
}

// ─────────────────────────────────────────────
// Table printer
// ─────────────────────────────────────────────
std::string OutputFormatter::wrap(const std::string& txt, size_t w) {
    std::stringstream ss;
    size_t c = 0;
    for (char ch : txt) {
        if (c++ >= w) { ss << "\n"; c = 0; }
        ss << ch;
    }
    return ss.str();
}

void OutputFormatter::printTable(const std::vector<std::string>& h,
                                  const std::vector<std::vector<std::string>>& r,
                                  size_t page) {
    if (h.empty()) return;

    const size_t cols  = h.size();
    const size_t termW = (terminalWidth() > 4) ? (terminalWidth() - 4) : 80;
    const size_t colW  = std::max<size_t>(8, termW / cols);

    auto fit = [&](const std::string& s) -> std::string {
        if (s.size() <= colW) return s;
        if (colW <= 3) return s.substr(0, colW);
        return s.substr(0, colW - 3) + "...";
    };

    auto printDivider = [&]() {
        write("+");
        for (size_t i = 0; i < cols; i++)
            write(std::string(colW + 2, '-') + "+");
        write("\n");
    };

    auto printRow = [&](const std::vector<std::string>& row, bool isHeader) {
        write("|");
        for (size_t i = 0; i < cols; i++) {
            const std::string cell = (i < row.size()) ? fit(row[i]) : std::string();
            if (isHeader) setColor(Color::Cyan);
            write(" " + cell + std::string(colW - cell.size(), ' ') + " |");
            if (isHeader) resetColor();
        }
        write("\n");
    };

    for (size_t p = 0; p < r.size(); p += page) {
        printDivider();
        printRow(h, true);
        printDivider();

        for (size_t i = p; i < std::min(p + page, r.size()); i++)
            printRow(r[i], false);

        printDivider();

        if (p + page < r.size()) {
            write("-- More -- press Enter to continue --");
            std::string tmp;
            std::getline(std::cin, tmp);
        }
    }
}
