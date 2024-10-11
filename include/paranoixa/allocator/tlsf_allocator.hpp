#ifndef PARANOIXA_TLSF_ALLOCATOR_HPP
#define PARANOIXA_TLSF_ALLOCATOR_HPP
#include "allocator.hpp"

namespace paranoixa {
class TLSFAllocator : public Allocator {
public:
  TLSFAllocator(const std::size_t &size);
  ~TLSFAllocator() override;
  void *Allocate(const std::size_t &size) override;
  void Free(void *ptr, const std::size_t &size) override;

private:
  void *mem;
  void *tlsf;
};

} // namespace paranoixa
#endif // PARANOIXA_TLSF_ALLOCATOR_HPP