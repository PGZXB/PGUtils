#include "test/pgtest.h" // Using PGTEST_*

PGTEST_CASE(pgtest_captureStdoutAndStderr) {

    PGTEST_STDOUT_EQ("ABC") {
        std::cout << "Hello World;";
    }

    return true;
}
