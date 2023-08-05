#include "test/pgtest.h" // Using PGTEST_*

#ifndef _WIN32
PGTEST_CASE(pgtest_captureStdoutAndStderr) {
    PGTEST_STDOUT_EQ("std::cout << ABC;printf(ABC);") {
        std::cout << "std::cout << ABC;";
        printf("printf(ABC);");
    }

    PGTEST_STDERR_EQ("std::cerr << ABC;fprintf(stderr, ABC);") {
        std::cerr << "std::cerr << ABC;";
        fprintf(stderr, "fprintf(stderr, ABC);");
    }

    PGTEST_STDERR_EQ("stderr") {
        PGTEST_STDOUT_EQ("stdout") {
            std::cout << "stdout";
            std::cerr << "stderr";
        }
    }
}
#endif


PGTEST_CASE(pgtest_exception) {
    class TestException : public std::exception {
        using Base = std::exception;

     public:
        using Base::Base;
    };

    PGTEST_EXPECT_EXCEPTION(const TestException &, "This is a exception hahaha!!") {
        throw TestException("a exception hahaha!!");
    };
}
