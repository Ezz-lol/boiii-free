#pragma once

#include "updater/file_info.hpp"
#include "updater/progress_listener.hpp"

#include <utils/info_string.hpp>
#include <utils/progress_ui.hpp>

#include <chrono>

namespace fastdl
{
	struct speed_tracker
	{
		std::chrono::steady_clock::time_point last_time{std::chrono::steady_clock::now()};
		std::size_t last_bytes{0};
		float speed_bps{0.0f};

		float update(std::size_t current_bytes)
		{
			const auto now = std::chrono::steady_clock::now();
			const float elapsed = std::chrono::duration<float>(now - last_time).count();
			if (elapsed >= 0.3f)
			{
				const float instant = static_cast<float>(current_bytes - last_bytes) / elapsed;
				speed_bps = speed_bps * 0.7f + instant * 0.3f;
				last_bytes = current_bytes;
				last_time  = now;
			}
			return speed_bps;
		}

		int eta(std::size_t total, std::size_t downloaded) const
		{
			if (speed_bps < 1024.f || total == 0) return -1;
			return static_cast<int>(static_cast<float>(total - downloaded) / speed_bps);
		}
	};

	struct download_context
	{
		std::string mapname;
		std::string pub_id;
		std::string map_path;
		std::string base_url;
		std::function<void()> success_callback;
	};

	void start_map_download(const download_context& context);
	void cancel_download();
	bool is_downloading();

	class fastdl_ui : public updater::progress_listener
	{
	public:
		fastdl_ui();
		~fastdl_ui();

		void update_files(const std::vector<updater::file_info>& files) override;
		void done_update() override;

		void begin_file(const updater::file_info& file) override;
		void end_file(const updater::file_info& file) override;

		void file_progress(const updater::file_info& file, size_t progress) override;

	private:
		mutable std::recursive_mutex mutex_;
		std::vector<updater::file_info> total_files_{};
		std::vector<updater::file_info> downloaded_files_{};
		std::unordered_map<std::string, std::pair<size_t, size_t>> downloading_files_{};

		utils::progress_ui progress_ui_{false};
		speed_tracker speed_{};
		std::string current_filename_{};

		void handle_cancellation() const;
		void update_progress();
		void update_file_name();

		size_t get_total_size() const;
		size_t get_downloaded_size() const;
		size_t get_total_files() const;
		size_t get_downloaded_files() const;

		std::string get_relevant_file_name() const;
	};

	class download_is_cancelled : public std::exception
	{
	public:
		[[nodiscard]] const char* what() const noexcept override
		{
			return "Download was cancelled";
		}
	};
}
