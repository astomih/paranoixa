#ifndef PRANOIXA_MEMORY_PTR_HPP
#define PRANOIXA_MEMORY_PTR_HPP
#include "allocator.hpp"
#include "control_block.hpp"
#include <cassert>
#include <print>
namespace paranoixa {

template <typename T> class Ref;

template <typename T> class DefaultDeleter {
public:
  void Delete(T *ptr) {};
};
template <class T, typename... Args>
void CallConstructor(T *ptr, Args &&...args) {
  new (ptr) T(std::forward<Args>(args)...);
}

template <class T, typename... Args>
T *NewWithConstructor(AllocatorPtr allocator, Args &&...args) {
  T *ptr = reinterpret_cast<T *>(allocator->Allocate(sizeof(T)));
  CallConstructor(ptr, std::forward<Args>(args)...);
  return ptr;
}
// SharedPtr
template <typename T, class Deleter = DefaultDeleter<T>> class Ptr {
public:
  Ptr(std::nullptr_t) : controlBlock(nullptr) {}
  Ptr(AllocatorPtr allocator, T *ptr = nullptr)
      : controlBlock(NewWithConstructor<ControlBlock<T>>(allocator, ptr, 0, 0,
                                                         allocator)) {
    assert(allocator);
    if (controlBlock->ptr) {
      controlBlock->refCount = 1;
    }
  }

  Ptr(const Ptr<T> &other) : controlBlock(other.controlBlock) {
    if (controlBlock->ptr) {
      controlBlock->refCount++;
    }
  }

  template <typename U> Ptr(const Ptr<U> &other) {
    memcpy(this, &other, sizeof(Ptr<T>));
    if (controlBlock->ptr) {
      controlBlock->refCount++;
    }
  }

  Ptr &operator=(const Ptr<T> &other) {
    if (this != &other) {
      Release();
      memcpy(this, &other, sizeof(Ptr<T>));
      if (controlBlock->ptr) {
        controlBlock->refCount++;
      }
    }
    return *this;
  }

  ~Ptr() { Release(); }

  T *operator->() const { return controlBlock->ptr; }

  T &operator*() const { return *(controlBlock->ptr); }

  void Reset(T *newPtr = nullptr) {
    Release();
    if (newPtr) {
      controlBlock = new ControlBlock<T>;
      controlBlock->ptr = newPtr;
      controlBlock->refCount = 1;
    }
  }

  T *Get() const { return controlBlock->ptr; }
  T **GetAddress() { return &controlBlock->ptr; }

private:
  friend class Ref<T>;
  ControlBlock<T> *controlBlock;

  void Release() {
    if (controlBlock && controlBlock->ptr && --controlBlock->refCount == 0) {
      controlBlock->ptr->~T();
      controlBlock->allocator->Free(controlBlock->ptr, sizeof(T));
      controlBlock->ptr = nullptr;
      if (controlBlock->weakRefCount == 0) {
        AllocatorPtr allocator = nullptr;
        controlBlock->allocator.swap(allocator);
        allocator->Free(controlBlock, sizeof(ControlBlock<T>));
        controlBlock = nullptr;
      } else {
        std::println("Weak ref count: {}", controlBlock->weakRefCount);
      }
    } else if (controlBlock && controlBlock->ptr == nullptr &&
               controlBlock->refCount == 0 && controlBlock->weakRefCount == 0) {
      AllocatorPtr allocator = nullptr;
      controlBlock->allocator.swap(allocator);
      allocator->Free(controlBlock, sizeof(ControlBlock<T>));
      controlBlock = nullptr;
    } else if (controlBlock) {
      std::println("ref count: {}", controlBlock->refCount);
      assert(controlBlock->refCount > 0);
    }
  }
};

template <typename T, class... Args>
Ptr<T> MakePtr(AllocatorPtr allocator, Args &&...args) {
  return Ptr<T>{allocator,
                NewWithConstructor<T>(allocator, std::forward<Args>(args)...)};
}

// UniquePtr
template <typename T> class UniquePtr {

public:
  UniquePtr(AllocatorPtr allocator, T *ptr = nullptr)
      : controlBlock(NewWithConstructor<ControlBlock<T>>(allocator, ptr, 0, 0,
                                                         allocator)) {
    assert(allocator);
    if (controlBlock->ptr) {
      controlBlock->refCount = 1;
    }
  }
  UniquePtr(UniquePtr<T> &&other) {
    controlBlock = other.controlBlock;
    other.controlBlock = nullptr;
  }
  template <typename U> UniquePtr(UniquePtr<U> &&other) {
    memcpy(this, &other, sizeof(UniquePtr<T>));
    memset(&other, 0, sizeof(UniquePtr<T>));
  }

  UniquePtr(const UniquePtr<T> &other) = delete;

  UniquePtr &operator=(const UniquePtr<T> &other) = delete;

  UniquePtr &operator=(UniquePtr<T> &&other) {
    if (this != &other) {
      Release();
      controlBlock = other.controlBlock;
      other.controlBlock = nullptr;
    }
    return *this;
  }
  template <typename U> UniquePtr &operator=(UniquePtr<U> &&other) {
    Release();
    memcpy(this, &other, sizeof(UniquePtr<T>));
    memset(&other, 0, sizeof(UniquePtr<T>));
    return *this;
  }

  ~UniquePtr() { Release(); }

  T *operator->() const { return controlBlock->ptr; }

  T &operator*() const { return *(controlBlock->ptr); }

  void Reset(T *newPtr = nullptr) {
    Release();
    if (newPtr) {
      controlBlock = new ControlBlock<T>;
      controlBlock->ptr = newPtr;
      controlBlock->refCount = 1;
    }
  }

private:
  friend class Ref<T>;
  ControlBlock<T> *controlBlock;

  void Release() {
    if (controlBlock && controlBlock->ptr && --controlBlock->refCount == 0) {
      controlBlock->ptr->~T();
      controlBlock->allocator->Free(controlBlock->ptr, sizeof(T));
      controlBlock->ptr = nullptr;
      if (controlBlock->weakRefCount == 0) {
        AllocatorPtr allocator = nullptr;
        controlBlock->allocator.swap(allocator);
        allocator->Free(controlBlock, sizeof(ControlBlock<T>));
        controlBlock = nullptr;
      } else {
        std::cout << "weakRef count: " << controlBlock->weakRefCount
                  << std::endl;
      }
    } else if (controlBlock && controlBlock->ptr == nullptr &&
               controlBlock->refCount == 0 && controlBlock->weakRefCount == 0) {
      AllocatorPtr allocator = nullptr;
      controlBlock->allocator.swap(allocator);
      allocator->Free(controlBlock, sizeof(ControlBlock<T>));
      controlBlock = nullptr;
    } else if (controlBlock) {
      std::cout << "ref count: " << controlBlock->refCount << std::endl;
    }
  }
};

template <typename T, class... Args>
UniquePtr<T> MakeUnique(AllocatorPtr allocator, Args &&...args) {
  return UniquePtr<T>({allocator, new (allocator->Allocate(sizeof(T)))
                                      T(std::forward<Args>(args)...)});
}

} // namespace paranoixa

#endif