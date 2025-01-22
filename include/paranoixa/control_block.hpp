#ifndef PARANOIXA_CONTROL_BLOCK_HPP
#define PARANOIXA_CONTROL_BLOCK_HPP
#include "allocator.hpp"
#include <cstdint>
namespace paranoixa {
template <typename T> struct ControlBlock {
public:
  T *ptr;
  uint32_t refCount;
  uint32_t weakRefCount;
  AllocatorPtr allocator;
};
} // namespace paranoixa
#endif
