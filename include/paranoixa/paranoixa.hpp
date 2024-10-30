#ifndef PARANOIXA_HPP
#define PARANOIXA_HPP
#include "allocator/allocator.hpp"
#include <cassert>
#include <cstring>
#include <functional>
#include <memory>

#ifdef _DEBUG
#define PARANOIXA_BUILD_DEBUG
#elif NDEBUG
#define PARANOIXA_BUILD_RELEASE
#endif

namespace paranoixa {
enum class GraphicsAPI {
  Vulkan,
#ifdef _WIN32
  D3D12U,
#endif
  WebGPU,
};
template <typename T> struct ControlBlock {
public:
  T *ptr;
  uint32_t size;
  uint32_t refCount;
  uint32_t weakRefCount;
  AllocatorPtr allocator;
};

template <typename T> class Ref;

// SharedPtr
template <typename T> class Ptr {
public:
  Ptr(AllocatorPtr allocator, uint32_t size = 0, T *ptr = nullptr)
      : controlBlock(new(reinterpret_cast<ControlBlock<T> *>(
            allocator->Allocate(sizeof(ControlBlock<T>))))
                         ControlBlock<T>({.ptr = ptr,
                                          .size = size,
                                          .refCount = 0,
                                          .weakRefCount = 0,
                                          .allocator = allocator})) {
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

private:
  friend class Ref<T>;
  ControlBlock<T> *controlBlock;

  void Release() {
    if (controlBlock && controlBlock->ptr && --controlBlock->refCount == 0) {
      controlBlock->allocator->Free(controlBlock->ptr, controlBlock->size);
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
      assert(controlBlock->refCount > 0);
    }
  }
};

template <typename T, class... Args>
Ptr<T> MakePtr(AllocatorPtr allocator, Args &&...args) {
  return Ptr<T>{allocator, sizeof(T),
                new (allocator->Allocate(sizeof(T)))
                    T(std::forward<Args>(args)...)};
}

// UniquePtr
template <typename T> class UniquePtr {

public:
  UniquePtr(AllocatorPtr allocator, uint32_t size = 0, T *ptr = nullptr)
      : controlBlock(new(reinterpret_cast<ControlBlock<T> *>(
            allocator->Allocate(sizeof(ControlBlock<T>))))
                         ControlBlock<T>({.ptr = ptr,
                                          .size = size,
                                          .refCount = 0,
                                          .weakRefCount = 0,
                                          .allocator = allocator})) {
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
      controlBlock->allocator->Free(controlBlock->ptr, controlBlock->size);
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
  return UniquePtr<T>({allocator, sizeof(T),
                       new (allocator->Allocate(sizeof(T)))
                           T(std::forward<Args>(args)...)});
}

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

class Paranoixa {
public:
  struct Desc {
    AllocatorPtr allocator;
    GraphicsAPI api;
  };
  Paranoixa(const Desc &desc);
  ~Paranoixa();
  Ref<class Renderer> GetRenderer();

  void Run();

private:
  bool IsRunning();
  void Loop();
  AllocatorPtr allocator;
  UniquePtr<class Renderer> renderer;
};
} // namespace paranoixa
#endif