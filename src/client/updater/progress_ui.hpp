#pragma once

#include <utils/com.hpp>

namespace updater
{
	class progress_ui
	{
	public:
		progress_ui();
		~progress_ui();

		void show() const;

		void set_progress(size_t current, size_t max) const;
		void set_line(int line, const std::string& text) const;
		void set_title(const std::string& title) const;

		bool is_cancelled() const;

	private:
		CComPtr<IProgressDialog> dialog_{};
	};
}
