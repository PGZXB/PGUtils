// PGTest: a lightweight test framework
#ifndef PGZXB_PGTEST_H
#define PGZXB_PGTEST_H

#include "../pgfwd.h"
#include "../pghfmt.h"
#include "../pgutil.h"
#include <functional>

#define PGTEST_CASE_EX(name, testsInstance) \
    bool test_##name(pgimpl::test::Context&); \
    bool testAdder_##name = pgimpl::test::addTest(testsInstance, #name, test_##name, __FILE__, __LINE__); \
    bool test_##name(pgimpl::test::Context & pgtestContext)

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
namespace pgimpl{ namespace test{template <typename T> struct AllwaysTrue { operator bool(){return true;}; T t; T t2; };}}
#define PGTEST_STDOUT_EQ_EX(expected, failingMsg) \
    std::string outputBuffer; \
    if (auto _ = pgimpl::test::AllwaysTrue<pgutil::RaiiCleanup>{ \
        {[&outputBuffer](){ std::cerr << expected << "==" << outputBuffer << "\n"; }}, \
        pgtestContext.capureStdoutAndCheck( \
            [&outputBuffer](const std::string & output) -> void { \
                outputBuffer = output; \
            } \
        )})
#define PGTEST_STDOUT_EQ(expected) PGTEST_STDOUT_EQ_EX(expected, "")

namespace pgimpl {
namespace test {

class Context {
    class Impl;
    using Checker = std::function<void(const std::string &)>;
public:
    Context();
    ~Context();
    
    pgutil::RaiiCleanup capureStdoutAndCheck(const Checker &checker);
    pgutil::RaiiCleanup capureStderrAndCheck(const Checker &checker);
private:
    Impl *impl_{nullptr};
};

class Tests;
using TestFunction = std::function<bool(Context&)>;

Tests &getDefaultTestsInstance();
bool addTest(Tests &tests, const std::string &name, const TestFunction &testFunc, const char *file, long long line);

} // namespace test
} // namespace pgimpl
#endif
