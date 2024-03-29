#ifndef PGUTILS_PGREFL_CLASSBUILDER_H
#define PGUTILS_PGREFL_CLASSBUILDER_H

#include "../status/pgstatus.h"
#include "ClassMetaInfo.h"
#include "TypeID.h"

namespace pgimpl {
namespace refl {

class ClassManager;

class ClassBuilder {
public:
    using UniformFunc = FuncMetaInfo::NativeFunc;

    enum class Error : unsigned {
        kNone = pgstatus::Status::kOk,
        kMinErroEnumVal = pgstatus::Status::kMinValidStatusCode,
        kAddFieldWithSameName = kMinErroEnumVal,
        kAddFuncWithSameSignature,
        kAddClassWithSameClassID,
        kEmptyClassID,
        kZeroMemorySize,
        kBindingNullFunction,
        kMaxErrorEnumVal,
    };
    static_assert((unsigned)Error::kMaxErrorEnumVal < pgstatus::Status::kMaxValidStatusCode, "");

    // Core & Stable APIs
    ClassBuilder();

    ClassBuilder & id(const ClassID & classID);

    ClassBuilder & memSize(std::size_t sizeInBytes);
    
    ClassBuilder & addField(
        const std::string & name,
        const TypeID & type,
        const UniformFunc & getter,
        const UniformFunc & setter);
    
    ClassBuilder & addFunction(
        const std::string & name, 
        const TypeID & retType,
        const std::vector<TypeID> & paramTypes,
        const UniformFunc & fn);

    bool commit(ClassManager *mgr);

    // Ext & Helpful APIs
    template <typename C>
    ClassBuilder & autoBuild() {
        // TODO: Gen TypeID & Set id
        // TODO: Set memSize
        // TODO: Auto add functions (ctors, dtor, operator() ...)
        return *this;
    }

    template <typename C, typename T>
    ClassBuilder & addField(const std::string & name, T C::* fieldPtr) {
        // FIXME: Use TypeID::of<T>() instead of TypeID().TOTEST_setInternalData(typeid(T).name())
        const auto typeID = TypeID().TOTEST_setInternalData(typeid(T).name()); // Tmp
        UniformFunc getter = [fieldPtr](utils::MemRef obj, std::vector<utils::MemRef>) -> utils::MemRef {
            return utils::MemRef{&(( *(C*)obj.get() ).*fieldPtr )};
        };

        UniformFunc setter = [fieldPtr](utils::MemRef obj, std::vector<utils::MemRef> args) -> utils::MemRef {
            PGZXB_DEBUG_ASSERT(args.size() == 1);
            ( *(C*)obj.get() ).*fieldPtr = std::move(*(T*)args[0].get());
            return utils::MemRef::null;
        };

        return addField(name, typeID, getter, setter);
    }

    template <typename C, typename T>
    ClassBuilder & addField(const std::string & name, const T C::* fieldPtr)  {}

    template <typename T>
    ClassBuilder & addField(const std::string & name, T * fieldPtr) {}

    template <typename T>
    ClassBuilder & addField(const std::string & name, const T * fieldPtr) {}

    template <typename C, typename Fn, typename Ret, typename ... Args>
    ClassBuilder & addFunction(const std::string & name, Ret (C::*funcPtr)(Args...)) {}

    template <typename C, typename Fn, typename Ret, typename ... Args>
    ClassBuilder & addFunction(const std::string & name, Ret (*funcPtr)(Args...)) {}

    template <typename Fn>
    ClassBuilder & addFunction(const std::string & name, Fn && func) {}

    // Get current status (Get last error)
    const pgstatus::Status & getCurrentStatus() const;
private:
    static pgstatus::ErrorManager * getErrorManager();

#if defined(PGZXB_DEBUG)
    void * debuggingContext_{nullptr};
#endif  // !PGZXB_DEBUG
    pgstatus::Status status_;
    ClassID classID_;
    ClassMetaInfo buildingClassInfo_;
};

}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_CLASSBUILDER_H
