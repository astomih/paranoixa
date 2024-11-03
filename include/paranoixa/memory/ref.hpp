#ifndef PARANOIXA_MEMORY_REF_HPP
#define PARANOIXA_MEMORY_REF_HPP
#include "allocator.hpp"
#include "ptr.hpp"
namespace paranoixa
{
template <typename T> class Ref {
public:
  Ref() : controlBlock(nullptr) {}

  Ref(const Ptr<T> &ptr) : controlBlock(ptr.controlBlock) {
    if (controlBlock) {
      controlBlock->weakRefCount++;
    }
  }
  Ref(const UniquePtr<T> &ptr) : controlBlock(ptr.controlBlock) {
    if (controlBlock) {
      controlBlock->weakRefCount++;
    }
  }

  Ref(const Ref<T> &other) : controlBlock(other.controlBlock) {
    if (controlBlock) {
      controlBlock->ptr = other.controlBlock->ptr;
      controlBlock->weakRefCount++;
    }
  }

  Ref &operator=(const Ref<T> &other) {
    if (this != &other) {
      Release();
      controlBlock = other.controlBlock;
      if (controlBlock) {
        controlBlock->weakRefCount++;
      }
    }
    return *this;
  }

  ~Ref() { Release(); }

  bool isValid() const { return controlBlock && controlBlock->refCount > 0; }

  T *operator->() const { return isValid() ? controlBlock->ptr : nullptr; }

private:
  ControlBlock<T> *controlBlock;

  void Release() {
    if (controlBlock && --controlBlock->weakRefCount == 0 &&
        controlBlock->refCount == 0) {
      AllocatorPtr allocator = nullptr;
      controlBlock->allocator.swap(allocator);
      allocator->Free(controlBlock, sizeof(ControlBlock<T>));
      controlBlock = nullptr;
    }
  }
};

}
#endif