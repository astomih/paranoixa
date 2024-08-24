#include <SDL3/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__
#include "paranoixa.hpp"
#include "renderer/renderer.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "renderer/webgpu/webgpu_renderer.hpp"
#include <iostream>
#include <memory>
namespace paranoixa {
std::unique_ptr<class Renderer> renderer;
#ifndef __EMSCRIPTEN__
VulkanRenderer &GetVulkanRenderer() {
  return dynamic_cast<VulkanRenderer &>(*renderer);
}
#endif
Application::~Application() {}
class Application::Implement {
public:
  Implement() = default;
  ~Implement();
  ::SDL_Window *window;
  bool running = true;
};
Application::Implement::~Implement() {
  renderer.reset();
  // SDL_DestroyWindow(window);
  SDL_Quit();
}
void Application::ImplementDeleter::operator()(Implement *implement) {
  delete implement;
}
void Application::Initialize(GraphicsAPI api) {
  implement = std::unique_ptr<Implement, ImplementDeleter>(new Implement());
#ifndef __EMSCRIPTEN__
  switch (api) {
  case GraphicsAPI::WebGPU: {
    renderer = std::make_unique<WebGPURenderer>();
    break;
  }
  case GraphicsAPI::Vulkan:
    renderer = std::make_unique<VulkanRenderer>();
    break;
  }
#else
  renderer = std::make_unique<WebGPURenderer>();
#endif

  if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s",
                 SDL_GetError());
  }
  uint32_t windowFlags = 0;
#ifdef __EMSCRIPTEN__
  std::string windowName = "Paranoixa ( WASM )";
#else
  std::string windowName;
  switch (api) {
  case GraphicsAPI::WebGPU:
    windowName = "Paranoixa ( Native WGPU )";
    break;
  case GraphicsAPI::Vulkan:
    windowFlags |= SDL_WINDOW_VULKAN;
    windowName = "Paranoixa ( Native Vulkan )";
    break;
  }
#endif // __EMSCRIPTEN__
  implement->window =
      SDL_CreateWindow(windowName.c_str(), 1280, 720, windowFlags);
  renderer->Initialize(implement->window);
}
void Application::Run() {
#ifndef __EMSCRIPTEN__
  while (this->IsRunning()) {
    this->Loop();
  }
#else
  emscripten_set_main_loop_arg(
      [](void *userData) {
        Application *app = reinterpret_cast<Application *>(userData);
        app->Loop();
      },
      this, 0, 1);
#endif // __EMSCRIPTEN__
}
bool Application::IsRunning() { return implement->running; }
void Application::Loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    renderer->ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
      implement->running = false;
    }
  }
  renderer->Render();
}
} // namespace paranoixa