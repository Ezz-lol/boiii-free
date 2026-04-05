#pragma once

#include "nt.hpp"
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

namespace utils
{
	class progress_ui
	{
	public:
		progress_ui(bool headless);
		~progress_ui();

		progress_ui(progress_ui&& other) noexcept;
		progress_ui& operator=(progress_ui&& other) noexcept;

		progress_ui(const progress_ui&) = delete;
		progress_ui& operator=(const progress_ui&) = delete;

		void show(bool marquee, HWND parent = nullptr) const;

		void set_progress(size_t current, size_t max) const;
		void set_line(int line, const std::string& text) const;
		void set_title(const std::string& title) const;

		bool is_cancelled() const;

		static void show_error(const std::string& title, const std::string& message);

		operator bool() const
		{
			return !headless_;
		}

	private:
		struct shared_state
		{
			std::string title = "BOIII Updater";
			std::string line1;
			std::string line2;
			std::string btn_text = "Cancel";
			size_t progress_current = 0;
			size_t progress_max = 0;
			bool marquee = false;
			bool visible = false;
		};

		bool headless_ = true;
		mutable std::recursive_mutex state_mutex_;
		mutable shared_state state_;
		mutable std::atomic<bool> cancelled_{false};

		mutable HWND hwnd_ = nullptr;
		mutable std::thread ui_thread_;
		mutable std::atomic<bool> thread_ready_{false};

		void create_ui_thread() const;
		void destroy_ui_thread() const;

		static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
		void paint(HDC hdc, const RECT& rc) const;
	};
}
