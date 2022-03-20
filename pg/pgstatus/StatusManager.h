#ifndef PGZXB_STATUSMANAGER_H
#define PGZXB_STATUSMANAGER_H

#include "pg/pgfwd.h"
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace pg {
namespace status {

class Status;

class StatusManger {
    struct StatusInfo {
        static constexpr std::uint64_t kInvalidCode = std::numeric_limits<std::uint64_t>::max();

        std::uint64_t code{kInvalidCode};
        std::string msg /* {""} */;
        std::function<std::string(Status&)> callback /* {nullptr} */;
    };
public:
    void register_status(
      std::uint64_t code,
      const StringArg &msg,
      const std::function<std::string(Status&)> &func = nullptr);
    void remove_status(std::uint64_t code);
    
    const std::string &get_msg(std::uint64_t code) const;
    std::function<std::string(Status&)> get_callback(std::uint64_t code) const;

    static StatusManger *try_get_status_manger(const std::string &name);
    static StatusManger &get_status_manager(const std::string &name);
    static void remove_status_manager(const std::string &name);
    static StatusManger &get_default_status_manager(); // name: ""
private:
    std::unique_ptr<Status> default_status_{nullptr};
    std::vector<StatusInfo> status_info_;

    static std::unordered_map<std::string, StatusManger> status_manager_map;
};

} // namespace status
} // namespace pg
#endif