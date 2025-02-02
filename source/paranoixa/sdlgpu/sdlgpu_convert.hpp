#include <SDL3/SDL_gpu.h>
#include <paranoixa.hpp>
namespace paranoixa {
namespace convert {
SDL_GPULoadOp LoadOpFrom(LoadOp loadOp);
SDL_GPUStoreOp StoreOpFrom(StoreOp storeOp);
SDL_GPUCullMode CullModeFrom(CullMode cullMode);
SDL_GPUFrontFace FrontFaceFrom(FrontFace frontFace);
SDL_GPUPrimitiveType PrimitiveTypeFrom(PrimitiveType primitiveType);
} // namespace convert
} // namespace paranoixa
