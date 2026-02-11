#pragma once

#include "updater/file_info.hpp"
#include "updater/progress_listener.hpp"

#include <utils/info_string.hpp>
#include <utils/progress_ui.hpp>

namespace fastdl
{
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

		void handle_cancellation() const;
		void update_progress() const;
		void update_file_name() const;

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
