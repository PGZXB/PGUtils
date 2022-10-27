#ifndef PGUTILS_PGREFL_UTILS_MEMREF_H
#define PGUTILS_PGREFL_UTILS_MEMREF_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "../../pgfwd.h"

namespace pgimpl {
namespace refl {
namespace utils {

struct ManagedMem;

class MemRef {
public:
    // MemRef::null: A MemRef which don't point to any memory
    /// The null can be constructed by:
    //// 1. MemRef ref = nullptr; // ref == MemRef::null
    //// 2. MemRef ref{ptr}; ref = nullptr; // ref == MemRef::null
    static const MemRef null;
    // The getRefCount() of MemRef::null
    static constexpr std::size_t kNullRefCount = -1;

    explicit MemRef(std::size_t sizeInBytes);
    MemRef(void * ptr);
    MemRef(const MemRef & other);
    MemRef(MemRef && other);

    ~MemRef();

    MemRef & operator=(std::nullptr_t);
    MemRef & operator=(const MemRef &other);
    MemRef & operator=(MemRef && other);

    bool managingMemeory() const;

    void * get();
    const void * get() const;
    std::size_t getRefCount() const;

    // Return true if `this` point to same memory with `other` else false
    bool weakEquals(const MemRef & other) const;

    // Return true if this->weakEquals(other) && this->managingMemeory() == other.managingMemeory() else false
    bool strongEquals(const MemRef & other) const;

    // Same with strongEquals
    bool operator== (const MemRef & other) const;
private:
    void clear();

    union {
        void * rawMem_;
        ManagedMem * managedMem_;
        std::uintptr_t unionData_{0};
    };
    bool managed_{false};
};

}  // namespace utils
}  // namespace refl
}  // namespace pgimpl
#endif  // !PGUTILS_PGREFL_UTILS_MEMREF_H
