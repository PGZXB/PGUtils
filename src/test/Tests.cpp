#include "Tests.h"

#include <iomanip>
#include <iostream>
#include "../pgfwd.h"

namespace pgimpl {
namespace test {

class TestRunFailed : std::exception { };

// class Tests
void Tests::addTest(const std::string &name, const TestFunction &func, const std::string &fileLine) {
    auto ret = tests_.insert(std::make_pair(name, TestInfo{nullptr, func, fileLine}));
    PGZXB_DEBUG_ASSERT(ret.second);
    auto iter = ret.first;
    iter->second.name = iter->first.c_str();
}

TestFunction Tests::getTest(const std::string &name) const {
    auto iter = tests_.find(name);
    if (iter == tests_.end()) return nullptr;
    return iter->second.testFunc;
}

std::vector<Tests::TestInfo*> Tests::getFailedTests() const {
    std::vector<TestInfo*> res;
    for (auto &e: tests_) {
        if (!e.second.lastRunOk)
            res.push_back(&e.second);
    }
    return res;
}

std::pair<int, int> Tests::runAllTests() const {
    int okCnt = 0, cnt = 1, total = tests_.size();
    auto strTotal = std::to_string(total);
    int width = (int)strTotal.size();

    for (auto &e: tests_) {
        const auto &name = e.first;
        const auto &func = e.second.testFunc;
        bool passed{false};
        try {
            passed = func();
        } catch (pgimpl::test::TestRunFailed &) {
            passed = false;
        }
        if (passed) ++okCnt;
        e.second.lastRunOk = passed;
        // FIXME: Cross platform: print colorful text
        std::cerr << std::setw(width) << cnt++ << '/' << strTotal << (passed ? "\033[32m[OK]\033[0m" : "\033[31m[NO]\033[0m") << ' ' << name << '\n';
    }

    return std::make_pair(okCnt, total);
}

// static functions of class Tests
Tests *Tests::getDefaultInstance() {
    static Tests t;
    return &t;
}

} // namespace test
} // namespace pgimpl
