#ifndef PARANOIXA_RENDERER_HPP
#define PARANOIXA_RENDERER_HPP
#include <filesystem>
#include <functional>
#include <vector>

#include "../memory/allocator.hpp"
#include "memory/ptr.hpp"

namespace paranoixa {
class FileLoader {
public:
  bool Load(const char *filePath, std::vector<char> &fileData,
            std::ios_base::openmode openMode = std::ios::in | std::ios::binary);
};

std::unique_ptr<FileLoader> &GetFileLoader();

enum class ShaderStage { Vertex, Fragment };
enum class TransferBufferUsage { Upload, Download };
enum class TextureFormat { R8G8B8A8_UNORM };
enum class TextureUsage { Sampler, ColorTarget, DepthStencilTarget };
enum class TextureType { Texture2D, Texture3D };
enum class BufferUsage { Vertex, Index, Uniform };
enum class SampleCount {
  x1,
  x2,
  x4,
  x8,
};
class Device;
class Texture {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    TextureType type;
    TextureFormat format;
    TextureUsage usage;
    uint32_t width;
    uint32_t height;
    uint32_t layerCountOrDepth;
    uint32_t numLevels;
    SampleCount sampleCount;
  };
  Texture(Ptr<Device> device, const CreateInfo &createInfo)
      : device(device), createInfo(createInfo) {}
  virtual ~Texture() = default;

private:
  Ptr<Device> device;
  CreateInfo createInfo;
};

class Sampler {
public:
  enum class Filter { Nearest, Linear };
  enum class MipmapMode { Nearest, Linear };
  enum class AddressMode { Repeat, MirroredRepeat, ClampToEdge };
  enum class CompareOp {
    Invalid,
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
  };
  struct CreateInfo {
    AllocatorPtr allocator;
    Filter minFilter;
    Filter magFilter;
    MipmapMode mipmapMode;
    AddressMode addressModeU;
    AddressMode addressModeV;
    AddressMode addressModeW;
    float mipLodBias;
    float maxAnisotropy;
    CompareOp compareOp;
    float minLod;
    float maxLod;
    bool enableAnisotropy;
    bool enableCompare;
  };
  Sampler(Ptr<Device> device, const CreateInfo &createInfo)
      : device(device), createInfo(createInfo) {}
  virtual ~Sampler() = default;

private:
  Ptr<Device> device;
  CreateInfo createInfo;
};

class Buffer {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    BufferUsage usage;
    uint32_t size;
  };
  Buffer(Ptr<Device> device, const CreateInfo &createInfo)
      : device(device), createInfo(createInfo) {}
  virtual ~Buffer() = default;

private:
  Ptr<Device> device;
  CreateInfo createInfo;
};

class TransferBuffer {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    TransferBufferUsage usage;
    uint32_t size;
  };
  TransferBuffer(Ptr<Device> device, const CreateInfo &createInfo)
      : device(device), createInfo(createInfo) {}
  virtual ~TransferBuffer() = default;

private:
  Ptr<Device> device;
  CreateInfo createInfo;
};

class Shader {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    size_t size;
    const void *data;
    const char *entrypoint;
    ShaderStage stage;
    uint32_t numSamplers;
    uint32_t numStorageTextures;
    uint32_t numUniformBuffers;
  };
  Shader(Ptr<Device> device, const CreateInfo &createInfo)
      : device(device), createInfo(createInfo) {}
  virtual ~Shader() = default;

private:
  Ptr<Device> device;
  CreateInfo createInfo;
};

class GraphicsPipeline {
public:
  enum VertexInputRate {
    Vertex,
    Instance,
  };
  struct VertexBufferDescription {
    uint32_t slot;
    uint32_t pitch;
    VertexInputRate inputRate;
    uint32_t instanceStepRate;
  };
  enum VertexElementFormat {
    Float1,
    Float2,
    Float3,
    Float4,
  };
  struct VertexAttribute {
    uint32_t location;
    uint32_t bufferSlot;
    VertexElementFormat format;
    uint32_t offset;
  };
  struct VertexInputState {
    const VertexBufferDescription *vertexBufferDescriptions;
    uint32_t numVertexBuffers;
    const VertexAttribute *vertexAttributes;
    uint32_t numVertexAttributes;
  };
  enum class PrimitiveType {
    TriangleList,
  };
  struct RasterizerState {
    enum class FillMode {
      Solid,
      Wireframe,
    };
    enum class CullMode {
      None,
      Front,
      Back,
    };
    enum class FrontFace {
      Clockwise,
      CounterClockwise,
    };
    FillMode fillMode;
    CullMode cullMode;
    FrontFace frontFace;
  };
  struct CreateInfo {
    AllocatorPtr allocator;
    Ptr<Shader> vertexShader;
    Ptr<Shader> fragmentShader;
    VertexInputState vertexInputState;
    PrimitiveType primitiveType;
    RasterizerState rasterizerState;
  };
};

class ComputePipeline {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    Ptr<Shader> computeShader;
  };
};

class CommandBuffer {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
  };
};

class Device {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
  };
  Device(const CreateInfo &createInfo) : allocator(createInfo.allocator) {}
  void ClaimWindow(void *window);
  virtual Ptr<Buffer> CreateBuffer(const Buffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Texture> CreateTexture(const Texture::CreateInfo &createInfo) = 0;
  virtual Ptr<Sampler> CreateSampler(const Sampler::CreateInfo &createInfo) = 0;
  virtual Ptr<TransferBuffer>
  CreateTransferBuffer(const TransferBuffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Shader> CreateShader(const Shader::CreateInfo &createInfo) = 0;
  virtual Ptr<GraphicsPipeline>
  CreatePipeline(const GraphicsPipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<ComputePipeline>
  CreatePipeline(const ComputePipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<CommandBuffer>
  CreateCommandBuffer(const CommandBuffer::CreateInfo &createInfo) = 0;

private:
  AllocatorPtr allocator;
};

class Renderer {
public:
  Renderer() = default;
  virtual ~Renderer() { std::cout << "Renderer::~Renderer()" << std::endl; };

  virtual Ptr<Device> CreateDevice(const Device::CreateInfo &createInfo) = 0;
  virtual void Initialize(void *window) = 0;
  virtual void ProcessEvent(void *event) = 0;

  virtual void BeginFrame() = 0;
  virtual void EndFrame() = 0;

  virtual void AddGuiUpdateCallBack(std::function<void()> callBack) = 0;
};
} // namespace paranoixa
#endif // PARANOIXA_RENDERER_HPP
