
#include <SDL2/SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__
#include "paranoixa.hpp"
#include "renderer/renderer.hpp"
#include "renderer/webgpu/webgpu_renderer.hpp"
#include <iostream>
#include <memory>
namespace paranoixa {
Application::~Application() {}
class Application::Implement {
public:
  Implement() = default;
  ~Implement();
  std::unique_ptr<class Renderer> renderer;
  ::SDL_Window *window;
  bool running = true;
};
Application::Implement::~Implement() {
  // SDL_DestroyWindow(window);
  SDL_Quit();
}
void Application::ImplementDeleter::operator()(Implement *implement) {
  delete implement;
}
void Application::Initialize(GraphicsAPI api) {
  m_implement = std::unique_ptr<Implement, ImplementDeleter>(new Implement());
  switch (api) {
  case GraphicsAPI::WebGPU: {
    m_implement->renderer = std::make_unique<WebGPURenderer>();
    break;
  }
  case GraphicsAPI::Vulkan:
    break;
  }

  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "Could not initialize SDL: " << SDL_GetError() << std::endl;
  }
  uint32_t windowFlags = 0;
  m_implement->window =
      SDL_CreateWindow("Paranoixa", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 640, 480, windowFlags);
  m_implement->renderer->Initialize(m_implement->window);
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(
      [](void *userData) {
        Application *app = reinterpret_cast<Application *>(userData);
        app->Loop();
      },
      this, 0, 1);
#endif // __EMSCRIPTEN__
}
void Application::Run() {
#ifndef __EMSCRIPTEN__
  while (this->IsRunning()) {
    this->Loop();
  }
#endif
}
bool Application::IsRunning() { return m_implement->running; }
void Application::Loop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      m_implement->running = false;
    }
  }
  m_implement->renderer->Render();
}
} // namespace paranoixa