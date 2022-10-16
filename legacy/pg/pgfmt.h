//
// Created by PGZXB on 2021/4/21
//
// My fmtlib. -- PGZXB(pgzxb@qq.com)
// Quick Start:
//   Use like python format(Placehold: {<index-from-0>:<limit>}):
//     pgfmt::format("{0} + {1} = {3}", 1, 2, 3); // Return: "1 + 2 = 3"
//   Support format STL-Container:
//     std::vector<int> arr = {1, 2, 3, 4, 5};
//     pgfmt::format("arr: {0}", arr); // Return: "arr: [1, 2, 3, 4, 5]"
//   Customize formatting for other types:
//     struct Range { int lo{0}, hi{0}; };
//     namespace pg::fmt { // overload transToString
//     std::string transToString(const Range &e, const std::string & limit) {
//       char left = '[', right = ')';
//       if (limit.size() >= 1) left = limit[0];
//       if (limit.size() >= 2) righ = limit[1];
//       return pgfmt::format("{3}{0}, {1}{4}", e.lo, e.hi, left, right);
//     }
//     }
//     // use:
//     Range r{0, 100};
//     pgfmt::format("{0:[]}", r); // Return: "[0, 100]"
//
// TO-DO:
//   [*] Support change formatting during runrime.
//   [*] Support '{' in fmt-string
// 

#ifndef PGZXB_PGFMT_H
#define PGZXB_PGFMT_H

#include <cerrno>
#include <cstring>
#include <string>
#include <type_traits>
#include <iterator>
#include <memory>
#include <cinttypes>
#include <vector>
#include <string>
#include <algorithm>

#define PGZXB_PGFMT_MIN_CPLUSPLUS 201103L
#if PGZXB_PGFMT_MIN_CPLUSPLUS > __cplusplus
#error "Min __cplusplus: " ## #PGZXB_PGFMT_MIN_CPLUSPLUS
#endif

namespace pg {
namespace fmt {
namespace detail {
#if __cplusplus >= 201703L
    using std::void_t;
#else
    template<typename...> using void_t = void;
#endif
#if __cplusplus >= 201402L
    using std::enable_if_t;
#else
    template<bool B, class T = void>
    using enable_if_t = typename std::enable_if<B,T>::type;
#endif
} // namespace detail

template <typename _Type, typename _Iter = detail::void_t<>>
struct has_iterator : public std::false_type { };

template <typename _Type>
struct has_iterator<_Type, detail::void_t<typename _Type::iterator>>
  : public std::true_type { };

// function-delarations
template <typename _First, typename _Second>
std::string transToString(const std::pair<_First, _Second> & ele, const std::string &);

template<typename _Type>
detail::enable_if_t<has_iterator<_Type>::value, std::string>
  transToString(const _Type& ele, const std::string & limit);

template<typename _Type>
detail::enable_if_t<!has_iterator<_Type>::value, std::string>
  transToString(const _Type& ele, const std::string &);

// function-definitions
inline std::string transToString(const bool & ele, const std::string &) {
    return ele ? "true" : "false";
}

inline std::string transToString(const char & ele, const std::string &) {
    return std::string() + ele;
}

inline std::string transToString(const std::int16_t & ele, const std::string & limit) {
    char buf[20] = { 0 };
    char fmt[10] = { 0 };
    sprintf(fmt, "%%%s" PRId16 "", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const std::uint16_t & ele, const std::string & limit) {
    char buf[20] = { 0 };
    char fmt[10] = { 0 };
    sprintf(fmt, "%%%s" PRIu16 "", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const std::uint8_t & ele, const std::string &) {
    return transToString(static_cast<std::uint16_t>(ele), "");
}

inline std::string transToString(const std::int32_t & ele, const std::string & limit) {
    char buf[20] = { 0 };
    char fmt[10] = { 0 };
    sprintf(fmt, "%%%s" PRId32 "", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const std::uint32_t & ele, const std::string & limit) {
    char buf[20] = { 0 };
    char fmt[10] = { 0 };
    sprintf(fmt, "%%%s" PRIu32 "", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const std::int64_t & ele, const std::string & limit) {
    char buf[40] = { 0 };
    char fmt[20] = { 0 };
    sprintf(fmt, "%%%s" PRId64 "", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const std::uint64_t & ele, const std::string & limit) {
    char buf[40] = { 0 };
    char fmt[20] = { 0 };
    sprintf(fmt, "%%%s" PRIu64 "", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const float & ele, const std::string & limit) {
    char buf[40] = { 0 };
    char fmt[20] = { 0 };
    sprintf(fmt, "%%%sf", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const double & ele, const std::string & limit) {
    char buf[40] = { 0 };
    char fmt[20] = { 0 };
    sprintf(fmt, "%%%slf", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const long double & ele, const std::string & limit) {
    char buf[50] = { 0 };
    char fmt[30] = { 0 };
    sprintf(fmt, "%%%sllf", limit.c_str());
    int len = sprintf(buf, fmt, ele);
    return std::string().assign(buf, len);
}

inline std::string transToString(const std::string & ele, const std::string & limit) {
    struct HeapCharArrayWrapper {
        HeapCharArrayWrapper(char * arr) : ptr(arr) { }
        ~HeapCharArrayWrapper() { if (ptr != nullptr) delete [] ptr; }
        char * ptr = nullptr;
    };

    HeapCharArrayWrapper buf(new char[1 + (ele.size() << 1)]);
    char fmt[50] = { 0 };

    sprintf(fmt, "%%%ss", limit.c_str());
    sprintf(buf.ptr, fmt, ele.c_str());
    return std::string(buf.ptr);
}

inline std::string transToString(const char * ele, const std::string & limit) {
    struct HeapCharArrayWrapper {
        HeapCharArrayWrapper(char * arr) : ptr(arr) { }
        ~HeapCharArrayWrapper() { if (ptr != nullptr) delete [] ptr; }
        char * ptr = nullptr;
    };

    const size_t len = std::strlen(ele);
    HeapCharArrayWrapper buf(new char[len << 1]);
    char fmt[50] = { 0 };

    sprintf(fmt, "%%%ss", limit.c_str());
    sprintf(buf.ptr, fmt, ele);
    return std::string(buf.ptr);
}

template <typename _First, typename _Second>
std::string transToString(const std::pair<_First, _Second> & ele, const std::string &) {
    const std::string NULL_STRING;
    return std::string()
        .append(transToString(ele.first, NULL_STRING))
        .append(" : ")
        .append(transToString(ele.second, NULL_STRING));
}

// string, const char*
template<typename _Type>
detail::enable_if_t<has_iterator<_Type>::value, std::string> transToString(
  const _Type& ele, const std::string & limit) {
    typedef typename _Type::const_iterator Iter;
    
    const std::string NULL_STRING;
    std::string res("[");

    for (
        Iter iter = ele.begin(), end = ele.end();
        iter != end;
        ++iter
    )
        res.append( transToString(*iter, NULL_STRING) ).append(", ");  // useless second-param

    if (res.size() > 1) {
        res.pop_back(); // remove unnecessary ','
        res.back() = ']';
    } else {
        res.push_back(']');
    }
    return res;
}

template<typename _Type>
detail::enable_if_t<!has_iterator<_Type>::value, std::string> transToString(const _Type& ele, const std::string &) {  // temp-function to show
    return "<default-pgfmt-string>";
}

namespace detail {
// function-delaration(s)
std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> 
    parseBracket(const std::string & str, const char * leftBracket, const char * rightBracket);

template<typename..._Args>
std::vector<std::string> parseArgs(
    const std::vector<std::string>& limits, const _Args&... args);

template<typename _First, typename..._Args>
void parseArgsImpl(
    std::vector<std::string> & res, 
    const std::vector<std::string>& limits, 
    std::vector<std::string>::const_iterator & limStrIter,
    const _First& fArg, const _Args&... args);

void parseArgsImpl(
    std::vector<std::string> & res, 
    const std::vector<std::string> & limits, 
    std::vector<std::string>::const_iterator & limStrIter);

// function-definitions
template<typename..._Args>
std::vector<std::string> parseArgs(
    const std::vector<std::string>& limits, const _Args&... args) {
    std::vector<std::string> res;
    std::vector<std::string>::const_iterator iter = limits.begin();
    parseArgsImpl(res, limits, iter, args...);

    return res;
}

template<typename _First, typename..._Args>
void parseArgsImpl(
    std::vector<std::string> & res, 
    const std::vector<std::string>& limits, 
    std::vector<std::string>::const_iterator & limStrIter,
    const _First& fArg, const _Args&... args
    ) {
    res.push_back(transToString(fArg, *limStrIter));
    ++limStrIter;
    if (limStrIter == limits.end()) return;
    parseArgsImpl(res, limits, limStrIter, args...);
}

inline void parseArgsImpl(
    std::vector<std::string> & res,
    const std::vector<std::string>& limits,
    std::vector<std::string>::const_iterator & limStrIter
    ) { }

inline std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> 
  parseBracket(const std::string & str, const char * leftBracket, const char * rightBracket) {
    std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> res;

    size_t pos = 0, npos = std::string::npos;
    std::string::const_iterator begin = str.begin();
    while (true) {
        size_t beginPos = str.find(leftBracket, pos);
        if (beginPos == npos) break;
        size_t endPos = str.find(rightBracket, beginPos);
        if (endPos == npos) break;
        res.push_back({begin + beginPos + 1, begin + endPos});
        pos = endPos;
    }
    return res;
}

template <typename Result>
void pushStringIntoBracket(
  Result & res,
  const std::string & fmt,
  const std::vector<std::string> & contents,
  const std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>> & ranges,
  const std::vector<int> & nos) {
    res.append(fmt.begin(), ranges.front().first - 1);
    ssize_t cSize = contents.size();
    ssize_t idx = 0;
    for (ssize_t i = 0; i < static_cast<ssize_t>(nos.size()) - 1; ++i) {
        idx = nos.at(i);
        if (idx < cSize) res.append(contents.at(idx));
        res.append(ranges.at(i).second + 1, ranges.at(i + 1).first - 1);
    }
    if ((idx = nos.back()) < cSize) res.append(contents.at(idx));
    res.append(ranges.back().second + 1, fmt.end());
}

} // namespace detail

template<typename...Args>
std::string format(const std::string & fmt, Args&& ...args) {
    typedef std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>::iterator Iter;
    
    std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>
        contentRangeInBracket = detail::parseBracket(fmt, "{", "}");
    if (contentRangeInBracket.empty()) return fmt;

    std::vector<std::string> limits;
    std::vector<int> nos;
    
    for (
        Iter iter = contentRangeInBracket.begin(), end = contentRangeInBracket.end();
        iter != end; ++iter) {
        
        std::string::const_iterator partiIter =  std::find(iter->first, iter->second, ':');
        nos.push_back(std::atoi(fmt.c_str() + (iter->first - fmt.begin())));
        if (partiIter != iter->second)
            limits.push_back(std::string().assign(partiIter + 1, iter->second));
        else limits.emplace_back();
    }
    
    std::vector<std::string> contents =  detail::parseArgs(limits, args...);

    std::string res;
    res.append(fmt.begin(), contentRangeInBracket.front().first - 1);
    ssize_t cSize = contents.size();
    ssize_t idx = 0;
    for (ssize_t i = 0; i < static_cast<ssize_t>(nos.size()) - 1; ++i) {
        idx = nos.at(i);
        if (idx < cSize) res.append(contents.at(idx));
        res.append(contentRangeInBracket.at(i).second + 1, contentRangeInBracket.at(i + 1).first - 1);
    }
    if ((idx = nos.back()) < cSize) res.append(contents.at(idx));
    res.append(contentRangeInBracket.back().second + 1, fmt.end());

    return res;
}

template<typename _Res, typename..._Args>
void formatAppend(_Res & res, const std::string & fmt, _Args&& ...args) {
    typedef std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>::iterator Iter;
    
    std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>
        contentRangeInBracket = detail::parseBracket(fmt, "{", "}");
    if (contentRangeInBracket.empty()) { res = fmt; return; }

    std::vector<std::string> limits;
    std::vector<int> nos;
    
    for (
        Iter iter = contentRangeInBracket.begin(), end = contentRangeInBracket.end();
        iter != end; ++iter) {
        
        std::string::const_iterator partiIter =  std::find(iter->first, iter->second, ':');
        nos.push_back(std::atoi(fmt.c_str() + (iter->first - fmt.begin())));
        if (partiIter != iter->second)
            limits.push_back(std::string().assign(partiIter + 1, iter->second));
        else limits.emplace_back();
    }
    
    std::vector<std::string> contents =  detail::parseArgs(limits, args...);

    detail::pushStringIntoBracket(res, fmt, contents, contentRangeInBracket, nos);
}

} // namespace fmt
} // namespace pg

namespace pgfmt {

using ::pg::fmt::format;
using ::pg::fmt::formatAppend;

} // namespace pgfmt

#endif // !PGZXB_PGFMT_H
