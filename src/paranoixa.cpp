#include "paranoixa.hpp"
#include "renderer/renderer.hpp"
#include "renderer/webgpu/webgpu_renderer.hpp"
#include <memory>
namespace paranoixa {
void initialize(GraphicsAPI api) {
  std::unique_ptr<Renderer> renderer;
  switch (api) {
  case GraphicsAPI::WebGPU: {
    renderer = std::make_unique<WebGPURenderer>();
    break;
  }
  case GraphicsAPI::Vulkan:
    break;
  }
  renderer->initialize();
}
} // namespace paranoixa