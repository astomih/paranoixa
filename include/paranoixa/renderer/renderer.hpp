#ifndef PARANOIXA_RENDERER_HPP
#define PARANOIXA_RENDERER_HPP
#include <filesystem>
#include <vector>

#include "../memory/allocator.hpp"
namespace paranoixa {
class FileLoader {
public:
  bool Load(const char *filePath, std::vector<char> &fileData,
            std::ios_base::openmode openMode = std::ios::in | std::ios::binary);
};

std::unique_ptr<FileLoader> &GetFileLoader();
class Renderer {
public:
  Renderer() = default;
  virtual ~Renderer() { std::cout << "Renderer::~Renderer()" << std::endl; };
  virtual void Initialize(void *window) = 0;
  virtual void ProcessEvent(void *event) = 0;

  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;
};
} // namespace paranoixa
#endif // PARANOIXA_RENDERER_HPP