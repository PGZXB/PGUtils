cmake_minimum_required(VERSION 3.16)
set(DCMAKE_SH="CMAKE_SH-NOTFOUND")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
include_directories(.)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-Wall)

project(PGUtils)
set(PGUTILS_SRC_FILE
    pg/pgstatus/Status.cpp
    pg/pgstatus/StatusManager.cpp
)
add_library(PGUtils STATIC ${PGUTILS_SRC_FILE})

set(PGTEST_SRC_FILE pg/pgtest/pgtest_main.cpp pg/pgtest/Tests.cpp)
add_library(PGTestMain STATIC ${PGTEST_SRC_FILE})
add_subdirectory(tests)
