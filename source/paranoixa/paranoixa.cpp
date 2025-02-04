#include <SDL3/SDL.h>
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
static std::unique_ptr<FileLoader> gFileLoader = nullptr;

std::unique_ptr<FileLoader> &GetFileLoader() {
  if (gFileLoader == nullptr) {
    gFileLoader = std::make_unique<FileLoader>();
  }
  return gFileLoader;
}

bool FileLoader::Load(const char *filePath, std::vector<char> &fileData,
                      std::ios_base::openmode openMode) {
  std::string openModeStr;

  if (openMode & std::ios::in) {
    openModeStr += "r";
  }
  if (openMode & std::ios::binary) {
    openModeStr += "b";
  }

  auto file = SDL_IOFromFile(filePath, openModeStr.c_str());
  size_t size;
  void *data = SDL_LoadFile_IO(file, &size, true);

  if (data) {
    fileData.resize(size);
    memcpy(fileData.data(), data, size);
    return true;
  }
  return false;
}
Ptr<Backend> Paranoixa::CreateBackend(AllocatorPtr allocator,
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
    Ptr<Backend> p = MakePtr<SDLGPUBackend>(allocator);
    return p;
  }
  default:
    return nullptr;
  }
#endif
  return nullptr;
}
AllocatorPtr Paranoixa::CreateAllocator(size_t size) {
#ifdef _MSC_VER
  return MakeAllocatorPtr<TLSFAllocator>(size);
#else
  return MakeAllocatorPtr<StdAllocator>(size);
#endif
}
} // namespace paranoixa