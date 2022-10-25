#ifndef PGUTILS_PGREFL_TYPEINFO_H
#define PGUTILS_PGREFL_TYPEINFO_H

#include "FieldMetaInfo.h"
#include "FuncMetaInfo.h"

#include <cstddef>
#include <unordered_map>

namespace pgimpl {
namespace refl {

class TypeInfo {
    using FuncIter = std::unordered_multimap<std::string, FuncMetaInfo>::iterator;
    using ConstFuncIter = std::unordered_multimap<std::string, FuncMetaInfo>::const_iterator;
public:
    TypeInfo(std::size_t size);

    template <typename Visitor> // Visitor = void(const std::string &, FieldMetaInfo&)
    void forEachField(Visitor visitor) {
        for (auto &e : fieldMetaInfos_) {
            visitor(e.first, e.second);
        }
    }

    void addOrUpdateField(const std::string & name, const FieldMetaInfo & fieldMetaInfo);
    bool tryRemoveField(const std::string & name);
    FieldMetaInfo * tryGetField(const std::string & name);
    const FieldMetaInfo * tryGetField(const std::string & name) const;

    template <typename Visitor> // Visitor = void(const std::string &, FuncMetaInfo&)
    void forEachFunc(Visitor visitor) {
        for (auto &e : funcMetaInfos_) {
            visitor(e.first, e.second);
        }
    }

    void addFunc(const std::string & name, const FuncMetaInfo & funcMetaInfo);
    bool tryRemoveFuncs(const std::string & name);
    std::pair<FuncIter, FuncIter> getFuncs(const std::string & name);
    std::pair<ConstFuncIter, ConstFuncIter> getFuncs(const std::string & name) const;
private:
    std::size_t size_; // in Bytes
    std::unordered_multimap<std::string, FuncMetaInfo> funcMetaInfos_;
    std::unordered_map<std::string, FieldMetaInfo> fieldMetaInfos_;
};

}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_TYPEINFO_H
