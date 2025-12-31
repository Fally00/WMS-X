#pragma once
#include <string>

template<typename T>

// Simple Result struct for command execution outcomes
struct Result {
    bool ok;
    T value;
    std::string error;
    Result() = default;
    Result(bool o, T v, const std::string& e) : ok(o), value(v), error(e) {}
    // For void, default construct T
    static Result<T> success(T v = T{}) { return {true, v, ""}; }            
    static Result<T> fail(const std::string& e) { return {false, T{}, e}; }       
};
