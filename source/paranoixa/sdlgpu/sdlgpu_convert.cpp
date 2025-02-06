#include "sdlgpu_convert.hpp"
namespace paranoixa::sdlgpu {
namespace convert {
SDL_GPULoadOp LoadOpFrom(LoadOp loadOp) {
  switch (loadOp) {
  case LoadOp::Clear:
    return SDL_GPU_LOADOP_CLEAR;
  case LoadOp::Load:
    return SDL_GPU_LOADOP_LOAD;
  case LoadOp::DontCare:
    return SDL_GPU_LOADOP_DONT_CARE;
  }
  return SDL_GPU_LOADOP_LOAD;
}
SDL_GPUStoreOp StoreOpFrom(StoreOp storeOp) {
  switch (storeOp) {
  case StoreOp::Store:
    return SDL_GPU_STOREOP_STORE;
  case StoreOp::DontCare:
    return SDL_GPU_STOREOP_DONT_CARE;
  }
  return SDL_GPU_STOREOP_STORE;
}
SDL_GPUCullMode CullModeFrom(CullMode cullMode) {
  switch (cullMode) {
  case CullMode::None:
    return SDL_GPU_CULLMODE_NONE;
  case CullMode::Front:
    return SDL_GPU_CULLMODE_FRONT;
  case CullMode::Back:
    return SDL_GPU_CULLMODE_BACK;
  }
  return SDL_GPU_CULLMODE_NONE;
}
SDL_GPUFrontFace FrontFaceFrom(FrontFace frontFace) {
  switch (frontFace) {
  case FrontFace::Clockwise:
    return SDL_GPU_FRONTFACE_CLOCKWISE;
  case FrontFace::CounterClockwise:
    return SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
  }
  return SDL_GPU_FRONTFACE_CLOCKWISE;
}
SDL_GPUPrimitiveType PrimitiveTypeFrom(PrimitiveType primitiveType) {
  switch (primitiveType) {
  case PrimitiveType::TriangleList:
    return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
  case PrimitiveType::TriangleStrip:
    return SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP;
  case PrimitiveType::LineList:
    return SDL_GPU_PRIMITIVETYPE_LINELIST;
  case PrimitiveType::LineStrip:
    return SDL_GPU_PRIMITIVETYPE_LINESTRIP;
  case PrimitiveType::PointList:
    return SDL_GPU_PRIMITIVETYPE_POINTLIST;
  }
  return SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
}
SDL_GPUTextureFormat TextureFormatFrom(TextureFormat textureFormat) {
  switch (textureFormat) {
  case TextureFormat::R8G8B8A8_UNORM:
    return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
  case TextureFormat::B8G8R8A8_UNORM:
    return SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
  default:
    return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    break;
  }
}
SDL_GPUVertexElementFormat VertexElementFormatFrom(VertexElementFormat format) {
  switch (format) {
  case VertexElementFormat::Float1:
    return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
  case VertexElementFormat::Float2:
    return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
  case VertexElementFormat::Float3:
    return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
  case VertexElementFormat::Float4:
    return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
  default:
    return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
  }
}
SDL_GPUVertexInputRate VertexInputRateFrom(VertexInputRate vertexInputRate) {
  switch (vertexInputRate) {
  case VertexInputRate::Vertex:
    return SDL_GPU_VERTEXINPUTRATE_VERTEX;
  case VertexInputRate::Instance:
    return SDL_GPU_VERTEXINPUTRATE_INSTANCE;
  default:
    return SDL_GPU_VERTEXINPUTRATE_VERTEX;
  }
}
SDL_GPUTextureType TextureTypeFrom(TextureType textureType) {
  switch (textureType) {
  case TextureType::Texture2D:
    return SDL_GPU_TEXTURETYPE_2D;
  case TextureType::Texture3D:
    return SDL_GPU_TEXTURETYPE_3D;
  default:
    return SDL_GPU_TEXTURETYPE_2D;
  }
}
SDL_GPUTextureUsageFlags TextureUsageFrom(TextureUsage textureUsage) {
  switch (textureUsage) {
  case TextureUsage::Sampler:
    return SDL_GPU_TEXTUREUSAGE_SAMPLER;
  case TextureUsage::ColorTarget:
    return SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
  case TextureUsage::DepthStencilTarget:
    return SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
  default:
    return SDL_GPU_TEXTUREUSAGE_SAMPLER;
  }
} // namespace convert
SDL_GPUSampleCount SampleCountFrom(SampleCount sampleCount) {
  switch (sampleCount) {
  case SampleCount::x1:
    return SDL_GPU_SAMPLECOUNT_1;
  case SampleCount::x2:
    return SDL_GPU_SAMPLECOUNT_2;
  case SampleCount::x4:
    return SDL_GPU_SAMPLECOUNT_4;
  case SampleCount::x8:
    return SDL_GPU_SAMPLECOUNT_8;
  default:
    return SDL_GPU_SAMPLECOUNT_1;
  }
}
SDL_GPUFilter FilterFrom(Filter filter) {
  switch (filter) {
  case Filter::Nearest:
    return SDL_GPU_FILTER_NEAREST;
  case Filter::Linear:
    return SDL_GPU_FILTER_LINEAR;
  default:
    return SDL_GPU_FILTER_NEAREST;
  }
}
SDL_GPUSamplerMipmapMode MipmapModeFrom(MipmapMode mipmapMode) {
  switch (mipmapMode) {
  case MipmapMode::Nearest:
    return SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
  case MipmapMode::Linear:
    return SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
  default:
    return SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
  }
}
SDL_GPUSamplerAddressMode AddressModeFrom(AddressMode addressMode) {
  switch (addressMode) {
  case AddressMode::Repeat:
    return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
  case AddressMode::MirroredRepeat:
    return SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT;
  case AddressMode::ClampToEdge:
    return SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
  default:
    return SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
  }
}
SDL_GPUCompareOp CompareOpFrom(CompareOp compareOp) {
  switch (compareOp) {
  case CompareOp::Never:
    return SDL_GPU_COMPAREOP_NEVER;
  case CompareOp::Less:
    return SDL_GPU_COMPAREOP_LESS;
  case CompareOp::Equal:
    return SDL_GPU_COMPAREOP_EQUAL;
  case CompareOp::LessOrEqual:
    return SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
  case CompareOp::Greater:
    return SDL_GPU_COMPAREOP_GREATER;
  case CompareOp::NotEqual:
    return SDL_GPU_COMPAREOP_NOT_EQUAL;
  case CompareOp::GreaterOrEqual:
    return SDL_GPU_COMPAREOP_GREATER_OR_EQUAL;
  case CompareOp::Always:
    return SDL_GPU_COMPAREOP_ALWAYS;
  default:
    return SDL_GPU_COMPAREOP_NEVER;
  }
}
SDL_GPUBufferUsageFlags BufferUsageFrom(BufferUsage bufferUsage) {
  switch (bufferUsage) {
  case BufferUsage::Vertex:
    return SDL_GPU_BUFFERUSAGE_VERTEX;
  case BufferUsage::Index:
    return SDL_GPU_BUFFERUSAGE_INDEX;
  case BufferUsage::Indirect:
    return SDL_GPU_BUFFERUSAGE_INDIRECT;
  default:
    return SDL_GPU_BUFFERUSAGE_VERTEX;
  }
}

SDL_GPUTransferBufferUsage
TransferBufferUsageFrom(TransferBufferUsage transferBufferUsage) {
  switch (transferBufferUsage) {
  case TransferBufferUsage::Upload:
    return SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  case TransferBufferUsage::Download:
    return SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
  default:
    return SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  }
}

} // namespace convert
} // namespace paranoixa::sdlgpu