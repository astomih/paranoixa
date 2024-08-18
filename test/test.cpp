#include <paranoixa/paranoixa.hpp>

int main() {
  paranoixa::Application app;
  app.Initialize(paranoixa::GraphicsAPI::Vulkan);
  app.Run();
  return 0;
}