#include "ClassBuilder.h"
#include "ClassManager.h"
#include "../pghfmt.h"

namespace pgimpl {
namespace refl {

#if defined(PGZXB_DEBUG)
#define errorReturnEx(code, ret) \
    return setDebuggingInfo((DebuggingInfo*)debuggingContext_, __FILE__, __LINE__), \
        (status_ = (std::uint64_t)(code)), (ret)
#define okReturnEx(ret) \
    return setDebuggingInfo((DebuggingInfo*)debuggingContext_, __FILE__, __LINE__), \
        (status_ = (std::uint64_t)Err::kNone), (ret)
#else
#define okReturnEx(ret) return (status_ = (std::uint64_t)Err::kNone), (ret)
#define errorReturnEx(code, ret) return (status_ = (std::uint64_t)(code)), (ret)
#endif // !PGZXB_DEBUG
#define errorReturn(code) errorReturnEx(code, *this)
#define okReturn() okReturnEx(*this)

using Err = ClassBuilder::Error;

namespace {

#if defined(PGZXB_DEBUG)
struct DebuggingInfo {
    const char * filename{""};
    std::size_t lineno{0};
};
#endif // !PGZXB_DEBUG

}  // namespace

// internal functions
static void setDebuggingInfo(DebuggingInfo * d, const char *filename, std::size_t lineno) {
    d->filename = filename;
    d->lineno = lineno;
}

static std::string debuggingProcErrOfClassBuilder(pgstatus::Status & s) {
    static const char * MSG_TABLE[(std::size_t)ClassBuilder::Error::kMaxErrorEnumVal] = {
        "", //
        "[WARN] Add field with same name", // kAddFieldWithSameName
        // kAddFuncWithSameSignature
        // kAddClassWithSameClassID
    };
    PGZXB_DEBUG_ASSERT(s.code() < (sizeof(MSG_TABLE) / sizeof(*MSG_TABLE)));
    const char * msg = MSG_TABLE[s.code()];
#if defined(PGZXB_DEBUG)
    auto * dbgInfo = s.getContext<DebuggingInfo*>();
    PGZXB_DEBUG_ASSERT(dbgInfo);
    auto dbgMsg = pghfmt::format("{0}:{1}: msg='{2}'",
                            dbgInfo->filename, dbgInfo->lineno, msg);
    std::cerr << dbgMsg << '\n';
    return dbgMsg;
#else
    return msg;    
#endif // !PGZXB_DEBUG
}

static pgstatus::ErrorManager * getErrorManagerOfClassBuilder() {
    static status::ErrorManager mgr;
    for (unsigned i = (unsigned)Err::kMinErroEnumVal; i < (unsigned)Err::kMaxErrorEnumVal; ++i) {
        auto ok = mgr.tryRegisterError(i, "", debuggingProcErrOfClassBuilder);
        PGZXB_DEBUG_ASSERT(ok);
    }
    return &mgr;    
}

// Core & Stable APIs
ClassBuilder::ClassBuilder() {
#if defined(PGZXB_DEBUG)
    debuggingContext_ = new DebuggingInfo{};
    status_.makeContext<DebuggingInfo*>((DebuggingInfo*)debuggingContext_);
#endif // !PGZXB_DEBUG
    status_.setManager(ClassBuilder::getErrorManager());
}

ClassBuilder & ClassBuilder::id(const ClassID & classID) {
    classID_ = classID;
    return *this;
}

ClassBuilder & ClassBuilder::memSize(std::size_t sizeInBytes) {
    buildingClassInfo_.memMetaInfo.size = sizeInBytes;
    return *this;
}

ClassBuilder & ClassBuilder::addField(
  const std::string & name,
  const TypeID & type,
  const UniformFunc & getter,
  const UniformFunc & setter) {
    auto & fields = buildingClassInfo_.fields;
    auto iter = fields.find(name);
    if (iter != fields.end()) errorReturn(Err::kAddFieldWithSameName);

    FieldMetaInfo info;
    auto & set = info.setter;
    auto & get = info.getter;
    info.type = type;
    set.paramTypes = {type};
    set.returnType = TypeID::kVoid;
    set.func = setter;
    get.paramTypes = {};
    get.returnType = type;
    get.func = getter;
    auto ret = fields.insert({name, std::move(info)});
    PGZXB_DEBUG_ASSERT(ret.second);

    okReturn();
}

ClassBuilder & ClassBuilder::addFunction(
  const std::string & name, 
  const TypeID & retType,
  const std::vector<TypeID> & paramTypes,
  const UniformFunc & fn) {
    auto & funcs = buildingClassInfo_.funcs;
    auto iterPair = funcs.equal_range(name);
    for (auto iter = iterPair.first, end = iterPair.second; iter != end; ++iter) {
        if (iter->second.paramTypes == paramTypes) errorReturn(Err::kAddFuncWithSameSignature);
    }

    FuncMetaInfo info;
    info.paramTypes = paramTypes;
    info.returnType = retType;
    info.func = fn;
    funcs.insert({name, std::move(info)});

    okReturn();
}

bool ClassBuilder::commit(ClassManager *mgr) {
    auto ok = mgr->tryRegisterClass(classID_, std::move(buildingClassInfo_));
    if (!ok) errorReturnEx(Err::kAddClassWithSameClassID, false);
    okReturnEx(true);    
}

const pgstatus::Status & ClassBuilder::getCurrentStatus() const {
    return status_;
}


// private functions
pgstatus::ErrorManager * ClassBuilder::getErrorManager() {
    static pgstatus::ErrorManager * ins{getErrorManagerOfClassBuilder()};
    return ins;
}

}  // namespace refl
}  // namespace pgimpl
