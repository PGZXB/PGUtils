#ifndef PGUTILS_PGREFL_BUILTIN_NAMES_H
#define PGUTILS_PGREFL_BUILTIN_NAMES_H

namespace pgimpl {
namespace refl {
namespace builtin_names {

#define MAKE_BUILTIN_NAME(name) "__builtin_" name;

constexpr const char kConstructorFunctionName[] = MAKE_BUILTIN_NAME("ctor");
constexpr const char kDeconstructorFunctionName[] = MAKE_BUILTIN_NAME("dtor");
constexpr const char kOperatorAssignFunctionName[] = MAKE_BUILTIN_NAME("op_assign");
constexpr const char kOperatorCallFunctionName[] = MAKE_BUILTIN_NAME("op_call");
constexpr const char kOperatorEqFunctionName[] = MAKE_BUILTIN_NAME("op_eq");
constexpr const char kOperatorLtFunctionName[] = MAKE_BUILTIN_NAME("op_lt");
constexpr const char kHashcodeFunctionName[] = MAKE_BUILTIN_NAME("hashcode");

}  // namespace builtin_names
}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_BUILTIN_NAMES_H
