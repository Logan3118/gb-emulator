#pragma once

// Tiny self-contained test harness, so the project has no external test dependency.

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace gbtest {

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int& failures() {
    static int count = 0;
    return count;
}

struct Registrar {
    Registrar(const char* name, std::function<void()> fn) { registry().push_back({name, std::move(fn)}); }
};

} // namespace gbtest

#define GB_CONCAT_INNER(a, b) a##b
#define GB_CONCAT(a, b) GB_CONCAT_INNER(a, b)

#define TEST(name)                                                                     \
    static void name();                                                                \
    static gbtest::Registrar GB_CONCAT(registrar_, name)(#name, name);                 \
    static void name()

#define CHECK(expr)                                                                    \
    do {                                                                               \
        if (!(expr)) {                                                                 \
            std::fprintf(stderr, "  %s:%d: CHECK failed: %s\n", __FILE__, __LINE__, #expr); \
            ++gbtest::failures();                                                      \
        }                                                                              \
    } while (0)
