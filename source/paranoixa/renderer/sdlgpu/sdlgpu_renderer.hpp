#ifndef PARANOIXA_SDLGPU_RENDERER_HPP
#define PARANOIXA_SDLGPU_RENDERER_HPP
#include "renderer/renderer.hpp"

namespace paranoixa {
class SDLGPURenderer : public Renderer {

public:
  SDLGPURenderer(AllocatorPtr allcator);
  ~SDLGPURenderer() override;

  void Initialize(void *window) override;
  void ProcessEvent(void *event) override;
  void BeginFrame() override;
  void EndFrame() override;
  void AddGuiUpdateCallBack(std::function<void()> callBack) override;
};
} // namespace paranoixa
#endif // PARANOIXA_SDLGPU_RENDERER_HPP