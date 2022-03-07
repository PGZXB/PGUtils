#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>

#include "pg/pgfmt.h"
#include "pg/pgtest/pgtest.h"

using pgfmt::format;

PGTEST_CASE(pgfmt_formatInterger) {
#define T_VAR(n) PGTEST_EXPECT(format("{0}", (n)) == std::to_string(n));
#define T_GROUP_FOR(type) \
    T_VAR((type)0); \
    T_VAR((type)100); \
    T_VAR((type)-100); \
    T_VAR(std::numeric_limits<type>::min()); \
    T_VAR(std::numeric_limits<type>::max());

    // int, long, long long, unsigned int, unsigned long, unsigned long long
    T_GROUP_FOR(int);
    T_GROUP_FOR(long);
    // T_GROUP_FOR(long long); // FIXME:Fix this bug
    T_GROUP_FOR(unsigned int);
    T_GROUP_FOR(unsigned long);
    // T_GROUP_FOR(unsigned long long); // FIXME:Fix this bug

    // char, unsigned char
    PGTEST_EXPECT(format("{0}", '0') == "0");
    PGTEST_EXPECT(format("{0}", 'A') == "A");
    PGTEST_EXPECT(format("{0}", '\n') == "\n");
    PGTEST_EXPECT(format("{0}", '\033') == "\033");

    // bool
    PGTEST_EXPECT(format("{0}", true) == "true");
    PGTEST_EXPECT(format("{0}", false) == "false");

    // mixed
    auto str1 = format("{0} + {1} = {2}", 100, 200, 100 + 200);
    PGTEST_EXPECT(str1 == "100 + 200 = 300");

    auto str2 = format("1 == 2 : {0}", 1 == 2);
    PGTEST_EXPECT(str2 == "1 == 2 : false");

    return true;
#undef T_GROUP_FOR
#undef T_VAR
}
