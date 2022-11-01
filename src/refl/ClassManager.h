#ifndef PGUTILS_PGREFL_CLASSMANAGER_H
#define PGUTILS_PGREFL_CLASSMANAGER_H

#include "ClassMetaInfo.h"

namespace pgimpl {
namespace refl {

class ClassManager {
    using ClassMap = std::unordered_map<ClassID, ClassMetaInfo>;
public:
    static constexpr const char * kGlobalClsMgrName = "PGReflGlobal";

    explicit ClassManager(std::string name);

    // default copy/move ctor
    // default copy/move assignment

    const std::string & getName() const;

    bool tryRegisterClass(const ClassID & classID, ClassMetaInfo classInfo);
    void updateOrRegisterClass(const ClassID & classID, ClassMetaInfo classInfo);
    bool tryRemoveClass(const ClassID & classID);
    ClassMetaInfo * tryGetClass(const ClassID & classID);
    const ClassMetaInfo * tryGetClass(const ClassID & classID) const;

    static ClassManager * getGlobalInstacne();
private:
    std::string name_;
    ClassMap classes_;
};

}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_CLASSMANAGER_H
