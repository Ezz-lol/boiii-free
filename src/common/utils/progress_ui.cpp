#include "progress_ui.hpp"
#include "string.hpp"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace utils {
namespace {
constexpr int WINDOW_W = 580;
constexpr int WINDOW_H = 260;
constexpr UINT WM_UPDATE_UI = WM_APP + 1;
constexpr UINT TIMER_MARQUEE = 1;

constexpr COLORREF COL_BG = RGB(10, 10, 10);
constexpr COLORREF COL_SURFACE = RGB(20, 20, 20);
constexpr COLORREF COL_BORDER = RGB(40, 35, 30);
constexpr COLORREF COL_TEXT = RGB(230, 225, 220);
constexpr COLORREF COL_TEXT_DIM = RGB(160, 155, 150);
constexpr COLORREF COL_ACCENT = RGB(249, 115, 22);
constexpr COLORREF COL_ACCENT_GLOW = RGB(251, 146, 60);
constexpr COLORREF COL_TRACK = RGB(35, 35, 35);
constexpr COLORREF COL_CANCEL_BG = RGB(30, 30, 30);
constexpr COLORREF COL_CANCEL_HOVER = RGB(45, 40, 35);
constexpr COLORREF COL_CLOSE_HOVER = RGB(180, 40, 40);

constexpr int TITLE_BAR_H = 42;
constexpr int PADDING = 28;
constexpr int BAR_H = 18;
constexpr int BAR_RADIUS = 9;
constexpr int BTN_W = 100;
constexpr int BTN_H = 34;

struct window_data {
  const progress_ui *self = nullptr;
  bool close_hover = false;
  bool cancel_hover = false;
  bool dragging = false;
  POINT drag_start = {};
  int marquee_offset = 0;
};

RECT get_close_btn_rect(const RECT &rc) {
  return {rc.right - 46, 0, rc.right, TITLE_BAR_H};
}

RECT get_cancel_btn_rect(const RECT &rc) {
  return {rc.right - PADDING - BTN_W, rc.bottom - PADDING - BTN_H + 4,
          rc.right - PADDING, rc.bottom - PADDING + 4};
}

HBRUSH create_solid(COLORREF c) { return CreateSolidBrush(c); }

void fill_rounded_rect(HDC hdc, const RECT &r, int radius, COLORREF fill) {
  HBRUSH brush = CreateSolidBrush(fill);
  HPEN pen = CreatePen(PS_SOLID, 0, fill);
  auto old_brush = SelectObject(hdc, brush);
  auto old_pen = SelectObject(hdc, pen);
  RoundRect(hdc, r.left, r.top, r.right, r.bottom, radius, radius);
  SelectObject(hdc, old_brush);
  SelectObject(hdc, old_pen);
  DeleteObject(brush);
  DeleteObject(pen);
}
} // namespace

progress_ui::progress_ui(const bool headless) : headless_(headless) {}

progress_ui::~progress_ui() { destroy_ui_thread(); }

progress_ui::progress_ui(progress_ui &&other) noexcept
    : headless_(other.headless_) {
  other.destroy_ui_thread();

  std::lock_guard<std::recursive_mutex> lock(other.state_mutex_);
  state_ = other.state_;
  cancelled_.store(other.cancelled_.load());
  other.headless_ = true;
}

progress_ui &progress_ui::operator=(progress_ui &&other) noexcept {
  if (this != &other) {
    destroy_ui_thread();
    other.destroy_ui_thread();

    std::lock_guard<std::recursive_mutex> lock(other.state_mutex_);
    headless_ = other.headless_;
    state_ = other.state_;
    cancelled_.store(other.cancelled_.load());
    other.headless_ = true;
  }
  return *this;
}

void progress_ui::show(const bool marquee, HWND /*parent*/) const {
  if (headless_)
    return;

  {
    std::lock_guard<std::recursive_mutex> lock(state_mutex_);
    state_.marquee = marquee;
    state_.visible = true;
  }

  create_ui_thread();
}

void progress_ui::set_progress(const size_t current, const size_t max) const {
  if (headless_)
    return;

  {
    std::lock_guard<std::recursive_mutex> lock(state_mutex_);
    state_.progress_current = current;
    state_.progress_max = max;
  }

  if (hwnd_)
    PostMessage(hwnd_, WM_UPDATE_UI, 0, 0);
}

void progress_ui::set_line(const int line, const std::string &text) const {
  if (headless_)
    return;

  {
    std::lock_guard<std::recursive_mutex> lock(state_mutex_);
    if (line == 1)
      state_.line1 = text;
    else if (line == 2)
      state_.line2 = text;
  }

  if (hwnd_)
    PostMessage(hwnd_, WM_UPDATE_UI, 0, 0);
}

void progress_ui::set_title(const std::string &title) const {
  if (headless_)
    return;

  {
    std::lock_guard<std::recursive_mutex> lock(state_mutex_);
    state_.title = title;
  }

  if (hwnd_)
    PostMessage(hwnd_, WM_UPDATE_UI, 0, 0);
}

bool progress_ui::is_cancelled() const { return cancelled_.load(); }

void progress_ui::create_ui_thread() const {
  if (hwnd_ || headless_)
    return;

  thread_ready_ = false;

  ui_thread_ = std::thread([this]() {
    const auto class_name =
        "BOIII_ProgressUI_" + std::to_string(reinterpret_cast<uintptr_t>(this));

    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = create_solid(COL_BG);
    wc.lpszClassName = class_name.c_str();
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(102));
    wc.hIconSm = wc.hIcon;
    RegisterClassExA(&wc);

    auto *wd = new window_data{};
    wd->self = this;

    hwnd_ =
        CreateWindowExA(WS_EX_APPWINDOW, class_name.c_str(), "BOIII Updater",
                        WS_POPUP | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                        WINDOW_W, WINDOW_H, nullptr, nullptr, wc.hInstance, wd);

    if (hwnd_) {
      BOOL dark = TRUE;
      DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark,
                            sizeof(dark));

      const HMONITOR monitor =
          MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
      MONITORINFO mi{};
      mi.cbSize = sizeof(mi);
      if (GetMonitorInfoA(monitor, &mi)) {
        const int x =
            mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - WINDOW_W) / 2;
        const int y =
            mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - WINDOW_H) / 2;
        MoveWindow(hwnd_, x, y, WINDOW_W, WINDOW_H, TRUE);
      }

      SetForegroundWindow(hwnd_);
      SetTimer(hwnd_, TIMER_MARQUEE, 40, nullptr);
    }

    thread_ready_ = true;

    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    UnregisterClassA(class_name.c_str(), wc.hInstance);
  });
}

void progress_ui::destroy_ui_thread() const {
  if (hwnd_) {
    PostMessage(hwnd_, WM_CLOSE, 0, 0);
  }

  if (ui_thread_.joinable()) {
    ui_thread_.join();
  }

  hwnd_ = nullptr;
}

LRESULT CALLBACK progress_ui::wnd_proc(HWND hwnd, UINT msg, WPARAM wp,
                                       LPARAM lp) {
  window_data *wd = nullptr;

  if (msg == WM_CREATE) {
    auto *cs = reinterpret_cast<CREATESTRUCT *>(lp);
    wd = static_cast<window_data *>(cs->lpCreateParams);
    SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wd));
  } else {
    wd =
        reinterpret_cast<window_data *>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
  }

  switch (msg) {
  case WM_PAINT: {
    if (!wd || !wd->self)
      break;
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);

    HDC mem_dc = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    auto old_bmp = SelectObject(mem_dc, bmp);

    wd->self->paint(mem_dc, rc);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem_dc, 0, 0, SRCCOPY);

    SelectObject(mem_dc, old_bmp);
    DeleteObject(bmp);
    DeleteDC(mem_dc);

    EndPaint(hwnd, &ps);
    return 0;
  }

  case WM_UPDATE_UI:
    InvalidateRect(hwnd, nullptr, FALSE);
    return 0;

  case WM_TIMER:
    if (wp == TIMER_MARQUEE && wd) {
      wd->marquee_offset = (wd->marquee_offset + 4) % (WINDOW_W);
      InvalidateRect(hwnd, nullptr, FALSE);
    }
    return 0;

  case WM_MOUSEMOVE: {
    if (!wd)
      break;

    POINT pt = {LOWORD(lp), HIWORD(lp)};
    RECT rc;
    GetClientRect(hwnd, &rc);

    auto close_rc = get_close_btn_rect(rc);
    auto cancel_rc = get_cancel_btn_rect(rc);

    bool new_close = PtInRect(&close_rc, pt) != 0;
    bool new_cancel = PtInRect(&cancel_rc, pt) != 0;

    if (new_close != wd->close_hover || new_cancel != wd->cancel_hover) {
      wd->close_hover = new_close;
      wd->cancel_hover = new_cancel;
      InvalidateRect(hwnd, nullptr, FALSE);
    }

    if (wd->dragging) {
      POINT screen_pt;
      GetCursorPos(&screen_pt);
      RECT wr;
      GetWindowRect(hwnd, &wr);
      MoveWindow(hwnd, wr.left + (screen_pt.x - wd->drag_start.x),
                 wr.top + (screen_pt.y - wd->drag_start.y), wr.right - wr.left,
                 wr.bottom - wr.top, TRUE);
      wd->drag_start = screen_pt;
    }

    TRACKMOUSEEVENT tme{};
    tme.cbSize = sizeof(tme);
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = hwnd;
    TrackMouseEvent(&tme);
    return 0;
  }

  case WM_MOUSELEAVE:
    if (wd) {
      wd->close_hover = false;
      wd->cancel_hover = false;
      InvalidateRect(hwnd, nullptr, FALSE);
    }
    return 0;

  case WM_LBUTTONDOWN: {
    if (!wd)
      break;
    POINT pt = {LOWORD(lp), HIWORD(lp)};

    RECT rc;
    GetClientRect(hwnd, &rc);
    auto close_rc = get_close_btn_rect(rc);
    auto cancel_rc = get_cancel_btn_rect(rc);

    if (PtInRect(&close_rc, pt) || PtInRect(&cancel_rc, pt)) {
      if (wd->self)
        wd->self->cancelled_ = true;
      return 0;
    }

    if (pt.y < TITLE_BAR_H) {
      wd->dragging = true;
      GetCursorPos(&wd->drag_start);
      SetCapture(hwnd);
    }
    return 0;
  }

  case WM_LBUTTONUP:
    if (wd && wd->dragging) {
      wd->dragging = false;
      ReleaseCapture();
    }
    return 0;

  case WM_CLOSE:
    KillTimer(hwnd, TIMER_MARQUEE);
    if (wd) {
      delete wd;
      SetWindowLongPtrA(hwnd, GWLP_USERDATA, 0);
    }
    DestroyWindow(hwnd);
    return 0;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wp, lp);
}

void progress_ui::paint(HDC hdc, const RECT &rc) const {
  HBRUSH bg_brush = create_solid(COL_BG);
  FillRect(hdc, &rc, bg_brush);
  DeleteObject(bg_brush);

  HPEN border_pen = CreatePen(PS_SOLID, 1, COL_BORDER);
  auto old_pen = SelectObject(hdc, border_pen);
  auto old_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
  Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
  SelectObject(hdc, old_pen);
  SelectObject(hdc, old_brush);
  DeleteObject(border_pen);

  SetBkMode(hdc, TRANSPARENT);

  RECT title_rc = {1, 1, rc.right - 1, TITLE_BAR_H};
  HBRUSH title_brush = create_solid(COL_SURFACE);
  FillRect(hdc, &title_rc, title_brush);
  DeleteObject(title_brush);

  HPEN line_pen = CreatePen(PS_SOLID, 1, COL_BORDER);
  old_pen = SelectObject(hdc, line_pen);
  MoveToEx(hdc, 0, TITLE_BAR_H, nullptr);
  LineTo(hdc, rc.right, TITLE_BAR_H);
  SelectObject(hdc, old_pen);
  DeleteObject(line_pen);

  HFONT title_font =
      CreateFontA(17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0,
                  0, CLEARTYPE_QUALITY, 0, "Segoe UI");
  auto old_font = SelectObject(hdc, title_font);
  SetTextColor(hdc, COL_TEXT);

  std::string title;
  {
    std::lock_guard<std::recursive_mutex> lock(state_mutex_);
    title = state_.title;
  }

  RECT title_text_rc = {16, 0, rc.right - 56, TITLE_BAR_H};
  DrawTextA(hdc, title.c_str(), -1, &title_text_rc,
            DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX);

  SelectObject(hdc, old_font);
  DeleteObject(title_font);

  auto close_rc = get_close_btn_rect(rc);
  auto *wd =
      reinterpret_cast<window_data *>(GetWindowLongPtrA(hwnd_, GWLP_USERDATA));
  if (wd && wd->close_hover) {
    HBRUSH close_brush = create_solid(COL_CLOSE_HOVER);
    FillRect(hdc, &close_rc, close_brush);
    DeleteObject(close_brush);
  }

  HFONT close_font =
      CreateFontA(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                  0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI");
  old_font = SelectObject(hdc, close_font);
  SetTextColor(hdc, COL_TEXT);
  DrawTextA(hdc, "X", -1, &close_rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
  SelectObject(hdc, old_font);
  DeleteObject(close_font);

  int y = TITLE_BAR_H + 24;

  std::string line1, line2;
  size_t cur = 0, max_val = 0;
  bool marquee = false;
  {
    std::lock_guard<std::recursive_mutex> lock(state_mutex_);
    line1 = state_.line1;
    line2 = state_.line2;
    cur = state_.progress_current;
    max_val = state_.progress_max;
    marquee = state_.marquee;
  }

  HFONT text_font =
      CreateFontA(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                  0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI");
  old_font = SelectObject(hdc, text_font);
  SetTextColor(hdc, COL_TEXT);

  if (!line1.empty()) {
    RECT l1_rc = {PADDING, y, rc.right - PADDING, y + 20};
    DrawTextA(hdc, line1.c_str(), -1, &l1_rc,
              DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX |
                  DT_END_ELLIPSIS);
  }

  y += 28;

  const int bar_left = PADDING;
  const int bar_right = rc.right - PADDING - 76;
  const int bar_w = bar_right - bar_left;

  RECT bar_track = {bar_left, y, bar_right, y + BAR_H};
  fill_rounded_rect(hdc, bar_track, BAR_RADIUS, COL_TRACK);

  if (marquee) {
    int marquee_w = bar_w / 3;
    int moff = wd ? wd->marquee_offset : 0;
    int fill_start = bar_left + (moff % (bar_w + marquee_w)) - marquee_w;
    int fill_end = fill_start + marquee_w;
    if (fill_start < bar_left)
      fill_start = bar_left;
    if (fill_end > bar_right)
      fill_end = bar_right;

    if (fill_start < fill_end) {
      RECT bar_fill = {fill_start, y, fill_end, y + BAR_H};
      fill_rounded_rect(hdc, bar_fill, BAR_RADIUS, COL_ACCENT);
    }
  } else if (max_val > 0) {
    double pct = static_cast<double>(cur) / static_cast<double>(max_val);
    if (pct > 1.0)
      pct = 1.0;
    int fill_w = static_cast<int>(bar_w * pct);
    if (fill_w > 2) {
      RECT bar_fill = {bar_left, y, bar_left + fill_w, y + BAR_H};
      fill_rounded_rect(hdc, bar_fill, BAR_RADIUS, COL_ACCENT);

      RECT glow = {bar_left, y, bar_left + fill_w, y + BAR_H / 2};
      fill_rounded_rect(hdc, glow, BAR_RADIUS, COL_ACCENT_GLOW);
    }
  }

  std::string pct_text;
  if (!marquee && max_val > 0) {
    double pct =
        static_cast<double>(cur) / static_cast<double>(max_val) * 100.0;
    if (pct > 100.0)
      pct = 100.0;
    pct_text = string::va("%.1f%%", pct);
  }

  if (!pct_text.empty()) {
    SetTextColor(hdc, COL_TEXT_DIM);
    RECT pct_rc = {bar_right + 10, y - 1, rc.right - PADDING, y + BAR_H + 1};
    DrawTextA(hdc, pct_text.c_str(), -1, &pct_rc,
              DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);
  }

  y += BAR_H + 14;

  if (!line2.empty()) {
    SetTextColor(hdc, COL_TEXT_DIM);
    RECT l2_rc = {PADDING, y, rc.right - PADDING, y + 18};
    DrawTextA(hdc, line2.c_str(), -1, &l2_rc,
              DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX |
                  DT_END_ELLIPSIS);
  }

  SelectObject(hdc, old_font);
  DeleteObject(text_font);

  auto cancel_rc = get_cancel_btn_rect(rc);
  COLORREF cancel_bg =
      (wd && wd->cancel_hover) ? COL_CANCEL_HOVER : COL_CANCEL_BG;
  fill_rounded_rect(hdc, cancel_rc, 5, cancel_bg);

  HPEN cancel_pen = CreatePen(PS_SOLID, 1, COL_BORDER);
  old_pen = SelectObject(hdc, cancel_pen);
  old_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
  RoundRect(hdc, cancel_rc.left, cancel_rc.top, cancel_rc.right,
            cancel_rc.bottom, 5, 5);
  SelectObject(hdc, old_pen);
  SelectObject(hdc, old_brush);
  DeleteObject(cancel_pen);

  HFONT btn_font =
      CreateFontA(15, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                  DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, "Segoe UI");
  old_font = SelectObject(hdc, btn_font);
  SetTextColor(hdc, COL_TEXT);
  std::string btn_text;
  {
    std::lock_guard<std::recursive_mutex> lock(state_mutex_);
    btn_text = state_.btn_text;
  }
  DrawTextA(hdc, btn_text.c_str(), -1, &cancel_rc,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER);
  SelectObject(hdc, old_font);
  DeleteObject(btn_font);
}

void progress_ui::show_error(const std::string &title,
                             const std::string &message) {
  progress_ui ui(false);
  ui.set_title(title);
  ui.set_line(1, message);
  ui.set_progress(1, 1);
  {
    std::lock_guard<std::recursive_mutex> lock(ui.state_mutex_);
    ui.state_.btn_text = "OK";
  }
  ui.show(false);

  while (!ui.is_cancelled()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}
} // namespace utils
