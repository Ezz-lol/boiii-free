#include "progress_ui.hpp"

#include <utils/string.hpp>

namespace utils
{
	progress_ui::progress_ui(const bool headless)
	{
		if (headless)
		{
			return;
		}

		this->dialog_ = com::create_progress_dialog();
		if (!this->dialog_)
		{
			throw std::runtime_error{"Failed to create dialog"};
		}
	}

	progress_ui::~progress_ui()
	{
		if (this->dialog_)
		{
			this->dialog_->StopProgressDialog();
		}
	}

	void progress_ui::show(const bool marquee, HWND parent) const
	{
		if (this->dialog_)
		{
			this->dialog_->StartProgressDialog(parent, nullptr,
			                                   PROGDLG_AUTOTIME | (marquee ? PROGDLG_MARQUEEPROGRESS : 0), nullptr);
		}
	}

	void progress_ui::set_progress(const size_t current, const size_t max) const
	{
		if (this->dialog_)
		{
			this->dialog_->SetProgress64(current, max);
		}
	}

	void progress_ui::set_line(const int line, const std::string& text) const
	{
		if (this->dialog_)
		{
			this->dialog_->SetLine(line, utils::string::convert(text).data(), false, nullptr);
		}
	}

	void progress_ui::set_title(const std::string& title) const
	{
		if (this->dialog_)
		{
			this->dialog_->SetTitle(utils::string::convert(title).data());
		}
	}

	bool progress_ui::is_cancelled() const
	{
		if (this->dialog_)
		{
			return this->dialog_->HasUserCancelled();
		}

		return false;
	}
}
