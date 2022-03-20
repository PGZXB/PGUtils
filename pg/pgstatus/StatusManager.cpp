#include "StatusManager.h"
#include "pg/pgstatus/Status.h"
#include <string>

using pg::status::Status;
using pg::status::StatusManger;

void StatusManger::register_status(
    std::uint64_t code,
    const StringArg &msg,
    const std::function<std::string(Status&)> &func) {
    status_info_.resize(code + 1);
    auto &info = status_info_[code];
    info.code = code;
    info.msg = msg;
    info.callback = func;
}

void StatusManger::remove_status(std::uint64_t code) {
    if (code >= status_info_.size()) {
        return;
    }
    auto &info = status_info_[code];
    info.code = StatusInfo::kInvalidCode;
    std::string().swap(info.msg);
    info.callback = nullptr;
}

const std::string &StatusManger::get_msg(std::uint64_t code) const {
    static std::string empty_str /* {""} */;
    if (code >= status_info_.size()) {
        return empty_str;
    }
    if (status_info_[code].code == StatusInfo::kInvalidCode) {
        return empty_str;
    }

    return status_info_[code].msg;
}

std::function<std::string(Status&)> StatusManger::get_callback(std::uint64_t code) const {
    if (code >= status_info_.size()) {
        return nullptr;
    }
    if (status_info_[code].code == StatusInfo::kInvalidCode) {
        return nullptr;
    }

    return status_info_[code].callback;
}

// static members & functions
std::unordered_map<std::string, StatusManger> StatusManger::status_manager_map;

StatusManger *StatusManger::try_get_status_manger(const std::string &name) {
    auto iter = status_manager_map.find(name);
    return iter == status_manager_map.end() ? nullptr : (&iter->second);
}


StatusManger &StatusManger::get_status_manager(const std::string &name) {
    return status_manager_map[name];
}

void StatusManger::remove_status_manager(const std::string &name) {
    auto iter = status_manager_map.find(name);
    if (iter == status_manager_map.end()) return;
    status_manager_map.erase(iter);
}

StatusManger &StatusManger::get_default_status_manager() {
    static StatusManger instance;
    return instance;
}