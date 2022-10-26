#include "MemRef.h"

namespace pgimpl {
namespace refl {
namespace utils {

struct ManagedMem {
    int refCount{0};
    unsigned char mem[1];
    static_assert(sizeof(*mem) == 1, "");
};

static ManagedMem * managedMalloc(std::size_t sizeInBytes) {
    std::size_t extSize = 0;
    if (sizeInBytes > sizeof(ManagedMem::mem)) {
        extSize = sizeInBytes - sizeof(ManagedMem);
    }
    ManagedMem * p = (ManagedMem*)std::malloc(sizeof(ManagedMem) + extSize);
    return p;
}

MemRef::MemRef(std::size_t sizeInBytes) : managedMem_(managedMalloc(sizeInBytes)), managed_(true) {
    PGZXB_DEBUG_ASSERT(managed_ && managedMem_->refCount == 0);
    ++managedMem_->refCount;
}

MemRef::MemRef(void * ptr) : rawMem_(ptr), managed_(false) {
}

MemRef::MemRef(const MemRef & other) : unionData_(other.unionData_), managed_(other.managed_) {
    if (managed_) {
        PGZXB_DEBUG_ASSERT(managedMem_);
        PGZXB_DEBUG_ASSERT(other.managedMem_ == managedMem_);
        ++managedMem_->refCount;
    }
}

MemRef::MemRef(MemRef && other) = default;

MemRef & MemRef::operator=(const MemRef &other) {
    clear();
    return *(new (this) MemRef(other));
}

MemRef & MemRef::operator=(MemRef && other) = default;

void * MemRef::get() {
    return managed_ ? managedMem_->mem : rawMem_;
}

const void * MemRef::get() const {
    return const_cast<MemRef * const>(this)->get();
}

std::size_t MemRef::getRefCount() const {
    return managed_ ? managedMem_->refCount : static_cast<std::size_t>(-1);
}

MemRef::~MemRef() {
    clear();
}

void MemRef::clear() {
    PGZXB_DEBUG_ASSERT(!managed_ || managedMem_->refCount > 0);
    if (managed_ && --managedMem_->refCount == 0) {
        std::free(managedMem_);
    }
    managed_ = false;
    unionData_ = 0;
}

}  // namespace utils
}  // namespace refl
}  // namespace pgimpl
