#include <std_include.hpp>
#include "html_window.hpp"

html_window::html_window(const std::string& title, int width, int height, long flags)
	: window_(title, width, height,
	          [this](window*, const UINT message, const WPARAM w_param, const LPARAM l_param) -> std::optional<LRESULT>
	          {
		          return this->processor(message, w_param, l_param);
	          }, flags)
{
}

window* html_window::get_window()
{
	return &this->window_;
}

html_frame* html_window::get_html_frame()
{
	return &this->frame_;
}

std::optional<LRESULT> html_window::processor(const UINT message, const WPARAM w_param, const LPARAM l_param)
{
	if (message == WM_ERASEBKGND)
	{
		return 1;
	}

	if (message == WM_SIZE)
	{
		this->frame_.resize(LOWORD(l_param), HIWORD(l_param));
		return 0;
	}

	if (message == WM_GETMINMAXINFO)
	{
		auto* mmi = reinterpret_cast<MINMAXINFO*>(l_param);
		mmi->ptMinTrackSize.x = 900;
		mmi->ptMinTrackSize.y = 500;
		return 0;
	}

	if (message == WM_CREATE)
	{
		this->frame_.initialize(this->window_);

		// Forward keyboard messages to the browser's IOleInPlaceActiveObject
		// so that Ctrl+A/C/V/X/Z work in text inputs
		auto* frame = &this->frame_;
		window::set_message_filter([frame](MSG* msg) -> bool
		{
			if (msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST)
			{
				auto* active = frame->get_active_object();
				if (active && active->TranslateAccelerator(msg) == S_OK)
				{
					return true;
				}
			}
			return false;
		});

		return 0;
	}

	return {};
}
