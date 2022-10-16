#include "Status.h"
#include "pg/pgstatus/StatusManager.h"

using pg::status::Status;
using pg::status::StatusManger;

Status::Status(StatusManger *mgr)
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
        auto callback = mgr_->get_callback(code_);
        if (!!callback) return callback(*this);
        return mgr_->get_msg(code_);
    }

    return "";
}
