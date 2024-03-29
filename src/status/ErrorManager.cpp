#include "ErrorManager.h"

#include "Status.h"

using pgimpl::status::Status;
using pgimpl::status::ErrorManager;
using pgimpl::status::ErrorInfo;
using pgimpl::status::ErrorCallback;
using pgimpl::status::RaiiTmpErrorInfoUpdater;

ErrorManager::ErrorManager(const ErrorManager &other)
    : name_(other.name_),
      globalStatus_(new Status(*other.globalStatus_)),
      managedErrorInfo_(other.managedErrorInfo_) {
}

ErrorManager::ErrorManager(ErrorManager &&other) = default;

ErrorManager & ErrorManager::operator=(const ErrorManager &other) {
    name_ = other.name_;
    globalStatus_.reset(new Status(*other.globalStatus_));
    managedErrorInfo_ = other.managedErrorInfo_;
    return *this;
}

ErrorManager & ErrorManager::operator=(ErrorManager &&other) = default;

ErrorManager::~ErrorManager() = default;

bool ErrorManager::tryRegisterError(
  std::uint64_t code,
  const std::string &msg,
  const ErrorCallback &func) {
    managedErrorInfo_.resize(code + 1);

    auto &info = managedErrorInfo_[code];

    if (info.code != ErrorInfo::kInvalidCode) {
        return false;
    }

    info.code = code;
    info.msg = msg;
    info.callback = func;
    return true;
}

bool ErrorManager::tryUpdateError(
  std::uint64_t code,
  const std::string * msg,
  const ErrorCallback * func) {

    auto *info = tryGetErrorInfoImpl(code);
    if (!info) return false;
    if (msg) info->msg = *msg;
    if (func) info->callback = *func;
    return true;
}

RaiiTmpErrorInfoUpdater ErrorManager::tryTmpUpdateError(
  std::uint64_t code,
  const std::string * msg,
  const ErrorCallback * func) {
    auto *info = tryGetErrorInfoImpl(code);
    RaiiTmpErrorInfoUpdater updater;
    if (!info) return updater; // std::move
    updater.errorInfo_ = info;
    updater.oldMsg_ = info->msg;
    updater.oldCallback_ = info->callback;
    if (msg) info->msg = *msg;
    if (func) info->callback = *func;
    return updater;
}

bool ErrorManager::tryUnregisterError(std::uint64_t code) {
    auto *info = tryGetErrorInfoImpl(code);
    if (!info) return false;
    info->code = ErrorInfo::kInvalidCode;
    std::string().swap(info->msg);
    info->callback = nullptr;
    return true;
}

const ErrorInfo * ErrorManager::tryGetErrorInfo(std::uint64_t code) const {
    return const_cast<ErrorManager * const>(this)->tryGetErrorInfoImpl(code);
}

Status & ErrorManager::globalStatus() {
    PGZXB_DEBUG_ASSERT(globalStatus_);
    return *globalStatus_;
}


// static members & functions
std::deque<ErrorManager> ErrorManager::errorManagerMap;

ErrorManager * ErrorManager::tryGetErrorManager(const std::string & name) {
    for (auto &mgr : errorManagerMap) {
        if (mgr.name_ == name) {
            return &mgr;
        }
    }
    return nullptr;
}

ErrorManager & ErrorManager::getOrMakeErrorManager(const std::string & name) {
    if (auto *mgr = tryGetErrorManager(name)) return *mgr;
    ErrorManager mgr{name};
    errorManagerMap.push_back(std::move(mgr));
    return errorManagerMap.back();
}

bool ErrorManager::tryRemoveErrorManager(const std::string & name) {
    auto iter = errorManagerMap.begin();
    for (; iter != errorManagerMap.end(); ++iter) {
        if (iter->name_ == name) break;
    }
    if (iter == errorManagerMap.end()) return false;
    errorManagerMap.erase(iter);
    return true;
}

ErrorManager & ErrorManager::getGlobalErrorManager() {
    static ErrorManager *global_mgr{&getOrMakeErrorManager(kGlobalErrorManagerName)};
    return *global_mgr;
}

ErrorManager &ErrorManager::makeErrorManagerAndRegisterErrorInfos(const char * name, const ErrorInfo * errorInfos, const std::size_t errorInfosSize) {
    auto &mgr = ErrorManager::getOrMakeErrorManager(name);
    for (std::size_t i = 0; i < errorInfosSize; ++i) {
        mgr.tryRegisterError(errorInfos[i].code, errorInfos[i].msg, errorInfos[i].callback);
    }
    return mgr;
}

// private member functions
ErrorManager::ErrorManager(std::string name) : name_(std::move(name)), globalStatus_(new Status(this)) {

}

ErrorInfo * ErrorManager::tryGetErrorInfoImpl(std::uint64_t code) {
    auto &infos = managedErrorInfo_;
    if (code < infos.size() && infos[code].code != ErrorInfo::kInvalidCode) {
        return &infos[code];
    }
    return nullptr;
}
