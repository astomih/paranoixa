#ifndef PARANOIXA_HPP
#define PARANOIXA_HPP
#include "memory/allocator.hpp"
#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <print>

#include "memory/ptr.hpp"
#include "memory/ref.hpp"

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