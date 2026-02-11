#pragma once

#include <string>
#include <optional>
#include <future>
#include <unordered_map>
#include <functional>

namespace utils::http
{
	using headers = std::unordered_map<std::string, std::string>;

	std::optional<std::string> get_data(const std::string& url, const headers& headers = {},
	                                    const std::function<void(size_t)>& callback = {}, uint32_t retries = 2);
	// POST with application/x-www-form-urlencoded body. Returns response body on 200, nullopt on any failure. No throw.
	std::optional<std::string> post_data(const std::string& url, const std::string& post_body,
	                                    uint32_t timeout_sec = 5);
	// Stream download with progress (bytes received) and chunk write callback. Returns CURLcode (CURLE_OK on success).
	int get_data_stream(const std::string& url, const headers& headers = {},
		const std::function<void(size_t)>& progress_cb = {},
		const std::function<void(const char*, size_t)>& write_cb = {}, uint32_t retries = 2);
	std::future<std::optional<std::string>> get_data_async(const std::string& url, const headers& headers = {});
}
