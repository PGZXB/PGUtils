cmake_minimum_required(VERSION 3.16)
set(DCMAKE_SH="CMAKE_SH-NOTFOUND")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

project(PGUtils)

# Options
option(PGZXB_DEBUG "Build for debugging" OFF)

# Set C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_FORMAT_MACROS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(PGZXB_DEBUG)
    set(CMAKE_BUILD_TYPE "Debug") # debug mode ( -g )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPGZXB_DEBUG")
endif()

if(MSVC)
    # See https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif()

# Add sub directories
if(PGZXB_DEBUG)
    add_subdirectory(ext/BackwardCpp)
endif()

add_subdirectory(tests)

# Add libraries
set(PGUTILS_SRC_FILE
    src/status/Status.cpp
    src/status/ErrorManager.cpp
    src/refl/utils/MemRef.cpp
    src/refl/ClassManager.cpp
    src/refl/ClassBuilder.cpp
)
add_library(PGUtils STATIC ${PGUTILS_SRC_FILE})

set(PGTEST_SRC_FILE
    src/test/utils.cpp
    src/test/pgtest.cpp
    src/test/Tests.cpp
    src/test/pgtest_main.cpp)
add_library(PGTestMain STATIC ${PGTEST_SRC_FILE})
