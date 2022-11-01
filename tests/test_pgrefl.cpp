#include "refl/ClassBuilder.h"
#include "test/pgtest.h" // Using PGTEST_*

#include <cstring>
#include <string>
#include <iostream> // required by pgtest
#include <utility>
#include <unordered_map>

#include "pgfwd.h"
#include "pghfmt.h"
#include "refl/TypeID.h"
#include "refl/ClassMetaInfo.h"
#include "refl/utils/MemRef.h"

namespace {

template <typename T>
pgimpl::refl::utils::MemRef wrapAsObject(const T & t) {
    return pgimpl::refl::utils::MemRef{const_cast<T*>(&t)};
}

pgimpl::refl::utils::MemRef newObject(
  std::unordered_map<pgimpl::refl::TypeID, pgimpl::refl::ClassMetaInfo> &types,
  const pgimpl::refl::TypeID &typeID) {
    return pgimpl::refl::utils::MemRef(types[typeID].memMetaInfo.size);
}

template <typename T>
pgimpl::refl::FieldMetaInfo buildValueFieldMetaInfo( // Value: special field, getter: copy and seter: assign
  std::unordered_map<pgimpl::refl::TypeID, pgimpl::refl::ClassMetaInfo> &types,
  const pgimpl::refl::TypeID &typeID) {
    using namespace pgimpl::refl;
    using namespace pgimpl::refl::utils;

    FieldMetaInfo result;
    result.type = typeID;  
    auto & setter = result.setter;
    auto & getter = result.getter;

    setter.returnType = TypeID().TOTEST_setInternalData("void");
    setter.paramTypes = {typeID};
    setter.func = [](MemRef obj, FuncMetaInfo::ArgList arglist) -> MemRef {
        // NOTE: The validness of arglist should be checked by pgrefl::Object::invoke
        PGZXB_DEBUG_ASSERT(arglist.size() == 1);
        auto *self = (T*)obj.get();
        auto *arg0 = (T*)arglist[0].get();
        *self = *arg0;
        return MemRef::null;
    };

    getter.returnType = typeID;
    getter.paramTypes = {};
    getter.func = [typeID, &types](MemRef obj, FuncMetaInfo::ArgList arglist) -> MemRef {
        // NOTE: The validness of arglist should be checked by pgrefl::Object::invoke
        PGZXB_DEBUG_ASSERT(arglist.size() == 0);
        auto *self = (T*)obj.get();
        MemRef ret = newObject(types, typeID);
        *(T*)ret.get() = *self;
        return MemRef{std::move(ret)};
    };

    return result;
}

template <typename O, typename T>
pgimpl::refl::FieldMetaInfo buildFieldMetaInfo( // Value: special field, getter: copy and seter: assign
  std::unordered_map<pgimpl::refl::TypeID, pgimpl::refl::ClassMetaInfo> &types,
  const pgimpl::refl::TypeID &typeID,
  T O::* memberPtr, const std::string & name) {
    using namespace pgimpl::refl;
    using namespace pgimpl::refl::utils;

    FieldMetaInfo result;
    result.type = typeID;  
    auto & setter = result.setter;
    auto & getter = result.getter;

    setter.returnType = TypeID().TOTEST_setInternalData("void");
    setter.paramTypes = {typeID};
    setter.func = [memberPtr](MemRef obj, FuncMetaInfo::ArgList arglist) -> MemRef {
        // NOTE: The validness of arglist should be checked by pgrefl::Object::invoke
        PGZXB_DEBUG_ASSERT(arglist.size() == 1);
        auto *self = (O*)obj.get();
        auto *field = &((*self).*memberPtr);
        auto *arg0 = (T*)arglist[0].get();
        *field = *arg0;
        return MemRef::null;
    };

    getter.returnType = typeID;
    getter.paramTypes = {};
    getter.func = [name, memberPtr, typeID, &types](MemRef obj, FuncMetaInfo::ArgList arglist) -> MemRef {
        // NOTE: The validness of arglist should be checked by pgrefl::Object::invoke
        PGZXB_DEBUG_ASSERT(arglist.size() == 0);
        auto *self = (O*)obj.get();
        auto *field = &((*self).*memberPtr);
        MemRef ret = newObject(types, types[typeID].fields[name].type);
        *(T*)ret.get() = *field;
        return MemRef{std::move(ret)};
    };

    return result;
}

template <typename T>
void set(
  std::unordered_map<pgimpl::refl::TypeID, pgimpl::refl::ClassMetaInfo> &types, 
  const pgimpl::refl::TypeID &type,
  pgimpl::refl::utils::MemRef obj, const std::string &name, const T &a) {
    auto &fn = types[type].fields[name].setter.func;
    fn(obj, {wrapAsObject(a)});
} 

template <typename T>
T get(
  std::unordered_map<pgimpl::refl::TypeID, pgimpl::refl::ClassMetaInfo> &types, 
  const pgimpl::refl::TypeID &type,
  pgimpl::refl::utils::MemRef obj, const std::string &name) {
    auto &fn = types[type].fields[name].getter.func;
    auto ret = fn(obj, {});
    return T{std::move(*(T*)ret.get())};
}

}  // namespace

PGTEST_CASE(pgrefl_utils) {
    using pgimpl::refl::utils::MemRef;

    char buffer[sizeof(std::string)] = { 0 };
    void *pMem{nullptr}; PGZXB_UNUSED(pMem);

    {
        MemRef refStackMem{buffer};
        PGTEST_EQ(refStackMem.getRefCount(), (std::size_t)-1);
        auto *pStr = new (refStackMem.get()) std::string{"HELLO WORLD"};
        pStr->append("HELLO WORLD");
        PGTEST_EQ((void*)buffer, (void*)pStr);
        PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
    }
    { // The `buffer` is not managed by refStackMem:MemRef
        std::string *pStr = (std::string*)(void*)buffer;
        PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
    }

    {
        MemRef ref0(sizeof(std::string));
        PGTEST_EQ(ref0.getRefCount(), (std::size_t)1);
        auto *pStr = new (ref0.get()) std::string{"HELLO WORLD"};
        pStr->append("HELLO WORLD");
        PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
        pStr->~basic_string(); // Deconstruct *pStr
        pMem = pStr;
    }
    // { // The `*pMem` is managed by ref0:MemRef
    //     std::string *pStr = (std::string*)(void*)pMem;
    //     // (Maybe) !!Crashed!!
    //     PGTEST_EQ(*pStr, "HELLO WORLDHELLO WORLD");
    // }

    {
        using Bytes = char[1024];
        const char STR[] = "Hello, PGUtils!!";
        MemRef ref0(sizeof(Bytes)), ref0copy = ref0;
        PGTEST_EQ(ref0.getRefCount(), (std::size_t)2);
        PGTEST_EQ(ref0.get(), ref0copy.get());
        {
            MemRef ref1(sizeof(Bytes));
            PGTEST_EQ(ref1.getRefCount(), (std::size_t)1);
            
            char * bytes = (char*)ref1.get();
            std::strcpy(bytes, STR);
            PGTEST_EXPECT(std::strcmp((char*)ref1.get(), STR) == 0);
            
            ref0 = ref1;
            PGTEST_EQ(ref0copy.getRefCount(), (std::size_t)1);;
            PGTEST_EQ(ref1.getRefCount(), (std::size_t)2);
            PGTEST_EQ(ref0.getRefCount(), (std::size_t)2);
            PGTEST_EQ(ref0.get(), ref1.get());
        }
        {
            PGTEST_EQ(ref0.getRefCount(), (std::size_t)1);
            MemRef ref2 = std::move(ref0);
            PGTEST_EQ(ref0.get(), nullptr);
            PGTEST_EQ(ref0.getRefCount(), (std::size_t)-1);
            PGTEST_EQ(ref2.getRefCount(), (std::size_t)1);
            PGTEST_EXPECT(std::strcmp((char*)ref2.get(), STR) == 0);

            pMem = ref2.get();
        }
        // { // The memory pointed by `pMem` has been deallocated
        //     // (Maybe) !!Crashed!!
        //     std::free(std::calloc(4096, 1));
        //     PGTEST_EXPECT(std::strcmp((char*)pMem, STR) == 0);
        // }
        pMem = ref0copy.get();
        PGTEST_EQ(ref0copy.getRefCount(), (std::size_t)1);
        MemRef ref0move = std::move(ref0copy);
        PGTEST_EQ(ref0move.get(), pMem);
        PGTEST_EQ(ref0move.getRefCount(), (std::size_t)1);
        PGTEST_EQ(ref0copy.get(), nullptr);
        PGTEST_EQ(ref0copy.getRefCount(), (std::size_t)-1);

        ref0move = nullptr;
        PGTEST_EQ(ref0move, MemRef::null);
        
        MemRef ref3(1024);
        ref0move = ref3.get();
        PGTEST_EXPECT(!ref0move.managingMemeory());
        PGTEST_EXPECT(ref3.managingMemeory());
        PGTEST_EXPECT(ref0move.weakEquals(ref3));
        PGTEST_EXPECT(!ref0move.strongEquals(ref3));
        PGTEST_EXPECT(!(ref0move == ref3));
    }
}

PGTEST_CASE(pgrefl_TypeMetaInfo) {
    using namespace pgimpl::refl;
    using namespace pgimpl::refl::utils;

    //////////////////////////////////////
    //        structs for Testing       //

    struct Vec2f {
        std::string name;
        char USELESS1[31];
        int x{0};
        char USELESS2[23];
        int y{0};
        char USELESS3[3];

        Vec2f add(const Vec2f &a) {
            Vec2f r;
            r.x = x + a.x;
            r.y = y + a.y;
            return r;
        }
        
        std::pair<int, int> getXY() const {
            return {x, y};
        }

        void printToStdCerr() const {
            std::cerr << pghfmt::format("Vec2f: {0}=({1}, {2})", name, x, y);
        }
    };

    //                                  //
    //////////////////////////////////////
    

    // Build ClassMetaInfo manually & Test
    
    // Simulate pgrefl::TypeManager
    std::unordered_map<TypeID, ClassMetaInfo> types;

    // Type ids
    TypeID voidID = TypeID().TOTEST_setInternalData("void"); 
    TypeID intID = TypeID().TOTEST_setInternalData("int");
    TypeID stdStringID = TypeID().TOTEST_setInternalData("std::string");
    TypeID stdIntPairID = TypeID().TOTEST_setInternalData("std::pair<int,int>");
    TypeID vec2fID = TypeID().TOTEST_setInternalData("Vec2f");

    // Register base type
    { /// void // useless
        types.insert({voidID, {}});
    }
    { /// int
        ClassMetaInfo typeInfo;
        typeInfo.memMetaInfo.size = sizeof(int);        
        typeInfo.fields["value"] = buildValueFieldMetaInfo<int>(types, intID);
        types.insert({intID, std::move(typeInfo)});
    }

    // Register STL type
    { /// std::pair<int, int>
    }
    { /// std::string
    }

    // Resgiter ...
    { /// Vec2f
        ClassMetaInfo typeInfo;
        typeInfo.memMetaInfo.size = sizeof(Vec2f);
        typeInfo.fields["x"] = buildFieldMetaInfo(types, intID, &Vec2f::x, "x");
        typeInfo.fields["y"] = buildFieldMetaInfo(types, intID, &Vec2f::y, "y");
        types.insert({vec2fID, std::move(typeInfo)});
    }

    // Test
    {
        auto i = newObject(types, intID);
        set(types, intID, i, "value", 1024);
        auto iGet = get<int>(types, intID, i, "value");
        PGTEST_EQ(iGet, 1024);

        int j = 1024;
        PGTEST_EQ(j, 1024);
        set(types, intID, wrapAsObject(j), "value", -1000);
        PGTEST_EQ(j, -1000);
        set(types, intID, wrapAsObject(j), "value", 233333);
        PGTEST_EQ(j, 233333);
    }
    {
        auto vec = newObject(types, vec2fID);
        new (vec.get()) Vec2f{};
        set(types, vec2fID, vec, "x", 1024);
        set(types, vec2fID, vec, "y", 2048);
        PGTEST_EQ(get<int>(types, vec2fID, vec, "x"), 1024);
        PGTEST_EQ(get<int>(types, vec2fID, vec, "y"), 2048);
    
        Vec2f vec2;
        set(types, vec2fID, wrapAsObject(vec2), "x", 1024);
        set(types, vec2fID, wrapAsObject(vec2), "y", 2048);
        PGTEST_EQ(vec2.x, 1024);
        PGTEST_EQ(vec2.y, 2048);
        PGTEST_EQ(get<int>(types, vec2fID, wrapAsObject(vec2), "x"), 1024);
        PGTEST_EQ(get<int>(types, vec2fID, wrapAsObject(vec2), "y"), 2048);
    }
}

PGTEST_CASE(pgrefl_ClassBuilder_basic) {
    using namespace pgimpl::refl;

    ClassBuilder builder;
    builder.addField("a", TypeID::kVoid, nullptr, nullptr);
    PGTEST_EQ(builder.getCurrentStatus().code(), (std::uint64_t)ClassBuilder::Error::kNone);
    builder.addField("a", TypeID::kVoid, nullptr, nullptr); // U
    PGTEST_EQ(builder.getCurrentStatus().code(), (std::uint64_t)ClassBuilder::Error::kAddFieldWithSameName);

    return true;
}
