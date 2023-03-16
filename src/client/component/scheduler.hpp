#pragma once

namespace scheduler
{
	enum pipeline
	{
		// Asynchronuous pipeline, disconnected from the game
		async = 0,

		// The game's rendering pipeline
		renderer,

		// The game's server thread
		server,

		// The game's main thread
		main,

		// Dvars flags have been patched, ready to be set from config file
		dvars_flags_patched,

		// Dvars are done loading from the config file
		dvars_loaded,

		count,
	};

	static const bool cond_continue = false;
	static const bool cond_end = true;

	void execute(const pipeline type);

	void schedule(const std::function<bool()>& callback, pipeline type = pipeline::async,
	              std::chrono::milliseconds delay = 0ms);
	void loop(const std::function<void()>& callback, pipeline type = pipeline::async,
	          std::chrono::milliseconds delay = 0ms);
	void once(const std::function<void()>& callback, pipeline type = pipeline::async,
	          std::chrono::milliseconds delay = 0ms);
	void on_game_initialized(const std::function<void()>& callback, pipeline type = pipeline::async,
	                         std::chrono::milliseconds delay = 0ms);
}
