#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include "gpu.hpp"
#include "imgui_overlay.hpp"
#include "workshop/workshop.hpp"

#include <utils/concurrency.hpp>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

// Declared in imgui_impl_win32.cpp but guarded by #if 0 in the header
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg,
                                              WPARAM wParam, LPARAM lParam);

namespace imgui_overlay {
namespace {
utils::concurrency::container<download_state> state_{};
bool imgui_initialized{false};
std::atomic<bool> shutting_down{false};
HWND game_hwnd{nullptr};
ID3D11Device *imgui_device{nullptr};
WNDPROC original_wndproc{nullptr};

// Confirmation dialog state
struct confirm_state {
  bool active{false};
  std::string title;
  std::string message;
  std::function<void()> on_yes;
  std::function<void()> on_no;
};
utils::concurrency::container<confirm_state> confirm_{};

// Forward declaration
LRESULT CALLBACK wndproc_stub(HWND hwnd, UINT msg, WPARAM wparam,
                              LPARAM lparam);

void apply_cod_theme() {
  auto &style = ImGui::GetStyle();

  // Rounding & spacing
  style.WindowRounding = 4.0f;
  style.FrameRounding = 3.0f;
  style.GrabRounding = 2.0f;
  style.WindowPadding = ImVec2(16.f, 12.f);
  style.FramePadding = ImVec2(8.f, 5.f);
  style.ItemSpacing = ImVec2(10.f, 8.f);
  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 0.0f;
  style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

  // BO3-inspired orange palette
  constexpr auto orange_hi = ImVec4(0.91f, 0.46f, 0.07f, 1.00f); // #E87512
  constexpr auto orange_mid = ImVec4(0.80f, 0.40f, 0.05f, 1.00f);
  constexpr auto orange_lo = ImVec4(0.65f, 0.32f, 0.04f, 1.00f);
  constexpr auto orange_dim = ImVec4(0.40f, 0.20f, 0.03f, 1.00f);

  constexpr auto bg_dark = ImVec4(0.06f, 0.06f, 0.07f, 0.94f);
  constexpr auto bg_panel = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
  constexpr auto bg_frame = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
  constexpr auto border_col = ImVec4(0.30f, 0.18f, 0.06f, 0.60f);

  constexpr auto text_main = ImVec4(0.93f, 0.93f, 0.93f, 1.00f);
  constexpr auto text_dim = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);

  auto *c = style.Colors;

  // Backgrounds
  c[ImGuiCol_WindowBg] = bg_dark;
  c[ImGuiCol_PopupBg] = bg_panel;
  c[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.f, 0.f);

  // Title bar
  c[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.08f, 0.06f, 1.00f);
  c[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.12f, 0.05f, 1.00f);
  c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.06f, 0.06f, 0.80f);

  // Borders
  c[ImGuiCol_Border] = border_col;
  c[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);

  // Text
  c[ImGuiCol_Text] = text_main;
  c[ImGuiCol_TextDisabled] = text_dim;

  // Frames (input boxes, progress bar bg)
  c[ImGuiCol_FrameBg] = bg_frame;
  c[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.14f, 0.12f, 1.00f);
  c[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.16f, 0.10f, 1.00f);

  // Buttons
  c[ImGuiCol_Button] = orange_lo;
  c[ImGuiCol_ButtonHovered] = orange_mid;
  c[ImGuiCol_ButtonActive] = orange_hi;

  // Headers (collapsing, selectable)
  c[ImGuiCol_Header] = orange_dim;
  c[ImGuiCol_HeaderHovered] = orange_lo;
  c[ImGuiCol_HeaderActive] = orange_mid;

  // Scroll bar
  c[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.60f);
  c[ImGuiCol_ScrollbarGrab] = orange_dim;
  c[ImGuiCol_ScrollbarGrabHovered] = orange_lo;
  c[ImGuiCol_ScrollbarGrabActive] = orange_mid;

  // Separator
  c[ImGuiCol_Separator] = ImVec4(0.30f, 0.20f, 0.08f, 0.50f);
  c[ImGuiCol_SeparatorHovered] = orange_lo;
  c[ImGuiCol_SeparatorActive] = orange_mid;

  // Check / slider
  c[ImGuiCol_CheckMark] = orange_hi;
  c[ImGuiCol_SliderGrab] = orange_mid;
  c[ImGuiCol_SliderGrabActive] = orange_hi;

  // Progress bar fill uses PlotHistogram
  c[ImGuiCol_PlotHistogram] = orange_hi;
  c[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.55f, 0.10f, 1.00f);

  // Resize grip
  c[ImGuiCol_ResizeGrip] = orange_dim;
  c[ImGuiCol_ResizeGripHovered] = orange_lo;
  c[ImGuiCol_ResizeGripActive] = orange_mid;

  // Tabs (override default blue)
  c[ImGuiCol_Tab] = ImVec4(0.14f, 0.11f, 0.08f, 1.00f);
  c[ImGuiCol_TabHovered] = orange_mid;
  c[ImGuiCol_TabSelected] = orange_lo;
  c[ImGuiCol_TabDimmed] = ImVec4(0.10f, 0.08f, 0.06f, 1.00f);
  c[ImGuiCol_TabDimmedSelected] = ImVec4(0.16f, 0.12f, 0.08f, 1.00f);

  // Nav / selection highlight
  c[ImGuiCol_NavHighlight] = orange_hi;
  c[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
  c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

  // Modal dim
  c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.55f);

  // Plot lines
  c[ImGuiCol_PlotLines] = orange_mid;
  c[ImGuiCol_PlotLinesHovered] = orange_hi;

  // Drag/drop target
  c[ImGuiCol_DragDropTarget] = orange_hi;
  c[ImGuiCol_TextSelectedBg] = ImVec4(0.91f, 0.46f, 0.07f, 0.35f);
}

void render_confirmation() {
  const auto cstate = confirm_.copy();
  if (!cstate.active)
    return;

  const auto &io = ImGui::GetIO();
  if (io.DisplaySize.x <= 0.f || io.DisplaySize.y <= 0.f)
    return;

  ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
      ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(560.f, 0.f), ImGuiCond_Always);

  constexpr ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
      ImGuiWindowFlags_AlwaysAutoResize;

  if (!ImGui::Begin(cstate.title.c_str(), nullptr, flags)) {
    ImGui::End();
    return;
  }

  ImGui::TextWrapped("%s", cstate.message.c_str());
  ImGui::Spacing();
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();
  ImGui::Spacing();

  const float button_width =
      (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) *
      0.5f;
  const float btn_h = 32.f;

  // Yes — bright orange (action)
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.43f, 0.06f, 1.f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0.95f, 0.52f, 0.10f, 1.f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0.70f, 0.35f, 0.04f, 1.f));
  if (ImGui::Button("Yes", ImVec2(button_width, btn_h))) {
    const auto cb = cstate.on_yes;
    confirm_.access([](confirm_state &s) { s = {}; });
    if (cb)
      cb();
  }
  ImGui::PopStyleColor(3);

  ImGui::SameLine();

  // No — muted dark gray (dismiss)
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.22f, 0.24f, 1.f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0.32f, 0.32f, 0.34f, 1.f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0.16f, 0.16f, 0.18f, 1.f));
  if (ImGui::Button("No", ImVec2(button_width, btn_h))) {
    const auto no_cb = cstate.on_no;
    confirm_.access([](confirm_state &s) { s = {}; });
    if (no_cb)
      no_cb();
  }
  ImGui::PopStyleColor(3);

  ImGui::End();
}

void render() {
  const auto s = state_.copy();
  if (!s.active || s.hidden)
    return;

  const auto &io = ImGui::GetIO();
  if (io.DisplaySize.x <= 0.f || io.DisplaySize.y <= 0.f)
    return;

  ImGui::SetNextWindowPos(
      ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
      ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(620.f, 0.f), ImGuiCond_Always);

  constexpr ImGuiWindowFlags flags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
      ImGuiWindowFlags_AlwaysAutoResize;

  if (!ImGui::Begin("Downloading Content", nullptr, flags)) {
    ImGui::End();
    return;
  }

  // Title line
  ImGui::TextUnformatted(s.item_name.c_str());

  // Secondary info (file name / file count)
  if (!s.status_line.empty()) {
    ImGui::TextDisabled("%s", s.status_line.c_str());
  }

  ImGui::Spacing();

  // Progress bar
  if (s.total_bytes > 0) {
    float pct = static_cast<float>(s.downloaded_bytes) /
                static_cast<float>(s.total_bytes);
    if (pct > 0.99f)
      pct = 0.99f;
    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%.1f%%", pct * 100.f);
    ImGui::ProgressBar(pct, ImVec2(-1.f, 0.f), overlay);

    ImGui::Text("%.2f MB / %.2f MB", s.downloaded_bytes / 1048576.f,
                s.total_bytes / 1048576.f);
  } else if (s.downloaded_bytes > 0) {
    const double mb =
        static_cast<double>(s.downloaded_bytes) / (1024.0 * 1024.0);
    float pct = static_cast<float>((mb / (mb + 2000.0)) * 0.95);
    if (pct > 0.99f)
      pct = 0.99f;
    if (pct < 0.001f)
      pct = 0.001f;
    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%.1f%%", pct * 100.f);
    ImGui::ProgressBar(pct, ImVec2(-1.f, 0.f), overlay);
    ImGui::Text("%.2f MB downloaded", s.downloaded_bytes / 1048576.f);
  } else {
    // Indeterminate — show "Preparing" or pulsing bar
    const float t = static_cast<float>(ImGui::GetTime());
    const char *label = s.status_line.find("Preparing") != std::string::npos
                            ? "Preparing..."
                            : "Starting download...";
    ImGui::ProgressBar(-1.f * (t - floorf(t)), ImVec2(-1.f, 0.f), label);
  }

  ImGui::Spacing();

  // Speed and ETA on same row when both available
  const bool has_speed = s.speed_bps > 1024.f;
  const bool has_eta = s.eta_seconds >= 0;

  if (has_speed) {
    const auto speed_str =
        workshop::human_readable_size(static_cast<std::uint64_t>(s.speed_bps)) +
        "/s";
    ImGui::Text("Speed: %s", speed_str.c_str());
    if (has_eta)
      ImGui::SameLine(0.f, 24.f);
  }
  if (has_eta) {
    if (s.eta_seconds >= 3600)
      ImGui::Text("ETA: %dh %dm", s.eta_seconds / 3600,
                  (s.eta_seconds % 3600) / 60);
    else if (s.eta_seconds >= 60)
      ImGui::Text("ETA: %dm %ds", s.eta_seconds / 60, s.eta_seconds % 60);
    else
      ImGui::Text("ETA: %ds", s.eta_seconds);
  }

  // Stop button
  if (s.on_cancel) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.12f, 0.12f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.72f, 0.18f, 0.18f, 1.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4(0.42f, 0.08f, 0.08f, 1.f));
    if (ImGui::Button("Stop Download", ImVec2(-1.f, 30.f))) {
      const auto cb = s.on_cancel;
      clear();
      if (cb)
        cb();
    }
    ImGui::PopStyleColor(3);
  }

  ImGui::End();
}

LRESULT CALLBACK wndproc_stub(HWND hwnd, UINT msg, WPARAM wparam,
                              LPARAM lparam) {
  if (imgui_initialized) {
    const auto s = state_.copy();
    const auto c = confirm_.copy();

    if ((s.active && !s.hidden) || c.active) {
      if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
        return TRUE;
      }
    }
  }
  return CallWindowProcA(original_wndproc, hwnd, msg, wparam, lparam);
}

void shutdown_overlay() {
  if (original_wndproc)
    SetWindowLongPtrA(game_hwnd, GWLP_WNDPROC,
                      reinterpret_cast<LONG_PTR>(original_wndproc));
  imgui_initialized = false;
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

void render_to_back_buffer(IDXGISwapChain *swap_chain, ID3D11Device *device,
                           ID3D11DeviceContext *context) {
  ImDrawData *draw_data = ImGui::GetDrawData();
  if (draw_data->TotalVtxCount == 0)
    return;

  CComPtr<ID3D11Texture2D> buffer;
  CComPtr<ID3D11RenderTargetView> target;
  if (FAILED(swap_chain->GetBuffer(0, IID_PPV_ARGS(&buffer))) ||
      FAILED(device->CreateRenderTargetView(buffer, nullptr, &target))) {
    return;
  }

  ID3D11RenderTargetView *previous[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
  CComPtr<ID3D11DepthStencilView> depth;
  context->OMGetRenderTargets(std::size(previous), previous, &depth);
  context->OMSetRenderTargets(1, &target.p, nullptr);
  ImGui_ImplDX11_RenderDrawData(draw_data);
  context->OMSetRenderTargets(std::size(previous), previous, depth);
  for (auto *view : previous) {
    if (view)
      view->Release();
  }
}
} // namespace

void render_frame(IDXGISwapChain *swap_chain) {
  const bool visible =
      confirmation_active() || state_.access<bool>([](const auto &state) {
        return state.active && !state.hidden;
      });
  if (shutting_down || (imgui_initialized && !visible)) {
    return;
  }

  DXGI_SWAP_CHAIN_DESC desc{};
  CComPtr<ID3D11Device> device;
  if (FAILED(swap_chain->GetDesc(&desc)) ||
      FAILED(swap_chain->GetDevice(IID_PPV_ARGS(&device)))) {
    return;
  }
  CComPtr<ID3D11DeviceContext> context;
  device->GetImmediateContext(&context);

  if (imgui_initialized &&
      (game_hwnd != desc.OutputWindow || imgui_device != device.p)) {
    shutdown_overlay();
  }
  if (!imgui_initialized) {
    gpu::check_device(device);
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().LogFilename = nullptr;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    apply_cod_theme();

    if (!ImGui_ImplWin32_Init(desc.OutputWindow)) {
      ImGui::DestroyContext();
      return;
    }
    if (!ImGui_ImplDX11_Init(device, context)) {
      ImGui_ImplWin32_Shutdown();
      ImGui::DestroyContext();
      return;
    }

    game_hwnd = desc.OutputWindow;
    imgui_device = device;
    original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
        game_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndproc_stub)));
    if (!original_wndproc) {
      shutdown_overlay();
      return;
    }
    imgui_initialized = true;
  }

  if (!visible)
    return;

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::GetIO().DisplaySize =
      ImVec2(static_cast<float>(desc.BufferDesc.Width),
             static_cast<float>(desc.BufferDesc.Height));
  ImGui::NewFrame();
  render_confirmation();
  render();
  ImGui::Render();
  render_to_back_buffer(swap_chain, device, context);
}

void update(const download_state &s) {
  state_.access([&](download_state &st) { st = s; });
}

void clear() {
  state_.access([](download_state &st) { st = {}; });
}

download_state get_state() { return state_.copy(); }

void show_confirmation(const std::string &title, const std::string &message,
                       std::function<void()> on_yes,
                       std::function<void()> on_no) {
  std::function<void()> replaced{};
  confirm_.access([&](confirm_state &cstate) {
    if (cstate.active) {
      replaced = std::move(cstate.on_no);
    }
    cstate.active = true;
    cstate.title = title;
    cstate.message = message;
    cstate.on_yes = std::move(on_yes);
    cstate.on_no = std::move(on_no);
  });
  if (replaced) {
    replaced();
  }
}

bool confirmation_active() {
  return confirm_.access<bool>(
      [](const confirm_state &cstate) { return cstate.active; });
}

struct component final : client_component {
  DEFINE_COMPONENT_NAME("imgui_overlay");

  void pre_destroy() override {
    shutting_down = true;
    if (imgui_initialized)
      shutdown_overlay();
  }
};
} // namespace imgui_overlay

REGISTER_COMPONENT(imgui_overlay::component)
