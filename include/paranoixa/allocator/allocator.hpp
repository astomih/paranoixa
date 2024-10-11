#ifndef PARANOIXA_ALLOCATOR_HPP
#define PARANOIXA_ALLOCATOR_HPP
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <memory>
namespace paranoixa {

class Allocator {
public:
  virtual ~Allocator() {}
  virtual void *Allocate(const std::size_t &size) = 0;
  virtual void Free(void *ptr, const std::size_t &size) = 0;
};

template <typename T,
          typename = std::enable_if_t<std::is_base_of_v<Allocator, T>>,
          class... Args>
std::shared_ptr<T> MakeAllocatorPtr(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}
using AllocatorPtr = std::shared_ptr<Allocator>;

template <typename T> class StdAllocator : public std::allocator<T> {
public:
  using value_type = T;
  StdAllocator(AllocatorPtr allocator) : allocator(allocator) {}
  template <typename U>
  StdAllocator(const StdAllocator<U> &other) : allocator(other.allocator) {}

  T *allocate(std::size_t n) {
    return reinterpret_cast<T *>(allocator->Allocate(n * sizeof(T)));
  }
  void deallocate(T *p, std::size_t size) noexcept { allocator->Free(p, size); }

  AllocatorPtr allocator;
};
} // namespace paranoixa
#endif // PARANOIXA_ALLOCATOR_HPP