#pragma once
#include <cstddef>
#include <functional>
#include <string>

struct IDXGISwapChain;

namespace imgui_overlay {
struct download_state {
  bool active{false};
  bool hidden{false};
  std::string item_name; // e.g. "Map: zm_cosmodrome"
  std::size_t downloaded_bytes{0};
  std::size_t total_bytes{0}; // 0 = unknown (indeterminate bar)
  float speed_bps{0.0f};      // bytes/sec
  int eta_seconds{-1};        // -1 = unknown
  std::string status_line;    // secondary info, e.g. "File 3/12: map.xpak"
  std::function<void()> on_cancel{nullptr}; // nullptr = hide Stop button
};

/**
 * @brief Draw the download UI immediately before the game presents a frame.
 * @details Called by the game swap-chain hook in flip_model.cpp. The caller
 * owns the chain and calls the original DXGI Present after this returns.
 * @param swap_chain The game's Direct3D 11 swap chain.
 */
void render_frame(IDXGISwapChain *swap_chain);

void update(const download_state &state);
void clear();
download_state get_state();

void show_confirmation(const std::string &title, const std::string &message,
                       std::function<void()> on_yes,
                       std::function<void()> on_no = {});
bool confirmation_active();
} // namespace imgui_overlay
