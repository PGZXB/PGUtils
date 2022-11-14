// PGZXB's human-friendly fmtlib (pghfmt)
// Quick Start:
//   Use like python format(Placehold: {<index-from-0>:<limit>}):
//     pgfmt::format("{0} + {1} = {3}", 1, 2, 3); // Return: "1 + 2 = 3"
//   Support format STL-Container:
//     std::vector<int> arr = {1, 2, 3, 4, 5};
//     pgfmt::format("arr: {0}", arr); // Return: "arr: [1, 2, 3, 4, 5]"
//   Customize formatting for other types:
//     struct Range { int lo{0}, hi{0}; };
//     namespace pgimpl { 
//     namespace hfmt {
//       // overload transToString
//       std::string transToString(const Range &e, const std::string & limit) {
//         char left = '[', right = ')';
//         if (limit.size() >= 1) left = limit[0];
//         if (limit.size() >= 2) righ = limit[1];
//         return pgfmt::format("{3}{0}, {1}{4}", e.lo, e.hi, left, right);
//       }
//     }  // namespace hfmt
//     }  // namespace pgimpl
//     // usage:
//     Range r{0, 100};
//     pgfmt::format("{0:[]}", r); // Return: "[0, 100]"
//
// TO-DO:
//   [ ] Support change formatting during runtime.
//   [ ] Support '{' in fmt-string
// 

#ifndef PGZXB_PGHFMT_H
#define PGZXB_PGHFMT_H

#include <string>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstring>

#define PGZXB_PGFMT_MIN_CPLUSPLUS 201103L
#if PGZXB_PGFMT_MIN_CPLUSPLUS > __cplusplus
#error "Min __cplusplus: " ## #PGZXB_PGFMT_MIN_CPLUSPLUS
#endif

namespace pgimpl { // implementation of PGZXB's lib
namespace hfmt {
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

template <typename _Type, typename _Iter = void_t<>>
struct has_iterator : public std::false_type { };

template <typename _Type>
struct has_iterator<_Type, void_t<typename _Type::iterator>>
  : public std::true_type { };

template <typename _First, typename _Second>
std::string transToString(const std::pair<_First, _Second> & ele, const std::string &);

template<typename _Type>
enable_if_t<has_iterator<_Type>::value, std::string>
  transToString(const _Type& ele, const std::string & limit);

template<typename _Type>
enable_if_t<!has_iterator<_Type>::value, std::string>
  transToString(const _Type& ele, const std::string &);

// bool
inline std::string transToString(const bool & ele, const std::string &) {
    return ele ? "true" : "false";
}

// char, unsigned char
inline std::string transToString(const char & ele, const std::string &) {
    return std::string() + ele;
}

inline std::string transToString(const unsigned char & ele, const std::string & limit) {
    return transToString(static_cast<char>(ele), limit);
}

// short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long
inline std::string transToString(const long long & ele, const std::string & limit) {
    std::string fmt = "%" + limit + "lld";
    std::string buf(128, '\0');
    int len = std::snprintf(&buf[0], buf.size(), fmt.c_str(), ele);
    buf.resize(len);
    return buf;
}

inline std::string transToString(const unsigned long long & ele, const std::string & limit) {
    std::string fmt = "%" + limit + "llu";
    std::string buf(128, '\0');
    int len = std::snprintf(&buf[0], buf.size(), fmt.c_str(), ele);
    buf.resize(len);
    return buf;
}

#define PGHFMT_DEFINE_TTS(type, destType) \
    inline std::string transToString(const type & ele, const std::string & limit) { \
        return transToString(static_cast<destType>(ele), limit); \
    }
PGHFMT_DEFINE_TTS(short, long long);
PGHFMT_DEFINE_TTS(int, long long);
PGHFMT_DEFINE_TTS(long, long long);
PGHFMT_DEFINE_TTS(unsigned short, unsigned long long);
PGHFMT_DEFINE_TTS(unsigned int, unsigned long long);
PGHFMT_DEFINE_TTS(unsigned long, unsigned long long);
#undef PGHFMT_DEFINE_TTS

// float, double
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
    HeapCharArrayWrapper buf(new char[1 + (len * 2)]);
    char fmt[50] = { 0 };

    sprintf(fmt, "%%%ss", limit.c_str());
    sprintf(buf.ptr, fmt, ele);
    return std::string(buf.ptr);
}

template <typename _First, typename _Second>
std::string transToString(const std::pair<_First, _Second> & ele, const std::string & limit) {
    return std::string()
        .append(transToString(ele.first, limit))
        .append(" : ")
        .append(transToString(ele.second, limit));
}

template<typename _Type>
enable_if_t<has_iterator<_Type>::value, std::string> transToString(
  const _Type& ele, const std::string & limit) {
    std::string res("[");

    for (const auto &e : ele) {
        res.append(transToString(e, limit)).append(", ");        
    }

    if (res.size() > 1) {
        res.pop_back(); // remove unnecessary ','
        res.back() = ']';
    } else {
        res.push_back(']');
    }
    return res;
}

template<typename _Type>
enable_if_t<!has_iterator<_Type>::value, std::string> transToString(const _Type& ele, const std::string &) {  // temp-function to show
    return "<default-pghfmt-string>";
}

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
    long cSize = contents.size();
    long idx = 0;
    for (long i = 0; i < static_cast<long>(nos.size()) - 1; ++i) {
        idx = nos.at(i);
        if (idx < cSize) res.append(contents.at(idx));
        res.append(ranges.at(i).second + 1, ranges.at(i + 1).first - 1);
    }
    if ((idx = nos.back()) < cSize) res.append(contents.at(idx));
    res.append(ranges.back().second + 1, fmt.end());
}

template<typename _Res, typename..._Args>
void formatAppend(_Res & res, const std::string & fmt, _Args&& ...args) {
    typedef std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>::iterator Iter;
    
    std::vector<std::pair<std::string::const_iterator, std::string::const_iterator>>
        contentRangeInBracket = parseBracket(fmt, "{", "}");
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
    
    std::vector<std::string> contents = parseArgs(limits, args...);
    pushStringIntoBracket(res, fmt, contents, contentRangeInBracket, nos);
}

template<typename...Args>
std::string format(const std::string & fmt, Args&& ...args) {
    std::string res;
    formatAppend<std::string>(res, fmt, std::forward<Args>(args)...);
    return res;
}

}  // namespace hfmt
}  // namespace pgimpl

namespace pghfmt {

using ::pgimpl::hfmt::format;
using ::pgimpl::hfmt::formatAppend;

}  // namespace pghfmt

#endif // !PGZXB_PGHFMT_H
