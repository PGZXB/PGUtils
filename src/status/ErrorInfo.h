#ifndef PGUTILS_STATUS_ERRORINFO_H
#define PGUTILS_STATUS_ERRORINFO_H

#include <cstdint>
#include <string>
#include <limits>
#include <functional>

namespace pgimpl {
namespace status {

class Status;

using ErrorCallback = std::function<std::string(Status&)>;

struct ErrorInfo {
    static constexpr std::uint64_t kInvalidCode = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t code{kInvalidCode};
    std::string msg /* {""} */;
    ErrorCallback callback /* {nullptr} */;
};


}  // namespace status
}  // namespace pgimpl

#endif  // !PGUTILS_STATUS_ERRORINFO_H
