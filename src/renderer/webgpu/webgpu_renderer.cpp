#include <webgpu/webgpu.h>

#include "webgpu_renderer.hpp"
#include <iostream>

namespace paranoixa {
static const WGPUInstance instance = wgpuCreateInstance(nullptr);
WebGPURenderer::WebGPURenderer() {}
WebGPURenderer::~WebGPURenderer() {}
void WebGPURenderer::initialize() {
  if (!instance) {
    std::cerr << "Could not initialize WebGPU!" << std::endl;
  }
  std::cout << "WGPU instance: " << instance << std::endl;
}
} // namespace paranoixa