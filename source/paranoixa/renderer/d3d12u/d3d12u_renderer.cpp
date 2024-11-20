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

namespace paranoixa {
#define QUERY_INTERFACE(from, to)                                              \
  from->QueryInterface(__uuidof(decltype(to)), (void **)&to)
D3d12uRenderer::D3d12uRenderer(AllocatorPtr allocator)
    : allocator(allocator), device(nullptr) {}
D3d12uRenderer::~D3d12uRenderer() {
  samplerDescriptorHeap.heap->Release();
  srvDescriptorHeap.heap->Release();
  dsvDescriptorHeap.heap->Release();
  rtvDescriptorHeap.heap->Release();

  this->commandQueue->Release();
#ifdef _DEBUG
  this->d3d12Debug3->Release();
  this->d3d12Debug->Release();
#endif
  device->Release();
}

void D3d12uRenderer::Initialize(void *window) {
  auto sdlWindow = static_cast<SDL_Window *>(window);
  auto hwnd = static_cast<HWND>(
      SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow),
                             SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

  PrepareDevice();
  PrepareCommandQueue();
  PrepareDescriptorHeap();

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer bindings
  ImGui_ImplSDL3_InitForD3D(sdlWindow);
  // ImGui_ImplDX12_Init(hwnd, 3, D3D_FEATURE_LEVEL_12_0);
}
void D3d12uRenderer::ProcessEvent(void *event) {}
void D3d12uRenderer::BeginFrame() {}
void D3d12uRenderer::EndFrame() {}
void D3d12uRenderer::AddGuiUpdateCallBack(std::function<void()> callBack) {}

void D3d12uRenderer::PrepareDevice() {
#ifdef _DEBUG
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug)))) {
    d3d12Debug->EnableDebugLayer();
    UINT dxgiFlags = DXGI_CREATE_FACTORY_DEBUG;
  }
  QUERY_INTERFACE(d3d12Debug, d3d12Debug3);
  if (d3d12Debug3) {
    d3d12Debug3->SetEnableGPUBasedValidation(TRUE);
  }
#endif

  UINT dxgiFlags = 0;
  CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&dxgiFactory));

  UINT adapterIndex = 0;
  IDXGIAdapter1 *adapter = nullptr;
  while (DXGI_ERROR_NOT_FOUND !=
         this->dxgiFactory->EnumAdapters1(adapterIndex, &adapter)) {
    DXGI_ADAPTER_DESC1 desc1{};
    adapter->GetDesc1(&desc1);
    ++adapterIndex;
    if (desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }
    // Use able to D3D12?
    HRESULT hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2,
                                   __uuidof(ID3D12Device), nullptr);
    if (SUCCEEDED(hr)) {
      QUERY_INTERFACE(adapter, this->adapter);
      break;
    }
  }
  D3D12CreateDevice(this->adapter, D3D_FEATURE_LEVEL_12_2,
                    IID_PPV_ARGS(&this->device));
}
void D3d12uRenderer::PrepareCommandQueue() {
  D3D12_COMMAND_QUEUE_DESC queueDesc{.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     .Priority = 0,
                                     .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                                     .NodeMask = 0};
  this->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
}
void D3d12uRenderer::PrepareDescriptorHeap() {
  D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                        .NumDescriptors = 32,
                                        .Flags =
                                            D3D12_DESCRIPTOR_HEAP_FLAG_NONE};
  this->device->CreateDescriptorHeap(
      &rtvDesc, IID_PPV_ARGS(&this->rtvDescriptorHeap.heap));
  this->rtvDescriptorHeap.handleSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
      .NumDescriptors = 32,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
  };
  this->device->CreateDescriptorHeap(&dsvHeapDesc,
                                     IID_PPV_ARGS(&dsvDescriptorHeap.heap));
  this->dsvDescriptorHeap.handleSize =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

  D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
      .NumDescriptors = 2048,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE};
  this->device->CreateDescriptorHeap(&srvHeapDesc,
                                     IID_PPV_ARGS(&srvDescriptorHeap.heap));
  this->srvDescriptorHeap.handleSize = device->CreateDescriptorHeap(
      &srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap.heap));

  D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{
      .Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
      .NumDescriptors = 2048,
      .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE};
  this->device->CreateDescriptorHeap(&samplerHeapDesc,
                                     IID_PPV_ARGS(&samplerDescriptorHeap.heap));
  this->samplerDescriptorHeap.handleSize =
      device->GetDescriptorHandleIncrementSize(
          D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}
D3d12uRenderer::DescriptorHandle
D3d12uRenderer::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type) {

  DescriptorHandle handle = {};
  auto info = GetDescriptorHeapInfo(type);
  if (!info->handles.empty()) {
    handle = info->handles.back();
    info->handles.pop_back();
    return handle;
  }
  auto desc = info->heap->GetDesc();
  handle.type = type;
  handle.hCPU = info->heap->GetCPUDescriptorHandleForHeapStart();
  handle.hCPU.ptr += info->handleSize * info->usedIndex;
  handle.hGPU.ptr = 0;
  if (desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
    handle.hGPU = info->heap->GetGPUDescriptorHandleForHeapStart();
    handle.hGPU.ptr += info->handleSize * info->usedIndex;
  }
  info->usedIndex++;
  return handle;
}
void D3d12uRenderer::DeallocateDescriptor(DescriptorHandle descriptor) {
  auto info = GetDescriptorHeapInfo(descriptor.type);
  info->handles.push_back(descriptor);
}
ID3D12DescriptorHeap *
D3d12uRenderer::GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) {
  auto info = GetDescriptorHeapInfo(type);
  return info->heap;
}
D3d12uRenderer::DescriptorHeapInfo *
D3d12uRenderer::GetDescriptorHeapInfo(D3D12_DESCRIPTOR_HEAP_TYPE type) {
  switch (type) {
  case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
    return &srvDescriptorHeap;
  case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
    return &rtvDescriptorHeap;
  case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
    return &dsvDescriptorHeap;
  case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
    return &samplerDescriptorHeap;
  default:
    return nullptr;
  }
}

// namespace paranoixa
} // namespace paranoixa
#endif // _WIN32