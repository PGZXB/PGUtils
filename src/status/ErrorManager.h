#ifndef PGZXB_STATUSMANAGER_H
#define PGZXB_STATUSMANAGER_H

#include <deque>
#include <string>
#include <limits>
#include <functional>

#include "../pgfwd.h"

namespace pgimpl {
namespace status {

constexpr char kGlobalErrorManagerName[] = "PGUtils :: Global Error Manager";

class Status;
class ErrorManager;
using ErrorCallback = std::function<std::string(Status&)>;

struct ErrorInfo {
    static constexpr std::uint64_t kInvalidCode = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t code{kInvalidCode};
    std::string msg /* {""} */;
    ErrorCallback callback /* {nullptr} */;
};

class RaiiTmpErrorInfoUpdater {
    friend class ErrorManager;
private:
    ErrorInfo *errorInfo_{nullptr};
    std::string oldMsg_;
    ErrorCallback oldCallback_;
public:
    bool holdErrorInfo() const {
        return !!errorInfo_;
    }

    RaiiTmpErrorInfoUpdater() = default;
    RaiiTmpErrorInfoUpdater(const RaiiTmpErrorInfoUpdater &) = delete;
    RaiiTmpErrorInfoUpdater(RaiiTmpErrorInfoUpdater &&) = default;

    ~RaiiTmpErrorInfoUpdater() {
        if (errorInfo_) {
            errorInfo_->msg = std::move(oldMsg_);
            errorInfo_->callback = std::move(oldCallback_);
        }
    }
};

class ErrorManager {
public:
    bool tryRegisterError(
        std::uint64_t code,
        const std::string & msg,
        const ErrorCallback & func = nullptr);
    bool tryUpdateError(
        std::uint64_t code,
        const std::string * msg = nullptr,
        const ErrorCallback * func = nullptr);
    RaiiTmpErrorInfoUpdater tryTmpUpdateError(
        std::uint64_t code,
        const std::string * msg = nullptr,
        const ErrorCallback * func = nullptr);
    bool tryUnregisterError(std::uint64_t code);
    const ErrorInfo * tryGetErrorInfo(std::uint64_t code) const;

    const std::string & getName() const {
        return name_;
    }

    static ErrorManager * tryGetErrorManager(const std::string & name);
    static ErrorManager & getOrMakeErrorManager(const std::string & name);
    static bool tryRemoveErrorManager(const std::string & name);
    static ErrorManager & getGlobalErrorManager();
private:
    ErrorInfo * tryGetErrorInfoImpl(std::uint64_t code);

    std::string name_/*{ "" }*/;
    std::deque<ErrorInfo> managedErrorInfo_;

    static std::deque<ErrorManager> errorManagerMap;
};

}  // namespace status
}  // namespace pgimpl
#endif