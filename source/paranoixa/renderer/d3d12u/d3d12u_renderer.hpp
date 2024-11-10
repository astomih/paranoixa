#ifndef PARANOIXA_D3D12U_RENDERER_HPP
#define PARANOIXA_D3D12U_RENDERER_HPP
#include <d3d12.h>
#include <renderer/renderer.hpp>

#include <SDL3/SDL.h>

#include "paranoixa.hpp"

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
  AllocatorPtr allocator;
  ID3D12Device *device;
#ifdef _DEBUG
  ID3D12Debug *d3d12Debug;
  ID3D12Debug3 *d3d12Debug3;
#endif
};
} // namespace paranoixa
#endif // PARANOIXA_D3D12U_RENDERER_HPP