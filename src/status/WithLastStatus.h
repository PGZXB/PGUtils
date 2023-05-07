#ifndef PGUTILS_STATUS_WITHLASTSTATUS_H
#define PGUTILS_STATUS_WITHLASTSTATUS_H

#include "../pgutil.h"
#include "Status.h"
#include "ErrorInfo.h"
#include "ErrorManager.h"

namespace pgimpl {
namespace status {

template <typename Class>
class WithLastStatus {
    using Status = pgimpl::status::Status;
    using ErrorManager = pgimpl::status::ErrorManager;

    // Required by WithLastStatus (
    //      Class::StatusContext
    //      Class::Error
    //      Class::kPrintWarnings
    //      Class::kErrorManagerName
    //      Class::kErrorInfos
    // )

    // Required by WithLastStatus (Class::StatusContext : struct)
    // Use to: Store context for status
    // Example:
    // struct StatusContext {
    //     std::string argName;
    //     std::string stateName;
    //     std::string operationName;
    // };
    // Use:
    // using StatusContext = typename Class::StatusContext;

    // Required by WithLastStatus (Class::Error: enum/enum-class)
    // Use to: Enumerate error codes
    // Example:
    // enum Error {
    //     kInvalidArgument = Status::kMinValidStatusCode,
    //     kInvalidState,
    //     kInvalidOperation,
    // };
    // using Error = typename Class::Error;
    // Use:
    // static_assert(std::is_enum<Error>::value, "Class::Error must be enum/enum-class");
    
    // Required by WithLastStatus (Class::kPrintWarnings: static constexpr const bool kPrintWarnings)
    // Use to: Enable/disable warnings printing
    // Example:
    // static constexpr const bool kPrintWarnings = true;
    // Use:
    // static constexpr const bool kPrintWarnings = Class::kPrintWarnings; 

    // Required by WithLastStatus (Class::kErrorManagerName: static constexpr const char * const kErrorManagerName)
    // Use to: Register error manager name
    // Example:
    // static constexpr const char * const kErrorManagerName = "<Some name>";
    // Use:
    // static constexpr const char * const kErrorManagerName = Class::kErrorManagerName;

    // Required by WithLastStatus (Class::kErrorInfos: static constexpr const ErrorInfo kErrorInfos[])
    // Use to: Register error infos for the error manager
    // Example:
    // static const ErrorInfo kErrorInfos[] = {
    //     {Error::kInvalidArgument, "Invalid argument", [] (Status &status) {
    //         return "Invalid argument: " + status.getContext<StatusContext>().argName;
    //     }},
    //     {Error::kInvalidState, "Invalid state", [] (Status &status) {
    //         return "Invalid state: " + status.getContext<StatusContext>().stateName;
    //     }},
    //     {Error::kInvalidOperation, "Invalid operation", [] (Status &status) {
    //         return "Invalid operation: " + status.getContext<StatusContext>().operationName;
    //     }},
    // };
    // Use:
    // static const ErrorInfo (&kErrorInfos)[pgutil::cArraySize(Class::kErrorInfos)] = Class::kErrorInfos;

public:
    WithLastStatus() : lastStatus_(&errorManager()) {
        lastStatus_.makeContext<typename Class::StatusContext>();
    }

    const Status &getLastStatus() const {
        return lastStatus_;
    }

    static ErrorManager &errorManager() {
        static ErrorManager &mgr = ErrorManager::makeErrorManagerAndRegisterErrorInfos(
            Class::kErrorManagerName, Class::kErrorInfos, pgutil::cArraySize(Class::kErrorInfos));
        return mgr;
    }
protected:
    void setLastStatusAsOk() {
        lastStatus_.onlySetCode(Status::kOk);
    }

    template <typename ErrorT, typename StatusContextT>
    void setLastStatus(ErrorT error, StatusContextT context, const Status *wrapped = nullptr) {
        PGZXB_DEBUG_ASSERT(lastStatus_.getRawContext() != nullptr);
        lastStatus_.getContext<typename Class::StatusContext>() = std::move(context);
        if (wrapped) {
            lastStatus_.setWrappedStatus(*wrapped, (std::uint64_t)error);
        } else {
            lastStatus_.onlySetCode((std::uint64_t)error);
        }
        if (Class::kPrintWarnings && !lastStatus_.isOk()) {  // FIXME: use if constexpr (C++17) + if
            auto msg = lastStatus_.invoke();
            std::cerr << "\033[33mWARN["<< Class::kErrorManagerName << "] " << msg << "\033[0m" << std::endl;
        }
    }

private:
    Status lastStatus_;
};

}  // namespace status
}  // namespace pgimpl

#endif  //! PGUTILS_STATUS_WITHLASTSTATUS_H