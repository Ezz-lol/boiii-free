#include <std_include.hpp>
#include "window.hpp"

#include <utils/nt.hpp>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace
{
	thread_local uint32_t window_count = 0;
	
	uint32_t get_dpi_for_window(const HWND window)
	{
		const utils::nt::library user32{"user32.dll"};
		const auto get_dpi = user32 ? user32.get_proc<UINT(WINAPI *)(HWND)>("GetDpiForWindow") : nullptr;

		if (!get_dpi)
		{
			return USER_DEFAULT_SCREEN_DPI;
		}

		return get_dpi(window);
	}
}

window::window(const std::string& title, const int width, const int height,
	std::function<std::optional<LRESULT>(window*, UINT, WPARAM, LPARAM)> callback,
	const long flags)
	: callback_(std::move(callback))
{
	ZeroMemory(&this->wc_, sizeof(this->wc_));

	this->classname_ = "window-base-" + std::to_string(time(nullptr));

	this->wc_.cbSize = sizeof(this->wc_);
	this->wc_.style = CS_HREDRAW | CS_VREDRAW;
	this->wc_.lpfnWndProc = static_processor;
	this->wc_.hInstance = GetModuleHandle(nullptr);
	this->wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
	this->wc_.hIcon = LoadIcon(this->wc_.hInstance, MAKEINTRESOURCE(102));
	this->wc_.hIconSm = this->wc_.hIcon;
	this->wc_.hbrBackground = HBRUSH(COLOR_WINDOW);
	this->wc_.lpszClassName = this->classname_.data();
	RegisterClassEx(&this->wc_);

	const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

	++window_count;

	this->handle_ = CreateWindowExA(NULL, this->wc_.lpszClassName, title.data(), flags, x, y, width, height, nullptr,
		nullptr, this->wc_.hInstance, this);

	BOOL value = TRUE;
	DwmSetWindowAttribute(this->handle_,
		DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

	SendMessageA(this->handle_, WM_DPICHANGED, 0, 0);
	ShowWindow(this->handle_, SW_SHOW);
	SetForegroundWindow(this->handle_);
}

window::~window()
{
	this->close();
	UnregisterClass(this->wc_.lpszClassName, this->wc_.hInstance);
}

void window::close()
{
	if (!this->handle_) return;

	DestroyWindow(this->handle_);
	this->handle_ = nullptr;
}

void window::run()
{
	MSG msg{};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT window::processor(const UINT message, const WPARAM w_param, const LPARAM l_param)
{
	if (message == WM_DPICHANGED)
	{
		const auto dpi = get_dpi_for_window(*this);
		if (dpi != this->last_dpi_)
		{
			RECT rect;
			GetWindowRect(*this, &rect);

			const auto scale = dpi * 1.0 / this->last_dpi_;
			this->last_dpi_ = dpi;

			const auto width = rect.right - rect.left;
			const auto height = rect.bottom - rect.top;

			MoveWindow(*this, rect.left, rect.top, int(width * scale), int(height * scale), TRUE);
		}
	}

	if (message == WM_DESTROY)
	{
		if (--window_count == 0)
		{
			PostQuitMessage(0);
		}

		return TRUE;
	}

	if (this->callback_)
	{
		const auto res = this->callback_(this, message, w_param, l_param);
		if (res)
		{
			return *res;
		}
	}

	return DefWindowProc(*this, message, w_param, l_param);
}

LRESULT CALLBACK window::static_processor(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	if (message == WM_CREATE)
	{
		auto data = reinterpret_cast<LPCREATESTRUCT>(l_param);
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, LONG_PTR(data->lpCreateParams));

		reinterpret_cast<window*>(data->lpCreateParams)->handle_ = hwnd;
	}

	const auto self = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (self) return self->processor(message, w_param, l_param);

	return DefWindowProc(hwnd, message, w_param, l_param);
}


window::operator HWND() const
{
	return this->handle_;
}
