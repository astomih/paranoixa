#ifndef PARANOIXA_SDLGPU_CONVERT_HPP
#define PARANOIXA_SDLGPU_CONVERT_HPP
#include <SDL3/SDL_gpu.h>
#include <paranoixa.hpp>
namespace paranoixa {
namespace convert {
SDL_GPULoadOp LoadOpFrom(LoadOp loadOp);
SDL_GPUStoreOp StoreOpFrom(StoreOp storeOp);
SDL_GPUCullMode CullModeFrom(CullMode cullMode);
SDL_GPUFrontFace FrontFaceFrom(FrontFace frontFace);
SDL_GPUPrimitiveType PrimitiveTypeFrom(PrimitiveType primitiveType);
SDL_GPUTextureFormat TextureFormatFrom(TextureFormat textureFormat);
SDL_GPUVertexElementFormat
VertexElementFormatFrom(VertexElementFormat vertexElementFormat);
SDL_GPUVertexInputRate VertexInputRateFrom(VertexInputRate vertexInputRate);

} // namespace convert
} // namespace paranoixa
#endif // !PARANOIXA_SDLGPU_CONVERT_HPP