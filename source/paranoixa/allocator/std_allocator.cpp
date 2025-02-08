#include "std_allocator.hpp"

namespace paranoixa {
StdAllocator::StdAllocator(const std::size_t &size) {}

StdAllocator::~StdAllocator() {}

void *StdAllocator::Allocate(const std::size_t &size) { return malloc(size); }

void StdAllocator::Free(void *ptr, const std::size_t &size) { if(ptr)free(ptr); }
} // namespace paranoixa