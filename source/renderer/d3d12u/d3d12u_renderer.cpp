#include "d3d12u_renderer.hpp"

#ifdef _WIN32
#include <d3d12.h>
#include <dxgi1_6.h>
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_system.h>

#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>

#include <windows.h>

#include <iostream>

#include <paranoixa.hpp>

// ComPtr
#include <wrl/client.h>

namespace paranoixa {
D3d12uRenderer::D3d12uRenderer(AllocatorPtr allocator) : allocator(allocator),device(nullptr) {}
D3d12uRenderer::~D3d12uRenderer() {
#ifdef _DEBUG
  d3d12Debug3->Release();
  d3d12Debug->Release();
#endif
  device->Release();
}
#define QUERY_INTERFACE(from,to)\
  from->QueryInterface(__uuidof(decltype(to)),(void**)&to)
  
void D3d12uRenderer::Initialize(void *window) {
  SDL_Window *sdlWindow = static_cast<SDL_Window *>(window);
  HWND hwnd =
      (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow),
                                   SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
  D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device));
#ifdef _DEBUG
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug))))
  {
    d3d12Debug->EnableDebugLayer();
    UINT dxgiFlags = DXGI_CREATE_FACTORY_DEBUG;
  }
  QUERY_INTERFACE(d3d12Debug, d3d12Debug3);
  if(d3d12Debug3)
  {
    d3d12Debug3->SetEnableGPUBasedValidation(TRUE);
  } 
#endif






  

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer bindings
  ImGui_ImplSDL3_InitForD3D(sdlWindow);
  //ImGui_ImplDX12_Init(hwnd, 3, D3D_FEATURE_LEVEL_12_0);
}
void D3d12uRenderer::ProcessEvent(void *event) {}
void D3d12uRenderer::BeginFrame() {}
void D3d12uRenderer::EndFrame() {}
// namespace paranoixa
} // namespace paranoixa
#endif // _WIN32