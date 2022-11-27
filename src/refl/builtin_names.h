#ifndef PGUTILS_PGREFL_BUILTIN_NAMES_H
#define PGUTILS_PGREFL_BUILTIN_NAMES_H

namespace pgimpl {
namespace refl {
namespace builtin_names {

#define MAKE_BUILTIN_NAME(name) "__builtin_" name;

constexpr const char kConstructorFunctionName[] = MAKE_BUILTIN_NAME("ctor");
constexpr const char kDeconstructorFunctionName[] = MAKE_BUILTIN_NAME("dtor");
constexpr const char kOperatorAssignFunctionName[] = MAKE_BUILTIN_NAME("opAssign");
constexpr const char kOperatorCallFunctionName[] = MAKE_BUILTIN_NAME("opCall");
constexpr const char kOperatorEqFunctionName[] = MAKE_BUILTIN_NAME("opEq");
constexpr const char kOperatorCmpFunctionName[] = MAKE_BUILTIN_NAME("cmp");
constexpr const char kHashcodeFunctionName[] = MAKE_BUILTIN_NAME("hashcode");
constexpr const char kDeepcopyFunctionName[] = MAKE_BUILTIN_NAME("deepcopy");
constexpr const char kToStringFunctionName[] = MAKE_BUILTIN_NAME("toString");

#undef MAKE_BUILTIN_NAME

}  // namespace builtin_names
}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_BUILTIN_NAMES_H
