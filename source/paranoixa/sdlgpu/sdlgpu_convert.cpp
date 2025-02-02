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
} // namespace convert
} // namespace paranoixa