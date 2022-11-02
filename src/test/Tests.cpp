#include "pgtest.h"
#include "Tests.h"

#include <fstream>
#include <unistd.h>

namespace pgimpl {
namespace test {

class Context::Impl {
    using Checker = std::function<void(const std::string &)>;
public:
    pgutil::RaiiCleanup capureFdAndCheck(int fd, const Checker &checker) {
        // FIXME: Adapt Windows
        auto copyFd = ::dup(fd);
        std::string tmpFile = "/tmp/pgtest_cap_XXXXXX";
        auto capFd = ::mkstemp(const_cast<char*>(tmpFile.data()));
        PGZXB_DEBUG_ASSERT(capFd != -1);
        ::fflush(nullptr);
        ::dup2(capFd, fd);
        ::close(capFd);
        
        return {[copyFd, fd, tmpFile, checker]() {
            // Un-capture
            ::fflush(nullptr);
            ::dup2(copyFd, fd);
            ::close(copyFd);

            // Read tmp file & Remove
            std::string buffer;
            {
                std::ifstream f(tmpFile);
                // Make buffer
                f.seekg(0, std::ios::end);
                auto len = f.tellg();
                f.seekg(0, std::ios::beg);
                buffer.resize(len, '\0');
                // Read bytes to buffer
                f.read(const_cast<char*>(buffer.data()), len);
                PGZXB_DEBUG_ASSERT(f.tellg() == len);
            }
            auto ret = std::remove(tmpFile.c_str());
            PGZXB_DEBUG_ASSERT(ret == 0);

            // Check
            checker(buffer);
        }};
    }

    pgutil::RaiiCleanup capureStdoutAndCheck(const Checker &checker) {
        return capureFdAndCheck(1, checker);
    }

    pgutil::RaiiCleanup capureStderrAndCheck(const Checker &checker) {
        return capureFdAndCheck(2, checker);
    }
private:
    /* !!TODO!! */
};

Context::Context() {
    impl_ = new Impl{};
}

pgutil::RaiiCleanup Context::capureStdoutAndCheck(const Checker &checker) {
    return impl_->capureStdoutAndCheck(checker);
}

pgutil::RaiiCleanup Context::capureStderrAndCheck(const Checker &checker) {
    return impl_->capureStderrAndCheck(checker);
}

Context::~Context() {
    delete impl_;
}

Tests &getDefaultTestsInstance() {
    return *Tests::getDefaultInstance();
}

bool addTest(Tests &tests, const std::string &name, const TestFunction &testFunc, const char *file, long long line) {
    tests.addTest(name, testFunc, std::string(file).append(1, ':').append(std::to_string(line)));
    return true;
}

} // namespace test
} // namespace pg
