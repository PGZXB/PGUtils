#ifndef PGZXB_STATUS_H
#define PGZXB_STATUS_H

#include "../pgfwd.h"

namespace pgimpl {
namespace status {

class Status;
class ErrorManager;

class Status {
private:
    static constexpr std::uint64_t kCodeBits            = 56;
    static constexpr std::uint64_t kFlagsBits           = 64 - kCodeBits;
    static constexpr std::uint64_t kAllOneBits          = std::numeric_limits<std::uint64_t>::max();
    static constexpr std::uint64_t kCodeAllOneBits      = kAllOneBits >> kFlagsBits;
    // flags for internal_
    static constexpr std::uint64_t kNotUseInternalData  = 0;
    static constexpr std::uint64_t kUseInternalMgr      = 1;
    static constexpr std::uint64_t kUseInternalCallback = 2;
    // flags for context_
    static constexpr std::uint64_t kNoContext           = 0;
    static constexpr std::uint64_t kUsePlacedContext    = 1;
    static constexpr std::uint64_t kUseMallocContext    = 2;
public:
    using StatusInternalErrCallback = std::string(Status&);
    static constexpr std::uint64_t kOk                 = 0;
    static constexpr std::uint64_t kUnkown             = kCodeAllOneBits;
    static constexpr std::uint64_t kMinValidStatusCode = 1;
    static constexpr std::uint64_t kMaxValidStatusCode = kUnkown - 1;
    static constexpr std::uint64_t kMaxBits            = 3 * 8;
    static_assert(kOk == 0, ""); // 0 == OK

    Status();
    explicit Status(ErrorManager *mgr);
    explicit Status(StatusInternalErrCallback *callback);
    ~Status();

    Status(const Status &) = default;
    Status(Status &&) = default;

    Status &operator=(const Status &) = default;
    Status &operator=(Status &&) = default;

    Status &operator=(std::uint64_t code) {
        setCodeAndInvoke(code);
        return *this;
    }

    bool isOk() const {
        return this->is(kOk);
    }

    bool isUnkown() const {
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

    void onlySetCode(std::uint64_t code) {
        code_ = code;
    }

    std::string setCodeAndInvoke(std::uint64_t code, bool * ok = nullptr) {
        onlySetCode(code);
        return this->invoke(ok);
    }

    StatusInternalErrCallback * setCallback(StatusInternalErrCallback *callback);

    void setManager(ErrorManager * mgr) {
        internalDataFlags_ = kUseInternalMgr;
        internal_.mgr = mgr;
    }

    std::string invoke(bool * ok = nullptr);

    template <typename T, typename ...Args>
    T& makeContext(Args &&...args) {
        T * ptr = new (newContext(sizeof(T))) T{std::forward<Args>(args)...};
        return *ptr;
    }

    void clearContext();

    template <typename T>
    const T &getContext(bool * ok = nullptr) const {
        const_cast<Status&>(*this).getContext<T>(ok);
    }

    template <typename T>
    T &getContext(bool * ok = nullptr) {
        if (ok) *ok = contextFlags_ != kNoContext;
        return contextFlags_ == kUseMallocContext ?
            *(T*)context_.ptr :
            *(T*)&context_.mem;
    }

    void * getRawContext() const {
        if (contextFlags_ == kUsePlacedContext) return (void*)&context_.mem;
        if (contextFlags_ == kUseMallocContext) return context_.ptr;
        PGZXB_DEBUG_ASSERT(contextFlags_ == kNoContext);
        return nullptr;
    }

    std::uint64_t code() const {
        return code_;
    }

    ErrorManager *getMgr() const {
        return internalDataFlags_ == kUseInternalMgr ? internal_.mgr : nullptr;
    }
private:
    void * newContext(std::size_t sizeInBytes);

    std::uint64_t contextFlags_: 4;
    std::uint64_t internalDataFlags_: 4;
    std::uint64_t code_: kCodeBits;
    union {
        ErrorManager *mgr;
        StatusInternalErrCallback *callback;
    } internal_;
    union {
        void *ptr;
        std::uintptr_t mem;
    } context_;
};

}  // namespace status
}  // namespace pgimpl
#endif // !PGZXB_STATUS_H
