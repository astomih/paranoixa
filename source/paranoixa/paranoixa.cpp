#include <SDL3/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__
#include "paranoixa.hpp"
#include "tlsf_allocator.hpp"

#include "d3d12u/d3d12u_renderer.hpp"
#include "sdlgpu/sdlgpu_renderer.hpp"
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
static SDL_Window *window = nullptr;
static bool running = true;
Paranoixa::~Paranoixa() {
  SDL_DestroyWindow(window);
  SDL_Quit();
}
Ptr<Backend> Paranoixa::CreateBackend(const GraphicsAPI &api) {
#ifndef _EMSCRIPTEN_
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
  return MakeAllocatorPtr<TLSFAllocator>(size);
}
Paranoixa::Paranoixa(const Desc &desc) : allocator(desc.allocator) {}
void *Paranoixa::GetWindow() { return static_cast<void *>(window); }
void Paranoixa::Run() {
#ifndef __EMSCRIPTEN__
  while (this->IsRunning()) {
    this->Loop();
  }
#else
  emscripten_set_main_loop_arg(
      [](void *userData) {
        Paranoixa *app = reinterpret_cast<Paranoixa *>(userData);
        app->Loop();
      },
      this, 0, 1);
#endif // __EMSCRIPTEN__
}
bool Paranoixa::IsRunning() { return running; }
void Paranoixa::Loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      running = false;
    }
  }
}
} // namespace paranoixa