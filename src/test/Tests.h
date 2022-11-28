#ifndef PGZXB_PGTEST_TESTS_H
#define PGZXB_PGTEST_TESTS_H

#include <map>
#include <string>
#include <vector>
#include <functional>
#include "utils.h"

namespace pgimpl {
namespace test {

using TestFunction = std::function<void(TestCaseContext&)>;

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

    void addTest(const std::string &name, const TestFunction &func, const std::string &fileLine);
    TestFunction getTest(const std::string &name) const;
    std::vector<TestInfo*> getFailedTests() const;
    std::pair<int, int> runAllTests() const;

    static Tests *getDefaultInstance();
private:
    mutable std::map<std::string, TestInfo> tests_;
};

} // namespace test
} // namespace pgimpl
#endif
