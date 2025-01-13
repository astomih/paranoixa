#include "sdlgpu_renderer.hpp"

#include "../../../../library/dawn/third_party/spirv-headers/src/include/spirv/unified1/spirv.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace paranoixa {

SDLGPURenderer::SDLGPURenderer(AllocatorPtr allcator) {}
SDLGPURenderer::~SDLGPURenderer() {}
void SDLGPURenderer::Initialize(void *window) {

  bool debugMode = true;
#ifdef _DEBUG
  debugMode = true;
#endif
  SDL_GPUDevice *device =
      SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, debugMode, nullptr);
  if (!device) {
    std::cout << "Failed to create GPU device" << std::endl;
  }
  if (!SDL_ClaimWindowForGPUDevice(device, static_cast<SDL_Window *>(window))) {
    std::cout << "Failed to claim window for GPU device" << std::endl;
  }

  // Load texture from SDL
  SDL_Surface *surface = SDL_LoadBMP("res/texture.bmp");
  std::vector<uint8_t> data;
  data.resize(surface->w * surface->h * 4);
  for (int y = 0; y < surface->h; ++y) {
    for (int x = 0; x < surface->w; ++x) {
      auto pixel =
          static_cast<uint32_t *>(surface->pixels) + y * surface->w + x;
      auto r = (*pixel & 0x00FF0000) >> 16;
      auto g = (*pixel & 0x0000FF00) >> 8;
      auto b = (*pixel & 0x000000FF);
      auto a = (*pixel & 0xFF000000) >> 24;
      auto index = (y * surface->w + x) * 4;
      data[index + 0] = r;
      data[index + 1] = g;
      data[index + 2] = b;
      data[index + 3] = a;
    }
  }
  SDL_GPUTextureCreateInfo textureCreateInfo = {};
  textureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
  textureCreateInfo.width = surface->w;
  textureCreateInfo.height = surface->h;
  textureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
  textureCreateInfo.num_levels = 1;
  textureCreateInfo.layer_count_or_depth = 1;
  SDL_GPUTexture *texture = SDL_CreateGPUTexture(device, &textureCreateInfo);
  SDL_GPUTransferBufferCreateInfo stagingTextureBufferCI{
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = (Uint32)data.size()};
  SDL_GPUTransferBuffer *stagingTextureBuffer =
      SDL_CreateGPUTransferBuffer(device, &stagingTextureBufferCI);
  auto *mapped = SDL_MapGPUTransferBuffer(device, stagingTextureBuffer, false);
  memcpy(mapped, data.data(), data.size());
  SDL_UnmapGPUTransferBuffer(device, stagingTextureBuffer);

  auto &fileLoader = GetFileLoader();
  std::vector<char> vertCode, fragCode;
  // load in plain
  fileLoader->Load("res/shader.vert.spv", vertCode);
  fileLoader->Load("res/shader.frag.spv", fragCode);

  SDL_GPUShaderCreateInfo vsci = {};
  vsci.stage = SDL_GPU_SHADERSTAGE_VERTEX;
  vsci.code_size = vertCode.size();
  vsci.code = reinterpret_cast<const Uint8 *>(vertCode.data());
  vsci.format = SDL_GPU_SHADERFORMAT_SPIRV;
  vsci.entrypoint = "main";
  auto *vs = SDL_CreateGPUShader(device, &vsci);

  SDL_GPUShaderCreateInfo fsci = {};
  fsci.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
  fsci.code_size = fragCode.size();
  fsci.code = reinterpret_cast<const Uint8 *>(fragCode.data());
  fsci.format = SDL_GPU_SHADERFORMAT_SPIRV;
  fsci.entrypoint = "main";
  auto *fs = SDL_CreateGPUShader(device, &fsci);

  /*
 (-1, -1)  (1, -1)
    +--------+
    |        |
    |        |
    |        |
    +--------+
 (-1,  1)  (1,  1)
  */
  float triangleVerts[] = {
      -1.f, -1.f, 0.f, 0, 1, 0, 0, 1, // position, uv, color
      -1.f, 1.f,  0.f, 0, 0, 1, 0, 0, //
      1.f,  -1.f, 0.f, 1, 1, 0, 0, 1, //
      1.f,  -1.f, 0.f, 1, 1, 0, 0, 1, //
      -1.f, 1.f,  0.f, 0, 0, 1, 0, 0, //
      1.f,  1.f,  0.f, 1, 0, 1, 0, 0, //
  };
  auto vbci = SDL_GPUBufferCreateInfo{};
  vbci.size = _countof(triangleVerts) * sizeof(float);
  vbci.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  auto *vertexBuffer = SDL_CreateGPUBuffer(device, &vbci);
  SDL_GPUTransferBufferCreateInfo stagingVertexBufferCI{
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = vbci.size};
  auto *stagingVertexBuffer = SDL_CreateGPUTransferBuffer(
      device,
      &stagingVertexBufferCI); // create a staging buffer to upload vertex data
  mapped = SDL_MapGPUTransferBuffer(device, stagingVertexBuffer, false);
  memcpy(mapped, triangleVerts, vbci.size);
  SDL_UnmapGPUTransferBuffer(device, stagingVertexBuffer);

  // transfer texture/vertex buffer to gpu
  SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(commandBuffer);

  {
    SDL_GPUTextureTransferInfo info{
        .transfer_buffer = stagingTextureBuffer,
        .offset = 0,
    };
    SDL_GPUTextureRegion region{
        .texture = texture,
        .w = (Uint32)surface->w,
        .h = (Uint32)surface->h,
    };
    SDL_UploadToGPUTexture(copyPass, &info, &region, false);
  }

  {
    SDL_GPUTransferBufferLocation location{
        .transfer_buffer = stagingVertexBuffer, .offset = 0};
    SDL_GPUBufferRegion region{
        .buffer = vertexBuffer, .offset = 0, .size = vbci.size};
    SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
  }

  SDL_EndGPUCopyPass(copyPass);
  SDL_SubmitGPUCommandBuffer(commandBuffer);

  SDL_GPUVertexBufferDescription vbDesc = {};
  vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vbDesc.instance_step_rate = 0;
  vbDesc.pitch = sizeof(float) * 8;
  vbDesc.slot = 0;

  SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.vertex_shader = vs;
  pipelineCreateInfo.fragment_shader = fs;
  pipelineCreateInfo.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
  pipelineCreateInfo.rasterizer_state.front_face =
      SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
  pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
  pipelineCreateInfo.target_info.num_color_targets = 1;
  SDL_GPUColorTargetDescription colorTargetDesc{};
  colorTargetDesc.format = SDL_GetGPUSwapchainTextureFormat(
      device, static_cast<SDL_Window *>(window));
  SDL_GPUColorTargetDescription colorTargetDescs[] = {colorTargetDesc};
  pipelineCreateInfo.target_info.color_target_descriptions = colorTargetDescs;
  pipelineCreateInfo.vertex_input_state.num_vertex_attributes = 3;
  pipelineCreateInfo.vertex_input_state.num_vertex_buffers = 1;
  SDL_GPUVertexAttribute vertexAttributes[] = {
      {0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0},
      {1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, sizeof(float) * 3},
      {2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 5},
  };
  pipelineCreateInfo.vertex_input_state.vertex_attributes = vertexAttributes;
  SDL_GPUVertexBufferDescription vbDescs[] = {vbDesc};
  pipelineCreateInfo.vertex_input_state.vertex_buffer_descriptions = vbDescs;

  auto *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);

  SDL_GPUSamplerCreateInfo samplerCreateInfo = {};
  samplerCreateInfo.min_filter = SDL_GPU_FILTER_LINEAR;
  samplerCreateInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
  samplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
  samplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
  samplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
  samplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
  auto *sampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);
}
void SDLGPURenderer::ProcessEvent(void *event) {}
void SDLGPURenderer::BeginFrame() {}
void SDLGPURenderer::EndFrame() {}
void SDLGPURenderer::AddGuiUpdateCallBack(std::function<void()> callBack) {}
} // namespace paranoixa