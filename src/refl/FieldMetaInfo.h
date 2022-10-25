#ifndef PGUTILS_PGREFL_FieldMetaInfo_H
#define PGUTILS_PGREFL_FieldMetaInfo_H

#include "FuncMetaInfo.h"
#include "TypeID.h"

namespace pgimpl {
namespace refl {

class ManagedObject;

class FieldMetaInfo {
    using Obj = ManagedObject;
public:
    FieldMetaInfo(const FuncMetaInfo &setter, const FuncMetaInfo & getter);

    void set(Obj * receiver, const Obj & val) const;
    Obj get(const Obj * receiver) const;
private:
    TypeID type_;
    FuncMetaInfo setter_;
    FuncMetaInfo getter_;
};

}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_FieldMetaInfo_H
