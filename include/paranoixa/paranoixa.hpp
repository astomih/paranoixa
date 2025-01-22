#ifndef PARANOIXA_HPP
#define PARANOIXA_HPP
#include "allocator.hpp"
#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <print>

#include "ptr.hpp"
#include "ref.hpp"

#include "renderer.hpp"

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
  SDLGPU,
};

class Backend {
public:
  Backend() = default;
  virtual ~Backend() = default;
  virtual Ptr<Device> CreateDevice(const Device::CreateInfo &createInfo) = 0;
};

class Paranoixa {
public:
  struct Desc {
    AllocatorPtr allocator;
    GraphicsAPI api;
  };

  Paranoixa(const Desc &desc);
  ~Paranoixa();
  Ptr<Backend> CreateBackend(const GraphicsAPI &api);
  void *GetWindow();
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
