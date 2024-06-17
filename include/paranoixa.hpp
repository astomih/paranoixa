#ifndef PARANOIXA_HPP
#define PARANOIXA_HPP
#include <memory>

namespace paranoixa {
enum class GraphicsAPI {
  WebGPU,
  Vulkan,
};
class Application {
public:
  Application() = default;
  ~Application();
  void Initialize(GraphicsAPI api);
  void Run();

private:
  bool IsRunning();
  void Loop();

  class Implement;
  // Custom deleter for std::unique_ptr<Implement>
  struct ImplementDeleter {
    void operator()(Implement *implement);
  };
  std::unique_ptr<Implement, ImplementDeleter> m_implement;
};
} // namespace paranoixa
#endif