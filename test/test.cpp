#include "../library/imgui/imgui.h"
#include "paranoixa/renderer/renderer.hpp"

#include <paranoixa/memory/tlsf_allocator.hpp>
#include <paranoixa/paranoixa.hpp>

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imnodes.h>

void MemoryAllocatorTest();
void PtrTest();

int main() {

  using namespace paranoixa;

  // TODO: Add unit tests
  MemoryAllocatorTest();
  PtrTest();
  auto allocator = MakeAllocatorPtr<TLSFAllocator>(0x2000);
  StdAllocator<int> stdAllocator{allocator};
  std::vector<int, StdAllocator<int>> vec({allocator});
  vec.push_back(1);
  {

    auto app = Paranoixa({.allocator = allocator, .api = GraphicsAPI::SDLGPU});
    app.GetRenderer()->AddGuiUpdateCallBack([]() {
      struct Node {
        int id;
        float value;
        Node(const int i, const float v) : id(i), value(v) {}
      };

      ImGui::Begin("Test");
      ImGui::End();
    });
    app.Run();
  }
  return 0;
}

void MemoryAllocatorTest() {
  using namespace paranoixa;
  std::print("Memory Allocator Test");
  Allocator *allocator = new TLSFAllocator(0x2000);
  void *ptr = allocator->Allocate(128);
  allocator->Free(ptr, 128);
  delete allocator;
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
    auto allocator = MakeAllocatorPtr<TLSFAllocator>(0x2000);
    {
      Ptr<A> ptr{allocator};
      ptr = MakePtr<B>(allocator, 10);
      std::cout << ptr->a << std::endl;
      ptr.Reset();
    }
    {
      UniquePtr<A> ptr{allocator};
      ptr = MakeUnique<B>(allocator, 10);
    }
    std::cout << "allocator.count : " << allocator.use_count() << std::endl;
  }
  std::cout << "---------------------------------" << std::endl;
}