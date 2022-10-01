#pragma once

#include "com.hpp"

namespace utils
{
	class progress_ui
	{
	public:
		progress_ui();
		~progress_ui();

		void show(bool marquee, HWND parent = nullptr) const;

		void set_progress(size_t current, size_t max) const;
		void set_line(int line, const std::string& text) const;
		void set_title(const std::string& title) const;

		bool is_cancelled() const;

	private:
		CComPtr<IProgressDialog> dialog_{};
	};
}
