#pragma once
#include "core/commands/results.hpp"
#include <string>

inline Result<int> safetyparse(const std::string& s) {
    try {
        size_t i;
        int v = std::stoi(s, &i);
        if (i != s.size()) return Result<int>::fail("Invalid number: " + s);
        return Result<int>::success(v);
    } catch (...) {
        return Result<int>::fail("Invalid number: " + s);
    }
}
