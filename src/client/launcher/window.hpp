#pragma once
#pragma comment (lib, "dwmapi.lib")

class window
{
public:
	window(const std::string& title, int width, int height,
		std::function<std::optional<LRESULT>(window*, UINT, WPARAM, LPARAM)> callback,
		long flags = (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX)));

	virtual ~window();

	void close();

	operator HWND() const;

	static void run();

	LRESULT processor(UINT message, WPARAM w_param, LPARAM l_param);

private:
	uint32_t last_dpi_ = 96;

	WNDCLASSEX wc_{};
	HWND handle_ = nullptr;
	std::string classname_;
	std::function<std::optional<LRESULT>(window*, UINT, WPARAM, LPARAM)> callback_;

	static LRESULT CALLBACK static_processor(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);
};
