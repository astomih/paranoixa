#ifndef PARANOIXA_HPP
#define PARANOIXA_HPP

namespace paranoixa {
enum class GraphicsAPI {
  WebGPU,
  Vulkan,
};
void initialize(GraphicsAPI api);
} // namespace paranoixa
#endif