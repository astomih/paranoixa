#ifndef PARANOIXA_WEBGPU_RENDERER_HPP
#define PARANOIXA_WEBGPU_RENDERER_HPP
#include "../renderer.hpp"
namespace paranoixa {
class WebGPURenderer : public Renderer {
public:
  WebGPURenderer();
  ~WebGPURenderer();
  void initialize() override;

private:
};
} // namespace paranoixa
#endif // PARANOIXA_WEBGPU_RENDERER_HPP