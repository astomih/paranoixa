#ifndef PARANOIXA_RENDERER_HPP
#define PARANOIXA_RENDERER_HPP
#include <filesystem>
namespace paranoixa {
class FileLoader {
public:
  bool Load(std::filesystem::path filePath, std::vector<char> &fileData,
            std::ios_base::openmode openMode = std::ios::binary);
};

std::unique_ptr<FileLoader> &GetFileLoader();
class Renderer {
public:
  Renderer() = default;
  virtual ~Renderer() = default;
  virtual void Initialize(void *window) = 0;
  virtual void ProcessEvent(void *event) = 0;
  virtual void Render() = 0;
};
} // namespace paranoixa
#endif // PARANOIXA_RENDERER_HPP