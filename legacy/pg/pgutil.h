#ifndef PGZXB_PGUTILS_H
#define PGZXB_PGUTILS_H

#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <cerrno>
#include <utility>
#include <functional>
#include <unordered_map>

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace pg {
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
    DWORD trailing_zero = 0;

    if (_BitScanForward(&trailing_zero, value)) return (int)(std::uint32_t)trailing_zero;
    else return 32;
}

int __inline clz(std::uint32_t value) {
    DWORD leading_zero = 0;

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

namespace detail {

template <typename VISITOR> // VISITOR : void(SizeType, ARG);
inline void visitParamPackageHelper(VISITOR visitor, std::size_t index) { }

template <typename VISITOR, typename ARG, typename ... ARGS>
inline void visitParamPackageHelper(VISITOR visitor, std::size_t index, ARG &&arg, ARGS &&...args) {
    visitor(index, std::forward<ARG>(arg));
    visitParamPackageHelper(visitor, index + 1, std::forward<ARGS>(args)...);
}

}

template <typename VISITOR, typename ... ARGS>
inline void visitParamPackage(VISITOR visitor, ARGS &&...args) {
    detail::visitParamPackageHelper(visitor, 0, std::forward<ARGS>(args)...);
}

#if __cplusplus >= 201703L
namespace detail {

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

}

template<typename ... TYPES>
class TypeArray {
private:
    using Base = detail::TypeArrayBase<0, TYPES..., detail::TypeArrayInvalidDataType>;
    
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
        if constexpr (std::is_same_v<TAB, detail::TypeArrayBaseInvalidType>)
            return npos;

        if constexpr (std::is_same_v<T, typename TAB::Front>)
            return TAB::INDEX;

        return indexImpl<T, typename TAB::Other>();
    }
};
#endif

struct ParseCmdConfig {
    using ParseCmdCallback = std::function<bool(const char* str)>;
    enum {
        OPTION,
        PARAM,
    } type;
    ParseCmdCallback callbcak;
};

inline void parseCmdSimply(int argc, char* argv[], const std::unordered_map<char, ParseCmdConfig>& cmdConfig) {
    // FIXME: Unsafe
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
        }
    }
}

} // namespace util
} // namespace pg

namespace pgutils {

using pg::util::ParseCmdConfig;

using pg::util::cStrToU32;
using pg::util::cStrToU64;
using pg::util::ceilDivide;
using pg::util::clz;
using pg::util::ctz;
using pg::util::popcnt;
using pg::util::visitParamPackage;
using pg::util::parseCmdSimply;

} // namespace pgutils

#endif // !PGZXB_PGUTILS_H
