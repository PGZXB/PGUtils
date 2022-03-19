// Status, err or ok; support register callback on err

#ifndef PGZXB_STATUS_H
#define PGZXB_STATUS_H

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "pg/pgfwd.h"

#if __cplusplus >= 201703L
#include <any> // Only support for >= C++17
#endif

namespace pg {
namespace status {

class Status;
class StatusManger;

class Status {
private:
    static constexpr std::uint64_t kCodeBits            = 56;
    static constexpr std::uint64_t kFlagsBits           = 64 - kCodeBits;
    static constexpr std::uint64_t kAllOneBits          = std::numeric_limits<std::uint64_t>::max();
    static constexpr std::uint64_t kCodeAllOneBits      = kAllOneBits >> kFlagsBits;
    static constexpr std::uint64_t kNotUseInternalData  = 0;
    static constexpr std::uint64_t kUseInternalMgr      = 1;
    static constexpr std::uint64_t kUseInternalCallback = 2;
    static constexpr std::uint64_t kUseMallocContext    = 1;
public:
    using StatusInternalErrCallback = std::string(Status&);
    static constexpr std::uint64_t kOk                 = 0;
    static constexpr std::uint64_t kUnkown             = kCodeAllOneBits;
    static constexpr std::uint64_t kMinValidStatusCode = 1;
    static constexpr std::uint64_t kMaxValidStatusCode = kUnkown - 1;
    static constexpr std::uint64_t kMaxBits            = 3 * 8;

    explicit Status(StatusManger *mgr)
      : context_flags_(0),
        internal_data_flags_(kUseInternalMgr),
        code_(kOk),
        mgr_(mgr) {
    }

    explicit Status(StatusInternalErrCallback *callback)
      : context_flags_(0),
        internal_data_flags_(kUseInternalCallback),
        code_(kOk),
        callback_(callback) {
    }

    ~Status() {
        if (context_flags_ == kUseMallocContext)
            std::free(context_);
    }

    Status(const Status &) = default;
    Status(Status &&) = default;

    Status &operator=(const Status &) = default;
    Status &operator=(Status &&) = default;

    Status &operator=(std::uint64_t code) {
        PGZXB_DEBUG_Print(set_code_and_invoke(code));
        return *this;
    }

    bool is_ok() const {
        return this->is(kOk);
    }

    bool is_unkown() const {
        return this->is(kUnkown);
    }

    bool is(std::uint64_t code) const {
        return code_ == code;
    }

    bool operator==(std::uint64_t code) const {
        return this->is(code);
    }

    bool operator==(const Status &other) const {
        return this->is(other.code_);
    }

    void only_set_code(std::uint64_t code) {
        code_ = code;
    }

    std::string set_code_and_invoke(std::uint64_t code) {
        code_ = code;
        return this->invoke();
    }

    StatusInternalErrCallback *set_callback(StatusInternalErrCallback *callback) {
        StatusInternalErrCallback *old_callback = internal_data_flags_ == kUseInternalCallback ? callback_ : nullptr;
        internal_data_flags_ = kUseInternalCallback;
        callback_ = callback;
        return old_callback;
    }

    void set_manager(StatusManger *mgr) {
        internal_data_flags_ = kUseInternalMgr;
        mgr_ = mgr;
    }

    std::string invoke() {
        if (internal_data_flags_ == kUseInternalCallback && !is_ok()) {
            PGZXB_DEBUG_ASSERT(!!callback_);
            return callback_(*this);
        }
        /* return mgr_->process_status(*this); */
        return "";
    }

    template <typename T, typename ...Args>
    void set_context(Args &&...args) {
        if (context_flags_ == kUseMallocContext)
            std::free(context_);
        if
#if __cplusplus >= 201703L
        constexpr
#endif
        (sizeof(T) < sizeof(context_)) {
            *(std::uintptr_t*)&context_ = 0;
            new (&context_) T(std::forward<Args>(args)...);
            context_flags_ = 0;
        } else {
            context_ = std::malloc(sizeof(T));
            new (context_) T(std::forward<Args>(args)...);
            context_flags_ = kUseMallocContext;
        }
    }

    template <typename T>
    const T &get_context() const {
        const_cast<Status&>(*this).get_context<T>();
    }

    template <typename T>
    T &get_context() {
        return context_flags_ == kUseMallocContext ?
            *(T*)context_ :
            *(T*)&context_;
    }

    std::uint64_t get_code() const {
        return code_;
    }
private:
    // higher 1 byte: as flags
    std::uint64_t context_flags_: 4;
    std::uint64_t internal_data_flags_: 4;
    std::uint64_t code_: kCodeBits;
    // internal data: as context, ptr to manager or callback
    union {
        StatusManger *mgr_;
        StatusInternalErrCallback *callback_;
    };
    void *context_{nullptr};
};

} // namespace status
} // namespace pg
#endif // !PGZXB_STATUS_H
