#ifndef PARANOIXA_D3D12U_RENDERER_HPP
#define PARANOIXA_D3D12U_RENDERER_HPP
#include <d3d12.h>
#include <renderer/renderer.hpp>

#include <SDL3/SDL.h>

#include "paranoixa.hpp"

#include <dxgi1_6.h>

namespace paranoixa {
class D3d12uRenderer : public Renderer {
public:
  D3d12uRenderer(AllocatorPtr allcator);
  ~D3d12uRenderer() override;
  void Initialize(void *window) override;
  void ProcessEvent(void *event) override;
  void BeginFrame() override;
  void EndFrame() override;

  void AddGuiUpdateCallBack(std::function<void()> callBack) override;

private:
  struct DescriptorHandle {
    D3D12_CPU_DESCRIPTOR_HANDLE hCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE hGPU;
    D3D12_DESCRIPTOR_HEAP_TYPE type;
  };
  struct DescriptorHeapInfo {
    ID3D12DescriptorHeap *heap;
    UINT handleSize = 0;
    UINT usedIndex = 0;
    std::vector<DescriptorHandle> handles;
  };
  void PrepareDevice();
  void PrepareCommandQueue();
  void PrepareDescriptorHeap();
  DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);
  void DeallocateDescriptor(DescriptorHandle descriptor);
  ID3D12DescriptorHeap* GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type);
  DescriptorHeapInfo* GetDescriptorHeapInfo(D3D12_DESCRIPTOR_HEAP_TYPE);

  void Prepare();

  AllocatorPtr allocator;
  ID3D12Device *device;
#ifdef _DEBUG
  ID3D12Debug *d3d12Debug;
  ID3D12Debug3 *d3d12Debug3;
#endif
  IDXGIFactory7* dxgiFactory;
  IDXGIAdapter4* adapter;
  ID3D12CommandQueue *commandQueue;
  DescriptorHeapInfo rtvDescriptorHeap;
  DescriptorHeapInfo dsvDescriptorHeap;
  DescriptorHeapInfo srvDescriptorHeap;
  DescriptorHeapInfo samplerDescriptorHeap;
  ID3D12CommandAllocator *commandAllocator;
};
} // namespace paranoixa
#endif // PARANOIXA_D3D12U_RENDERER_HPP