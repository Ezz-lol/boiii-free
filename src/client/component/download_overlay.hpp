#pragma once
#include <string>
#include <cstddef>
#include <functional>

namespace download_overlay
{
	struct download_state
	{
		bool active{false};
		std::string item_name;                    // e.g. "Map: zm_cosmodrome"
		std::size_t downloaded_bytes{0};
		std::size_t total_bytes{0};               // 0 = unknown (indeterminate bar)
		float speed_bps{0.0f};                    // bytes/sec
		int eta_seconds{-1};                      // -1 = unknown
		std::string status_line;                  // secondary info, e.g. "File 3/12: map.xpak"
		std::function<void()> on_cancel{nullptr}; // nullptr = hide Stop button
	};

	void update(const download_state& state);
	void clear();
}
