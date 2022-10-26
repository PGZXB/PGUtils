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
    explicit MemRef(std::size_t sizeInBytes);
    MemRef(void * ptr);
    MemRef(const MemRef & other);
    MemRef(MemRef && other);

    ~MemRef();

    MemRef & operator=(const MemRef &other);
    MemRef & operator=(MemRef && other);

    void * get();
    const void * get() const;
    std::size_t getRefCount() const;

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
