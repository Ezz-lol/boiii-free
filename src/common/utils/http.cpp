#include "http.hpp"
#include <curl/curl.h>
#include "finally.hpp"

#pragma comment(lib, "ws2_32.lib")

namespace utils::http
{
	namespace
	{
		struct progress_helper
		{
			const std::function<void(size_t)>* callback{};
			std::exception_ptr exception{};
		};

		struct stream_helper
		{
			const std::function<void(const char*, size_t)>* callback{};
			std::exception_ptr exception{};
		};

		int progress_callback(void* clientp, const curl_off_t /*dltotal*/, const curl_off_t dlnow,
		                      const curl_off_t /*ultotal*/, const curl_off_t /*ulnow*/)
		{
			auto* helper = static_cast<progress_helper*>(clientp);

			try
			{
				if (*helper->callback)
				{
					(*helper->callback)(dlnow);
				}
			}
			catch (...)
			{
				helper->exception = std::current_exception();
				return -1;
			}

			return 0;
		}

		size_t write_callback(void* contents, const size_t size, const size_t nmemb, void* userp)
		{
			auto* buffer = static_cast<std::string*>(userp);

			const auto total_size = size * nmemb;
			buffer->append(static_cast<char*>(contents), total_size);
			return total_size;
		}

		size_t write_callback_stream(void* contents, const size_t size, const size_t nmemb, void* userp)
		{
			const auto total_size = size * nmemb;
			auto* write_helper = static_cast<stream_helper*>(userp);

			try
			{
				if (write_helper->callback && *write_helper->callback)
				{
					(*write_helper->callback)(static_cast<const char*>(contents), total_size);
				}
			}
			catch (...)
			{
				write_helper->exception = std::current_exception();
			}

			return total_size;
		}
	}

	std::optional<std::string> get_data(const std::string& url, const headers& headers,
	                                    const std::function<void(size_t)>& callback, const uint32_t retries)
	{
		curl_slist* header_list = nullptr;
		auto* curl = curl_easy_init();
		if (!curl)
		{
			return {};
		}

		auto _ = finally([&]()
		{
			curl_slist_free_all(header_list);
			curl_easy_cleanup(curl);
		});

		for (const auto& header : headers)
		{
			auto data = header.first + ": " + header.second;
			header_list = curl_slist_append(header_list, data.data());
		}

		std::string buffer{};
		progress_helper helper{};
		helper.callback = &callback;

		// Add Accept header for binary data to prevent CDN content modification
		header_list = curl_slist_append(header_list, "Accept: application/octet-stream");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.data());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &helper);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "ezz-updater/1.0");
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		// Request raw data without encoding modifications
		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");

		for (auto i = 0u; i < retries + 1; ++i)
		{
			// Due to CURLOPT_FAILONERROR, CURLE_OK will not be met when the server returns 400 or 500
			if (curl_easy_perform(curl) == CURLE_OK)
			{
				long http_code = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

				if (http_code >= 200)
				{
					return {std::move(buffer)};
				}

				throw std::runtime_error(
					"Bad status code " + std::to_string(http_code) + " met while trying to download file " + url);
			}

			if (helper.exception)
			{
				std::rethrow_exception(helper.exception);
			}

			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

			if (http_code > 0)
			{
				break;
			}
		}

		return {};
	}

	std::optional<std::string> post_data(const std::string& url, const std::string& post_body,
	                                      const uint32_t timeout_sec)
	{
		auto* curl = curl_easy_init();
		if (!curl)
		{
			return {};
		}

		curl_slist* header_list = curl_slist_append(nullptr, "Content-Type: application/x-www-form-urlencoded");
		auto _ = finally([&]()
		{
			curl_slist_free_all(header_list);
			curl_easy_cleanup(curl);
		});

		std::string buffer{};
		curl_easy_setopt(curl, CURLOPT_URL, url.data());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body.data());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(timeout_sec));
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "ezz-updater/1.0");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

		const auto code = curl_easy_perform(curl);
		if (code != CURLE_OK)
		{
			return {};
		}

		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (http_code != 200)
		{
			return {};
		}

		return {std::move(buffer)};
	}

	int get_data_stream(const std::string& url, const headers& headers,
		const std::function<void(size_t)>& progress_cb,
		const std::function<void(const char*, size_t)>& write_cb, const uint32_t retries)
	{
		auto* curl = curl_easy_init();
		if (!curl)
		{
			return CURLE_FAILED_INIT;
		}

		curl_slist* header_list = nullptr;
		for (const auto& header : headers)
		{
			auto data = header.first + ": " + header.second;
			header_list = curl_slist_append(header_list, data.data());
		}
		header_list = curl_slist_append(header_list, "Accept: application/octet-stream");

		auto _ = finally([&]()
		{
			curl_slist_free_all(header_list);
			curl_easy_cleanup(curl);
		});

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.data());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "ezz-updater/1.0");
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");

		for (auto i = 0u; i < retries + 1; ++i)
		{
			progress_helper progress_helper{};
			progress_helper.callback = &progress_cb;

			stream_helper write_helper{};
			write_helper.callback = &write_cb;

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_stream);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_helper);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress_helper);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

			const auto code = curl_easy_perform(curl);

			if (code == CURLE_OK)
			{
				long http_code = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
				if (http_code >= 200)
				{
					return CURLE_OK;
				}
			}

			if (progress_helper.exception)
			{
				std::rethrow_exception(progress_helper.exception);
			}
			if (write_helper.exception)
			{
				std::rethrow_exception(write_helper.exception);
			}

			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			if (http_code > 0)
			{
				return code;
			}
		}

		return CURLE_OPERATION_TIMEDOUT;
	}

	std::future<std::optional<std::string>> get_data_async(const std::string& url, const headers& headers)
	{
		return std::async(std::launch::async, [url, headers]()
		{
			return get_data(url, headers);
		});
	}
}
