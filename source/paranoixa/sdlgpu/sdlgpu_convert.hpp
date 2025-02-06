#ifndef PARANOIXA_SDLGPU_CONVERT_HPP
#define PARANOIXA_SDLGPU_CONVERT_HPP
#include <SDL3/SDL_gpu.h>
#include <paranoixa.hpp>
namespace paranoixa::sdlgpu {
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
SDL_GPUTextureType TextureTypeFrom(TextureType textureType);
SDL_GPUTextureUsageFlags TextureUsageFrom(TextureUsage textureUsage);
SDL_GPUSampleCount SampleCountFrom(SampleCount sampleCount);
SDL_GPUFilter FilterFrom(Filter filter);
SDL_GPUSamplerMipmapMode MipmapModeFrom(MipmapMode mipmapMode);
SDL_GPUSamplerAddressMode AddressModeFrom(AddressMode addressMode);
SDL_GPUCompareOp CompareOpFrom(CompareOp compareOp);
SDL_GPUBufferUsageFlags BufferUsageFrom(BufferUsage bufferUsage);
SDL_GPUTransferBufferUsage
TransferBufferUsageFrom(TransferBufferUsage transferBufferUsage);
} // namespace convert
} // namespace paranoixa::sdlgpu
#endif // !PARANOIXA_SDLGPU_CONVERT_HPP