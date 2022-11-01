#include "TypeID.h"

namespace pgimpl {
namespace refl {

const TypeID TypeID::kNull = TypeID().TOTEST_setInternalData("");
const TypeID TypeID::kVoid = TypeID().TOTEST_setInternalData("void");

}  // namespace refl
}  // namespace pgimpl
