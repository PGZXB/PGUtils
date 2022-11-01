#include "ClassManager.h"

namespace pgimpl {
namespace refl {

// static fields
constexpr const char * ClassManager::kGlobalClsMgrName;

ClassManager::ClassManager(std::string name) : name_(std::move(name)) {
}

const std::string & ClassManager::getName() const {
    return name_;    
}

bool ClassManager::tryRegisterClass(const ClassID & classID, ClassMetaInfo classInfo) {
    return classes_.insert({classID, std::move(classInfo)}).second;
}

void ClassManager::updateOrRegisterClass(const ClassID & classID, ClassMetaInfo classInfo) {
    classes_[classID] = std::move(classInfo);
}

bool ClassManager::tryRemoveClass(const ClassID & classID) {
    return classes_.erase(classID) > 0;
}

ClassMetaInfo * ClassManager::tryGetClass(const ClassID & classID) {
    auto iter = classes_.find(classID);
    return iter == classes_.end() ? nullptr : &(iter->second);
}

const ClassMetaInfo * ClassManager::tryGetClass(const ClassID & classID) const {
    return const_cast<ClassManager*const>(this)->tryGetClass(classID);
}

// static functions
ClassManager * ClassManager::getGlobalInstacne() {
    static ClassManager ins{kGlobalClsMgrName};
    return &ins;
}

}  // namespace refl
}  // namespace pgimpl
