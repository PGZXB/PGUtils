include_directories(../)

file(GLOB test_src_files *.cpp)
add_executable(pgutils_test ${test_src_files})
target_link_libraries(pgutils_test PGTestMain)
