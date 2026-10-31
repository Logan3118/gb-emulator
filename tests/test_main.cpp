#include "test.h"

int main() {
    int failedTests = 0;
    for (const auto& test : gbtest::registry()) {
        const int before = gbtest::failures();
        test.fn();
        const bool ok = gbtest::failures() == before;
        std::printf("[%s] %s\n", ok ? " OK " : "FAIL", test.name.c_str());
        if (!ok) {
            ++failedTests;
        }
    }
    std::printf("%zu tests, %d failed\n", gbtest::registry().size(), failedTests);
    return failedTests == 0 ? 0 : 1;
}
