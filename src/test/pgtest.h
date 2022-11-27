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
    if (auto _ = pgutil::AllwaysTrue<pgimpl::test::CapturedStdoutEqHelper>{ \
        pgimpl::test::CapturedStdoutEqHelper{expected, __FILE__, __LINE__, failingMsg} \
    })
#define PGTEST_STDOUT_EQ(expected) PGTEST_STDOUT_EQ_EX(expected, "")

namespace pgimpl {
namespace test {

class TestRunFailed : std::exception { };

// for PGTEST_STDOUT_EQ
template <typename FdCapture, const char * NAME>
class CapturedFdMsgEqHelper {
public:
    CapturedFdMsgEqHelper(std::string expectedOutput, const char * filename, std::size_t lineno, std::string failingMsg) :
      filename_(filename),
      lineno_(lineno),
      failingMsg_(failingMsg),
      expectedOutput_(std::move(expectedOutput)) {
        auto ok = capture_.begin();
        PGZXB_DEBUG_ASSERT(ok);
    }

    ~CapturedFdMsgEqHelper() throw(TestRunFailed) {
        std::string capturedOutputMsg;
        auto ok = capture_.end(capturedOutputMsg);
        PGZXB_DEBUG_ASSERT(ok);
        if (capturedOutputMsg != expectedOutput_) {
            std::string msg = pghfmt::format(
                "Expect {0} \"{1}\" but not({2}:{3}", NAME, expectedOutput_, filename_, lineno_);
            std::string hint = pghfmt::format(
                "Hint: Captured {0} is \"{1}\"{2}", NAME, capturedOutputMsg, !failingMsg_.empty() ? (", " + failingMsg_) : failingMsg_);
            std::cerr << msg << '\n';
            std::cerr << hint << '\n';
            throw TestRunFailed{};
        }
    }
private:
    FdCapture capture_;
    const char * filename_{nullptr};
    std::size_t lineno_{0};
    std::string failingMsg_;
    std::string expectedOutput_;
};

constexpr char kStdoutName[] = "stdout";
constexpr char kStderrName[] = "stderr";
using CapturedStdoutEqHelper = CapturedFdMsgEqHelper<pgutil::StdoutCapture, kStdoutName>;
using CapturedStderrEqHelper = CapturedFdMsgEqHelper<pgutil::StderrCapture, kStderrName>;

class Tests;
using TestFunction = std::function<bool()>;

Tests &getDefaultTestsInstance();
bool addTest(Tests &tests, const std::string &name, const TestFunction &testFunc, const char *file, long long line);

} // namespace test
} // namespace pgimpl
#endif
