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
     public:
      const char *msg{nullptr};
      TestException(const char *msg) : msg(msg) {
      }

      const char *what() const noexcept override {
        return msg;
      }
    };

    PGTEST_EXPECT_EXCEPTION(const TestException &, "a exception hahaha!!") {
        throw TestException("This is a exception hahaha!!");
    };
}
