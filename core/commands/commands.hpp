#pragma once
#include <string>

template<typename T>
struct Result {
    bool ok;
    T value;
    std::string error;

    static Result<T> success(T v = T{}) { return {true, v, ""}; }
    static Result<T> fail(const std::string& e) { return {false, T{}, e}; }
};
