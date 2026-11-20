#pragma once

#include <functional>
#include <future>
#include <optional>
#include <string>
#include <unordered_map>

namespace utils::http {

constexpr const char *PREFERRED_ACCEPT_ENCODING_HEADER =
    "zstd;q=1.0,gzip;q=0.9,br;q=0.8,deflate;q=0.7,identity;q=0.5";

using headers = std::unordered_map<std::string, std::string>;

std::optional<std::string>
get_data(const std::string_view &url, const headers &headers = {},
         const std::function<void(size_t)> &callback = {},
         uint32_t retries = 2);
// POST with application/x-www-form-urlencoded body. Returns response body on
// 200, nullopt on any failure. No throw.
std::optional<std::string> post_data(const std::string_view &url,
                                     const std::string &post_body,
                                     uint32_t timeout_sec = 5);
// Stream download with progress (bytes received) and chunk write callback.
// Returns CURLcode (CURLE_OK on success).
int get_data_stream(
    const std::string_view &url, const headers &headers = {},
    const std::function<void(size_t, size_t)> &progress_cb = {},
    const std::function<void(const char *, size_t)> &write_cb = {},
    uint32_t retries = 2);
std::future<std::optional<std::string>>
get_data_async(const std::string_view &url, const headers &headers = {});

struct response {
  long status{};
  std::string body;
  std::string error;
};

struct request_options {
  std::string method = "GET";
  std::string body;
  headers headers;
  uint32_t timeout_sec = 30;
  size_t max_size = 16 * 1024 * 1024;
};

response request(const std::string &url, const request_options &options);
} // namespace utils::http
