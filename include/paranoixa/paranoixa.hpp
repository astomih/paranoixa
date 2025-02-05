#ifndef PARANOIXA_HPP
#define PARANOIXA_HPP
#include "allocator.hpp"
#include "define.hpp"
#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <print>

namespace paranoixa {
enum class GraphicsAPI {
  Vulkan,
#ifdef PARANOIXA_PLATFORM_WINDOWS
  D3D12U,
#endif
  WebGPU,
  SDLGPU,
};
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
enum VertexInputRate {
  Vertex,
  Instance,
};
struct VertexBufferDescription {
  uint32 slot;
  uint32 pitch;
  VertexInputRate inputRate;
  uint32 instanceStepRate;
};
enum VertexElementFormat {
  Float1,
  Float2,
  Float3,
  Float4,
};
enum class LoadOp { Load, Clear, DontCare };
enum class StoreOp { Store, DontCare };
struct VertexAttribute {
  uint32 location;
  uint32 bufferSlot;
  VertexElementFormat format;
  uint32 offset;
};
struct VertexInputState {
  Array<VertexBufferDescription> vertexBufferDescriptions;
  Array<VertexAttribute> vertexAttributes;
};
enum class PrimitiveType {
  TriangleList,
  TriangleStrip,
  LineList,
  LineStrip,
  PointList,

};
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
enum class BlendFactor {
  Zero,
  One,
  SrcColor,
  OneMinusSrcColor,
  DstColor,
  OneMinusDstColor,
  SrcAlpha,
  OneMinusSrcAlpha,
  DstAlpha,
  OneMinusDstAlpha,
  ConstantColor,
  OneMinusConstantColor,
  ConstantAlpha,
  OneMinusConstantAlpha,
  SrcAlphaSaturate,
  Src1Color,
  OneMinusSrc1Color,
  Src1Alpha,
  OneMinusSrc1Alpha,
};
enum class BlendOp {
  Add,
  Subtract,
  ReverseSubtract,
  Min,
  Max,
};
struct RasterizerState {
  FillMode fillMode;
  CullMode cullMode;
  FrontFace frontFace;
};
struct MultiSampleState {
  SampleCount sampleCount;
  uint32 sampleMask;
  bool enableMask;
};
struct DepthStencilState {
  CompareOp compareOp;
  bool enableDepthTest;
  bool enableDepthWrite;
  bool enableStencilTest;
  uint32 stencilReadMask;
  uint32 stencilWriteMask;
};
struct ColorTargetBlendState {
  BlendFactor srcColorBlendFactor;
  BlendFactor dstColorBlendFactor;
  BlendOp colorBlendOp;
  BlendFactor srcAlphaBlendFactor;
  BlendFactor dstAlphaBlendFactor;
  BlendOp alphaBlendOp;
  uint8_t colorWriteMask;
  bool enableBlend;
  bool enableColorWriteMask;
};
struct ColorTargetDescription {
  TextureFormat format;
  ColorTargetBlendState blendState;
};
struct TargetInfo {
  Array<ColorTargetDescription> colorTargetDescriptions;
  const TextureFormat *depthStencilTargetFormat;
  bool hasDepthStencilTarget;
};
class Device;
class Texture {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    TextureType type;
    TextureFormat format;
    TextureUsage usage;
    uint32 width;
    uint32 height;
    uint32 layerCountOrDepth;
    uint32 numLevels;
    SampleCount sampleCount;
  };
  virtual ~Texture() = default;

protected:
  Texture(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class Sampler {
public:
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
  Sampler(const CreateInfo &createInfo) : createInfo(createInfo) {}
  virtual ~Sampler() = default;

private:
  CreateInfo createInfo;
};

class Buffer {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    BufferUsage usage;
    uint32 size;
  };
  virtual ~Buffer() = default;

protected:
  Buffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class TransferBuffer {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    TransferBufferUsage usage;
    uint32 size;
  };
  virtual ~TransferBuffer() = default;

  const CreateInfo &GetCreateInfo() const { return createInfo; }

  virtual void *Map() = 0;
  virtual void Unmap() = 0;

protected:
  TransferBuffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
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
    uint32 numSamplers;
    uint32 numStorageTextures;
    uint32 numUniformBuffers;
  };
  virtual ~Shader() = default;

protected:
  Shader(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class GraphicsPipeline {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    Ptr<Shader> vertexShader;
    Ptr<Shader> fragmentShader;
    VertexInputState vertexInputState;
    PrimitiveType primitiveType;
    RasterizerState rasterizerState;
    MultiSampleState multiSampleState;
    DepthStencilState depthStencilState;
    TargetInfo targetInfo;
  };
  virtual ~GraphicsPipeline() = default;

protected:
  GraphicsPipeline(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class ComputePipeline {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    Ptr<Shader> computeShader;
  };
  virtual ~ComputePipeline() = default;

protected:
  ComputePipeline(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class CopyPass {
public:
  struct TextureTransferInfo {
    Ptr<TransferBuffer> transferBuffer;
    uint32 offset;
  };
  struct TextureRegion {
    Ptr<Texture> texture;
    uint32 x, y, z;
    uint32 width;
    uint32 height;
    uint32 depth;
  };
  struct BufferTransferInfo {
    Ptr<TransferBuffer> transferBuffer;
    uint32 offset;
  };
  struct BufferRegion {
    Ptr<Buffer> buffer;
    uint32 offset;
    uint32 size;
  };
  virtual ~CopyPass() = default;

  virtual void UploadTexture(const TextureTransferInfo &src,
                             const TextureRegion &dst, bool cycle) = 0;
  virtual void DownloadTexture(const TextureRegion &src,
                               const TextureTransferInfo &dst, bool cycle) = 0;
  virtual void UploadBuffer(const BufferTransferInfo &src,
                            const BufferRegion &dst, bool cycle) = 0;
  virtual void DownloadBuffer(const BufferRegion &src,
                              const BufferTransferInfo &dst, bool cycle) = 0;
};
class RenderPass {
public:
  struct ColorTargetInfo {
    Ptr<Texture> texture;
    // clearColor
    LoadOp loadOp;
    StoreOp storeOp;
  };
  struct BufferBinding {
    Ptr<Buffer> buffer;
    uint32 offset;
  };
  struct TextureSamplerBinding {
    Ptr<Sampler> sampler;
    Ptr<Texture> texture;
  };
  virtual ~RenderPass() = default;

  virtual void BindGraphicsPipeline(Ptr<GraphicsPipeline> graphicsPipeline) = 0;
  virtual void BindVertexBuffers(uint32 slot,
                                 const Array<BufferBinding> &bindings) = 0;
  virtual void
  BindFragmentSamplers(uint32 slot,
                       const Array<TextureSamplerBinding> &bindings) = 0;
  virtual void DrawPrimitives(uint32 numVertices, uint32 numInstances,
                              uint32 firstVertex, uint32 firstInstance) = 0;

protected:
  RenderPass() = default;
};

class CommandBuffer {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
  };
  virtual ~CommandBuffer() = default;

  inline CreateInfo GetCreateInfo() const { return createInfo; }

  virtual Ptr<class CopyPass> BeginCopyPass() = 0;
  virtual void EndCopyPass(Ptr<class CopyPass> copyPass) = 0;

  virtual Ptr<class RenderPass>
  BeginRenderPass(const Array<RenderPass::ColorTargetInfo> &infos) = 0;
  virtual void EndRenderPass(Ptr<RenderPass> renderPass) = 0;

protected:
  CommandBuffer(const CreateInfo &createInfo) : createInfo(createInfo) {}

private:
  CreateInfo createInfo;
};

class Device {
public:
  struct CreateInfo {
    AllocatorPtr allocator;
    bool debugMode;
  };
  Device(const CreateInfo &createInfo) : createInfo(createInfo) {}
  virtual ~Device() = default;
  const CreateInfo &GetCreateInfo() const { return createInfo; }

  /**
   * @brief Claim the SDL_Window for the device
   * @param window SDL_Window pointer
   */
  virtual void ClaimWindow(void *window) = 0;
  virtual Ptr<Buffer> CreateBuffer(const Buffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Texture> CreateTexture(const Texture::CreateInfo &createInfo) = 0;
  virtual Ptr<Sampler> CreateSampler(const Sampler::CreateInfo &createInfo) = 0;
  virtual Ptr<TransferBuffer>
  CreateTransferBuffer(const TransferBuffer::CreateInfo &createInfo) = 0;
  virtual Ptr<Shader> CreateShader(const Shader::CreateInfo &createInfo) = 0;
  virtual Ptr<GraphicsPipeline>
  CreateGraphicsPipeline(const GraphicsPipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<ComputePipeline>
  CreateComputePipeline(const ComputePipeline::CreateInfo &createInfo) = 0;
  virtual Ptr<CommandBuffer>
  CreateCommandBuffer(const CommandBuffer::CreateInfo &createInfo) = 0;
  virtual void SubmitCommandBuffer(Ptr<CommandBuffer> commandBuffer) = 0;
  virtual Ptr<Texture>
  AcquireSwapchainTexture(Ptr<CommandBuffer> commandBuffer) = 0;

private:
  CreateInfo createInfo;
};

class Backend {
public:
  Backend() = default;
  virtual ~Backend() = default;
  virtual Ptr<Device> CreateDevice(const Device::CreateInfo &createInfo) = 0;
};

class Paranoixa {
public:
  static Ptr<Backend> CreateBackend(AllocatorPtr allocator,
                                    const GraphicsAPI &api);
  static AllocatorPtr CreateAllocator(size_t size);
};
} // namespace paranoixa
#endif
