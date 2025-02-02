#ifndef EMSCRIPTEN
#include "sdlgpu_renderer.hpp"
#include "sdlgpu_convert.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace paranoixa {

SDLGPURenderer::SDLGPURenderer(AllocatorPtr allcator) {}
SDLGPURenderer::~SDLGPURenderer() {}
Ptr<Device> SDLGPUBackend::CreateDevice(const Device::CreateInfo &createInfo) {
  SDL_GPUDevice *device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                              createInfo.debugMode, nullptr);
  if (!device) {
    std::cout << "Failed to create GPU device" << std::endl;
  }
  return MakePtr<SDLGPUDevice>(createInfo.allocator, createInfo, device);
}
void SDLGPUCopyPass::UploadTexture(const TextureTransferInfo &src,
                                   const TextureRegion &dst, bool cycle) {
  SDL_GPUTextureTransferInfo transferInfo = {
      .transfer_buffer =
          DownCast<SDLGPUTransferBuffer>(src.transferBuffer)->GetNative(),
      .offset = src.offset,
  };
  SDL_GPUTextureRegion region = {
      .texture = DownCast<SDLGPUTexture>(dst.texture)->GetNative(),
      .x = 0,
      .y = 0,
      .w = dst.width,
      .h = dst.height,
      .d = dst.depth,
  };
  SDL_UploadToGPUTexture(this->copyPass, &transferInfo, &region, cycle);
}
void SDLGPUCopyPass::DownloadTexture(const TextureRegion &src,
                                     const TextureTransferInfo &dst,
                                     bool cycle) {}
void SDLGPUCopyPass::UploadBuffer(const BufferTransferInfo &src,
                                  const BufferRegion &dst, bool cycle) {
  SDL_GPUTransferBufferLocation transferInfo = {
      .transfer_buffer =
          DownCast<SDLGPUTransferBuffer>(src.transferBuffer)->GetNative(),
      .offset = src.offset};
  SDL_GPUBufferRegion region = {
      .buffer = DownCast<SDLGPUBuffer>(dst.buffer)->GetNative(),
      .offset = dst.offset,
      .size = dst.size};
  SDL_UploadToGPUBuffer(this->copyPass, &transferInfo, &region, cycle);
}
void SDLGPUCopyPass::DownloadBuffer(const BufferRegion &src,
                                    const BufferTransferInfo &dst, bool cycle) {
}
void SDLGPURenderPass::BindGraphicsPipeline(Ptr<GraphicsPipeline> pipeline) {
  SDL_BindGPUGraphicsPipeline(
      this->renderPass,
      DownCast<SDLGPUGraphicsPipeline>(pipeline)->GetNative());
}
void SDLGPURenderPass::BindVertexBuffers(uint32_t startSlot,
                                         const Array<BufferBinding> &bindings) {
  Array<SDL_GPUBufferBinding> bufferBindings(allocator);
  bufferBindings.resize(bindings.size());
  for (int i = 0; i < bindings.size(); ++i) {
    bufferBindings[i] = {};
    bufferBindings[i].buffer =
        DownCast<SDLGPUBuffer>(bindings[i].buffer)->GetNative();
    bufferBindings[i].offset = bindings[i].offset;
  }
  SDL_BindGPUVertexBuffers(this->renderPass, startSlot, bufferBindings.data(),
                           bufferBindings.size());
}
void SDLGPURenderPass::BindFragmentSamplers(
    uint32_t startSlot, const Array<TextureSamplerBinding> &bindings) {
  Array<SDL_GPUTextureSamplerBinding> samplerBindings(allocator);
  samplerBindings.resize(bindings.size());
  for (int i = 0; i < samplerBindings.size(); ++i) {
    samplerBindings[i] = {};
    samplerBindings[i].sampler =
        DownCast<SDLGPUSampler>(bindings[i].sampler)->GetNative();
    samplerBindings[i].texture =
        DownCast<SDLGPUTexture>(bindings[i].texture)->GetNative();
  }
  SDL_BindGPUFragmentSamplers(this->renderPass, startSlot,
                              samplerBindings.data(), samplerBindings.size());
}
void SDLGPURenderPass::DrawPrimitives(uint32_t vertexCount,
                                      uint32_t instanceCount,
                                      uint32_t firstVertex,
                                      uint32_t firstInstance) {
  SDL_DrawGPUPrimitives(this->renderPass, vertexCount, instanceCount,
                        firstVertex, firstInstance);
}
Ptr<CopyPass> SDLGPUCommandBuffer::BeginCopyPass() {
  auto *pass = SDL_BeginGPUCopyPass(this->commandBuffer);
  return MakePtr<SDLGPUCopyPass>(GetCreateInfo().allocator,
                                 GetCreateInfo().allocator, *this, pass);
}
void SDLGPUCommandBuffer::EndCopyPass(Ptr<CopyPass> copyPass) {
  SDL_EndGPUCopyPass(DownCast<SDLGPUCopyPass>(copyPass)->GetNative());
}
Ptr<RenderPass> SDLGPUCommandBuffer::BeginRenderPass(
    const Array<RenderPass::ColorTargetInfo> &infos) {
  Array<SDL_GPUColorTargetInfo> colorTargetInfos(GetCreateInfo().allocator);
  colorTargetInfos.resize(infos.size());
  for (int i = 0; i < infos.size(); ++i) {
    colorTargetInfos[i] = {};
    colorTargetInfos[i].texture =
        DownCast<SDLGPUTexture>(infos[i].texture)->GetNative();
    colorTargetInfos[i].load_op = convert::LoadOpFrom(infos[i].loadOp);
    colorTargetInfos[i].store_op = convert::StoreOpFrom(infos[i].storeOp);
    colorTargetInfos[i].clear_color = {0, 0, 0, 0};
  }
  auto *renderPass = SDL_BeginGPURenderPass(
      commandBuffer, colorTargetInfos.data(), colorTargetInfos.size(), NULL);
  return MakePtr<SDLGPURenderPass>(
      GetCreateInfo().allocator, GetCreateInfo().allocator, *this, renderPass);
}
void SDLGPUCommandBuffer::EndRenderPass(Ptr<RenderPass> renderPass) {
  SDL_EndGPURenderPass(DownCast<SDLGPURenderPass>(renderPass)->GetNative());
}
SDLGPUGraphicsPipeline::~SDLGPUGraphicsPipeline() {
  SDL_ReleaseGPUGraphicsPipeline(device.GetNative(), pipeline);
}
SDLGPUDevice::~SDLGPUDevice() {
  if (window)
    SDL_ReleaseWindowFromGPUDevice(device, window);
  SDL_DestroyGPUDevice(device);
}
void SDLGPUDevice::ClaimWindow(void *window) {
  if (!SDL_ClaimWindowForGPUDevice(device, static_cast<SDL_Window *>(window))) {
    std::cout << "Failed to claim window for GPU device" << std::endl;
  }
  this->window = static_cast<SDL_Window *>(window);
}

Ptr<TransferBuffer> SDLGPUDevice::CreateTransferBuffer(
    const TransferBuffer::CreateInfo &createInfo) {
  SDL_GPUTransferBufferCreateInfo stagingTextureBufferCI{};
  stagingTextureBufferCI.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
  stagingTextureBufferCI.size = createInfo.size;

  SDL_GPUTransferBuffer *stagingTextureBuffer =
      SDL_CreateGPUTransferBuffer(device, &stagingTextureBufferCI);
  return MakePtr<SDLGPUTransferBuffer>(createInfo.allocator, createInfo, *this,
                                       stagingTextureBuffer);
}

Ptr<Buffer> SDLGPUDevice::CreateBuffer(const Buffer::CreateInfo &createInfo) {
  SDL_GPUBufferCreateInfo bufferCI{};
  bufferCI.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
  bufferCI.size = createInfo.size;
  SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(device, &bufferCI);
  return MakePtr<SDLGPUBuffer>(createInfo.allocator, createInfo, *this, buffer);
}

Ptr<Texture>
SDLGPUDevice::CreateTexture(const Texture::CreateInfo &createInfo) {
  SDL_GPUTextureCreateInfo textureCreateInfo = {
      .type = SDL_GPU_TEXTURETYPE_2D,
      .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
      .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
      .width = createInfo.width,
      .height = createInfo.height,
      .layer_count_or_depth = createInfo.layerCountOrDepth,
      .num_levels = createInfo.numLevels,
      .sample_count = SDL_GPU_SAMPLECOUNT_1};

  SDL_GPUTexture *texture = SDL_CreateGPUTexture(device, &textureCreateInfo);
  return MakePtr<SDLGPUTexture>(createInfo.allocator, createInfo, *this,
                                texture, false);
}
Ptr<Sampler>
SDLGPUDevice::CreateSampler(const Sampler::CreateInfo &createInfo) {
  SDL_GPUSamplerCreateInfo samplerCreateInfo = {
      .min_filter = SDL_GPU_FILTER_LINEAR,
      .mag_filter = SDL_GPU_FILTER_LINEAR,
      .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
      .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
      .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
      .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
  };
  SDL_GPUSampler *sampler = SDL_CreateGPUSampler(device, &samplerCreateInfo);
  return MakePtr<SDLGPUSampler>(createInfo.allocator, createInfo, *this,
                                sampler);
}

SDLGPUTransferBuffer::~SDLGPUTransferBuffer() {
  SDL_ReleaseGPUTransferBuffer(device.GetNative(), transferBuffer);
}

void *SDLGPUTransferBuffer::Map() {
  return SDL_MapGPUTransferBuffer(device.GetNative(), this->transferBuffer,
                                  false);
}
void SDLGPUTransferBuffer::Unmap() {
  SDL_UnmapGPUTransferBuffer(device.GetNative(), this->transferBuffer);
}

SDLGPUBuffer::~SDLGPUBuffer() {
  SDL_ReleaseGPUBuffer(device.GetNative(), buffer);
}

Ptr<Shader> SDLGPUDevice::CreateShader(const Shader::CreateInfo &createInfo) {
  SDL_GPUShaderCreateInfo shaderCI = {};
  shaderCI.stage = createInfo.stage == ShaderStage::Vertex
                       ? SDL_GPU_SHADERSTAGE_VERTEX
                       : SDL_GPU_SHADERSTAGE_FRAGMENT;
  shaderCI.code_size = createInfo.size;
  shaderCI.code = reinterpret_cast<const Uint8 *>(createInfo.data);
  shaderCI.format = SDL_GPU_SHADERFORMAT_SPIRV;
  shaderCI.entrypoint = createInfo.entrypoint;
  shaderCI.num_samplers = createInfo.numSamplers;
  shaderCI.num_storage_textures = createInfo.numStorageTextures;
  shaderCI.num_uniform_buffers = createInfo.numUniformBuffers;

  auto *shader = SDL_CreateGPUShader(device, &shaderCI);
  return MakePtr<SDLGPUShader>(createInfo.allocator, createInfo, *this, shader);
}
Ptr<CommandBuffer>
SDLGPUDevice::CreateCommandBuffer(const CommandBuffer::CreateInfo &createInfo) {
  SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(device);
  return MakePtr<SDLGPUCommandBuffer>(createInfo.allocator, createInfo,
                                      commandBuffer);
}

static SDL_GPUCullMode CullModeFrom(CullMode cullMode) {
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
static SDL_GPUFrontFace FrontFaceFrom(FrontFace frontFace) {
  switch (frontFace) {
  case FrontFace::Clockwise:
    return SDL_GPU_FRONTFACE_CLOCKWISE;
  case FrontFace::CounterClockwise:
    return SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
  }
  return SDL_GPU_FRONTFACE_CLOCKWISE;
}
static SDL_GPUPrimitiveType PrimitiveTypeFrom(PrimitiveType primitiveType) {
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
Ptr<GraphicsPipeline> SDLGPUDevice::CreateGraphicsPipeline(
    const GraphicsPipeline::CreateInfo &createInfo) {
  SDL_GPUGraphicsPipelineCreateInfo pipelineCI = {};
  pipelineCI.vertex_shader =
      DownCast<SDLGPUShader>(createInfo.vertexShader)->GetNative();
  pipelineCI.fragment_shader =
      DownCast<SDLGPUShader>(createInfo.fragmentShader)->GetNative();
  pipelineCI.rasterizer_state.cull_mode =
      CullModeFrom(createInfo.rasterizerState.cullMode);
  pipelineCI.rasterizer_state.front_face =
      FrontFaceFrom(createInfo.rasterizerState.frontFace);
  pipelineCI.primitive_type = PrimitiveTypeFrom(createInfo.primitiveType);
  pipelineCI.target_info.num_color_targets =
      createInfo.targetInfo.numColorTargets;
  SDL_GPUColorTargetDescription colorTargetDesc{};
  colorTargetDesc.format = SDL_GetGPUSwapchainTextureFormat(
      device, static_cast<SDL_Window *>(window));
  SDL_GPUColorTargetDescription colorTargetDescs[] = {colorTargetDesc};
  pipelineCI.target_info.color_target_descriptions = colorTargetDescs;
  pipelineCI.vertex_input_state.num_vertex_attributes = 3;
  pipelineCI.vertex_input_state.num_vertex_buffers = 1;
  SDL_GPUVertexAttribute vertexAttributes[] = {
      {0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, 0},
      {1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, sizeof(float) * 3},
      {2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, sizeof(float) * 5},
  };
  pipelineCI.vertex_input_state.vertex_attributes = vertexAttributes;
  SDL_GPUVertexBufferDescription vbDesc = {};
  vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vbDesc.instance_step_rate = 0;
  vbDesc.pitch = sizeof(float) * 8;
  vbDesc.slot = 0;
  SDL_GPUVertexBufferDescription vbDescs[] = {vbDesc};
  pipelineCI.vertex_input_state.vertex_buffer_descriptions = vbDescs;

  auto *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCI);
  return MakePtr<SDLGPUGraphicsPipeline>(createInfo.allocator, createInfo,
                                         *this, pipeline);
}
Ptr<ComputePipeline> SDLGPUDevice::CreateComputePipeline(
    const ComputePipeline::CreateInfo &createInfo) {
  return MakePtr<SDLGPUComputePipeline>(createInfo.allocator, createInfo,
                                        nullptr);
}
void SDLGPUDevice::SubmitCommandBuffer(Ptr<CommandBuffer> commandBuffer) {
  SDL_SubmitGPUCommandBuffer(
      DownCast<SDLGPUCommandBuffer>(commandBuffer)->GetNative());
}
Ptr<Texture>
SDLGPUDevice::AcquireSwapchainTexture(Ptr<CommandBuffer> commandBuffer) {

  auto raw = DownCast<SDLGPUCommandBuffer>(commandBuffer);
  SDL_GPUTexture *nativeTex;
  SDL_AcquireGPUSwapchainTexture(raw->GetNative(), window, &nativeTex, nullptr,
                                 nullptr);

  SDLGPUTexture::CreateInfo ci{};
  ci.allocator = commandBuffer->GetCreateInfo().allocator;
  auto texture = MakePtr<SDLGPUTexture>(
      commandBuffer->GetCreateInfo().allocator, ci, *this, nativeTex, true);
  return texture;
}
SDLGPUTexture::~SDLGPUTexture() {
  if (!isSwapchainTexture)
    SDL_ReleaseGPUTexture(device.GetNative(), texture);
}

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
  textureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
  textureCreateInfo.num_levels = 1;
  textureCreateInfo.layer_count_or_depth = 1;
  SDL_GPUTexture *texture = SDL_CreateGPUTexture(device, &textureCreateInfo);
  if (!texture) {
    std::cout << "Failed to create texture" << std::endl;
    return;
  }
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
  fsci.num_samplers = 1;
  fsci.num_uniform_buffers = 2;
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
        .d = 1,
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

  SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(device);
  if (cmdbuf == NULL) {
    SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
    return;
  }

  SDL_GPUTexture *swapchainTexture;
  if (!SDL_AcquireGPUSwapchainTexture(cmdbuf, static_cast<SDL_Window *>(window),
                                      &swapchainTexture, NULL, NULL)) {
    SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
    return;
  }

  if (swapchainTexture != NULL) {
    SDL_GPUColorTargetInfo colorTargetInfo = {0};
    colorTargetInfo.texture = swapchainTexture;
    colorTargetInfo.clear_color = {1.0f, 0.0f, 0.0f, 1.0f};
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass *renderPass =
        SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

    SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
    SDL_GPUBufferBinding vertexBufferBinding = {.buffer = vertexBuffer,
                                                .offset = 0};
    SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
    SDL_GPUTextureSamplerBinding textureSamplerBinding = {.texture = texture,
                                                          .sampler = sampler};
    SDL_BindGPUFragmentSamplers(renderPass, 0, &textureSamplerBinding, 1);
    SDL_DrawGPUPrimitives(renderPass, 6, 1, 0, 0);

    SDL_EndGPURenderPass(renderPass);
  }
  SDL_SubmitGPUCommandBuffer(cmdbuf);
}
void SDLGPURenderer::ProcessEvent(void *event) {}
void SDLGPURenderer::BeginFrame() {}
void SDLGPURenderer::EndFrame() {}
void SDLGPURenderer::AddGuiUpdateCallBack(std::function<void()> callBack) {}
SDLGPUShader::~SDLGPUShader() {
  SDL_ReleaseGPUShader(device.GetNative(), shader);
}
SDLGPUSampler::~SDLGPUSampler() {
  SDL_ReleaseGPUSampler(device.GetNative(), sampler);
}
} // namespace paranoixa
#endif // EMSCRIPTEN