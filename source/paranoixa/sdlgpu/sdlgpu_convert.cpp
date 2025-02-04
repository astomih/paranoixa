#include "sdlgpu_convert.hpp"
namespace paranoixa {
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
  default:
    return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    break;
  }
}
} // namespace convert
} // namespace paranoixa