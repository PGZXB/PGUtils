#include "Status.h"
#include "StatusManager.h"

using pgimpl::status::Status;
using pgimpl::status::ErrorManager;

Status::Status(ErrorManager *mgr)
    : context_flags_(0),
    internal_data_flags_(kUseInternalMgr),
    code_(kOk),
    mgr_(mgr) {
}

Status::Status(StatusInternalErrCallback *callback)
    : context_flags_(0),
    internal_data_flags_(kUseInternalCallback),
    code_(kOk),
    callback_(callback) {
}

Status::~Status() {
    if (context_flags_ == kUseMallocContext)
        std::free(context_);
}

Status::StatusInternalErrCallback *Status::set_callback(StatusInternalErrCallback *callback) {
    StatusInternalErrCallback *old_callback = 
        internal_data_flags_ == kUseInternalCallback ? callback_ : nullptr;
    internal_data_flags_ = kUseInternalCallback;
    callback_ = callback;
    return old_callback;
}

std::string Status::invoke() {
    if (internal_data_flags_ == kUseInternalCallback && !is_ok()) {
        PGZXB_DEBUG_ASSERT(!!callback_);
        return callback_(*this);
    }

    if (internal_data_flags_ == kUseInternalMgr && !is_ok()) {
        PGZXB_DEBUG_ASSERT(!!mgr_);
        const auto &errInfo = mgr_->tryGetErrorInfo(code_);
        if (!!errInfo->callback) return errInfo->callback(*this);
        return errInfo->msg;
    }

    return "";
}
