#ifndef PGZXB_PGTEST_CONTEXTS_H
#define PGZXB_PGTEST_CONTEXTS_H

#include <string>
#include <exception>

namespace pgimpl {
namespace test {

// contexts
class TestCaseContext {
public:
    unsigned failedCount() const { return failedCount_; }
    unsigned passedCount() const { return passedCount_; }
    bool failed() const { return failedCount() > 0; }
    void incFailedCount() { ++failedCount_; }
    void incPassedCount() { ++passedCount_; }
private:
    unsigned failedCount_{0};
    unsigned passedCount_{0};
};

} // namespace test
} // namespace pgimpl

#endif // !PGZXB_PGTEST_CONTEXTS_H
