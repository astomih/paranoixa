#include <SDL3/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__
#include "paranoixa.hpp"
#include "renderer/d3d12u/d3d12u_renderer.hpp"
#include "renderer/renderer.hpp"
#include "renderer/sdlgpu/sdlgpu_renderer.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "renderer/webgpu/webgpu_renderer.hpp"
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
  renderer.Reset();
  SDL_DestroyWindow(window);
  SDL_Quit();
}
Ptr<Backend> Paranoixa::CreateBackend(const GraphicsAPI &api) {
  switch (api) {
  case GraphicsAPI::Vulkan: {
    // TODO
  }
  case GraphicsAPI::D3D12U: {
    // TODO
  }
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
  return nullptr;
}
Paranoixa::Paranoixa(const Desc &desc)
    : allocator(desc.allocator), renderer(desc.allocator) {}
void *Paranoixa::GetWindow() { return static_cast<void *>(window); }
Ref<Renderer> Paranoixa::GetRenderer() { return Ref<Renderer>(renderer); }
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
    renderer->ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
      running = false;
    }
  }
  renderer->BeginFrame();
  renderer->EndFrame();
}
} // namespace paranoixa