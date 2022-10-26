#include "Status.h"
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
// flags for context_
constexpr std::uint64_t Status::kNoContext;
constexpr std::uint64_t Status::kUsePlacedContext;
constexpr std::uint64_t Status::kUseMallocContext;
// public constant
constexpr std::uint64_t Status::kOk;
constexpr std::uint64_t Status::kUnkown;
constexpr std::uint64_t Status::kMinValidStatusCode;
constexpr std::uint64_t Status::kMaxValidStatusCode;
constexpr std::uint64_t Status::kMaxBits;

Status::Status()
    : contextFlags_(kNoContext),
      internalDataFlags_(kNotUseInternalData),
      code_(kOk) { }

Status::Status(ErrorManager *mgr)
    : contextFlags_(kNoContext),
      internalDataFlags_(kUseInternalMgr),
      code_(kOk) {
    internal_.mgr = mgr;
}

Status::Status(StatusInternalErrCallback *callback)
    : contextFlags_(kNoContext),
      internalDataFlags_(kUseInternalCallback),
      code_(kOk) {
    internal_.callback = callback;
}

Status::~Status() {
    clearContext();
}

Status::StatusInternalErrCallback *Status::setCallback(StatusInternalErrCallback *callback) {
    StatusInternalErrCallback *old_callback = 
        internalDataFlags_ == kUseInternalCallback ? internal_.callback : nullptr;
    internalDataFlags_ = kUseInternalCallback;
    internal_.callback = callback;
    return old_callback;
}

void Status::clearContext() {
    if (contextFlags_ == kUseMallocContext) std::free(context_.ptr);
    std::memset(&context_, 0, sizeof(context_));
    contextFlags_ = kNoContext;
}

std::string Status::invoke(bool * ok) {
    bool _ = true, &ok_ = ok ? *ok : _;
    ok_ = true; // reset
    if (isOk()) return "";

    if (internalDataFlags_ == kUseInternalCallback) {
        PGZXB_DEBUG_ASSERT(!!internal_.callback);
        return (internal_.callback)(*this);
    }

    if (internalDataFlags_ == kUseInternalMgr) {
        PGZXB_DEBUG_ASSERT(!!internal_.mgr);
        const auto *errInfo = internal_.mgr->tryGetErrorInfo(code_);
        if (!errInfo) ok_ = false; // The code was not registered
        else if (!!errInfo->callback) return errInfo->callback(*this);
        else return errInfo->msg;
    }

    PGZXB_DEBUG_ASSERT(!ok_ || internalDataFlags_ == kNotUseInternalData);
    ok_ = false; // No invoke-able
    return ""; 
}

// private functions
void * Status::newContext(std::size_t sizeInBytes) {
    clearContext();

    static_assert(sizeof(context_) == sizeof(context_.mem), "");
    void *ptr = nullptr;
    if (sizeInBytes < sizeof(context_)) {
        std::memset(&context_.mem, 0, sizeof(context_.mem));
        ptr = &context_.mem;
        contextFlags_ = kUsePlacedContext;
    } else {
        context_.ptr = std::malloc(sizeInBytes);
        ptr = context_.ptr;
        contextFlags_ = kUseMallocContext;
    }

    return ptr;
}
