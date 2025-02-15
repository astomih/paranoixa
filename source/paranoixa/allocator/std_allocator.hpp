#ifndef PARANOIXA_STD_ALLOCATOR_HPP
#define PARANOIXA_STD_ALLOCATOR_HPP
#include "paranoixa.hpp"

namespace paranoixa {
class StdAllocator : public Allocator {
public:
  StdAllocator(const std::size_t &size);
  ~StdAllocator() override;
  void *Allocate(const std::size_t &size) override;
  void Free(void *ptr, const std::size_t &size) override;

private:
};

} // namespace paranoixa
#endif // PARANOIXA_TLSF_ALLOCATOR_HPP