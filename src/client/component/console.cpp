#include <atomic>
#include <std_include.hpp>
#include "console.hpp"
#include "loader/component_loader.hpp"
#include "resource.hpp"

#include "game/game.hpp"
#include "command.hpp"

#include <utils/thread.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/concurrency.hpp>
#include <utils/image.hpp>

#include "scheduler.hpp"

#include <utils/io.hpp>
#include <utils/nt.hpp>

#include <richedit.h>
#include <dwmapi.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <unordered_set>
#include <vector>

#pragma comment(lib, "dwmapi.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif

#define CONSOLE_BUFFER_SIZE 16384
constexpr int CONSOLE_MIN_WIDTH = 900;
constexpr int CONSOLE_MIN_HEIGHT = 520;
constexpr int CONSOLE_HEADER_HEIGHT = 36;
constexpr int CONSOLE_INPUT_HEIGHT = 26;
constexpr int CONSOLE_MARGIN = 8;
constexpr int COMPLETION_HINT_HEIGHT = 40;
constexpr int COMPLETION_HINT_MAX_HEIGHT = 8 + 14 * 11;

namespace console {
namespace {
utils::image::object logo;
std::atomic_bool started{false};
std::atomic_bool terminate_runner{false};
utils::concurrency::container<std::function<void(const std::string &message)>>
    interceptor{};
utils::concurrency::container<std::queue<std::string>> message_queue{};
std::vector<std::string> dvar_name_list{};
std::mutex dvar_list_mutex;
std::atomic_bool dvar_list_loaded{false};
std::atomic_bool dvar_list_loading{false};
std::atomic_size_t dynamic_name_count{0};
std::atomic<int64_t> buffer_char_count{0};
std::atomic_bool close_requested{false};
HWND completion_hint_hwnd{nullptr};
std::vector<std::string> tab_cycle_matches{};
std::string tab_cycle_partial{};
size_t tab_cycle_index{0};

// Command history navigation (Up/Down in the input line).
std::vector<std::string> command_history{};
size_t history_index{0};
std::string history_draft{};

constexpr UINT WM_APPEND_CONSOLE_TEXT = WM_APP + 0x1337;
constexpr size_t MAX_CONSOLE_CHARS = 1'000'000;
constexpr int COMPLETION_HINT_CONTROL_ID = 0x66;

bool full_logs_enabled() {
  static const bool enabled = utils::flags::has_flag("fulllogs");
  return enabled;
}

COLORREF get_cod_color(const char code) {
  switch (code) {
  case '0':
    return RGB(0, 0, 0);
  case '1':
    return RGB(237, 92, 92);
  case '2':
    return RGB(96, 200, 120);
  case '3':
    return RGB(232, 209, 96);
  case '4':
    return RGB(96, 150, 237);
  case '5':
    return RGB(96, 210, 220);
  case '6':
    return RGB(220, 120, 220);
  case '7':
    return RGB(235, 235, 238);
  case '8':
    return RGB(235, 160, 90);
  case '9':
    return RGB(150, 150, 156);
  default:
    return RGB(228, 228, 231);
  }
}

COLORREF get_error_color() { return RGB(248, 113, 113); }
COLORREF get_warning_color() { return RGB(232, 194, 84); }
COLORREF get_info_color() { return RGB(161, 161, 170); }
COLORREF get_bracket_tag_color() { return RGB(96, 165, 250); }
COLORREF get_default_console_color() { return RGB(228, 228, 231); }
COLORREF get_background_color() { return RGB(24, 24, 27); }
COLORREF get_hint_background_color() { return RGB(38, 38, 46); }

void reset_tab_cycle() {
  tab_cycle_matches.clear();
  tab_cycle_partial.clear();
  tab_cycle_index = 0;
}

int compare_dvar_names_ci(const std::string &a, const std::string &b) {
  const size_t min_len = std::min(a.size(), b.size());
  const int cmp =
      min_len ? _strnicmp(a.c_str(), b.c_str(), min_len) : 0;
  if (cmp != 0) {
    return cmp;
  }
  if (a.size() == b.size()) {
    return 0;
  }
  return a.size() < b.size() ? -1 : 1;
}

bool dvar_name_less(const std::string &a, const std::string &b) {
  const int cmp = _stricmp(a.c_str(), b.c_str());
  if (cmp != 0) {
    return cmp < 0;
  }
  return a < b;
}

std::string to_lower_copy(const std::string_view s) {
  std::string out(s);
  for (auto &c : out) {
    const auto uc = static_cast<unsigned char>(c);
    c = static_cast<char>(std::tolower(uc));
  }
  return out;
}

bool ci_contains(const std::string_view haystack, const std::string_view needle) {
  if (needle.empty()) {
    return true;
  }
  if (haystack.size() < needle.size()) {
    return false;
  }
  const auto end = haystack.size() - needle.size() + 1;
  for (size_t i = 0; i < end; ++i) {
    size_t j = 0;
    for (; j < needle.size(); ++j) {
      const auto a = static_cast<unsigned char>(haystack[i + j]);
      const auto b = static_cast<unsigned char>(needle[j]);
      if (std::tolower(a) != std::tolower(b)) {
        break;
      }
    }
    if (j == needle.size()) {
      return true;
    }
  }
  return false;
}

COLORREF get_line_base_color(const std::string_view line) {
  if (!line.empty() && line[0] == '[' &&
      line.find('^') == std::string_view::npos) {
    return get_bracket_tag_color();
  }

  if (!line.empty() && line[0] == ']') {
    return RGB(245, 242, 240);
  }

  const auto has = [&](const std::string_view needle) {
    return ci_contains(line, needle);
  };

  if (has("com_error:") || has("unrecoverable error") || has("script error")) {
    return get_error_color();
  }

  if (has("ui error") || has("unable to load module") ||
      has("stack traceback") || has("attempt to index a nil value") ||
      has("function expected instead of nil")) {
    return get_error_color();
  }

  if (has("error") || has("could not find") || has("exec from disk failed") ||
      has("invalid line") || has("missing asset") || has("failed")) {
    return get_error_color();
  }

  if (has("couldn't exec") || has("failed to open") ||
      has("tried to load asset") || has("could not load default asset")) {
    return get_error_color();
  }

  if (has("warn") || has("unknown command")) {
    return get_warning_color();
  }

  if (has("loading") || has("loaded") || has("connecting") ||
      has("connected")) {
    return get_info_color();
  }

  return get_default_console_color();
}

void append_colored_text(const HWND richedit, const char *text, size_t len,
                         COLORREF color) {
  if (!richedit || !text || len == 0) {
    return;
  }

  const int wlen =
      MultiByteToWideChar(CP_UTF8, 0, text, static_cast<int>(len), nullptr, 0);
  if (wlen <= 0) {
    return;
  }

  // Reused across calls instead of a fresh vector every time. This function
  // only ever runs on the console UI thread (con_wnd_proc's thread), so a
  // plain static is safe without extra locking. Capacity grows to the
  // largest batch seen and is then reused indefinitely, avoiding a
  // heap alloc/free cycle on every color run under sustained print load.
  static std::vector<wchar_t> wbuf;
  wbuf.resize(static_cast<size_t>(wlen) + 1);
  MultiByteToWideChar(CP_UTF8, 0, text, static_cast<int>(len), wbuf.data(),
                      wlen);
  wbuf[static_cast<size_t>(wlen)] = L'\0';

  CHARRANGE cr_end;
  cr_end.cpMin = -1;
  cr_end.cpMax = -1;
  SendMessageW(richedit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr_end));

  CHARFORMAT2W cf{};
  cf.cbSize = sizeof(cf);
  cf.dwMask = CFM_COLOR;
  cf.crTextColor = color;
  cf.dwEffects = 0;
  SendMessageW(richedit, EM_SETCHARFORMAT, SCF_SELECTION,
               reinterpret_cast<LPARAM>(&cf));
  SendMessageW(richedit, EM_REPLACESEL, FALSE,
               reinterpret_cast<LPARAM>(wbuf.data()));

  buffer_char_count.fetch_add(wlen, std::memory_order_relaxed);
}

void trim_console_buffer(const HWND richedit) {
  if (full_logs_enabled())
    return;

  const int64_t text_len = buffer_char_count.load(std::memory_order_relaxed);
  if (text_len > static_cast<int64_t>(MAX_CONSOLE_CHARS)) {
    const auto to_remove =
        static_cast<LONG>(text_len - MAX_CONSOLE_CHARS / 2);
    CHARRANGE cr;
    cr.cpMin = 0;
    cr.cpMax = to_remove;
    SendMessageW(richedit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr));
    SendMessageW(richedit, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L""));

    CHARRANGE cr_end;
    cr_end.cpMin = -1;
    cr_end.cpMax = -1;
    SendMessageW(richedit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr_end));

    buffer_char_count.fetch_sub(to_remove, std::memory_order_relaxed);
  }
}

struct run_accumulator {
  HWND richedit;
  std::string &buffer; // reused across calls, see append_text_with_severity
  COLORREF current_color{};
  bool has_color{false};

  run_accumulator(const HWND h, std::string &buf) : richedit(h), buffer(buf) {
    buffer.clear();
  }

  void add(const std::string_view text, const COLORREF color) {
    if (text.empty()) {
      return;
    }
    if (has_color && color != current_color) {
      flush();
    }
    current_color = color;
    has_color = true;
    buffer.append(text.data(), text.size());
  }

  void flush() {
    if (!buffer.empty()) {
      append_colored_text(richedit, buffer.data(), buffer.size(), current_color);
      buffer.clear();
    }
    has_color = false;
  }
};

void append_line_colored(run_accumulator &acc, const std::string_view line,
                         COLORREF base_color) {
  size_t i = 0;
  while (i < line.size()) {
    if (i + 1 < line.size() && line[i] == '^' && line[i + 1] >= '0' &&
        line[i + 1] <= '9') {
      const COLORREF cod_color = get_cod_color(line[i + 1]);
      i += 2;
      const size_t start = i;
      while (i < line.size()) {
        if (i + 1 < line.size() && line[i] == '^' && line[i + 1] >= '0' &&
            line[i + 1] <= '9') {
          break;
        }
        if (line[i] == '\n') {
          break;
        }
        i++;
      }
      if (i > start) {
        acc.add(line.substr(start, i - start), cod_color);
      }
    } else {
      const size_t start = i;
      while (i < line.size()) {
        if (i + 1 < line.size() && line[i] == '^' && line[i + 1] >= '0' &&
            line[i + 1] <= '9') {
          break;
        }
        if (line[i] == '\n') {
          i++;
          break;
        }
        i++;
      }
      acc.add(line.substr(start, i - start), base_color);
    }
  }
}

void append_text_with_severity(const HWND richedit, const std::string &text) {
  if (!richedit || text.empty()) {
    return;
  }

  static std::string run_buffer;

  SendMessageW(richedit, WM_SETREDRAW, FALSE, 0);

  SCROLLINFO scroll_info{};
  scroll_info.cbSize = sizeof(scroll_info);
  scroll_info.fMask = SIF_ALL;
  GetScrollInfo(richedit, SB_VERT, &scroll_info);
  const bool was_at_bottom =
      (scroll_info.nPos + static_cast<int>(scroll_info.nPage) >=
       scroll_info.nMax - 1) ||
      scroll_info.nMax == 0;

  trim_console_buffer(richedit);

  run_accumulator acc(richedit, run_buffer);

  std::string_view remaining(text);
  while (!remaining.empty()) {
    const auto nl = remaining.find('\n');
    std::string_view line_view;
    if (nl != std::string_view::npos) {
      line_view = remaining.substr(0, nl + 1);
      remaining.remove_prefix(nl + 1);
    } else {
      line_view = remaining;
      remaining = {};
    }

    const COLORREF base_color = get_line_base_color(line_view);
    append_line_colored(acc, line_view, base_color);
  }

  acc.flush();

  if (was_at_bottom) {
    SendMessageW(richedit, WM_VSCROLL, SB_BOTTOM, 0);
  }

  SendMessageW(richedit, WM_SETREDRAW, TRUE, 0);
  InvalidateRect(richedit, nullptr, FALSE);
}

std::string get_window_text_safe(const HWND hwnd) {
  if (!hwnd) {
    return {};
  }

  const int len = GetWindowTextLengthA(hwnd);
  if (len <= 0) {
    return {};
  }

  std::string text(static_cast<size_t>(len), '\0');
  GetWindowTextA(hwnd, text.data(), len + 1);
  text.resize(static_cast<size_t>(len));
  return text;
}

void restore_input_caret() {
  if (!*game::s_wcd::hwndInputLine || !IsWindow(*game::s_wcd::hwndInputLine)) {
    return;
  }

  if (GetFocus() != *game::s_wcd::hwndInputLine) {
    return;
  }

  DWORD sel_start = 0;
  DWORD sel_end = 0;
  SendMessageA(*game::s_wcd::hwndInputLine, EM_GETSEL,
              reinterpret_cast<WPARAM>(&sel_start),
              reinterpret_cast<LPARAM>(&sel_end));
  SendMessageA(*game::s_wcd::hwndInputLine, EM_SETSEL, sel_start, sel_end);

  CreateCaret(*game::s_wcd::hwndInputLine, nullptr, 0,
             CONSOLE_INPUT_HEIGHT - 6);
  ShowCaret(*game::s_wcd::hwndInputLine);
}

std::string format_hint_text(const std::string &text) {
  std::string formatted;
  formatted.reserve(text.size());
  for (size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\n' &&
        (i == 0 || text[i - 1] != '\r')) {
      formatted += '\r';
    }
    formatted += text[i];
  }
  return formatted;
}

void reposition_completion_hint(const int popup_height_requested) {
  if (!completion_hint_hwnd || !*game::s_wcd::hwndInputLine ||
      !*game::s_wcd::hWnd) {
    return;
  }

  const int popup_height =
      (std::min)(popup_height_requested, COMPLETION_HINT_MAX_HEIGHT);

  RECT console_rect{};
  GetClientRect(*game::s_wcd::hWnd, &console_rect);
  const int client_width =
      (std::max)(0, static_cast<int>(console_rect.right - console_rect.left));
  const int client_height =
      (std::max)(0, static_cast<int>(console_rect.bottom - console_rect.top));
  const int input_y =
      client_height - CONSOLE_INPUT_HEIGHT - CONSOLE_MARGIN;
  const int hint_y =
      (std::max)(CONSOLE_MARGIN, input_y - popup_height - 4);
  const int popup_width =
      (std::max)(320, client_width - CONSOLE_MARGIN * 2);

  SetWindowPos(completion_hint_hwnd, HWND_TOP, CONSOLE_MARGIN, hint_y,
               popup_width, popup_height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
  InvalidateRect(completion_hint_hwnd, nullptr, TRUE);
}

void update_completion_hint(const std::string &text) {
  if (!completion_hint_hwnd) {
    return;
  }

  if (text.empty()) {
    SetWindowTextA(completion_hint_hwnd, "");
    ShowWindow(completion_hint_hwnd, SW_HIDE);
    return;
  }

  const std::string formatted = format_hint_text(text);
  SetWindowTextA(completion_hint_hwnd, formatted.c_str());

  const size_t line_count =
      1 + std::count(formatted.begin(), formatted.end(), '\n');
  const int popup_height =
      (std::max)(COMPLETION_HINT_HEIGHT, static_cast<int>(line_count * 14 + 8));
  reposition_completion_hint(popup_height);
  ShowWindow(completion_hint_hwnd, SW_SHOW);
  InvalidateRect(completion_hint_hwnd, nullptr, TRUE);
}

void collect_registered_commands(std::vector<std::string> &out) {
  const game::cmd::cmd_function_s *current_function = game::cmd::cmd_functions;
  while (current_function) {
    if (current_function->name && current_function->name[0]) {
      out.emplace_back(current_function->name);
    }
    current_function = current_function->next;
  }
}

void load_dvar_list() {
  if (dvar_list_loaded || dvar_list_loading.exchange(true)) {
    return;
  }

  try {
    std::vector<std::string> loaded;
    std::string data;

    const auto path = game::get_appdata_path() /
                      "data/lookup_tables/dvar_list.txt";
    if (utils::io::read_file(path.string(), &data)) {
      std::istringstream iss(data);
      std::string line;
      std::unordered_set<std::string> seen;
      while (std::getline(iss, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
          line.pop_back();
        if (!line.empty() && seen.insert(line).second)
          loaded.push_back(line);
      }
    }

    collect_registered_commands(loaded);

    if (!loaded.empty()) {
      std::sort(loaded.begin(), loaded.end(), dvar_name_less);
      loaded.erase(std::unique(loaded.begin(), loaded.end(),
                              [](const std::string &a, const std::string &b) {
                                return _stricmp(a.c_str(), b.c_str()) == 0;
                              }),
                  loaded.end());
      {
        std::lock_guard lock(dvar_list_mutex);
        dvar_name_list = std::move(loaded);
      }
      dvar_list_loaded = true;
    }
  } catch (...) {
  }

  dvar_list_loading = false;
}

void merge_dynamic_names() {
  const size_t total = game::get_registered_dvar_name_count() +
                       command::get_registered_command_count();

  if (total == dynamic_name_count && dvar_list_loaded) {
    return;
  }

  auto custom_dvars = game::get_registered_dvar_names();
  auto custom_commands = command::get_registered_command_names();

  std::vector<std::string> merged;
  {
    std::lock_guard lock(dvar_list_mutex);
    merged = dvar_name_list;
  }

  merged.reserve(merged.size() + custom_dvars.size() + custom_commands.size());
  for (auto &name : custom_dvars) {
    merged.push_back(std::move(name));
  }
  for (auto &name : custom_commands) {
    merged.push_back(std::move(name));
  }

  std::sort(merged.begin(), merged.end(), dvar_name_less);
  merged.erase(std::unique(merged.begin(), merged.end(),
                          [](const std::string &a, const std::string &b) {
                            return _stricmp(a.c_str(), b.c_str()) == 0;
                          }),
              merged.end());

  {
    std::lock_guard lock(dvar_list_mutex);
    dvar_name_list = std::move(merged);
  }

  dvar_list_loaded = true;
  dynamic_name_count = total;
}

bool starts_with_ci(const std::string &s, const std::string &prefix) {
  if (s.size() < prefix.size()) {
    return false;
  }
  return _strnicmp(s.c_str(), prefix.c_str(), prefix.size()) == 0;
}

bool collect_dvar_matches(const std::string &current,
                          std::vector<std::string> &matches,
                          std::string &prefix, std::string &partial) {
  if (current.empty()) {
    return false;
  }

  const auto last_semicolon = current.find_last_of(';');
  const std::string command_part = (last_semicolon == std::string::npos)
      ? current
      : current.substr(last_semicolon + 1);

  size_t first_non_space = command_part.find_first_not_of(" \t");
  if (first_non_space == std::string::npos) {
    return false;
  }
  const std::string trimmed_command = command_part.substr(first_non_space);

  if (trimmed_command.find_first_of(" \t") != std::string::npos) {
    return false;
  }

  prefix = (last_semicolon == std::string::npos) ? "" : current.substr(0, last_semicolon + 1);
  prefix += command_part.substr(0, first_non_space);

  partial = trimmed_command;
  if (partial.empty()) {
    return false;
  }

  std::vector<std::string> snapshot;
  {
    std::lock_guard lock(dvar_list_mutex);
    snapshot = dvar_name_list;
  }

  if (snapshot.empty()) {
    return false;
  }

  constexpr size_t max_matches = 50;
  matches.clear();

  const auto begin_it = std::lower_bound(
      snapshot.begin(), snapshot.end(), partial,
      [](const std::string &s, const std::string &p) {
        return compare_dvar_names_ci(s, p) < 0;
      });

  auto end_it = begin_it;
  while (end_it != snapshot.end() && starts_with_ci(*end_it, partial)) {
    ++end_it;
  }

  std::unordered_set<std::string> added_ci;
  for (auto it = begin_it; it != end_it && matches.size() < max_matches; ++it) {
    matches.push_back(*it);
    added_ci.insert(to_lower_copy(*it));
  }

  if (matches.size() < max_matches) {
    const std::string partial_lower = to_lower_copy(partial);

    std::vector<std::pair<size_t, const std::string *>> substring_hits;
    substring_hits.reserve(snapshot.size());

    for (const auto &name : snapshot) {
      const std::string name_lower = to_lower_copy(name);
      if (added_ci.contains(name_lower)) {
        continue;
      }

      const auto pos = name_lower.find(partial_lower);
      if (pos != std::string::npos) {
        substring_hits.emplace_back(pos, &name);
      }
    }

    std::sort(substring_hits.begin(), substring_hits.end(),
             [](const auto &a, const auto &b) {
               if (a.first != b.first) {
                 return a.first < b.first;
               }
               return dvar_name_less(*a.second, *b.second);
             });

    for (const auto &hit : substring_hits) {
      if (matches.size() >= max_matches) {
        break;
      }
      matches.push_back(*hit.second);
    }
  }

  return !matches.empty();
}

std::string build_match_hint(const std::string &partial,
                             const std::vector<std::string> &matches) {
  std::string hint;
  for (size_t i = 0; i < matches.size() && i < 10; ++i) {
    hint += matches[i] + "\n";
  }
  if (matches.size() > 10) {
    hint += "+" + std::to_string(matches.size() - 10) + " more";
  }
  return hint;
}

void preview_completion_hint(const HWND input_hwnd) {
  if (!dvar_list_loaded) {
    load_dvar_list();
  }
  merge_dynamic_names();

  if (!dvar_list_loaded || !completion_hint_hwnd) {
    update_completion_hint("");
    return;
  }

  const std::string current = get_window_text_safe(input_hwnd);
  if (current.find_first_not_of(" \t") == std::string::npos) {
    reset_tab_cycle();
    update_completion_hint("");
    return;
  }

  std::string prefix;
  std::string partial;
  std::vector<std::string> matches;
  if (!collect_dvar_matches(current, matches, prefix, partial)) {
    reset_tab_cycle();
    update_completion_hint("");
    return;
  }

  if (partial != tab_cycle_partial) {
    reset_tab_cycle();
  }

  const std::string hint = build_match_hint(partial, matches);
  update_completion_hint(hint);
}

bool try_autocomplete_dvar(const HWND input_hwnd) {
  if (!dvar_list_loaded) {
    load_dvar_list();
  }
  merge_dynamic_names();

  if (!dvar_list_loaded) {
    return false;
  }

  const std::string current = get_window_text_safe(input_hwnd);
  if (current.find_first_not_of(" \t") == std::string::npos) {
    return false;
  }

  std::string prefix;
  std::string partial;
  std::vector<std::string> matches;
  if (!collect_dvar_matches(current, matches, prefix, partial)) {
    update_completion_hint("");
    reset_tab_cycle();
    return false;
  }

  if (partial != tab_cycle_partial ||
      tab_cycle_matches.size() != matches.size() ||
      !std::equal(matches.begin(), matches.end(), tab_cycle_matches.begin(),
                  [](const std::string &a, const std::string &b) {
                    return _stricmp(a.c_str(), b.c_str()) == 0;
                  })) {
    tab_cycle_partial = partial;
    tab_cycle_matches = matches;
    tab_cycle_index = 0;
  }

  if (matches.size() == 1) {
    const std::string completed = prefix + matches[0];
    SetWindowTextA(input_hwnd, completed.c_str());
    SendMessageA(input_hwnd, EM_SETSEL, completed.size(),
                 static_cast<LPARAM>(completed.size()));
    update_completion_hint("");
    reset_tab_cycle();
    restore_input_caret();
    return true;
  }

  const bool all_prefix_matches = std::all_of(
      matches.begin(), matches.end(),
      [&](const std::string &m) { return starts_with_ci(m, partial); });

  if (all_prefix_matches) {
    size_t common_len = partial.size();
    for (; common_len < matches[0].size(); ++common_len) {
      const char c = matches[0][common_len];
      bool all_match = true;
      for (size_t i = 1; i < matches.size(); ++i) {
        if (common_len >= matches[i].size() ||
            static_cast<char>(std::tolower(
                static_cast<unsigned char>(matches[i][common_len]))) !=
                static_cast<char>(std::tolower(static_cast<unsigned char>(c)))) {
          all_match = false;
          break;
        }
      }
      if (!all_match) {
        break;
      }
    }

    if (common_len > partial.size()) {
      const std::string completed = prefix + matches[0].substr(0, common_len);
      SetWindowTextA(input_hwnd, completed.c_str());
      SendMessageA(input_hwnd, EM_SETSEL, completed.size(),
                   static_cast<LPARAM>(completed.size()));
      tab_cycle_partial = matches[0].substr(0, common_len);
      tab_cycle_index = 0;
      update_completion_hint(build_match_hint(tab_cycle_partial, matches));
      restore_input_caret();
      return true;
    }
  }

  const std::string &choice =
      tab_cycle_matches[tab_cycle_index % tab_cycle_matches.size()];
  const std::string completed = prefix + choice;
  SetWindowTextA(input_hwnd, completed.c_str());
  SendMessageA(input_hwnd, EM_SETSEL, completed.size(),
               static_cast<LPARAM>(completed.size()));
  ++tab_cycle_index;

  update_completion_hint(build_match_hint(partial, tab_cycle_matches));
  restore_input_caret();

  return true;
}

void print_message(const char *message) {
#ifndef NDEBUG
  OutputDebugStringA(message);
#endif

  if (started && !terminate_runner) {
    game::com::Com_Printf(0, game::consoleLabel_e::DEFAULT, "%s", message);
  }
}

void queue_message(const char *message) {
  std::string msg(message);

  interceptor.access(
      [&msg](const std::function<void(const std::string &)> &callback) {
        if (callback) {
          callback(msg);
        }
      });

  message_queue.access([&msg](std::queue<std::string> &queue) {
    queue.push(std::move(msg));
  });
}

std::queue<std::string> empty_message_queue() {
  std::queue<std::string> current_queue{};

  message_queue.access([&](std::queue<std::string> &queue) {
    current_queue = std::move(queue);
    queue = {};
  });

  return current_queue;
}

void print_stub(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  if (full_logs_enabled()) {
    va_list ap_copy;
    va_copy(ap_copy, ap);
    const int needed = _vscprintf(fmt, ap_copy);
    va_end(ap_copy);

    if (needed > 0) {
      std::string buffer(static_cast<size_t>(needed) + 1, '\0');
      vsnprintf_s(buffer.data(), buffer.size(), _TRUNCATE, fmt, ap);
      buffer.resize(static_cast<size_t>(needed));
      print_message(buffer.c_str());
    }
  } else {
    char buffer[1024]{0};
    const int res = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, ap);
    (void)res;
    print_message(buffer);
  }

  va_end(ap);
}

INT_PTR get_gray_brush() {
  static utils::image::object b(CreateSolidBrush(get_background_color()));
  return reinterpret_cast<INT_PTR>(b.get());
}

INT_PTR get_hint_brush() {
  static utils::image::object b(CreateSolidBrush(get_hint_background_color()));
  return reinterpret_cast<INT_PTR>(b.get());
}

bool font_family_exists(const std::wstring &name) {
  bool found = false;

  LOGFONTW lf{};
  lf.lfCharSet = DEFAULT_CHARSET;
  wcsncpy_s(lf.lfFaceName, name.c_str(), LF_FACESIZE - 1);

  const HDC dc = GetDC(nullptr);
  EnumFontFamiliesExW(
      dc, &lf,
      [](const LOGFONTW *, const TEXTMETRICW *, DWORD, LPARAM lparam) -> int {
        *reinterpret_cast<bool *>(lparam) = true;
        return 0;
      },
      reinterpret_cast<LPARAM>(&found), 0);
  ReleaseDC(nullptr, dc);

  return found;
}

std::wstring pick_console_font() {
  if (font_family_exists(L"Cascadia Mono")) {
    return L"Cascadia Mono";
  }
  if (font_family_exists(L"Cascadia Code")) {
    return L"Cascadia Code";
  }
  if (font_family_exists(L"Consolas")) {
    return L"Consolas";
  }
  return L"Courier New";
}

void apply_modern_window_style(const HWND hwnd) {
  if (!hwnd) {
    return;
  }

  BOOL dark_mode = TRUE;
  DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode,
                        sizeof(dark_mode));

  DWORD corner_preference = DWMWCP_ROUND;
  DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                        &corner_preference, sizeof(corner_preference));
}

void force_exit_after(const DWORD grace_ms) {
  std::thread([grace_ms] {
    std::this_thread::sleep_for(std::chrono::milliseconds(grace_ms));
    TerminateProcess(GetCurrentProcess(), 1);
  }).detach();
}

void resize_console_controls(const HWND hwnd) {
  if (!hwnd || !*game::s_wcd::hwndBuffer || !*game::s_wcd::hwndInputLine) {
    return;
  }

  RECT rect{};
  GetClientRect(hwnd, &rect);

  constexpr int margin = CONSOLE_MARGIN;
  constexpr int input_height = CONSOLE_INPUT_HEIGHT;
  const int client_width =
      (std::max)(0, static_cast<int>((rect.right - rect.left) - margin * 2));
  const int client_height =
      (std::max)(0, static_cast<int>((rect.bottom - rect.top)));

  int logo_width = 0;
  int logo_height = 0;
  HBITMAP logo_bmp = nullptr;
  if (*game::s_wcd::codLogo) {
    logo_bmp = reinterpret_cast<HBITMAP>(
        SendMessageA(*game::s_wcd::codLogo, STM_GETIMAGE, IMAGE_BITMAP, 0));
    BITMAP bm{};
    if (logo_bmp && GetObjectA(logo_bmp, sizeof(bm), &bm) == sizeof(bm)) {
      logo_width = bm.bmWidth;
      logo_height = bm.bmHeight;
    }
  }

  const int top_offset = (std::max)(CONSOLE_HEADER_HEIGHT, logo_height + 12);

  const int buffer_height = (std::max)(
      0, client_height - top_offset - input_height - margin * 2);
  const int input_y = client_height - input_height - margin;

  MoveWindow(*game::s_wcd::hwndBuffer, margin, top_offset, client_width,
             buffer_height, TRUE);
  MoveWindow(*game::s_wcd::hwndInputLine, margin, input_y, client_width,
             input_height, TRUE);

  if (completion_hint_hwnd && IsWindowVisible(completion_hint_hwnd)) {
    char hint_text[2048]{};
    GetWindowTextA(completion_hint_hwnd, hint_text, sizeof(hint_text));
    const size_t line_count =
        hint_text[0] == '\0'
            ? 0
            : 1 + std::count(std::begin(hint_text), std::end(hint_text), '\n');
    const int popup_height = (std::max)(
        COMPLETION_HINT_HEIGHT, static_cast<int>((std::max)(line_count, size_t{4}) * 14 + 8));
    reposition_completion_hint(popup_height);
  } else if (completion_hint_hwnd) {
    reposition_completion_hint((std::max)(COMPLETION_HINT_HEIGHT, 80));
  }

  if (*game::s_wcd::codLogo && logo_width > 0) {
    const int x = (std::max)(margin, margin + (client_width - logo_width) / 2);
    constexpr int y = 6;
    SetWindowPos(*game::s_wcd::codLogo, HWND_TOP, x, y, logo_width,
                 logo_height, SWP_NOACTIVATE);
  }

  restore_input_caret();
}

LRESULT con_wnd_proc(const HWND hwnd, const UINT msg, const WPARAM wparam,
                     const LPARAM lparam) {
  switch (msg) {
  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORSTATIC:
    if (reinterpret_cast<HWND>(lparam) == completion_hint_hwnd) {
      SetBkColor(reinterpret_cast<HDC>(wparam), get_hint_background_color());
      SetTextColor(reinterpret_cast<HDC>(wparam), RGB(96, 165, 250));
      return get_hint_brush();
    }
    SetBkColor(reinterpret_cast<HDC>(wparam), get_background_color());
    SetTextColor(reinterpret_cast<HDC>(wparam), get_default_console_color());
    return get_gray_brush();
  case WM_SIZE:
    resize_console_controls(hwnd);
    return 0;
  case WM_VSCROLL:
  case WM_MOUSEWHEEL:

    if (completion_hint_hwnd && IsWindowVisible(completion_hint_hwnd)) {
      SetWindowPos(completion_hint_hwnd, HWND_TOP, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    break;
  case WM_ACTIVATE:
    if (LOWORD(wparam) != WA_INACTIVE && *game::s_wcd::hwndInputLine) {
      preview_completion_hint(*game::s_wcd::hwndInputLine);
    }
    break;
  case WM_APPEND_CONSOLE_TEXT: {
    const auto *text = reinterpret_cast<std::string *>(lparam);
    if (text) {
      append_text_with_severity(*game::s_wcd::hwndBuffer, *text);
      delete text;
    }
    if (completion_hint_hwnd && IsWindowVisible(completion_hint_hwnd)) {
      SetWindowPos(completion_hint_hwnd, HWND_TOP, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    restore_input_caret();
    return 0;
  }
  case WM_CLOSE:
    if (!close_requested.exchange(true)) {
      ShowWindow(hwnd, SW_HIDE);
      force_exit_after(3000);
      game::cbuf::Cbuf_AddText(0, "quit\n");
    }
    [[fallthrough]];
  default:
    return utils::hook::invoke<LRESULT>(game::select(0x142332960, 0x1405973E0),
                                        hwnd, msg, wparam, lparam);
  }

  return utils::hook::invoke<LRESULT>(game::select(0x142332960, 0x1405973E0),
                                      hwnd, msg, wparam, lparam);
}

LRESULT input_line_wnd_proc(const HWND hwnd, const UINT msg,
                            const WPARAM wparam, const LPARAM lparam) {
  if ((msg == WM_KEYDOWN && wparam == VK_TAB) ||
      (msg == WM_CHAR && wparam == '\t')) {
    try_autocomplete_dvar(hwnd);
    return 0;
  }

  if (msg == WM_KEYDOWN && wparam == VK_UP) {
    if (!command_history.empty()) {
      if (history_index == command_history.size()) {
        history_draft = get_window_text_safe(hwnd);
      }
      if (history_index > 0) {
        --history_index;
      }
      const std::string &entry = command_history[history_index];
      SetWindowTextA(hwnd, entry.c_str());
      SendMessageA(hwnd, EM_SETSEL, entry.size(),
                   static_cast<LPARAM>(entry.size()));
      reset_tab_cycle();
      preview_completion_hint(hwnd);
      restore_input_caret();
    }
    return 0;
  }

  if (msg == WM_KEYDOWN && wparam == VK_DOWN) {
    if (!command_history.empty() && history_index < command_history.size()) {
      ++history_index;
      if (history_index == command_history.size()) {
        SetWindowTextA(hwnd, history_draft.c_str());
        SendMessageA(hwnd, EM_SETSEL, history_draft.size(),
                     static_cast<LPARAM>(history_draft.size()));
      } else {
        const std::string &entry = command_history[history_index];
        SetWindowTextA(hwnd, entry.c_str());
        SendMessageA(hwnd, EM_SETSEL, entry.size(),
                     static_cast<LPARAM>(entry.size()));
      }
      reset_tab_cycle();
      preview_completion_hint(hwnd);
      restore_input_caret();
    }
    return 0;
  }

  if (msg == WM_KEYDOWN && wparam == VK_RETURN) {
    const std::string entry = get_window_text_safe(hwnd);
    if (!entry.empty() &&
        (command_history.empty() || command_history.back() != entry)) {
      command_history.push_back(entry);
    }
    history_index = command_history.size();
    history_draft.clear();
  }

  const auto result = utils::hook::invoke<LRESULT>(
      game::select(0x142332C60, 0x1405976E0), hwnd, msg, wparam, lparam);

  if (msg == WM_SETFOCUS) {
    restore_input_caret();
    return result;
  }

  if (msg == WM_CHAR || msg == WM_KEYDOWN || msg == WM_PASTE) {
    if (msg == WM_KEYDOWN && (wparam == VK_RETURN || wparam == VK_ESCAPE ||
                             wparam == VK_UP || wparam == VK_DOWN ||
                             wparam == VK_LEFT || wparam == VK_RIGHT)) {
      restore_input_caret();
      return result;
    }
    preview_completion_hint(hwnd);
    restore_input_caret();
  }

  return result;
}

utils::hook::detour sys_show_console_hook;
std::atomic_bool console_shown_once{false};

void sys_show_console_stub() {
  if (!console_shown_once.exchange(true)) {
    sys_show_console_hook.invoke<void>();
    reset_tab_cycle();
    update_completion_hint("");
    restore_input_caret();
    return;
  }

  if (*game::s_wcd::hWnd) {
    ShowWindow(*game::s_wcd::hWnd, SW_SHOW);
    SetForegroundWindow(*game::s_wcd::hWnd);
    if (*game::s_wcd::hwndInputLine)
      SetFocus(*game::s_wcd::hwndInputLine);
    reset_tab_cycle();
    update_completion_hint("");
    restore_input_caret();
  }
}

void sys_create_console_stub(const HINSTANCE h_instance) {
  if (game::is_headless()) {
    return;
  }

  LoadLibraryA("Msftedit.dll");

  char text[CONSOLE_BUFFER_SIZE]{0};

  const auto *class_name = "BOIII WinConsole";
  const auto *window_name =
      game::is_server() ? "BOIII Server" : "BOIII Console";

  WNDCLASSA wnd_class{};
  wnd_class.style = 0;
  wnd_class.lpfnWndProc = con_wnd_proc;
  wnd_class.cbClsExtra = 0;
  wnd_class.cbWndExtra = 0;
  wnd_class.hInstance = h_instance;
  wnd_class.hIcon = LoadIconA(h_instance, reinterpret_cast<LPCSTR>(1));
  wnd_class.hCursor = LoadCursorA(nullptr, reinterpret_cast<LPCSTR>(0x7F00));
  wnd_class.hbrBackground = CreateSolidBrush(get_background_color());
  wnd_class.lpszMenuName = nullptr;
  wnd_class.lpszClassName = class_name;

  if (!RegisterClassA(&wnd_class)) {
    return;
  }

  RECT rect{};
  rect.left = 0;
  rect.right = 1200;
  rect.top = 0;
  rect.bottom = 720;
  constexpr DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
  AdjustWindowRect(&rect, window_style, FALSE);

  auto dc = GetDC(GetDesktopWindow());
  const auto swidth = GetDeviceCaps(dc, 8);
  const auto sheight = GetDeviceCaps(dc, 10);
  ReleaseDC(GetDesktopWindow(), dc);

  const int window_width = (std::min)(
      std::max(CONSOLE_MIN_WIDTH, swidth * 3 / 4), 1400);
  const int window_height = (std::min)(
      std::max(CONSOLE_MIN_HEIGHT, sheight * 3 / 4), 900);
  const int window_x = (swidth - window_width) / 2;
  const int window_y = (sheight - window_height) / 2;

  utils::hook::set<int>(game::s_wcd::windowWidth, window_width);
  utils::hook::set<int>(game::s_wcd::windowHeight, window_height);

  utils::hook::set<HWND>(
      game::s_wcd::hWnd,
      CreateWindowExA(0, class_name, window_name, window_style, window_x,
                      window_y, window_width, window_height, nullptr,
                      nullptr, h_instance, nullptr));

  if (!*game::s_wcd::hWnd) {
    return;
  }

  apply_modern_window_style(*game::s_wcd::hWnd);

  dc = GetDC(*game::s_wcd::hWnd);
  const auto n_height = MulDiv(10, GetDeviceCaps(dc, 90), 72);

  const auto font_name = pick_console_font();
  utils::hook::set<HFONT>(
      game::s_wcd::hfBufferFont,
      CreateFontW(-n_height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                  CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN,
                  font_name.c_str()));

  ReleaseDC(*game::s_wcd::hWnd, dc);

  if (logo) {
    utils::hook::set<HWND>(game::s_wcd::codLogo,
                           CreateWindowExA(0, "Static", nullptr, 0x5000000Eu, 5,
                                           5, 0, 0, *game::s_wcd::hWnd,
                                           reinterpret_cast<HMENU>(1),
                                           h_instance, nullptr));
    SendMessageA(*game::s_wcd::codLogo, STM_SETIMAGE, IMAGE_BITMAP, logo);
  }

  utils::hook::set<HWND>(
      game::s_wcd::hwndInputLine,
      CreateWindowExA(0, "edit", nullptr, 0x50800080u, CONSOLE_MARGIN,
                      500, 0, CONSOLE_INPUT_HEIGHT, *game::s_wcd::hWnd,
                      reinterpret_cast<HMENU>(0x65), h_instance, nullptr));

  completion_hint_hwnd = CreateWindowExA(
      0, "EDIT", "",
      WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS | ES_READONLY | ES_MULTILINE,
      CONSOLE_MARGIN, 0, 0, COMPLETION_HINT_HEIGHT, *game::s_wcd::hWnd,
      reinterpret_cast<HMENU>(COMPLETION_HINT_CONTROL_ID), h_instance, nullptr);
  if (completion_hint_hwnd) {
    SendMessageA(completion_hint_hwnd, WM_SETFONT,
                 reinterpret_cast<WPARAM>(*game::s_wcd::hfBufferFont), 0);
    SendMessageA(completion_hint_hwnd, EM_SETBKGNDCOLOR, 0,
                 get_hint_background_color());
    SetWindowTextA(completion_hint_hwnd, "");
    ShowWindow(completion_hint_hwnd, SW_HIDE);
  }
  utils::hook::set<HWND>(
      game::s_wcd::hwndBuffer,
      CreateWindowExW(0, L"RICHEDIT50W", nullptr,
                      WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS |
                          ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY |
                          ES_NOHIDESEL,
                      CONSOLE_MARGIN, CONSOLE_HEADER_HEIGHT, 0, 0,
                      *game::s_wcd::hWnd, reinterpret_cast<HMENU>(0x64),
                      h_instance, nullptr));
  SendMessageA(*game::s_wcd::hwndBuffer, WM_SETFONT,
               reinterpret_cast<WPARAM>(*game::s_wcd::hfBufferFont), 0);
  SendMessageA(*game::s_wcd::hwndBuffer, EM_SETBKGNDCOLOR, 0,
               get_background_color());
  SendMessageA(*game::s_wcd::hwndBuffer, EM_SETLIMITTEXT, 0, 0);
  SendMessageW(*game::s_wcd::hwndBuffer, EM_EXLIMITTEXT, 0,
               full_logs_enabled() ? 0x7FFFFFFF
                                   : static_cast<LPARAM>(MAX_CONSOLE_CHARS * 2));

  utils::hook::set<WNDPROC>(
      game::s_wcd::SysInputLineWndProc,
      reinterpret_cast<WNDPROC>(
          SetWindowLongPtrA(*game::s_wcd::hwndInputLine, -4,
                            reinterpret_cast<LONG_PTR>(input_line_wnd_proc))));
  SendMessageA(*game::s_wcd::hwndInputLine, WM_SETFONT,
               reinterpret_cast<WPARAM>(*game::s_wcd::hfBufferFont), 0);

  SetFocus(*game::s_wcd::hwndInputLine);
  restore_input_caret();
  game::con::Con_GetTextCopy(text,
                             std::min(0x4000, static_cast<int>(sizeof(text))));
  append_text_with_severity(*game::s_wcd::hwndBuffer, text);
  resize_console_controls(*game::s_wcd::hWnd);
}
} // namespace

void set_interceptor(std::function<void(const std::string &message)> callback) {
  interceptor.access([&callback](std::function<void(const std::string &)> &c) {
    c = std::move(callback);
  });
}

void remove_interceptor() { set_interceptor({}); }

bool is_ready() { return started && !terminate_runner; }

void set_title(const std::string &title) {
  if (game::is_headless()) {
    SetConsoleTitleA(title.data());
  } else {
    SetWindowTextA(*game::s_wcd::hWnd, title.data());
  }
}

struct component final : generic_component {
  component() {
    if (game::is_headless()) {
      if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        AllocConsole();
        AttachConsole(GetCurrentProcessId());
      }

      ShowWindow(GetConsoleWindow(), SW_SHOW);

      FILE *fp;
      freopen_s(&fp, "CONIN$", "r", stdin);
      freopen_s(&fp, "CONOUT$", "w", stdout);
      freopen_s(&fp, "CONOUT$", "w", stderr);
    }
  }

  void post_unpack() override {
    if (utils::flags::has_flag("nologs")) {
      return;
    }

    if (!game::is_server()) {
      utils::hook::set<uint8_t>(0x14133D2FE_g,
                                0xEB); // Always enable ingame console
      utils::hook::jump(0x141344E44_g, 0x141344E2E_g);

      if (utils::nt::is_wine() && !utils::flags::has_flag("console")) {
        return;
      }
    }

    utils::hook::jump(printf, print_stub);

    utils::hook::jump(game::select(0x142332C30, 0x1405976B0), queue_message);
    utils::hook::nop(game::select(0x142332C4A, 0x1405976CA),
                     2); // Print from every thread

    const auto res = utils::nt::load_resource(IMAGE_LOGO);
    const auto img = utils::image::load_image(res);
    logo = utils::image::create_bitmap(img);

    terminate_runner = false;
    load_dvar_list();

    this->message_runner_ =
        utils::thread::create_named_thread("Console IO", [] {
          std::string message_buffer;
          while (!terminate_runner) {
            message_buffer.clear();
            auto current_queue = empty_message_queue();

            while (!current_queue.empty()) {
              const auto &msg = current_queue.front();
              message_buffer.append(msg);
              current_queue.pop();
            }

            if (!message_buffer.empty()) {
              if (game::is_headless()) {
                fputs(message_buffer.data(), stdout);
              } else if (*game::s_wcd::hWnd) {
                auto *payload = new std::string(message_buffer);
                if (!PostMessageA(*game::s_wcd::hWnd, WM_APPEND_CONSOLE_TEXT,
                                  0, reinterpret_cast<LPARAM>(payload))) {
                  delete payload;
                }
              }
            }

            std::this_thread::sleep_for(5ms);
          }
        });

    this->console_runner_ =
        utils::thread::create_named_thread("Console Window", [] {
          {
            static utils::hook::detour sys_create_console_hook;
            sys_create_console_hook.create(
                game::select(0x142332E00, 0x140597880),
                sys_create_console_stub);

            sys_show_console_hook.create(game::sys::Sys_ShowConsole,
                                         sys_show_console_stub);

            game::sys::Sys_ShowConsole();
            started = true;
          }

          MSG msg{};
          while (!terminate_runner) {
            if (PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE)) {
              TranslateMessage(&msg);
              DispatchMessageW(&msg);
            } else {
              std::this_thread::sleep_for(5ms);
            }
          }
        });

    while (!started) {
      std::this_thread::sleep_for(10ms);
    }

#ifndef NDEBUG
    scheduler::once(
        []() {
          const utils::nt::library game_module{};
          printf("Entry Point: 0x%llX\n", reinterpret_cast<unsigned long long>(
                                              game_module.get_entry_point()));
        },
        scheduler::main);
#endif
  }

  void pre_destroy() override {
    terminate_runner = true;

    if (this->message_runner_.joinable()) {
      this->message_runner_.join();
    }

    if (this->console_runner_.joinable()) {
      this->console_runner_.join();
    }
  }

private:
  std::thread console_runner_{};
  std::thread message_runner_{};
};
} // namespace console

REGISTER_COMPONENT(console::component)