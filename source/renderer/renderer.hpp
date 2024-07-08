#ifndef PARANOIXA_RENDERER_HPP
#define PARANOIXA_RENDERER_HPP

namespace paranoixa {
class Renderer {
public:
  Renderer() = default;
  virtual ~Renderer() = default;
  virtual void Initialize(void *window) = 0;
  virtual void Render() = 0;
};
} // namespace paranoixa
#endif // PARANOIXA_RENDERER_HPP