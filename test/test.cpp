#include "../library/imgui/imgui.h"

#include <paranoixa/paranoixa.hpp>

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imnodes.h>

#include <backends/imgui_impl_sdl3.h>
#include <imgui_impl_paranoixa.hpp>

#include <iostream>

void MemoryAllocatorTest();
void PtrTest();

#ifndef _countof
#define _countof(x) (sizeof(x) / sizeof(x[0]))
#endif
class FileLoader {
public:
  bool Load(const char *filePath, std::vector<char> &fileData,
            std::ios_base::openmode openMode = std::ios::in | std::ios::binary);
};

std::unique_ptr<FileLoader> &GetFileLoader();

static std::unique_ptr<FileLoader> gFileLoader = nullptr;

std::unique_ptr<FileLoader> &GetFileLoader() {
  if (gFileLoader == nullptr) {
    gFileLoader = std::make_unique<FileLoader>();
  }
  return gFileLoader;
}

bool FileLoader::Load(const char *filePath, std::vector<char> &fileData,
                      std::ios_base::openmode openMode) {
  std::string openModeStr;

  if (openMode & std::ios::in) {
    openModeStr += "r";
  }
  if (openMode & std::ios::binary) {
    openModeStr += "b";
  }

  auto file = SDL_IOFromFile(filePath, openModeStr.c_str());
  size_t size;
  void *data = SDL_LoadFile_IO(file, &size, true);

  if (data) {
    fileData.resize(size);
    memcpy(fileData.data(), data, size);
    return true;
  }
  return false;
}

int main() {

  using namespace paranoixa;

  // TODO: Add unit tests
  MemoryAllocatorTest();
  PtrTest();
  auto allocator = Paranoixa::CreateAllocator(0x8000);
  STLAllocator<int> stdAllocator{allocator};
  std::vector<int, STLAllocator<int>> vec({allocator});
  vec.push_back(1);
  {
    if (!SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL: %s",
                   SDL_GetError());
    }
    SDL_Surface *surface = SDL_LoadBMP("res/texture.bmp");
    uint32_t windowFlags = SDL_WINDOW_RESIZABLE;
    auto *window =
        SDL_CreateWindow("test", surface->w, surface->h, windowFlags);
    {

      auto backend = Paranoixa::CreateBackend(allocator, GraphicsAPI::SDLGPU);
      auto device = backend->CreateDevice({allocator, true});
      device->ClaimWindow(window);
      // Setup Dear ImGui context
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO &io = ImGui::GetIO();
      (void)io;
      io.ConfigFlags |=
          ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

      // Setup Dear ImGui style
      ImGui::StyleColorsDark();
      // ImGui::StyleColorsLight();

      // Setup Platform/Renderer backends
      ImGui_ImplSDL3_InitForSDLGPU(window);
      ImGui_ImplParanoixa_InitInfo init_info = {};
      init_info.Allocator = allocator;
      init_info.Device = device;
      init_info.ColorTargetFormat = px::TextureFormat::B8G8R8A8_UNORM;
      init_info.MSAASamples = px::SampleCount::x1;
      ImGui_ImplParanoixa_Init(&init_info);

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
      Texture::CreateInfo textureCreateInfo{};
      textureCreateInfo.allocator = allocator;
      textureCreateInfo.width = static_cast<uint32_t>(surface->w);
      textureCreateInfo.height = static_cast<uint32_t>(surface->h);
      textureCreateInfo.layerCountOrDepth = 1, textureCreateInfo.numLevels = 1;
      textureCreateInfo.sampleCount = SampleCount::x1;
      textureCreateInfo.format = TextureFormat::R8G8B8A8_UNORM;
      textureCreateInfo.type = TextureType::Texture2D;
      textureCreateInfo.usage = TextureUsage::Sampler;
      auto texture = device->CreateTexture(textureCreateInfo);

      TransferBuffer::CreateInfo stagingTextureBufferCI = {
          .allocator = allocator,
          .usage = TransferBufferUsage::Upload,
          .size = textureCreateInfo.width * textureCreateInfo.height * 4,
      };
      auto stagingTextureBuffer =
          device->CreateTransferBuffer(stagingTextureBufferCI);
      auto mapped = stagingTextureBuffer->Map(false);
      memcpy(mapped, data.data(), data.size());
      stagingTextureBuffer->Unmap();

      auto &fileLoader = GetFileLoader();
      std::vector<char> vertCode, fragCode;
      // load in plain
      fileLoader->Load("res/shader.vert.spv", vertCode);
      fileLoader->Load("res/shader.frag.spv", fragCode);

      Shader::CreateInfo vsci = {
          .allocator = allocator,
          .size = vertCode.size(),
          .data = vertCode.data(),
          .entrypoint = "main",
          .stage = ShaderStage::Vertex,
          .numSamplers = 0,
          .numStorageTextures = 0,
          .numUniformBuffers = 0,
      };
      auto vs = device->CreateShader(vsci);

      Shader::CreateInfo fsci = {
          .allocator = allocator,
          .size = fragCode.size(),
          .data = fragCode.data(),
          .entrypoint = "main",
          .stage = ShaderStage::Fragment,
          .numSamplers = 1,
          .numStorageTextures = 0,
          .numUniformBuffers = 2,
      };
      auto fs = device->CreateShader(fsci);

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
      Buffer::CreateInfo vbci = {
          .allocator = allocator,
          .usage = BufferUsage::Vertex,
          .size = sizeof(triangleVerts),
      };
      auto vertexBuffer = device->CreateBuffer(vbci);
      TransferBuffer::CreateInfo stagingVertexBufferCI = {
          .allocator = allocator,
          .usage = TransferBufferUsage::Upload,
          .size = vbci.size,
      };
      auto stagingVertexBuffer =
          device->CreateTransferBuffer(stagingVertexBufferCI);
      mapped = stagingVertexBuffer->Map(false);
      memcpy(mapped, triangleVerts, vbci.size);
      stagingVertexBuffer->Unmap();

      // transfer texture/vertex buffer to gpu
      auto command = device->AcquireCommandBuffer({allocator});
      auto copyPass = command->BeginCopyPass();

      {

        TextureTransferInfo info{
            .transferBuffer = stagingTextureBuffer,
            .offset = 0,
        };
        TextureRegion region{
            .texture = texture,
            .width = textureCreateInfo.width,
            .height = textureCreateInfo.height,
            .depth = 1,
        };
        copyPass->UploadTexture(info, region, false);
      }
      {
        BufferTransferInfo location{
            .transferBuffer = stagingVertexBuffer,
            .offset = 0,
        };
        BufferRegion region{
            .buffer = vertexBuffer,
            .offset = 0,
            .size = vbci.size,
        };
        copyPass->UploadBuffer(location, region, false);
      }
      command->EndCopyPass(copyPass);
      device->SubmitCommandBuffer(command);

      VertexBufferDescription vbDesc = {};
      vbDesc.inputRate = VertexInputRate::Vertex;
      vbDesc.instanceStepRate = 0;
      vbDesc.pitch = sizeof(float) * 8;
      vbDesc.slot = 0;
      Array<VertexBufferDescription> vbDescs(allocator);
      vbDescs.push_back(vbDesc);

      Array<VertexAttribute> vertexAttributes(allocator);
      {
        vertexAttributes.push_back({0, 0, VertexElementFormat::Float3, 0});
        vertexAttributes.push_back(
            {1, 0, VertexElementFormat::Float2, sizeof(float) * 3});
        vertexAttributes.push_back(
            {2, 0, VertexElementFormat::Float3, sizeof(float) * 5});
      };
      ColorTargetDescription colorTargetDescription = {
          .format = TextureFormat::B8G8R8A8_UNORM,
          .blendState = ColorTargetBlendState{},
      };
      Array<ColorTargetDescription> colorTargetDescriptions(allocator);
      colorTargetDescriptions.push_back(colorTargetDescription);
      GraphicsPipeline::CreateInfo pipelineCreateInfo{allocator};
      pipelineCreateInfo.allocator = allocator;
      pipelineCreateInfo.vertexShader = vs;
      pipelineCreateInfo.fragmentShader = fs;
      pipelineCreateInfo.vertexInputState.vertexBufferDescriptions = vbDescs;
      pipelineCreateInfo.vertexInputState.vertexAttributes = vertexAttributes;
      pipelineCreateInfo.primitiveType = PrimitiveType::TriangleList;
      pipelineCreateInfo.rasterizerState.fillMode = FillMode::Fill;
      pipelineCreateInfo.rasterizerState.cullMode = CullMode::None;
      pipelineCreateInfo.rasterizerState.frontFace = FrontFace::Clockwise;
      pipelineCreateInfo.multiSampleState = {};
      pipelineCreateInfo.depthStencilState = {};
      pipelineCreateInfo.targetInfo = {allocator};
      pipelineCreateInfo.targetInfo.colorTargetDescriptions =
          colorTargetDescriptions;
      auto pipeline = device->CreateGraphicsPipeline(pipelineCreateInfo);

      Sampler::CreateInfo samplerCI{};
      samplerCI.allocator = allocator;
      samplerCI.minFilter = Filter::Linear;
      samplerCI.magFilter = Filter::Linear;
      samplerCI.mipmapMode = MipmapMode::Nearest;
      samplerCI.addressModeU = AddressMode::ClampToEdge;
      samplerCI.addressModeV = AddressMode::ClampToEdge;
      samplerCI.addressModeW = AddressMode::ClampToEdge;
      auto sampler = device->CreateSampler(samplerCI);

      Ptr<Texture> swapchainTexture = nullptr;
      bool running = true;
      while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
          ImGui_ImplSDL3_ProcessEvent(&event);
          if (event.type == SDL_EVENT_QUIT) {
            running = false;
            break;
          }
        }
        CommandBuffer::CreateInfo commandBufferCI{};
        commandBufferCI.allocator = allocator;
        auto cmdbuf = device->AcquireCommandBuffer(commandBufferCI);

        swapchainTexture = device->AcquireSwapchainTexture(cmdbuf);

        ColorTargetInfo colorTargetInfo = {
            .texture = swapchainTexture,
            .loadOp = LoadOp::Clear,
            .storeOp = StoreOp::Store,
        };
        auto colorTargetInfos = Array<ColorTargetInfo>(allocator);
        colorTargetInfos.push_back(colorTargetInfo);
        // Start the Dear ImGui frame
        ImGui_ImplParanoixa_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Hello, world!");
        ImGui::End();
        // Rendering
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();

        Imgui_ImplParanoixa_PrepareDrawData(draw_data, cmdbuf);
        auto renderPass = cmdbuf->BeginRenderPass(colorTargetInfos, {});
        renderPass->BindGraphicsPipeline(pipeline);
        Array<BufferBinding> bindings(allocator);
        bindings.push_back({vertexBuffer, 0});
        renderPass->BindVertexBuffers(0, bindings);
        auto textureBindings = Array<TextureSamplerBinding>(allocator);
        textureBindings.push_back({sampler, texture});
        renderPass->BindFragmentSamplers(0, textureBindings);
        renderPass->DrawPrimitives(6, 1, 0, 0);

        // Render ImGui
        ImGui_ImplParanoixa_RenderDrawData(draw_data, cmdbuf, renderPass);

        cmdbuf->EndRenderPass(renderPass);
        device->SubmitCommandBuffer(cmdbuf);
      }
    }

    return 0;
  }
}

void MemoryAllocatorTest() {
  using namespace paranoixa;
  std::print("Memory Allocator Test");
  AllocatorPtr allocator = Paranoixa::CreateAllocator(0x2000);
  void *ptr = allocator->Allocate(128);
  allocator->Free(ptr, 128);
  std::cout << "----------------------------------------------" << std::endl;
}

void PtrTest() {
  struct A {
    int a;
  };
  struct B : A {
    B(int a) : A{a}, b(0) {}
    int b;
  };
  using namespace paranoixa;
  std::cout << "---------------PtrTest------------" << std::endl;
  {
    auto allocator = Paranoixa::CreateAllocator(0x2000);
    {
      Ptr<A> ptr;
      ptr = MakePtr<B>(allocator, 10);
      std::cout << ptr->a << std::endl;
      ptr.reset();
    }
    std::cout << "allocator.count : " << allocator.use_count() << std::endl;
  }
  std::cout << "---------------------------------" << std::endl;
}