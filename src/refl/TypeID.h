#ifndef PGUTILS_PGREFL_TYPEID_H
#define PGUTILS_PGREFL_TYPEID_H

#include <string>

namespace pgimpl {
namespace refl {

// !!TMP!!
class TypeID {
public:
    static const TypeID kVoid;

    TypeID & TOTEST_setInternalData(const std::string & s) {
        id_ = s;
        return *this;
    }

    std::size_t hashcode() const {
        return std::hash<std::string>()(id_);
    }

    bool operator== (const TypeID &o) const {
        return id_ == o.id_;
    }
private:
    std::string id_;
};

}  // namespace refl
}  // namespace pgimpl

// Custom std::hash<TypeID>
template<>
struct std::hash<pgimpl::refl::TypeID> {
    std::size_t operator() (const pgimpl::refl::TypeID &id) const noexcept {
        return id.hashcode();
    }
};

#endif  // !PGUTILS_PGREFL_TYPEID_H
