#ifndef PGZXB_PGTEST_CONTEXTS_H
#define PGZXB_PGTEST_CONTEXTS_H

#include <string>
#include <exception>

namespace pgimpl {
namespace test {

// contexts
class TestCaseContext {
public:
    bool failed() const { return failCount_ > 0; }
    void incFailCount() { ++failCount_; }
private:
    unsigned failCount_{0};
};

} // namespace test
} // namespace pgimpl

#endif // !PGZXB_PGTEST_CONTEXTS_H
