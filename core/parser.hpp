#pragma once
#include <vector>
#include <string>

// Simple tokenizer for command line input
inline std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> args;
    std::string cur;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '"') inQuotes = !inQuotes;
        else if (c == ' ' && !inQuotes) {
            if (!cur.empty()) { args.push_back(cur); cur.clear(); }
        } else cur += c;
    }
    if (!cur.empty()) args.push_back(cur);
    return args;
}