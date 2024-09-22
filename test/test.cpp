#include <paranoixa/paranoixa.hpp>

int main() {
  paranoixa::Application app;
  app.Initialize(paranoixa::GraphicsAPI::WebGPU);
  app.Run();
  return 0;
}