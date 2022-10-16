// PGTest: a lightweight test framework
#ifndef PGZXB_PGTEST_H

#include "pg/pgfwd.h"
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
#define PGTEST_EXPECT(cond) PGTEST_EXPECT_EX(cond, ((const char *)nullptr))

namespace pgimpl {
namespace test {

class Tests;
using TestFunction = std::function<bool()>;

Tests &getDefaultTestsInstance();
bool addTest(Tests &tests, const std::string &name, const TestFunction &testFunc, const char *file, long long line);

} // namespace test
} // namespace pgimpl
#endif