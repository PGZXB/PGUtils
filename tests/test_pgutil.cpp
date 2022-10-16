#include <cassert>
#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <limits>

#include "pg/pghfmt.h"
#include "pg/pgtest.h"
#include "pg/pgutil.h"

using namespace pgutil;

// NOTE: Templates cannot be declared inside of a local class
struct ConvertAndAppend2 {
    std::string result;

    template<typename T>
    void operator() (int, const T & e) {
        pghfmt::formatAppend(result, "{0}", e);
    }
};

PGTEST_CASE(pgutil_basic) {
    // cStrToU32
    PGTEST_EXPECT(cStrToU32("1024", nullptr, 10) == 1024);
    PGTEST_EXPECT(cStrToU32("0xffffffff", nullptr, 16) == 0xffffffff);
    PGTEST_EXPECT(cStrToU32("0", nullptr, 10) == 0);
    // cStrToU64
    PGTEST_EXPECT(cStrToU64("1024", nullptr, 10) == 1024);
    PGTEST_EXPECT(cStrToU64("0xffffffffffffffff", nullptr, 16) == 0xffffffffffffffff);
    PGTEST_EXPECT(cStrToU64("0", nullptr, 10) == 0);
    // ceilDivide
    PGTEST_EXPECT(ceilDivide(100, 23) == 5);
    
    std::uint32_t bits = 0b1010101000111001;
    // clz
    PGTEST_EXPECT(clz(bits) == 32 - 16);
    // ctz
    PGTEST_EXPECT(ctz(bits) == 0);
    // popcnt
    PGTEST_EXPECT(popcnt(bits) == 8);

    return true;
}

PGTEST_CASE(pgutil_visitParamPackage) {
    // visitParamPackage
    /// 1. args with same type
    std::string str;
    visitParamPackage(
        [&str](int, const char *s) { str += s; },
        "a", "b", "c", "0", "1", "2", "3");
    PGTEST_EXPECT(str == "abc0123");

    /// 2. args with diff type
    //// 2.1 using functor with overloaded functions for arg-types
    struct ConvertAndAppend {
        std::string result;

        void operator() (int, const std::string &e) {
            result += e;
        }

        void operator() (int , int e) {
            result += std::to_string(e);
        }

        void operator() (int index, const std::vector<int> & v) {
            for (const auto &e : v) {
                (*this)(index, e);
            }
        }
    };
    ConvertAndAppend caa;
    visitParamPackage(
        std::ref(caa),
        "a", "b", "c", 0, std::vector<int>{1, 2, 3});
    PGTEST_EQ(caa.result, "abc0123");
    
    //// 2.2 using functor with template function
    ConvertAndAppend2 caa2;
    visitParamPackage(
        std::ref(caa2),
        "a",
        "b",
        "c",
        std::string("0.123"),
        std::vector<int>{1, 2, 3},
        std::map<int, char>{{1, '1'}, {2, '2'}});
    PGTEST_EQ(caa2.result, "abc0.123[1, 2, 3][1 : 1, 2 : 2]");
    return true;
}

PGTEST_CASE(pgutil_parseCmdSimply) {
    // parseCmdSimply

    constexpr int argc = 5;    
    char argvBuf[argc][10] = {
        "cmd",
        "-f",
        "a.txt",
        "-z",
        "100",
    };
    char *argv[argc] = {
        argvBuf[0],
        argvBuf[1],
        argvBuf[2],
        argvBuf[3],
        argvBuf[4],
    };

    using PCC = ParseCmdConfig;
    std::unordered_map<char, ParseCmdConfig> configs;
    bool f{false};
    int z{0};
    configs['f'] = PCC{
        PCC::OPTION,
        [&f](const char *) {
            f = true;
            return true;
        }
    };
    configs['z'] = PCC{
        PCC::PARAM,
        [&z](const char *v) {
            assert(v);
            z = cStrToU32(v, nullptr, 10);
            return true;
        }
    };

    auto ext = parseCmdSimply(argc, argv, configs);
    PGTEST_EQ(f, true);
    PGTEST_EQ(z, 100);
    PGTEST_EQ(ext, std::vector<const char *>{argvBuf[2]});

    return true;
}
