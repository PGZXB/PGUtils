#ifndef PGUTILS_PGREFL_CLASSBUILDER_H
#define PGUTILS_PGREFL_CLASSBUILDER_H

#include "ClassMetaInfo.h"
#include "utils/MemRef.h"

namespace pgimpl {
namespace refl {

class Object {
public:
    using FieldVisitor = std::function<void(const std::string &, const Object &)>;
    using ConstFieldVisitor = std::function<void(const std::string &, const Object &)>;
    using FunctionVisitor = std::function<void(const std::string &, const Object &)>;
    using ConstFunctionVisitor = std::function<void(const std::string &, const Object &)>;

    Object(); // call registered ctor

    Object(const Object & other); // call registered copy ctor
    Object(Object && other); // call registered move ctor

    Object & operator= (const Object & other); // call registered copy assign
    Object & operator= (Object && other); // call registered copy assign

    ~Object(); // call registered dtor

    Object operator() (const std::vector<Object> &args); // call registered operator()
    // Object operator() (const std::vector<Object> &args) const;
    bool operator< (const Object & other) const; // call registered operator<
    bool operator== (const Object & other) const; // call registered operator==
    std::size_t hashcode() const; // call registered hashcode

    Object operator[] (const std::string & name);
    Object getField(const std::string & name);
    Object invoke(const std::string & name, const std::vector<Object> & args);

    void forEachField(const FieldVisitor & vis);
    void forEachField(const ConstFieldVisitor & vis) const;
    void forEachFunc(const FieldVisitor & vis);
    void forEachFunc(const ConstFieldVisitor & vis) const;
private:
    TypeID classID;
};

}  // namespace refl
}  // namespace pgimpl

// Custom std::hash<TypeID>
template<>
struct std::hash<pgimpl::refl::Object> {
    std::size_t operator() (const pgimpl::refl::Object &o) const noexcept {
        return o.hashcode();
    }
};

#endif  // !PGUTILS_PGREFL_CLASSBUILDER_H
