#include "tlsf_allocator.hpp"

#include <tlsf.h>

namespace paranoixa {
TLSFAllocator::TLSFAllocator(const std::size_t &size)
    : mem(nullptr), tlsf(nullptr) {
  mem = malloc(size);
  tlsf = tlsf_create_with_pool(mem, size);
}

TLSFAllocator::~TLSFAllocator() { free(mem); }

void *TLSFAllocator::do_allocate(std::size_t bytes, std::size_t alignment) {
  return tlsf_malloc(tlsf, bytes);
}

void TLSFAllocator::do_deallocate(void *ptr, std::size_t size,
                                  std::size_t alignment) {
  assert(ptr != nullptr);
  assert(size > 0);
  tlsf_free(tlsf, ptr);
}

} // namespace paranoixa