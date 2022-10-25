#ifndef PGUTILS_PGREFL_FuncMetaInfo_H
#define PGUTILS_PGREFL_FuncMetaInfo_H

#include <vector>

#include "TypeID.h"

namespace pgimpl {
namespace refl {

class ManagedObject;

class FuncMetaInfo {
    using Obj = ManagedObject;
    using ArgList = std::vector<Obj>;
    using Func = std::function<Obj(Obj*, ArgList &args)>;
    using ParamTypes = std::vector<TypeID>;
public:
    FuncMetaInfo(const Func & fn, const ParamTypes & paramTypes, const TypeID & returnType);

    bool match(const ParamTypes & paramTypes) const;
    const TypeID & getReturnType() const;
    const ParamTypes & getParamType() const;

    Obj invoke(Obj *receiver, ArgList &args) const; // member function
    Obj invoke(ArgList &args) const; // static function
private:
    Func func_;
    ParamTypes paramTypes_;
    TypeID returnType_;
};

}  // namesapce refl
}  // namespace pgimpl
#endif // !PGUTILS_PGREFL_FuncMetaInfo_H
