#include "pgtest.h"
#include "Tests.h"

namespace pgimpl {
namespace test {

// class CapturedFdMsgEqHelper
CapturedFdMsgEqHelper::CapturedFdMsgEqHelper(pgutil::FdCapture::Fd capturedFd, std::string expectedOutput, const char * filename, std::size_t lineno, std::string failingMsg) :
    capture_(capturedFd),
    filename_(filename),
    lineno_(lineno),
    failingMsg_(failingMsg),
    expectedOutput_(std::move(expectedOutput)) {
    auto ok = capture_.begin();
    PGZXB_DEBUG_ASSERT(ok);
}

CapturedFdMsgEqHelper::~CapturedFdMsgEqHelper() throw(TestRunFailed) {
    using FC = pgutil::FdCapture;

    const char * fdName = nullptr;
    if (capture_.getCapturedFd() == FC::kStdoutFd) fdName = "stdout";
    else if (capture_.getCapturedFd() == FC::kStderrFd) fdName = "stderr";
    PGZXB_DEBUG_ASSERT(fdName != nullptr);

    std::string capturedOutputMsg;
    auto ok = capture_.end(capturedOutputMsg);
    PGZXB_DEBUG_ASSERT(ok);
    if (capturedOutputMsg != expectedOutput_) {
        std::string msg = pghfmt::format(
            "Expect {0} \"{1}\" but not({2}:{3}", fdName, expectedOutput_, filename_, lineno_);
        std::string hint = pghfmt::format(
            "Hint: Captured {0} is \"{1}\"{2}", fdName, capturedOutputMsg, !failingMsg_.empty() ? (", " + failingMsg_) : failingMsg_);
        std::cerr << msg << '\n';
        std::cerr << hint << '\n';
        throw TestRunFailed{};
    }
}

// Global functions
Tests &getDefaultTestsInstance() {
    return *Tests::getDefaultInstance();
}

bool addTest(Tests &tests, const std::string &name, const TestFunction &testFunc, const char *file, long long line) {
    tests.addTest(name, testFunc, std::string(file).append(1, ':').append(std::to_string(line)));
    return true;
}

} // namespace test
} // namespace pgimpl
