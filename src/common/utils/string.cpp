#include "string.hpp"

#include <cassert>
#include <windows.h>

#include "macros/ptr.hpp"

namespace utils::string {
// Defined primarily as a utility function to be used where the engine's `va`
// function is hooked or would be called in a decompiled function
// NOLINTNEXTLINE(modernize-avoid-variadic-functions)
const char *va(const char *fmt, ...) {
  constexpr size_t VA_PROVIDER_MIN_BUFFER_SIZE =
      static_cast<size_t>(std::numeric_limits<uint8_t>::max());
  constexpr size_t VA_PROVIDER_BUFFER_COUNT = 8;
  thread_local va_provider<VA_PROVIDER_BUFFER_COUNT,
                           VA_PROVIDER_MIN_BUFFER_SIZE>
      provider;

  va_list ap = nullptr;
  va_start(ap, fmt);

  const char *result = provider.get(fmt, ap);

  va_end(ap);
  return result;
}

std::vector<std::string_view> split(const std::string_view &s,
                                    const std::string_view &delim) {
  std::vector<std::string_view> result;
  size_t start = 0;
  size_t end = s.find(delim);

  while (end != std::string_view::npos) {
    result.push_back(s.substr(start, end - start));
    start = end + 1;
    end = s.find(delim, start);
  }
  // Add the final segment (or the only segment if no delimiter is found)
  result.push_back(s.substr(start));

  return result;
}

std::vector<std::string> split(const std::string &s,
                               const std::string_view &delim) {
  std::vector<std::string> result;
  size_t start = 0;
  size_t end = s.find(delim);

  while (end != std::string_view::npos) {
    result.push_back(s.substr(start, end - start));
    start = end + 1;
    end = s.find(delim, start);
  }
  // Add the final segment (or the only segment if no delimiter is found)
  result.push_back(s.substr(start));

  return result;
}

bool starts_with(const std::string_view &text,
                 const std::string_view &substring) {
  return text.starts_with(substring);
}

bool contains(const std::string_view &text, const std::string_view &substr) {
  return text.find(substr) != std::string::npos;
}

bool ends_with(const std::string_view &text,
               const std::string_view &substring) {
  return substring.size() <= text.size() &&
         std::equal(substring.rbegin(), substring.rend(), text.rbegin());
}

bool is_numeric(const std::string_view &text) {
  std::string_view::const_iterator it = text.begin();
  while (it != text.end() && std::isdigit(static_cast<uint8_t>(*it)) != 0) {
    ++it;
  }

  return !text.empty() && it == text.end();
}

std::string dump_hex(const std::string_view &data,
                     const std::string_view &separator) {
  std::string result;

  for (size_t i = 0; i < data.size(); ++i) {
    if (i > 0) {
      result.append(separator);
    }

    result.append(va("%02X", static_cast<uint8_t>(data[i])));
  }

  return result;
}

std::string get_clipboard_data() {
  if (OpenClipboard(nullptr) != 0) {
    std::string data;

    HANDLE clipboard_data = GetClipboardData(1u);
    if (clipboard_data != nullptr) {
      char *const cliptext = static_cast<char *>(GlobalLock(clipboard_data));
      if (cliptext != nullptr) {
        data.append(cliptext);
        GlobalUnlock(clipboard_data);
      }
    }
    CloseClipboard();

    return data;
  }
  return {};
}

void strip(const char *in, char *out, size_t max) {
  assert(max > 0);
  if (in != nullptr && out != nullptr) {
    max--;
    size_t current = 0;
    while (*in != '\0' && current < max) {
      const size_t color_index = (*(in + 1) - 48) >= 0xC ? 7 : (*(in + 1) - 48);

      if (*in == '^' && (color_index != 7 || *(in + 1) == '7')) {
        ++in;
      } else {
        *out = *in;
        ++out;
        ++current;
      }

      ++in;
    }

    *out = '\0';
  }
}

void strip_material(const char *in, char *out, size_t max) {
  assert(max > 0);
  if (in != nullptr && out != nullptr) {
    size_t i = 0;
    while (*in != '\0' && i < max - 1) {
      if (*in != '$' && *in != '{' && *in != '}') {
        *out++ = *in;
        ++i;
      }
      ++in;
    }

    *out = '\0';
  }
}

std::string convert(const std::wstring &wstr) {
  std::string result;
  result.reserve(wstr.size());

  for (const wchar_t chr : wstr) {
    result.push_back(static_cast<char>(chr));
  }

  return result;
}

std::wstring convert(const std::string &str) {
  std::wstring result;
  result.reserve(str.size());

  for (const char chr : str) {
    result.push_back(static_cast<wchar_t>(chr));
  }

  return result;
}

std::string replace(std::string str, const std::string &from,
                    const std::string &to) {
  if (from.empty()) {
    return str;
  }

  for (size_t start_pos = str.find(from); start_pos != std::string::npos;
       start_pos = str.find(from, start_pos)) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }

  return str;
}

std::string join(std::vector<std::string> strings,
                 const std::string &separator) {
  std::string result;
  if (strings.empty()) {
    return result;
  }
  result += strings[0];

  for (size_t i = 1; i < strings.size(); ++i) {
    result += separator;
    result += strings[i];
  }

  return result;
}

std::string join(std::vector<std::string_view> strings,
                 const std::string &separator) {
  std::string result;
  if (strings.empty()) {
    return result;
  }
  result += strings[0];

  for (size_t i = 1; i < strings.size(); ++i) {
    result += separator;
    result += strings[i];
  }

  return result;
}

constexpr uint8_t NIBBLE_MASK = 0x0F;
std::string hexdump(uintptr_t ptr, size_t size) {
  if (ptr != UINTPTR_NULLPTR && size > 0) {

    static constexpr char HEX_DIGITS[] = "0123456789ABCDEF";
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(ptr);

    std::string hex_str;
    hex_str.reserve(size * 2);

    for (size_t i = 0; i < size; ++i) {
      hex_str.push_back(
          HEX_DIGITS[(bytes[i] >> 4) & NIBBLE_MASK]);        // High nibble
      hex_str.push_back(HEX_DIGITS[bytes[i] & NIBBLE_MASK]); // Low nibble
    }

    return hex_str;
  }
  return "";
}

} // namespace utils::string
