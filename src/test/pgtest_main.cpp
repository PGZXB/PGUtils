#include <iostream>
#include <unordered_map>
#include "Tests.h"
#include "../pgutil.h"

// PGTestMain

// FIXME: Cross platform: print colorful text

#define DEFINE_SETFUNC_FOR_STRING(val)            \
    {                                             \
        PCC::PARAM, [&val](const char *arg) {     \
        if (!arg) {                               \
            std::cerr << "Failed to parse cmd\n"; \
            return false;                         \
        }                                         \
        val = arg;                                \
        return true;                              \
        }                                         \
    }

int main (int argc, char **argv) {
    using pgimpl::test::Tests;
    using PCC = pgutil::ParseCmdConfig;

    std::string runTestsKey; // To specify which tests to run

    std::unordered_map<char, PCC> cmdArgsConfig = {
        {'k', DEFINE_SETFUNC_FOR_STRING(runTestsKey)},
    };
    pgutil::parseCmdSimply(argc, argv, cmdArgsConfig);

    auto filter = [&runTestsKey](const Tests::TestInfo &info) {
        if (runTestsKey.empty()) return true;
        return info.name && strstr(info.name, runTestsKey.c_str());
    };

    auto p = Tests::getDefaultInstance()->runAllTests(filter);
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