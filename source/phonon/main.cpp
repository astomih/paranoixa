#include "../library/imgui/imgui.h"
#include <paranoixa/paranoixa.hpp>

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imnodes.h>

void MemoryAllocatorTest();
void PtrTest();

static SDL_AudioStream *stream;
namespace example {
namespace {
struct Node {
  int id;
  float value;

  Node(const int i, const float v) : id(i), value(v) {}
};

struct Link {
  int id;
  int start_attr, end_attr;
};

struct Editor {
  ImNodesEditorContext *context = nullptr;
  std::vector<Node> nodes;
  std::vector<Link> links;
  int current_id = 0;
};

void show_editor(const char *editor_name, Editor &editor) {
  ImNodes::EditorContextSet(editor.context);

  ImGui::Begin(editor_name);
  ImGui::TextUnformatted("A -- add node");

  ImNodes::BeginNodeEditor();

  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
      ImNodes::IsEditorHovered() && ImGui::IsKeyReleased(ImGuiKey_A)) {
    const int node_id = ++editor.current_id;
    ImNodes::SetNodeScreenSpacePos(node_id, ImGui::GetMousePos());
    ImNodes::SnapNodeToGrid(node_id);
    editor.nodes.push_back(Node(node_id, 0.f));
  }

  int freq = 0;
  for (Node &node : editor.nodes) {
    ImNodes::BeginNode(node.id);

    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted("node");
    ImNodes::EndNodeTitleBar();

    ImNodes::BeginInputAttribute(node.id << 8);
    ImGui::TextUnformatted("input");
    ImNodes::EndInputAttribute();

    ImNodes::BeginStaticAttribute(node.id << 16);
    ImGui::PushItemWidth(120.0f);
    ImGui::DragFloat("value", &node.value, 0.01f);
    ImGui::PopItemWidth();
    ImNodes::EndStaticAttribute();

    ImNodes::BeginOutputAttribute(node.id << 24);
    const float text_width = ImGui::CalcTextSize("output").x;
    ImGui::Indent(120.f + ImGui::CalcTextSize("value").x - text_width);
    ImGui::TextUnformatted("output");
    ImNodes::EndOutputAttribute();

    ImNodes::EndNode();
    freq += node.value;
  }

  for (const Link &link : editor.links) {
    ImNodes::Link(link.id, link.start_attr, link.end_attr);
  }

  ImNodes::EndNodeEditor();

  {
    Link link;
    if (ImNodes::IsLinkCreated(&link.start_attr, &link.end_attr)) {
      link.id = ++editor.current_id;
      editor.links.push_back(link);
    }
  }

  {
    int link_id;
    if (ImNodes::IsLinkDestroyed(&link_id)) {
      auto iter = std::find_if(
          editor.links.begin(), editor.links.end(),
          [link_id](const Link &link) -> bool { return link.id == link_id; });
      assert(iter != editor.links.end());
      editor.links.erase(iter);
    }
  }

  ImGui::End();
}

Editor editor1;
Editor editor2;
} // namespace

void NodeEditorInitialize() {
  editor1.context = ImNodes::EditorContextCreate();
  editor2.context = ImNodes::EditorContextCreate();
  ImNodes::PushAttributeFlag(
      ImNodesAttributeFlags_EnableLinkDetachWithDragClick);

  ImNodesIO &io = ImNodes::GetIO();
  io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
  io.MultipleSelectModifier.Modifier = &ImGui::GetIO().KeyCtrl;

  ImNodesStyle &style = ImNodes::GetStyle();
  style.Flags |=
      ImNodesStyleFlags_GridLinesPrimary | ImNodesStyleFlags_GridSnapping;
}

void NodeEditorShow() { show_editor("editor", editor1); }

void NodeEditorShutdown() {
  ImNodes::PopAttributeFlag();
  ImNodes::EditorContextFree(editor1.context);
  ImNodes::EditorContextFree(editor2.context);
}
} // namespace example

int main() {

  using namespace paranoixa;
  MemoryAllocatorTest();
  PtrTest();
  auto allocator = Paranoixa::CreateAllocator(0x2000);
  {

    SDL_AudioSpec desiredSpec;
    desiredSpec.freq = 44100;           // サンプルレート
    desiredSpec.format = SDL_AUDIO_F32; // 32ビットのオーディオフォーマット
    desiredSpec.channels = 2;           // ステレオ
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                       &desiredSpec, nullptr, nullptr);
    if (stream == nullptr) {
      printf("Uhoh, stream failed to create: %s\n", SDL_GetError());
    } else {

      SDL_ResumeAudioStreamDevice(stream);
    }
    SDL_SetAudioStreamGain(stream, 0.1f);
    example::NodeEditorInitialize();
    ([]() {
      struct Node {
        int id;
        float value;
        Node(const int i, const float v) : id(i), value(v) {}
      };

      // ImGui::Begin("Test");

      example::NodeEditorShow();

      constexpr int minimum_audio = (8000 * sizeof(float)) / 2;
      if (SDL_GetAudioStreamAvailable(stream) < minimum_audio) {
        int i, n;
        static float j = 0;
        float samples[44100] = {};
        // sine
        for (i = 0; i < SDL_arraysize(samples); i++) {
          const float time = j / 44100.0f;
          constexpr int sine_freq = 500;
          // samples[i] += SDL_sinf(6.283185f * sine_freq * time);
          j++;
        }
        j = 0;
        // Sawtooth
        for (i = 1; i <= 44; i++) {
          for (n = 0; n < 44100; n++) {
            const float time = n / 44100.0f;
            constexpr int sine_freq = 500;
            samples[n] += 1.0f / i * SDL_sinf(6.283185f * i * sine_freq * time);
            j++;
          }
        }
        float gain = 0.01f;
        for (n = 0; n < 44100; n++) {

          samples[n] *= gain;
        }
        int rc = SDL_PutAudioStreamData(stream, samples, sizeof(samples));
        if (rc == -1) {
          printf("Uhoh, failed to put samples in stream: %s\n", SDL_GetError());
        }
      }
      ImGui::End();
    });
  }
  return 0;
}

void MemoryAllocatorTest() {}

void PtrTest() {}