#ifndef PGZXB_PGUTILS_H
#define PGZXB_PGUTILS_H

#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <vector>
#include <utility>
#include <fstream>
#include <functional>
#include <unistd.h> // FIXME: Adapt Windows

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "pgfwd.h"

namespace pgimpl {
namespace util {

#if defined(__GNUC__)
int inline popcnt(std::uint32_t x) {
    return __builtin_popcount(x);
}
#else
int inline popcnt(std::uint32_t x) {
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    return (int)(std::uint32_t)(x & 0x0000003f);
}
#endif

#if defined(__GNUC__)
int inline clz(std::uint32_t x) {
    return __builtin_clz(x);
}

int inline ctz(std::uint32_t x) {
    return __builtin_ctz(x);
}
#elif defined(_MSC_VER)
int __inline ctz(std::uint32_t value) {
    unsigned long trailing_zero = 0;

    if (_BitScanForward(&trailing_zero, value)) return (int)(std::uint32_t)trailing_zero;
    else return 32;
}

int __inline clz(std::uint32_t value) {
    unsigned long leading_zero = 0;

    if (_BitScanReverse( &leading_zero, value )) return 31 - leading_zero;
    else return 32;
}
#else
int inline clz(std::uint32_t x) {
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return (int)(std::uint32_t)(32 - popcnt(x));
}
int inline ctz(std::uint32_t x) {
    return popcnt((x & -x) - 1);
}
#endif

template<typename A, typename B>
inline auto ceilDivide(const A &dividend, const B &divisor)
    -> decltype(dividend / divisor + (dividend % divisor != 0)) {
    return dividend / divisor + (dividend % divisor != 0);
}

inline std::uint64_t cStrToU64(const char *str, char **str_end, int radix) {
    return std::strtoull(str, str_end, radix);
}

inline std::uint32_t cStrToU32(const char *str, char **str_end, int radix) {
    errno = 0;
    std::uint64_t result = cStrToU64(str, str_end, radix);
    if (errno != 0 || // 出错或者大于u32最大值
        result > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())
    ) {
        errno = ERANGE;
        str_end && (*str_end = nullptr);
        return std::numeric_limits<std::uint32_t>::max();
    }
    return static_cast<std::uint32_t>(result);
}

template <typename VISITOR> // VISITOR : void(SizeType, ARG);
inline void visitParamPackageHelper(VISITOR visitor, std::size_t index) { }

template <typename VISITOR, typename ARG, typename ... ARGS>
inline void visitParamPackageHelper(VISITOR visitor, std::size_t index, ARG &&arg, ARGS &&...args) {
    visitor(index, std::forward<ARG>(arg));
    visitParamPackageHelper(visitor, index + 1, std::forward<ARGS>(args)...);
}

template <typename VISITOR, typename ... ARGS>
inline void visitParamPackage(VISITOR visitor, ARGS &&...args) {
    visitParamPackageHelper(visitor, 0, std::forward<ARGS>(args)...);
}

#if __cplusplus >= 201703L
struct TypeArrayInvalidDataType { };

struct TypeArrayBaseInvalidType {
    using Front = TypeArrayInvalidDataType;
    using Other = TypeArrayBaseInvalidType;

    static constexpr std::size_t INDEX = static_cast<std::size_t>(-1);
};

template <std::size_t FINDEX, typename FIRST, typename ... OTHER>
struct TypeArrayBase {
    using Front = FIRST;
    using Other = TypeArrayBase<FINDEX + 1, OTHER...>;

    static constexpr std::size_t INDEX = FINDEX;
};

template<std::size_t FINDEX>
struct TypeArrayBase<FINDEX, TypeArrayInvalidDataType> {
    using Front = TypeArrayInvalidDataType;
    using Other = TypeArrayBaseInvalidType;

    static constexpr std::size_t INDEX = FINDEX;
};

template<typename ... TYPES>
class TypeArray {
private:
    using Base = TypeArrayBase<0, TYPES..., TypeArrayInvalidDataType>;
    
public:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    template<typename T>
    static constexpr std::size_t index() {
        return indexImpl<T, Base>();
    }

    static constexpr std::size_t size() {
        return sizeof...(TYPES);
    }

private:
    template<typename T, typename TAB>
    static constexpr std::size_t indexImpl() {
        if constexpr (std::is_same_v<TAB, TypeArrayBaseInvalidType>)
            return npos;

        if constexpr (std::is_same_v<T, typename TAB::Front>)
            return TAB::INDEX;

        return indexImpl<T, typename TAB::Other>();
    }
};
#endif

template <typename Data>
struct AllwaysTrue {
    Data data;
    operator bool () const {
        return true;
    }
};

struct ParseCmdConfig {
    using ParseCmdCallback = std::function<bool(const char* str)>;
    enum {
        OPTION,
        PARAM,
    } type; // must be 1st
    ParseCmdCallback callbcak; // must be 2nd
};

template <typename Map>
inline std::vector<const char *> parseCmdSimply(int argc, char* argv[], const Map& cmdConfig) {
    // FIXME: Unsafe
    std::vector<const char *> ext;
    for (int i = 1; i < argc; ++i) {
        const char* str = argv[i];
        const std::size_t slen = std::strlen(str);
        if (slen >= 2 && str[0] == '-') {
            auto iter = cmdConfig.find(str[1]);
            if (iter != cmdConfig.end() && !!(iter->second.callbcak)) {
                if (!iter->second.callbcak((
                    iter->second.type == ParseCmdConfig::PARAM && i + 1 < argc) ? argv[++i] : nullptr
                )) exit(0);
            } /* else {
              // ignore
            } */
        } else {
            ext.push_back(str);
        }
    }
    return ext;
}

class RaiiCleanup {
public:
    RaiiCleanup(std::function<void()> fn) : fn_(std::move(fn)) {
    }

    RaiiCleanup(const RaiiCleanup &) = delete;
    RaiiCleanup(RaiiCleanup && o) = default;

    RaiiCleanup & operator= (const RaiiCleanup &) = delete;
    RaiiCleanup & operator= (RaiiCleanup &&) = default;

    ~RaiiCleanup() {
        if (fn_) {
            fn_();
            fn_ = nullptr;
        }
    }
private:
    std::function<void()> fn_{nullptr};
};

class FdCapture {
public:
    enum Fd { kNone = -1, kStdoutFd = 1, kStderrFd = 2 };

    explicit FdCapture(int fd) : capturedFd_(fd) {
    }

    bool begin() {
        // FIXME: Adapt Windows
        copyFd_ = ::dup(capturedFd_);
        tmpFile_ = "/tmp/pgtest_cap_XXXXXX";
        auto capFd = ::mkstemp(const_cast<char*>(tmpFile_.data()));
        if (capFd == -1) return false;
        ::fflush(nullptr);
        ::dup2(capFd, capturedFd_);
        ::close(capFd);
        return true;
    }

    bool end(std::string &outMsg) {
        if (copyFd_ == -1) return false;
        // Un-capture
        ::fflush(nullptr);
        ::dup2(copyFd_, capturedFd_);
        ::close(copyFd_);

        // Get captured msg
        std::string buffer;
        {
            std::ifstream f(tmpFile_);
            // Make buffer
            f.seekg(0, std::ios::end);
            auto len = f.tellg();
            f.seekg(0, std::ios::beg);
            buffer.resize(len, '\0');
            // Read bytes to buffer
            f.read(const_cast<char*>(buffer.data()), len);
            PGZXB_DEBUG_ASSERT(f.tellg() == len);
        }
        outMsg = std::move(buffer);
        std::remove(tmpFile_.c_str());

        // reset internal data
        copyFd_ = -1;

        return true;
    }

    Fd getCapturedFd() const {
        return (Fd)capturedFd_;
    }
private:
    int copyFd_{-1};
    int capturedFd_{-1};
    std::string tmpFile_;
};

class StdoutCapture : public FdCapture {
public:
    StdoutCapture() : FdCapture(FdCapture::kStdoutFd) {
    }
};

class StderrCapture : public FdCapture {
public:
    StderrCapture() : FdCapture(FdCapture::kStderrFd) {
    }
};

template <typename T>
class ObserverPtr {
public:
    ObserverPtr() = default;
    ObserverPtr(std::nullptr_t) { }
    ObserverPtr(T * ptr) : raw_ptr_(ptr) { }

    explicit operator bool() const {
        return !is_null();
    }
    
    T * operator-> () const {
        assert_raw_ptr_is_not_null();
        return raw_ptr_;
    }

    T & operator* () const {
        assert_raw_ptr_is_not_null();
        return *raw_ptr_;
    }

    T * get() const {
        assert_raw_ptr_is_not_null();
        return raw_ptr_;
    }

    bool is_null() const {
        return raw_ptr_ != nullptr;
    }
private:
    void assert_raw_ptr_is_not_null() const {
        PGZXB_DEBUG_ASSERT(raw_ptr_ != nullptr);
    }

    T *raw_ptr_{nullptr};
};

// TODO: Add friend functions for ObserverPtr (std::hash, operators(==, <, <=, ...))

}  // namespace util
}  // namespace pgimpl

namespace pgutil {

using pgimpl::util::ParseCmdConfig;
using pgimpl::util::RaiiCleanup;
using pgimpl::util::FdCapture;
using pgimpl::util::StdoutCapture;
using pgimpl::util::StderrCapture;
using pgimpl::util::AllwaysTrue;
using pgimpl::util::ObserverPtr;

using pgimpl::util::cStrToU32;
using pgimpl::util::cStrToU64;
using pgimpl::util::ceilDivide;
using pgimpl::util::clz;
using pgimpl::util::ctz;
using pgimpl::util::popcnt;
using pgimpl::util::visitParamPackage;
using pgimpl::util::parseCmdSimply;

}  // namespace pgutil

#endif // !PGZXB_PGUTILS_H
