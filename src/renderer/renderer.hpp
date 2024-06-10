#ifndef PARANOIXA_RENDERER_HPP
#define PARANOIXA_RENDERER_HPP

namespace paranoixa {
class Renderer {
public:
  Renderer() = default;
  ~Renderer() = default;
  virtual void initialize() = 0;
};
} // namespace paranoixa
#endif // PARANOIXA_RENDERER_HPP