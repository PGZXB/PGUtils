#ifndef PGUTILS_PGREFL_CLASSMETAINFO_H
#define PGUTILS_PGREFL_CLASSMETAINFO_H

#include <vector>
#include <functional>
#include <unordered_map>

#include "TypeID.h"
#include "utils/MemRef.h"

namespace pgimpl {
namespace refl {

using ClassID = TypeID;

struct MemMetaInfo {
    std::size_t size{0};
    /* std::size_t alignment{0}; */
};

struct FuncMetaInfo {
    using ReturnType = TypeID;
    using ParamTypes = std::vector<TypeID>;
    using ArgList = std::vector<utils::MemRef>;
    using NativeFunc = std::function<utils::MemRef(utils::MemRef, ArgList)>;

    ReturnType returnType;
    ParamTypes paramTypes;
    NativeFunc func{nullptr};
};

struct FieldMetaInfo {
    TypeID type;
    FuncMetaInfo setter;
    FuncMetaInfo getter;
};

struct ClassMetaInfo {
    MemMetaInfo memMetaInfo;
    std::unordered_map<std::string, FieldMetaInfo> fields;
    std::unordered_multimap<std::string, FuncMetaInfo> funcs;
};

}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_CLASSMETAINFO_H
