#include <cassert>
#include <iostream>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>
#include <list>
#include <set>
#include <map>

#include "pg/pghfmt.h"
#include "pg/pgtest.h"

using pghfmt::format;

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
    T_GROUP_FOR(long long);
    T_GROUP_FOR(unsigned int);
    T_GROUP_FOR(unsigned long);
    T_GROUP_FOR(unsigned long long);

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

PGTEST_CASE(pgfmt_formatSTLContainer) {
#define T_VALUES(C) { \
    C c(values, values + len); \
    PGTEST_EXPECT(format("{0}", c) == vStr); \
}
    constexpr const char * keys[] = {"-100", "zero", "100", "1000"};
    int values[] = {-100, 0, 100, 1000};
    const char * vStr = "[-100, 0, 100, 1000]";
    const char * kvStr = "[-100 : -100, zero : 0, 100 : 100, 1000 : 1000]";
    constexpr auto kLen = sizeof(keys) / sizeof(*keys);
    constexpr auto vLen = sizeof(values) / sizeof(*values);
    static_assert(kLen == vLen, "");
    constexpr auto len = kLen;

    T_VALUES(std::vector<int>); // vector
    T_VALUES(std::list<int>); // list
    T_VALUES(std::set<int>); // set
    { // map
        struct Less {
            bool operator() (const std::string & a, const std::string & b) const {
                int i{0}, j{0};
                for (; i < len && a != keys[i]; ++i);
                for (; j < len && b != keys[j]; ++j);
                return i < j;
            }
        };
        std::map<std::string, int, Less> c;
        for (int i = 0; i < len; ++i) c[keys[i]] = values[i];
        PGTEST_EXPECT(format("{0}", c) == kvStr);
    }

    return true;
}
