#pragma once

#include <utils/com.hpp>

namespace updater
{
	class progress_ui
	{
	public:
		progress_ui(bool allow_failure = true);
		~progress_ui();

		void show() const;

		void set_progress(size_t current, size_t max) const;
		void set_line(int line, const std::string& text) const;
		void set_title(const std::string& title) const;

		bool is_cancelled() const;

		operator bool() const
		{
			return this->dialog_;
		}

	private:
		CComPtr<IProgressDialog> dialog_{};
	};
}
