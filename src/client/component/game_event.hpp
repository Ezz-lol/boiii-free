#pragma once

namespace game_event
{
	void on_g_init_game(const std::function<void()>& callback);
	void on_g_shutdown_game(const std::function<void()>& callback);
}
