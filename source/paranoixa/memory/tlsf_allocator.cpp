#include <memory/tlsf_allocator.hpp>

#include <tlsf.h>

namespace paranoixa {
TLSFAllocator::TLSFAllocator(const std::size_t &size)
    : mem(nullptr), tlsf(nullptr) {
  mem = malloc(size);
  tlsf = tlsf_create_with_pool(mem, size);
}

TLSFAllocator::~TLSFAllocator() { free(mem); }

void *TLSFAllocator::Allocate(const std::size_t &size) {
  return tlsf_malloc(tlsf, size);
}

void TLSFAllocator::Free(void *ptr, const std::size_t &size) {
  tlsf_free(tlsf, ptr);
}

} // namespace paranoixa