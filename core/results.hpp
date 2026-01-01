#pragma once
#include <string>

// Primary template for T (non-void)
template<typename T>
struct Result {
    bool ok;
    T value;
    std::string error;

    Result(bool o, T v, std::string e)
        : ok(o), value(std::move(v)), error(std::move(e)) {}

    static Result success(T v) {
        return {true, std::move(v), ""};
    }

    static Result fail(std::string e) {
        return {false, T{}, std::move(e)};
    }
};

// Specialization for void
template<>
struct Result<void> {
    bool ok;
    std::string error;

    Result(bool o, std::string e)
        : ok(o), error(std::move(e)) {}

    static Result success() {
        return {true, ""};
    }

    // ✅ Fixed: renamed from 'failure' to 'fail' to match usage
    static Result fail(std::string e) {
        return {false, std::move(e)};
    }
};