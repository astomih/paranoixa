#include <SDL3/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__
#include "paranoixa.hpp"
#include "renderer/renderer.hpp"
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
    fileData.resize(size + 1);
    memcpy(fileData.data(), data, size);
    fileData[size] = '\0';
    for (int i = 0; i < size; i++) {
      std::cout << fileData[i];
    }
    return true;
  }

  return false;
}
static ::SDL_Window *window = nullptr;
static bool running = true;
Paranoixa::~Paranoixa() {
  renderer.Reset();
  SDL_DestroyWindow(window);
  SDL_Quit();
}
Paranoixa::Paranoixa(const Paranoixa::Desc &desc)
    : allocator(desc.allocator), renderer(desc.allocator) {
  std::string windowName;
  uint32_t windowFlags = 0;
#ifndef __EMSCRIPTEN__
  switch (desc.api) {
  case GraphicsAPI::WebGPU: {
    renderer = MakeUnique<WebGPURenderer>(allocator);
    windowName = "Paranoixa ( Native WGPU )";
    break;
  }
  case GraphicsAPI::Vulkan:
    renderer = MakeUnique<VulkanRenderer>(allocator);
    windowFlags |= SDL_WINDOW_VULKAN;
    windowName = "Paranoixa ( Native Vulkan )";
    break;
  }
#else
  renderer = MakeUnique<WebGPURenderer>(allocator);
  windowName = "Paranoixa ( WASM )";
#endif

  if (SDL_Init(SDL_INIT_EVENTS) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s",
                 SDL_GetError());
  }
  SDL_Surface *surface = SDL_LoadBMP("res/texture.bmp");
  window =
      SDL_CreateWindow(windowName.c_str(), surface->w, surface->h, windowFlags);
  renderer->Initialize(window);
  SDL_DestroySurface(surface);
}
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
  renderer->Render();
}
} // namespace paranoixa