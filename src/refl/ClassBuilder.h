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

    // Core & Stable APIs
    
    ClassBuilder & id(const ClassID & classID);

    ClassBuilder & memSize(std::size_t sizeInBytes);
    
    ClassBuilder & addField(
        const std::string & name,
        const TypeID & type,
        const UniformFunc & getter,
        const UniformFunc & setter);
    
    ClassBuilder & addStaticField(
        const std::string & name,
        const TypeID & type,
        void * ptr, bool readOnly);

    ClassBuilder & addFunction(
        const std::string & name, 
        const TypeID & retType,
        const std::vector<TypeID> & paramTypes,
        const UniformFunc & fn);

    bool commit(ClassManager *mgr);

    // Ext & Helpful APIs

    template <typename C, typename T>
    ClassBuilder & addField(const std::string & name, T C::* fieldPtr) {}

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
    pgstatus::ErrorManager * getErrorManager();

    pgstatus::Status status_;
    ClassID classID_;
    ClassMetaInfo buildingClassInfo_;
};

}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_CLASSBUILDER_H
