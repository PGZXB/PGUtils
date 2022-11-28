// PGTest: a lightweight test framework
#ifndef PGZXB_PGTEST_H
#define PGZXB_PGTEST_H

#include "../pgfwd.h"
#include "../pghfmt.h"
#include "../pgutil.h"
#include <iostream>
#include <functional>

#define PGTEST_CASE_EX(name, testsInstance) \
    bool test_##name(); \
    bool testAdder_##name = pgimpl::test::addTest(testsInstance, #name, test_##name, __FILE__, __LINE__); \
    bool test_##name()

#define PGTEST_CASE(name) PGTEST_CASE_EX(name, pgimpl::test::getDefaultTestsInstance())

#define PGTEST_EXPECT_EX_IMPL(cond, failingMsg, file, line) \
    if (!(cond)) { \
        std::cerr << "Expect "#cond" but not(" << file << ":" << line << '\n'; \
        if (failingMsg) std::cerr << "Hint: " << failingMsg << '\n'; \
        return false; \
    } PGZXB_PASS

#define PGTEST_EXPECT_EX(cond, failingMsg) PGTEST_EXPECT_EX_IMPL(cond, failingMsg, __FILE__, __LINE__)
#define PGTEST_EXPECT(cond) PGTEST_EXPECT_EX(cond, (const char *)nullptr)
#define PGTEST_EQ_EX(lVal, rVal, failingMsg) do { \
        auto&& l = (lVal); \
        auto&& r = (rVal); \
        std::string hint = pghfmt::format("{0} != {1} {2}", l, r, failingMsg ? failingMsg : ""); \
        PGTEST_EXPECT_EX(l == r, hint.c_str()); \
    } while(0);
#define PGTEST_EQ(lVal, rVal) PGTEST_EQ_EX(lVal, rVal, "")
#define PGTEST_STDOUT_EQ_EX(expected, failingMsg) \
    if (auto _ = pgutil::AllwaysTrue<pgimpl::test::CapturedFdMsgEqHelper>{ \
        pgimpl::test::CapturedFdMsgEqHelper{pgutil::FdCapture::Fd::kStdoutFd, expected, __FILE__, __LINE__, failingMsg} \
    })
#define PGTEST_STDOUT_EQ(expected) PGTEST_STDOUT_EQ_EX(expected, "")

namespace pgimpl {
namespace test {

class TestRunFailed : std::exception { };

// for PGTEST_STDOUT_EQ
class CapturedFdMsgEqHelper {
public:
    CapturedFdMsgEqHelper(
        pgutil::FdCapture::Fd capturedFd,
        std::string expectedOutput,
        const char * filename, std::size_t lineno,
        std::string failingMsg);

    ~CapturedFdMsgEqHelper() throw(TestRunFailed);
private:
    pgutil::FdCapture capture_;
    const char * filename_{nullptr};
    std::size_t lineno_{0};
    std::string failingMsg_;
    std::string expectedOutput_;
};

class Tests;
using TestFunction = std::function<bool()>;

Tests &getDefaultTestsInstance();
bool addTest(Tests &tests, const std::string &name, const TestFunction &testFunc, const char *file, long long line);

} // namespace test
} // namespace pgimpl
#endif
