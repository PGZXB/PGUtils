#NOTE: The tests' include directorie is PGUtils/src/
#NOTE: But the users' include directorie should be set PGUtils/include/
include_directories(../src/)

file(GLOB test_src_files *.cpp)
if(PGZXB_DEBUG)
    list(APPEND test_src_files ${BACKWARD_ENABLE})
endif()
add_executable(pgutils_test ${test_src_files})
target_link_libraries(pgutils_test PGUtils PGTestMain)
