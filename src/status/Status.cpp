#include "Status.h"

#include <cstring>
#include "ErrorManager.h"

using pgimpl::status::Status;

// bits count of flags
constexpr std::uint64_t Status::kCodeBits;
constexpr std::uint64_t Status::kFlagsBits;
constexpr std::uint64_t Status::kAllOneBits;
constexpr std::uint64_t Status::kCodeAllOneBits;
// flags for internal_
constexpr std::uint64_t Status::kNotUseInternalData;
constexpr std::uint64_t Status::kUseInternalMgr;
constexpr std::uint64_t Status::kUseInternalCallback;
// public constant
constexpr std::uint64_t Status::kOk;
constexpr std::uint64_t Status::kUnkown;
constexpr std::uint64_t Status::kMinValidStatusCode;
constexpr std::uint64_t Status::kMaxValidStatusCode;
constexpr std::uint64_t Status::kMaxBits;

Status::Status()
    : internalDataFlags_(kNotUseInternalData),
      code_(kOk) { }

Status::Status(ErrorManager *mgr)
    : internalDataFlags_(kUseInternalMgr),
      code_(kOk) {
    internal_.mgr = mgr;
}

Status::Status(StatusInternalErrCallback *callback)
    : internalDataFlags_(kUseInternalCallback),
      code_(kOk) {
    internal_.callback = callback;
}

Status::Status(const Status &other)
    : internalDataFlags_(other.internalDataFlags_),
      code_(other.code_),
      internal_(other.internal_),
      context_(other.context_), 
      wrappedStatus_(other.wrappedStatus_ ? new Status(*other.wrappedStatus_) : nullptr) {
}

Status &Status::operator=(const Status &other) {
    if (this != &other) {
        internalDataFlags_ = other.internalDataFlags_;
        code_ = other.code_;
        internal_ = other.internal_;
        context_ = other.context_;
        wrappedStatus_.reset(other.wrappedStatus_ ? new Status(*other.wrappedStatus_) : nullptr);
    }
    return *this;
}

Status::StatusInternalErrCallback *Status::setCallback(StatusInternalErrCallback *callback) {
    StatusInternalErrCallback *old_callback = 
        internalDataFlags_ == kUseInternalCallback ? internal_.callback : nullptr;
    internalDataFlags_ = kUseInternalCallback;
    internal_.callback = callback;
    return old_callback;
}

std::string Status::invoke(bool * ok) {
    bool _ = true, &ok_ = ok ? *ok : _;
    ok_ = true; // reset
    if (isOk()) return "Ok Status";

    std::string thisMsg, wrappedMsg;

    if (isUnkown() || internalDataFlags_ == kNotUseInternalData) {
        ok_ = false;
        thisMsg = "Unkown Status";
    } else if (internalDataFlags_ == kUseInternalCallback) {
        PGZXB_DEBUG_ASSERT(!!internal_.callback);
        thisMsg = (internal_.callback)(*this);
    } else if (internalDataFlags_ == kUseInternalMgr) {
        PGZXB_DEBUG_ASSERT(!!internal_.mgr);
        const auto *errInfo = internal_.mgr->tryGetErrorInfo(code_);
        if (!errInfo) ok_ = false; // The code was not registered
        else if (!!errInfo->callback) thisMsg = errInfo->callback(*this);
        else thisMsg = errInfo->msg;
    }

    if (wrappedStatus_) {
        wrappedMsg = wrappedStatus_->invoke(&ok_);
    }

    return thisMsg + (wrappedMsg.empty() ? "" : "{" + wrappedMsg + "}");
}
