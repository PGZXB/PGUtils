#include "StatusManager.h"

using pgimpl::status::ErrorManager;
using pgimpl::status::ErrorInfo;
using pgimpl::status::ErrorCallback;
using pgimpl::status::RaiiTmpErrorInfoUpdater;

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
    ErrorManager mgr;
    mgr.name_ = name;
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

// private member functions
ErrorInfo * ErrorManager::tryGetErrorInfoImpl(std::uint64_t code) {
    auto &infos = managedErrorInfo_;
    if (code < infos.size() && infos[code].code != ErrorInfo::kInvalidCode) {
        return &infos[code];
    }
    return nullptr;
}
