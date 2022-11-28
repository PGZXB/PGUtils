#include "test/pgtest.h" // Using PGTEST_*

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
