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

const MemRef MemRef::null = nullptr;
constexpr std::size_t MemRef::kNullRefCount;

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

MemRef::MemRef(MemRef && other) : unionData_(other.unionData_), managed_(other.managed_) {
    other.unionData_ = 0;
    other.managed_ = false;
}

MemRef & MemRef::operator=(std::nullptr_t) {
    clear();
    return *this;
}

MemRef & MemRef::operator=(const MemRef &other) {
    clear();
    return *(new (this) MemRef(other));
}

MemRef & MemRef::operator=(MemRef && other) {
    clear();
    return *(new (this) MemRef(std::move(other)));
}

bool MemRef::managingMemeory() const {
    return managed_;
}

void * MemRef::get() {
    return managed_ ? managedMem_->mem : rawMem_;
}

const void * MemRef::get() const {
    return const_cast<MemRef * const>(this)->get();
}

std::size_t MemRef::getRefCount() const {
    return managed_ ? managedMem_->refCount : static_cast<std::size_t>(-1);
}

bool MemRef::weakEquals(const MemRef & other) const {
    return get() == other.get();
}

bool MemRef::strongEquals(const MemRef & other) const {
    return weakEquals(other) && managingMemeory() == other.managingMemeory();
}

bool MemRef::operator== (const MemRef & other) const {
    return strongEquals(other);
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
