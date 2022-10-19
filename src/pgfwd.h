//
// Created by PGZXB on 2021/6/11.
//
// My uniform fwd-header-file
//                     ---- PGZXB

#ifndef PGZXB_PGFWD_H
#define PGZXB_PGFWD_H

#include <cstdint>
#include <string>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#define PGZXB_PASS (void(0))
#define PGZXB_UNUSED(val) (void(val))
#define PGZXB_PGFWD_MIN_CPP_VERSION 201103L
#if PGZXB_PGFWD_MIN_CPP_VERSION > __cplusplus
#error "Min __cplusplus: " ## #PGZXB_PGFMT_MIN_CPLUSPLUS
#endif

#define PGZXB_MALLOC(size) malloc(size)
#define PGZXB_REALLOC(ptr, newSize) realloc((ptr), (newSize))
#define PGZXB_FREE(ptr) free(ptr)

#define PGZXB_STATIC_ASSERT_EX(msg, exp) static_assert(exp, msg)
#define PGZXB_STATIC_ASSERT(exp) PGZXB_STATIC_ASSERT_EX("", exp)

// Assert macros
#if defined(PGZXB_DEBUG)
    #include <cstdio>
    #include <cassert>

    #define PGZXB_ASSERT_IF(x) for ( ; !(x) ; )

    #define PGZXB_DEBUG_ASSERT(exp) \
        PGZXB_ASSERT_IF(exp) { assert(exp); break; } \
        PGZXB_PASS

    #define PGZXB_DEBUG_ASSERT_EX(msg, exp) \
        PGZXB_ASSERT_IF((exp)) { \
            std::fputs(msg, stderr); \
            std::fputs(" : \n", stderr); \
            assert(exp); \
            break; \
        } PGZXB_PASS

    #define PGZXB_DEBUG_PTR_NON_NULL_CHECK(ptr) PGZXB_DEBUG_ASSERT_EX("Pointer Not Null", (ptr) != nullptr)

    #define PGZXB_DEBUG_EXEC(code) code

#else
    #define PGZXB_DEBUG_ASSERT_IF(x) PGZXB_UNUSED(x)
    #define PGZXB_DEBUG_ASSERT(exp) PGZXB_UNUSED(exp)
    #define PGZXB_DEBUG_ASSERT_EX(msg, exp) PGZXB_UNUSED(msg); PGZXB_UNUSED(exp)
    #define PGZXB_DEBUG_PTR_NON_NULL_CHECK(ptr) PGZXB_UNUSED(ptr)
    #define PGZXB_DEBUG_EXEC(code) PGZXB_UNUSED(code)
#endif

// Debug print macros
#if defined(PGZXB_DEBUG)
    #include <bitset>
    #include <iostream>
    #define PGZXB_DEBUG_INFO_HEADER "[DEBUG]In " << __FILE__ << " in " << __func__ << " at " << __LINE__ << " : "
    #define PGZXB_DEBUG_Print(msg) std::cout << PGZXB_DEBUG_INFO_HEADER << (msg) << "\n"
    #define PGZXB_DEBUG_PrintVar(val) std::cout << PGZXB_DEBUG_INFO_HEADER << #val" : " << (val) << "\n"
    #define PGZBX_DEBUG_PrintBin(val) std::cout << PGZXB_DEBUG_INFO_HEADER << #val" : " << std::bitset<sizeof(val) * 8>(val) << "\n"
    #define PGZXB_DEBUG_CallFunc(func) PGZXB_DEBUG_PrintVar(func)
#else
    #define PGZXB_DEBUG_INFO_HEADER ""
    #define PGZXB_DEBUG_Print(msg) PGZXB_UNUSED(msg)
    #define PGZXB_DEBUG_PrintVar(val) PGZXB_UNUSED(val)
    #define PGZBX_DEBUG_PrintBin(val) PGZXB_UNUSED(val)
    #define PGZXB_DEBUG_CallFunc(func) PGZXB_UNUSED(func)
#endif

#endif // PGZXB_PGFWD_H