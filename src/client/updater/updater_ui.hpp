#pragma once

#include "progress_ui.hpp"
#include "progress_listener.hpp"

#include <utils/concurrency.hpp>

namespace updater
{
	class updater_ui : public progress_listener
	{
	public:
		updater_ui();
		~updater_ui();

	private:
		mutable std::recursive_mutex mutex_;
		std::vector<file_info> total_files_{};
		std::vector<file_info> downloaded_files_{};
		std::unordered_map<std::string, std::pair<size_t, size_t>> downloading_files_{};

		progress_ui progress_ui_{};

		void update_files(const std::vector<file_info>& files) override;
		void done_update() override;

		void begin_file(const file_info& file) override;
		void end_file(const file_info& file) override;

		void file_progress(const file_info& file, size_t progress) override;

		void handle_cancellation() const;
		void update_progress() const;
		void update_file_name() const;

		size_t get_total_size() const;
		size_t get_downloaded_size() const;

		size_t get_total_files() const;
		size_t get_downloaded_files() const;

		std::string get_relevant_file_name() const;
	};
}
