#include "test/pgtest.h"

#include <string>
#include <iostream> // required by pgtest
#include "refl/utils/MemRef.h"

PGTEST_CASE(pgrefl_utils) {
    using pgimpl::refl::utils::MemRef;

    char buffer[sizeof(std::string)] = { 0 };

    {
        MemRef refStackMem{buffer};
        PGTEST_EQ(refStackMem.getRefCount(), -1);
        auto *pStr = new (refStackMem.get()) std::string{"HELLO WORLD"};
        pStr->append("HELLO WORLD");
        PGTEST_EQ((void*)buffer, (void*)pStr);
        PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
    }
    { // The `buffer` is not managed by refStackMem:MemRef
        std::string *pStr = (std::string*)(void*)buffer;
        PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
    }

    void *pMem{nullptr};
    {
        MemRef ref0(sizeof(std::string));
        PGTEST_EQ(ref0.getRefCount(), 1);
        auto *pStr = new (ref0.get()) std::string{"HELLO WORLD"};
        pStr->append("HELLO WORLD");
        PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
        pStr->~basic_string();
        pMem = pStr;
    }
    // { // The `*pMem` is managed by ref0:MemRef
    //     std::string *pStr = (std::string*)(void*)pMem;
    //     // (Maybe) !!Crashed!!
    //     PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
    // }

    MemRef ref{nullptr};

    return true;
}
