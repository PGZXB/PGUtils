#include "ClassBuilder.h"
#include "ClassManager.h"

namespace pgimpl {
namespace refl {

// Core & Stable APIs
ClassBuilder & ClassBuilder::id(const ClassID & classID) {
    classID_ = classID;
    return *this;
}

ClassBuilder & ClassBuilder::memSize(std::size_t sizeInBytes) {
    buildingClassInfo_.memMetaInfo.size = sizeInBytes;
    return *this;
}

ClassBuilder & ClassBuilder::addField(
  const std::string & name,
  const TypeID & type,
  const UniformFunc & getter,
  const UniformFunc & setter) {
    
}

ClassBuilder & ClassBuilder::addStaticField(
  const std::string & name,
  const TypeID & type,
  void * ptr, bool readOnly) {

}

ClassBuilder & ClassBuilder::addFunction(
  const std::string & name, 
  const TypeID & retType,
  const std::vector<TypeID> & paramTypes,
  const UniformFunc & fn) {

}

bool ClassBuilder::commit(ClassManager *mgr) {

}

}  // namespace refl
}  // namespace pgimpl
