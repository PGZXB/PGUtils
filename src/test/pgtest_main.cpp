#include <iostream>
#include "Tests.h"

// PGTestMain

// FIXME: Cross platform: print colorful text

int main () {
    using pgimpl::test::Tests;
    auto p = Tests::getDefaultInstance()->runAllTests();
    std::cerr << "\n+===================== "
              << "Summary(ok/total): " << p.first << '/' << p.second
              <<  " ======================+\n";
    if (p.first == p.second) std::cerr << "\033[32mAll tests passd\033[0m\n";
    else {
        std::cerr << "\033[31m" << p.second - p.first << " run-failed tests: {\n";
        for (const auto &t : Tests::getDefaultInstance()->getFailedTests()) {
            std::cerr << "  " << t->name << '(' << t->fileLine << "),\n";
        }
        std::cerr << "}\033[0m\n";
    }
}