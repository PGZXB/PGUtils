#define PGZXB_DEBUG
#include "pg/pghfmt.h"
#include "pg/pgtest.h"
#include "pg/pgstatus.h"

#include <string>
#include <iostream>

namespace {

using pgstatus::Status;
using pgstatus::ErrorManager;

enum ErrCode {
    kErrNotFound = Status::kMinValidStatusCode,
    kErrInvalidKey,
    kErrInsertRepeatly,
    kMaxErrCode
};

static_assert(kMaxErrCode < Status::kMaxValidStatusCode, "");

}

#define E(e) ((std::uint64_t)e)

static std::string process_err(Status &s) {
    if (s == E(kErrNotFound)) return "Not Found";
    else if (s == E(kErrInvalidKey)) {
        PGZXB_DEBUG_ASSERT(s.getRawContext());
        return pghfmt::format("Invalid key: {0}", s.getContext<std::string>());
    } else if (s == E(kErrInsertRepeatly)) {
        PGZXB_DEBUG_ASSERT(s.getRawContext());
        return pghfmt::format("Insert {0} repeatly", s.getContext<std::string>());
    }
    PGZXB_DEBUG_ASSERT(s.getRawContext());
    return "<default-err-string>" + std::to_string(s.getContext<int>());
}

static std::string process_err_2(Status &) {
    return "<default-err-string-2>";
}

PGTEST_CASE(pgstatus) {
    Status status{process_err};

    PGTEST_EXPECT(status.isOk());
    PGTEST_EQ(status.code(), Status::kOk);
    
    // Test for Status with internal-callback
    status = E(kErrNotFound);
    PGTEST_EQ(status.invoke(), "Not Found");

    status.makeContext<std::string>("KEY1");
    status = E(kErrInvalidKey);
    PGTEST_EQ(status.code(), E(kErrInvalidKey));
    PGTEST_EQ(status.getContext<std::string>(), "KEY1");
    PGTEST_EQ(status.invoke(), "Invalid key: KEY1");

    status.makeContext<std::string>("KEY2");
    status = E(kErrInsertRepeatly);
    PGTEST_EQ(status.code(), E(kErrInsertRepeatly));
    PGTEST_EQ(status.getContext<std::string>(), "KEY2");
    PGTEST_EQ(status.invoke(), "Insert KEY2 repeatly");

    status.makeContext<int>(111);
    status = status.getContext<int>();
    PGTEST_EQ(status.code(), 111);
    PGTEST_EQ(status.invoke(), "<default-err-string>111");
    status.setCallback(process_err_2);
    PGTEST_EQ(status.invoke(), "<default-err-string-2>");
    
    status.makeContext<int>(222);
    status = status.getContext<int>();
    PGTEST_EQ(status.code(), 222);
    PGTEST_EQ(status.invoke(), "<default-err-string-2>");
    status.setCallback(process_err);
    PGTEST_EQ(status.invoke(), "<default-err-string>222");

    status = 0; // 0 == OK
    PGTEST_EXPECT(status.isOk());
    PGTEST_EQ(status.code(), Status::kOk);

    // // Test for Status with ErrorManager
    // ErrorManager mgr;
    // PGTEST_EQ(true, mgr.tryRegisterError(E(kErrNotFound), "[Error]Not found", process_err));
    // PGTEST_EQ(true, mgr.tryRegisterError(E(kErrInvalidKey), "[Error]Invalid key", process_err));
    // PGTEST_EQ(true, mgr.tryRegisterError(E(kErrInsertRepeatly), "[Error]Insert a key repeatly", process_err));
    // PGTEST_EQ(false, mgr.tryRegisterError(E(kErrInsertRepeatly), "[Error]Insert a key repeatly", process_err));
    // status.setManager(&mgr);

    // PGTEST_EQ(mgr.tryGetErrorInfo(kErrNotFound)->msg, "[Error]Not found");
    // PGTEST_EQ(mgr.tryGetErrorInfo(kErrInvalidKey)->msg, "[Error]Invalid key");
    // PGTEST_EQ(mgr.tryGetErrorInfo(kErrInsertRepeatly)->msg, "[Error]Insert a key repeatly");

    // status = E(kErrNotFound);
    // PGTEST_EQ(status.invoke(), "Not Found");

    // status.makeContext<std::string>("KEY1");
    // status = E(kErrInvalidKey);
    // PGTEST_EQ(status.code(), E(kErrInvalidKey));
    // PGTEST_EQ(status.getContext<std::string>(), "KEY1");
    // PGTEST_EQ(status.invoke(), "Invalid key: KEY1");

    // status.makeContext<std::string>("KEY2");
    // status = E(kErrInsertRepeatly);
    // PGTEST_EQ(status.code(), E(kErrInsertRepeatly));
    // PGTEST_EQ(status.getContext<std::string>(), "KEY2");
    // PGTEST_EQ(status.invoke(), "Insert KEY2 repeatly");

    // status.makeContext<int>(111);
    // auto o = status.getContext<int>();
    // PGTEST_EQ(o, 111);
    // status = o;
    // return true;
    // PGTEST_EQ(status.code(), 111);
    // PGTEST_EQ(status.invoke(), "");
    
    // const std::string msg1 = "[Error]Not found";
    // const std::string msg2 = "[Error]Invalid key";
    // const std::string msg3 = "[Error]Insert a key repeatly";
    // mgr.tryUpdateError(E(kErrNotFound), &msg1, nullptr);
    // mgr.tryUpdateError(E(kErrInvalidKey), &msg2, nullptr);
    // mgr.tryUpdateError(E(kErrInsertRepeatly), &msg3, nullptr);

    // status = E(kErrNotFound);
    // PGTEST_EQ(status.invoke(), "[Error]Not found");

    // status = E(kErrInvalidKey);
    // PGTEST_EQ(status.invoke(), "[Error]Invalid key");

    // status = E(kErrInsertRepeatly);
    // PGTEST_EQ(status.invoke(), "[Error]Insert a key repeatly");

    // status = 0; // 0 == OK
    // PGTEST_EXPECT(status.isOk());
    // PGTEST_EQ(status.code(), Status::kOk);

    return true;
}