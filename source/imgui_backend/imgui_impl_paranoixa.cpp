#include "imgui_impl_paranoixa.hpp"
#include "imgui_impl_paranoixa_shaders.hpp"

struct ImGui_ImplParanoixa_FrameData {
  px::Ptr<px::Buffer> VertexBuffer = nullptr;
  px::Ptr<px::Buffer> IndexBuffer = nullptr;
  uint32_t VertexBufferSize = 0;
  uint32_t IndexBufferSize = 0;
};

struct ImGui_ImplParanoixa_Data {
  ImGui_ImplParanoixa_InitInfo InitInfo;

  // Graphics pipeline & shaders
  px::Ptr<px::Shader> VertexShader = nullptr;
  px::Ptr<px::Shader> FragmentShader = nullptr;
  px::Ptr<px::GraphicsPipeline> Pipeline = nullptr;

  // Font data
  px::Ptr<px::Sampler> FontSampler = nullptr;
  px::Ptr<px::Texture> FontTexture = nullptr;
  px::TextureSamplerBinding FontBinding = {nullptr, nullptr};

  // Frame data for main window
  ImGui_ImplParanoixa_FrameData MainWindowFrameData;
};
static ImGui_ImplParanoixa_Data *ImGui_ImplParanoixa_GetBackendData() {
  return ImGui::GetCurrentContext() ? (ImGui_ImplParanoixa_Data *)ImGui::GetIO()
                                          .BackendRendererUserData
                                    : nullptr;
}
static void Imgui_ImplParanoixa_CreateShaders() {
  // Create the shader modules
  ImGui_ImplParanoixa_Data *bd = ImGui_ImplParanoixa_GetBackendData();
  ImGui_ImplParanoixa_InitInfo *v = &bd->InitInfo;

  auto driver = v->Device->GetDriver();

  px::Shader::CreateInfo vertex_shader_info = {};
  vertex_shader_info.entrypoint = "main";
  vertex_shader_info.stage = px::ShaderStage::Vertex;
  vertex_shader_info.numUniformBuffers = 1;
  vertex_shader_info.numStorageBuffers = 0;
  vertex_shader_info.numStorageTextures = 0;
  vertex_shader_info.numSamplers = 0;

  px::Shader::CreateInfo fragment_shader_info = {};
  fragment_shader_info.entrypoint = "main";
  fragment_shader_info.stage = px::ShaderStage::Fragment;
  fragment_shader_info.numSamplers = 1;
  fragment_shader_info.numStorageBuffers = 0;
  fragment_shader_info.numStorageTextures = 0;
  fragment_shader_info.numUniformBuffers = 0;

  if (driver == "vulkan") {
    vertex_shader_info.format = px::ShaderFormat::SPIRV;
    vertex_shader_info.data = spirv_vertex;
    vertex_shader_info.size = sizeof(spirv_vertex);
    fragment_shader_info.format = px::ShaderFormat::SPIRV;
    fragment_shader_info.data = spirv_fragment;
    fragment_shader_info.size = sizeof(spirv_fragment);
  }
  //   else if (strcmp(driver, "direct3d12") == 0) {
  //     vertex_shader_info.format = SDL_GPU_SHADERFORMAT_DXBC;
  //     vertex_shader_info.code = dxbc_vertex;
  //     vertex_shader_info.code_size = sizeof(dxbc_vertex);
  //     fragment_shader_info.format = SDL_GPU_SHADERFORMAT_DXBC;
  //     fragment_shader_info.code = dxbc_fragment;
  //     fragment_shader_info.code_size = sizeof(dxbc_fragment);
  //   }
  // #ifdef __APPLE__
  //   else {
  //     vertex_shader_info.entrypoint = "main0";
  //     vertex_shader_info.format = SDL_GPU_SHADERFORMAT_METALLIB;
  //     vertex_shader_info.code = metallib_vertex;
  //     vertex_shader_info.code_size = sizeof(metallib_vertex);
  //     fragment_shader_info.entrypoint = "main0";
  //     fragment_shader_info.format = SDL_GPU_SHADERFORMAT_METALLIB;
  //     fragment_shader_info.code = metallib_fragment;
  //     fragment_shader_info.code_size = sizeof(metallib_fragment);
  //   }
  // #endif
  bd->VertexShader = v->Device->CreateShader(vertex_shader_info);
  bd->FragmentShader = v->Device->CreateShader(fragment_shader_info);
  IM_ASSERT(bd->VertexShader != nullptr);
  IM_ASSERT(bd->FragmentShader != nullptr);
}
static void ImGui_ImplParanoixa_CreateGraphicsPipeline() {
  ImGui_ImplParanoixa_Data *bd = ImGui_ImplParanoixa_GetBackendData();
  ImGui_ImplParanoixa_InitInfo *v = &bd->InitInfo;
  Imgui_ImplParanoixa_CreateShaders();

  px::Array<px::VertexBufferDescription> vertex_buffer_desc(v->Allocator);
  vertex_buffer_desc.resize(1);
  vertex_buffer_desc[0].slot = 0;
  vertex_buffer_desc[0].inputRate = px::VertexInputRate::Vertex;
  vertex_buffer_desc[0].instanceStepRate = 0;
  vertex_buffer_desc[0].pitch = sizeof(ImDrawVert);

  px::Array<px::VertexAttribute> vertex_attributes(v->Allocator);
  vertex_attributes.resize(3);
  vertex_attributes[0].bufferSlot = 0;
  vertex_attributes[0].format = px::VertexElementFormat::Float2;
  vertex_attributes[0].location = 0;
  vertex_attributes[0].offset = offsetof(ImDrawVert, pos);

  vertex_attributes[1].bufferSlot = 0;
  vertex_attributes[1].format = px::VertexElementFormat::Float2;
  vertex_attributes[1].location = 1;
  vertex_attributes[1].offset = offsetof(ImDrawVert, uv);

  vertex_attributes[2].bufferSlot = 0;
  vertex_attributes[2].format = px::VertexElementFormat::UByte4_NORM;
  vertex_attributes[2].location = 2;
  vertex_attributes[2].offset = offsetof(ImDrawVert, col);

  px::VertexInputState vertex_input_state{v->Allocator};
  vertex_input_state.vertexAttributes = vertex_attributes;
  vertex_input_state.vertexBufferDescriptions = vertex_buffer_desc;

  px::RasterizerState rasterizer_state = {};
  rasterizer_state.fillMode = px::FillMode::Solid;
  rasterizer_state.cullMode = px::CullMode::None;
  rasterizer_state.frontFace = px::FrontFace::CounterClockwise;
  rasterizer_state.enableDepthBias = false;
  rasterizer_state.enableDepthClip = false;

  px::MultiSampleState multisample_state{};
  multisample_state.sampleCount = v->MSAASamples;
  multisample_state.enableMask = false;

  px::DepthStencilState depth_stencil_state = {};
  depth_stencil_state.enableDepthTest = false;
  depth_stencil_state.enableDepthWrite = false;
  depth_stencil_state.enableStencilTest = false;

  px::ColorTargetBlendState blend_state = {};
  blend_state.enableBlend = true;
  blend_state.srcColorBlendFactor = px::BlendFactor::SrcAlpha;
  blend_state.dstColorBlendFactor = px::BlendFactor::OneMinusSrcAlpha;
  blend_state.colorBlendOp = px::BlendOp::Add;
  blend_state.srcAlphaBlendFactor = px::BlendFactor::One;
  blend_state.dstAlphaBlendFactor = px::BlendFactor::OneMinusSrcAlpha;
  blend_state.alphaBlendOp = px::BlendOp::Add;
  blend_state.colorWriteMask = px::ColorComponent::R | px::ColorComponent::G |
                               px::ColorComponent::B | px::ColorComponent::A;

  px::Array<px::ColorTargetDescription> color_target_desc(v->Allocator);
  color_target_desc.resize(1);
  color_target_desc[0].format = v->ColorTargetFormat;
  color_target_desc[0].blendState = blend_state;

  px::TargetInfo target_info = {v->Allocator};
  target_info.colorTargetDescriptions = color_target_desc;
  target_info.hasDepthStencilTarget = false;

  px::GraphicsPipeline::CreateInfo pipeline_info = {v->Allocator};
  pipeline_info.vertexShader = bd->VertexShader;
  pipeline_info.fragmentShader = bd->FragmentShader;
  pipeline_info.vertexInputState = vertex_input_state;
  pipeline_info.primitiveType = px::PrimitiveType::TriangleList;
  pipeline_info.rasterizerState = rasterizer_state;
  pipeline_info.multiSampleState = multisample_state;
  pipeline_info.depthStencilState = depth_stencil_state;
  pipeline_info.targetInfo = target_info;

  bd->Pipeline = v->Device->CreateGraphicsPipeline(pipeline_info);
  IM_ASSERT(bd->Pipeline != nullptr && "Failed to create graphics pipeline");
}
IMGUI_IMPL_API bool
ImGui_ImplParanoixa_Init(ImGui_ImplParanoixa_InitInfo *info) {
  ImGuiIO &io = ImGui::GetIO();
  IMGUI_CHECKVERSION();
  IM_ASSERT(io.BackendRendererUserData == nullptr &&
            "Already initialized a renderer backend!");

  // Setup backend capabilities flags
  ImGui_ImplParanoixa_Data *bd = IM_NEW(ImGui_ImplParanoixa_Data)();
  io.BackendRendererUserData = (void *)bd;
  io.BackendRendererName = "imgui_impl_paranoixa";
  io.BackendFlags |=
      ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the
                                              // ImDrawCmd::VtxOffset field,
                                              // allowing for large meshes.

  IM_ASSERT(info->Device != nullptr);
  IM_ASSERT(info->ColorTargetFormat != px::TextureFormat::Invalid);

  bd->InitInfo = *info;

  ImGui_ImplParanoixa_CreateDeviceObjects();
}

IMGUI_IMPL_API void ImGui_ImplParanoixa_Shutdown() {
  return IMGUI_IMPL_API void();
}

IMGUI_IMPL_API void ImGui_ImplParanoixa_NewFrame() {
  return IMGUI_IMPL_API void();
}

IMGUI_IMPL_API void
Imgui_ImplParanoixa_PrepareDrawData(ImDrawData *draw_data,
                                    px::Ptr<px::CommandBuffer> command_buffer) {
  return IMGUI_IMPL_API void();
}

IMGUI_IMPL_API void
ImGui_ImplParanoixa_RenderDrawData(ImDrawData *draw_data,
                                   px::Ptr<px::CommandBuffer> command_buffer,
                                   px::Ptr<px::RenderPass> render_pass,
                                   px::Ptr<px::GraphicsPipeline> pipeline) {
  return IMGUI_IMPL_API void();
}

IMGUI_IMPL_API void ImGui_ImplParanoixa_CreateDeviceObjects() {
  ImGui_ImplParanoixa_Data *bd = ImGui_ImplParanoixa_GetBackendData();
  ImGui_ImplParanoixa_InitInfo *v = &bd->InitInfo;

  if (!bd->FontSampler) {
    // Bilinear sampling is required by default. Set 'io.Fonts->Flags |=
    // ImFontAtlasFlags_NoBakedLines' or 'style.AntiAliasedLinesUseTex = false'
    // to allow point/nearest sampling.
    px::Sampler::CreateInfo sampler_info = {};
    sampler_info.minFilter = px::Filter::Linear;
    sampler_info.magFilter = px::Filter::Linear;
    sampler_info.mipmapMode = px::MipmapMode::Linear;
    sampler_info.addressModeU = px::AddressMode::ClampToEdge;
    sampler_info.addressModeV = px::AddressMode::ClampToEdge;
    sampler_info.addressModeW = px::AddressMode::ClampToEdge;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = -1000.0f;
    sampler_info.maxLod = 1000.0f;
    sampler_info.enableAnisotropy = false;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.enableCompare = false;

    bd->FontSampler = v->Device->CreateSampler(sampler_info);
    bd->FontBinding.sampler = bd->FontSampler;
    IM_ASSERT(bd->FontSampler != nullptr &&
              "Failed to create font sampler, call SDL_GetError() for more "
              "information");
  }

  ImGui_ImplParanoixa_CreateGraphicsPipeline();
  ImGui_ImplParanoixa_CreateFontsTexture();
}

IMGUI_IMPL_API void ImGui_ImplParanoixa_DestroyDeviceObjects() {
  return IMGUI_IMPL_API void();
}

IMGUI_IMPL_API void ImGui_ImplParanoixa_CreateFontsTexture() {
  return IMGUI_IMPL_API void();
}

IMGUI_IMPL_API void ImGui_ImplParanoixa_DestroyFontsTexture() {
  return IMGUI_IMPL_API void();
}
