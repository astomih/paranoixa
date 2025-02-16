#include <SDL3/SDL.h>
#include <memory_resource>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__
#include "paranoixa.hpp"

#include "allocator/std_allocator.hpp"
#include "allocator/tlsf_allocator.hpp"

#include "d3d12u/d3d12u_renderer.hpp"
#include "sdlgpu/sdlgpu_backend.hpp"
#include "vulkan/vulkan_renderer.hpp"
#include "webgpu/webgpu_renderer.hpp"

#include <SDL3/SDL.h>

#include <fstream>
#include <iostream>
namespace paranoixa {
Ptr<Backend> Paranoixa::CreateBackend(Allocator *allocator,
                                      const GraphicsAPI &api) {
#ifndef __EMSCRIPTEN__
  switch (api) {
  case GraphicsAPI::Vulkan: {
    // TODO
  }
#ifdef _WIN32
  case GraphicsAPI::D3D12U: {
    // TODO
  }
#endif
  case GraphicsAPI::WebGPU: {
    // TODO
  }
  case GraphicsAPI::SDLGPU: {
    Ptr<Backend> p = MakePtr<sdlgpu::Backend>(allocator);
    return p;
  }
  default:
    return nullptr;
  }
#endif
  return nullptr;
}
Allocator *Paranoixa::CreateAllocator(size_t size) {
#ifdef _MSC_VER
  return new TLSFAllocator(size);
#else
  return new StdAllocator(size);
#endif
}
} // namespace paranoixa