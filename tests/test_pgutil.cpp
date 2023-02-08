#include "test/pgtest.h"

#include <cassert>
#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <limits>

#include "pghfmt.h"
#include "pgutil.h"

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
}

PGTEST_CASE(pgutil_ObjectPool) {
    struct T {
        bool &constructed;
        bool &destroyed;

        int i = 0;
        char ch[513];
        int p, *pp;
        std::string s;
        std::vector<int> v;

        T(bool &c, bool &d) : constructed(c), destroyed(d) {
            constructed = true;
            destroyed = false;
        }

        ~T() {
            constructed = false;
            destroyed = true;
        }
    };


    bool c[100] = { 0 };
    bool d[100] = { 0 };


    {
        using Pool = ObjectPool<T>;
        Pool pool;

        for (int i = 0; i < 100; ++i) {
            pool.createObject(c[i], d[i])->i = i;
        }

        for (int i = 0; i< 100; ++i) {
            PGTEST_EQ(pool.id(), Pool::getPoolID(pool[i]));
            PGTEST_EQ((std::size_t)i, Pool::getIndexInPool(pool[i]));
            union {
                struct { std::uint64_t poolID: 16; std::uint64_t indexInPool: 48; };
                std::uint64_t uid;
            } u;
            u.poolID = Pool::getPoolID(pool[i]);
            u.indexInPool = Pool::getIndexInPool(pool[i]);
            PGTEST_EQ(u.uid, Pool::getUID(pool[i]));
            PGTEST_EQ(pool[i]->i, i);
            PGTEST_EXPECT(c[i]);
            PGTEST_EXPECT(!d[i]);
        }

        for (int i = 0; i < 100; i += 2) {
            pool.destroyObject(pool[i]);
        }

        for (int i = 0; i < 100; ++i) {
            if (i % 2 == 0) {
                PGTEST_EXPECT(!c[i]);
                PGTEST_EXPECT(d[i]);
            } else {
                PGTEST_EXPECT(c[i]);
                PGTEST_EXPECT(!d[i]);
            }
        }

        for (int i = 0; i < 100; ++i) {
            if (i % 2 != 0) { // not-destroyed objects
                auto pI = pool[i];
                PGTEST_EQ(pI.get(), pool[Pool::getIndexInPool(pI)].get());
            }
        }
    } // pool destroyed
    for (int i = 0; i < 100; ++i) {
        PGTEST_EXPECT(!c[i]);
        PGTEST_EXPECT(d[i]);
    }
}
