// PGTest: a lightweight test framework
#ifndef PGZXB_PGTEST_H
#define PGZXB_PGTEST_H

#include <iostream>
#include <functional>
#include "../pgfwd.h"
#include "../pghfmt.h"
#include "../pgutil.h"
#include "utils.h"

#define PGTEST_CASE_EX(name, testsInstance) \
    void test_##name(pgimpl::test::TestCaseContext&); \
    static bool testAdder_##name = pgimpl::test::addTest(testsInstance, #name, test_##name, __FILE__, __LINE__); \
    void test_##name(pgimpl::test::TestCaseContext &__pgtest_testCaseContext)

#define PGTEST_CASE(name) PGTEST_CASE_EX(name, pgimpl::test::getDefaultTestsInstance())

#define PGTEST_EXPECT_EX_IMPL(cond, failingMsg, file, line) \
    if (!(cond)) { \
        std::cerr << "Expect "#cond" but not(" << file << ":" << line << '\n'; \
        if (failingMsg) std::cerr << "Hint: " << failingMsg << '\n'; \
        __pgtest_testCaseContext.incFailCount(); \
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
        pgimpl::test::CapturedFdMsgEqHelper{__pgtest_testCaseContext, pgutil::FdCapture::Fd::kStdoutFd, expected, __FILE__, __LINE__, failingMsg} \
    })
#define PGTEST_STDOUT_EQ(expected) PGTEST_STDOUT_EQ_EX(expected, "")
#define PGTEST_STDERR_EQ_EX(expected, failingMsg) \
    if (auto _ = pgutil::AllwaysTrue<pgimpl::test::CapturedFdMsgEqHelper>{ \
        pgimpl::test::CapturedFdMsgEqHelper{__pgtest_testCaseContext, pgutil::FdCapture::Fd::kStderrFd, expected, __FILE__, __LINE__, failingMsg} \
    })
#define PGTEST_STDERR_EQ(expected) PGTEST_STDERR_EQ_EX(expected, "")

namespace pgimpl {
namespace test {

// for PGTEST_STDOUT_EQ
class CapturedFdMsgEqHelper {
public:
    CapturedFdMsgEqHelper(
        TestCaseContext &testCaseCtx,
        pgutil::FdCapture::Fd capturedFd,
        std::string expectedOutput,
        const char * filename, std::size_t lineno,
        std::string failingMsg);

    ~CapturedFdMsgEqHelper();
private:
    TestCaseContext &testCaseCtx_;
    pgutil::FdCapture capture_;
    const char * filename_{nullptr};
    std::size_t lineno_{0};
    std::string failingMsg_;
    std::string expectedOutput_;
};

class Tests;
using TestFunction = std::function<void(TestCaseContext&)>;

Tests &getDefaultTestsInstance();
bool addTest(Tests &tests, const std::string &name, const TestFunction &testFunc, const char *file, long long line);

} // namespace test
} // namespace pgimpl
#endif
