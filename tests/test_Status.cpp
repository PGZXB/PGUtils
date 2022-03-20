#define PGZXB_DEBUG
#include "pg/pgfmt.h"
#include "pg/pgtest/pgtest.h"
#include "pg/pgstatus/StatusManager.h"
#include "pg/pgstatus/Status.h"

#include <string>
#include <iostream>

namespace {

enum ErrCode {
    kErrNotFound = pg::status::Status::kMinValidStatusCode,
    kErrInvalidKey,
    kErrInsertRepeatly,
};

}

#define E(e) ((std::uint64_t)e)

static std::string process_err(pg::status::Status &s) {
    if (s == E(kErrNotFound)) return "Not Found";
    else if (s == E(kErrInvalidKey)) {
        return pgfmt::format("Invalid key: {0}", s.get_context<std::string>());
    } else if (s == E(kErrInsertRepeatly)) {
        return pgfmt::format("Insert {0} repeatly", s.get_context<std::string>());
    }
    return "<default-err-string>" + std::to_string(s.get_context<std::uint64_t>());
}

static std::string process_err_2(pg::status::Status &) {
    return "<default-err-string-2>";
}

PGTEST_CASE(test_Status) {
    using namespace pg::status;

    Status status{process_err};

    PGTEST_EXPECT(status.is_ok());
    PGTEST_EXPECT(status.get_code() == Status::kOk);
    
    // Test for Status with internal-callback
    status = E(kErrNotFound);
    PGTEST_EXPECT(status.invoke() == "Not Found");

    status.set_context<std::string>("KEY1");
    status = E(kErrInvalidKey);
    PGTEST_EXPECT(status.get_code() == E(kErrInvalidKey));
    PGTEST_EXPECT(status.get_context<std::string>() == "KEY1");
    PGTEST_EXPECT(status.invoke() == "Invalid key: KEY1");

    status.set_context<std::string>("KEY2");
    status = E(kErrInsertRepeatly);
    PGTEST_EXPECT(status.get_code() == E(kErrInsertRepeatly));
    PGTEST_EXPECT(status.get_context<std::string>() == "KEY2");
    PGTEST_EXPECT(status.invoke() == "Insert KEY2 repeatly");

    status.set_context<int>(111);
    status = status.get_context<int>();
    PGTEST_EXPECT(status.get_code() == 111);
    PGTEST_EXPECT(status.invoke() == "<default-err-string>111");
    status.set_callback(process_err_2);
    PGTEST_EXPECT(status.invoke() == "<default-err-string-2>");
    
    status.set_context<int>(222);
    status = status.get_context<int>();
    PGTEST_EXPECT(status.get_code() == 222);
    PGTEST_EXPECT(status.invoke() == "<default-err-string-2>");
    status.set_callback(process_err);
    PGTEST_EXPECT(status.invoke() == "<default-err-string>222");

    status = 0;
    PGTEST_EXPECT(status.is_ok());
    PGTEST_EXPECT(status.get_code() == Status::kOk);

    // Test for Status with StatusManager
    StatusManger mgr;
    mgr.register_status(E(kErrNotFound), "[Error]Not found", process_err);
    mgr.register_status(E(kErrInvalidKey), "[Error]Invalid key", process_err);
    mgr.register_status(E(kErrInsertRepeatly), "[Error]Insert a key repeatly", process_err);
    status.set_manager(&mgr);

    PGTEST_EXPECT(mgr.get_msg(kErrNotFound) == "[Error]Not found");
    PGTEST_EXPECT(mgr.get_msg(kErrInvalidKey) == "[Error]Invalid key");
    PGTEST_EXPECT(mgr.get_msg(kErrInsertRepeatly) == "[Error]Insert a key repeatly");

    status = E(kErrNotFound);
    PGTEST_EXPECT(status.invoke() == "Not Found");

    status.set_context<std::string>("KEY1");
    status = E(kErrInvalidKey);
    PGTEST_EXPECT(status.get_code() == E(kErrInvalidKey));
    PGTEST_EXPECT(status.get_context<std::string>() == "KEY1");
    PGTEST_EXPECT(status.invoke() == "Invalid key: KEY1");

    status.set_context<std::string>("KEY2");
    status = E(kErrInsertRepeatly);
    PGTEST_EXPECT(status.get_code() == E(kErrInsertRepeatly));
    PGTEST_EXPECT(status.get_context<std::string>() == "KEY2");
    PGTEST_EXPECT(status.invoke() == "Insert KEY2 repeatly");

    status.set_context<int>(111);
    status = status.get_context<int>();
    PGTEST_EXPECT(status.get_code() == 111);
    PGTEST_EXPECT(status.invoke() == "");
    
    mgr.register_status(E(kErrNotFound), "[Error]Not found");
    mgr.register_status(E(kErrInvalidKey), "[Error]Invalid key");
    mgr.register_status(E(kErrInsertRepeatly), "[Error]Insert a key repeatly");

    status = E(kErrNotFound);
    PGTEST_EXPECT(status.invoke() == "[Error]Not found");

    status = E(kErrInvalidKey);
    PGTEST_EXPECT(status.invoke() == "[Error]Invalid key");

    status = E(kErrInsertRepeatly);
    PGTEST_EXPECT(status.invoke() == "[Error]Insert a key repeatly");

    status = 0;
    PGTEST_EXPECT(status.is_ok());
    PGTEST_EXPECT(status.get_code() == Status::kOk);

    return true;
}