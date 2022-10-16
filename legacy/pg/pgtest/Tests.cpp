#include "pgtest.h"
#include "Tests.h"

namespace pg {
namespace test {

Tests &getDefaultTestsInstance() {
    return *Tests::getDefaultInstance();
}

bool addTest(Tests &tests, const std::string &name, const TestFunction &testFunc, const char *file, long long line) {
    tests.addTest(name, testFunc, std::string(file).append(1, ':').append(std::to_string(line)));
    return true;
}

} // namespace test
} // namespace pg
