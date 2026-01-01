#include "output.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <windows.h>

bool OutputFormatter::useColor = true;
bool OutputFormatter::logEnabled = false;
std::ofstream OutputFormatter::logFile;

static std::string RESET="\033[0m", RED="\033[31m", GREEN="\033[32m",
YELLOW="\033[33m", CYAN="\033[36m", BLUE="\033[34m", BOLD="\033[1m";

bool OutputFormatter::supportsColor() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    return GetConsoleMode(h, &mode);
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
    if(!useColor){ RESET=RED=GREEN=YELLOW=CYAN=BLUE=BOLD=""; }
}

void OutputFormatter::write(const std::string& t) {
    std::cout<<t;
    if(logEnabled && logFile.is_open()) logFile<<t;
}

void OutputFormatter::printHeader(const std::string& t){
    write("\n"+BLUE+BOLD+"== "+t+" ==\n"+RESET);
}
void OutputFormatter::printSuccess(const std::string& m){ write(GREEN+"[OK] "+m+"\n"+RESET);}
void OutputFormatter::printError(const std::string& m){ write(RED+"[ERR] "+m+"\n"+RESET);}
void OutputFormatter::printWarning(const std::string& m){ write(YELLOW+"[WARN] "+m+"\n"+RESET);}
void OutputFormatter::printInfo(const std::string& m){ write(CYAN+"[INFO] "+m+"\n"+RESET);}

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
    size_t termW = terminalWidth()-4;
    size_t cols = h.size();
    size_t colW = termW/cols;

    for(size_t p=0;p<r.size();p+=page){
        printHeader("TABLE");
        for(size_t i=0;i<h.size();i++)
            write(BOLD+wrap(h[i],colW)+RESET+"\t");
        write("\n");

        for(size_t i=p;i<std::min(p+page,r.size());i++){
            for(size_t c=0;c<r[i].size();c++)
                write(wrap(r[i][c],colW)+"\t");
            write("\n");
        }
        system("pause");
    }
}

void OutputFormatter::printLogo(const std::string&){
    write(+"W M S  -  X  1.4.1\n"+RESET);
}

void OutputFormatter::printVersion(const std::string& v){
    printLogo();
    write("Version: "+v+"\n");
}

void OutputFormatter::printHelp(const std::vector<std::pair<std::string,std::string>>& cmds){
    printHeader("HELP");
    for(auto& c:cmds)
        write(BOLD+c.first+RESET+" -> "+c.second+"\n");
}
