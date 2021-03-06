#ifndef PGZXB_PGTEST_TESTS_H
#define PGZXB_PGTEST_TESTS_H

#include "pg/pgfwd.h"
#include <utility>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <iostream>
#include <iomanip>

namespace pg {
namespace test {

using TestFunction = std::function<bool()>;

namespace detail {
inline bool empty_test() {
    return false;
}
}

class Tests {
public:
    struct TestInfo {
        const char * name{nullptr};
        TestFunction testFunc{nullptr};
        std::string fileLine{};
        bool lastRunOk{false};

        TestInfo() = default;
        TestInfo(const char * name, const TestFunction &func, const std::string &fileLine)
            : name(name), testFunc(func), fileLine(fileLine) { }
    };

    void addTest(const std::string &name, const TestFunction &func, const std::string &fileLine) {
        auto ret = tests_.insert(std::make_pair(name, TestInfo{nullptr, func, fileLine}));
        PGZXB_DEBUG_ASSERT_EX(ret.second, "Add test repeatly, which is not allowed");
        auto iter = ret.first;
        iter->second.name = iter->first.c_str();
    }

    TestFunction getTest(const std::string &name) const {
        auto iter = tests_.find(name);
        if (iter == tests_.end()) return detail::empty_test;
        return iter->second.testFunc;
    }

    std::vector<TestInfo*> getFailedTests() const {
        std::vector<TestInfo*> res;
        for (auto &e: tests_) {
            if (!e.second.lastRunOk)
                res.push_back(&e.second);
        }
        return res;
    }

    std::pair<int, int> runAllTests() const {
        int okCnt = 0, cnt = 1, total = tests_.size();
        auto strTotal = std::to_string(total);
        int width = (int)strTotal.size();

        for (auto &e: tests_) {
            const auto &name = e.first;
            const auto &func = e.second.testFunc;
            bool ok = func();
            if (ok) {
                ++okCnt;
                e.second.lastRunOk = true;
            } else
                e.second.lastRunOk = false;
            // FIXME: Cross platform: print colorful text
            std::cerr << std::setw(width) << cnt++ << '/' << strTotal << (ok ? "\033[32m[OK]\033[0m" : "\033[31m[NO]\033[0m") << ' ' << name << '\n';
        }

        return std::make_pair(okCnt, total);
    }

    static Tests *getDefaultInstance() {
        static Tests t;
        return &t;
    }
private:
    mutable std::unordered_map<std::string, TestInfo> tests_;
};

} // namespace test
} // namespace pg
#endif
