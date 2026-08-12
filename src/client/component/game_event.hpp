#pragma once

#include <structs/func.hpp>
namespace game_event {
typedef fastcallPtr_t<void()> game_event_cb;

void on_g_init_game(const game_event_cb callback);
void on_g_shutdown_game(const game_event_cb callback);

inline void on_any(const game_event_cb callback) {
  on_g_init_game(callback);
  on_g_shutdown_game(callback);
}

} // namespace game_event