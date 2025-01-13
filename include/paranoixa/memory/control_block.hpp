#ifndef PARANOIXA_CONTROL_BLOCK_HPP
#define PARANOIXA_CONTROL_BLOCK_HPP
#include <cstdint>
#include "allocator.hpp"
namespace paranoixa
{
  template <typename T> struct ControlBlock {
  public:
    T *ptr;
    uint32_t size;
    uint32_t refCount;
    uint32_t weakRefCount;
    AllocatorPtr allocator;
  };
}
#endif
