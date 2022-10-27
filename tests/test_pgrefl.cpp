#include "test/pgtest.h" // Using PGTEST_*

#include <cstring>
#include <string>
#include <iostream> // required by pgtest

#include "pgfwd.h"
#include "refl/utils/MemRef.h"

PGTEST_CASE(pgrefl_utils) {
    using pgimpl::refl::utils::MemRef;

    char buffer[sizeof(std::string)] = { 0 };
    void *pMem{nullptr}; PGZXB_UNUSED(pMem);

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

    {
        MemRef ref0(sizeof(std::string));
        PGTEST_EQ(ref0.getRefCount(), 1);
        auto *pStr = new (ref0.get()) std::string{"HELLO WORLD"};
        pStr->append("HELLO WORLD");
        PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
        pStr->~basic_string(); // Deconstruct *pStr
        pMem = pStr;
    }
    // { // The `*pMem` is managed by ref0:MemRef
    //     std::string *pStr = (std::string*)(void*)pMem;
    //     // (Maybe) !!Crashed!!
    //     PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
    // }

    {
        using Bytes = char[1024];
        const char STR[] = "Hello, PGUtils!!";
        MemRef ref0(sizeof(Bytes)), ref0copy = ref0;
        PGTEST_EQ(ref0.getRefCount(), 2);
        PGTEST_EQ(ref0.get(), ref0copy.get());
        {
            MemRef ref1(sizeof(Bytes));
            PGTEST_EQ(ref1.getRefCount(), 1);
            
            char * bytes = (char*)ref1.get();
            std::strncpy(bytes, STR, std::strlen(STR));
            PGTEST_EXPECT(std::strcmp((char*)ref1.get(), STR) == 0);
            
            ref0 = ref1;
            PGTEST_EQ(ref0copy.getRefCount(), 1);;
            PGTEST_EQ(ref1.getRefCount(), 2);
            PGTEST_EQ(ref0.getRefCount(), 2);
            PGTEST_EQ(ref0.get(), ref1.get());
        }
        {
            PGTEST_EQ(ref0.getRefCount(), 1);
            MemRef ref2 = std::move(ref0);
            PGTEST_EQ(ref0.get(), nullptr);
            PGTEST_EQ(ref0.getRefCount(), -1);
            PGTEST_EQ(ref2.getRefCount(), 1);
            PGTEST_EXPECT(std::strcmp((char*)ref2.get(), STR) == 0);

            pMem = ref2.get();
        }
        // { // The memory pointed by `pMem` has been deallocated
        //     // (Maybe) !!Crashed!!
        //     std::free(std::calloc(4096, 1));
        //     PGTEST_EXPECT(std::strcmp((char*)pMem, STR) == 0);
        // }
        pMem = ref0copy.get();
        PGTEST_EQ(ref0copy.getRefCount(), 1);
        MemRef ref0move = std::move(ref0copy);
        PGTEST_EQ(ref0move.get(), pMem);
        PGTEST_EQ(ref0move.getRefCount(), 1);
        PGTEST_EQ(ref0copy.get(), nullptr);
        PGTEST_EQ(ref0copy.getRefCount(), -1);

        ref0move = nullptr;
        PGTEST_EQ(ref0move, MemRef::null);
        
        MemRef ref3(1024);
        ref0move = ref3.get();
        PGTEST_EXPECT(!ref0move.managingMemeory());
        PGTEST_EXPECT(ref3.managingMemeory());
        PGTEST_EXPECT(ref0move.weakEquals(ref3));
        PGTEST_EXPECT(!ref0move.strongEquals(ref3));
        PGTEST_EXPECT(!(ref0move == ref3));
    }

    return true;
}
