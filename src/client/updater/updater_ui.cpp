#include <std_include.hpp>
#include "updater_ui.hpp"
#include "update_cancelled.hpp"

#include <utils/string.hpp>

namespace updater
{
	updater_ui::updater_ui() = default;
	updater_ui::~updater_ui() = default;

	void updater_ui::update_files(const std::vector<file_info>& files)
	{
		this->handle_cancellation();

		std::lock_guard<std::recursive_mutex> _{this->mutex_};
		this->total_files_ = files;
		this->downloaded_files_.clear();
		this->downloading_files_.clear();

		this->progress_ui_ = {};
		this->progress_ui_.set_title("BOIII Updater");
		this->progress_ui_.show();

		// Is it good to add artificial sleeps?
		// Makes the ui nice, for sure.
		std::this_thread::sleep_for(200ms);
	}

	void updater_ui::done_update()
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};

		const auto total_size = this->get_total_size();

		this->update_file_name();

		// Updater UI seems to be delayed a bit, so this triggers proper progress
		this->progress_ui_.set_progress(total_size > 1 ? total_size - 1 : 0, total_size);
		std::this_thread::sleep_for(100ms);

		this->progress_ui_.set_progress(total_size, total_size);

		this->total_files_.clear();
		this->downloaded_files_.clear();
		this->downloading_files_.clear();

		std::this_thread::sleep_for(200ms);
	}

	void updater_ui::begin_file(const file_info& file)
	{
		this->handle_cancellation();

		std::lock_guard<std::recursive_mutex> _{this->mutex_};

		this->file_progress(file, 0);
		this->update_file_name();
	}

	void updater_ui::end_file(const file_info& file)
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};

		this->downloaded_files_.emplace_back(file);
		const auto entry = this->downloading_files_.find(file.name);
		if (entry != this->downloading_files_.end())
		{
			this->downloading_files_.erase(entry);
		}
		else
		{
			assert(false && "Failed to erase file.");
		}

		this->update_progress();
		this->update_file_name();
	}

	void updater_ui::file_progress(const file_info& file, const size_t progress)
	{
		this->handle_cancellation();

		std::lock_guard<std::recursive_mutex> _{this->mutex_};

		this->downloading_files_[file.name] = {progress, file.size};
		this->update_progress();
	}

	void updater_ui::handle_cancellation() const
	{
		if (this->progress_ui_.is_cancelled())
		{
			throw update_cancelled();
		}
	}

	void updater_ui::update_progress() const
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};
		this->progress_ui_.set_progress(this->get_downloaded_size(), this->get_total_size());
	}

	void updater_ui::update_file_name() const
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};

		const auto downloaded_file_count = this->get_downloaded_files();
		const auto total_file_count = this->get_total_files();

		if (downloaded_file_count == total_file_count)
		{
			this->progress_ui_.set_line(1, "Update successful.");
		}
		else
		{
			this->progress_ui_.set_line(1, utils::string::va("Updating files... (%zu/%zu)", downloaded_file_count,
			                                                 total_file_count));
		}

		this->progress_ui_.set_line(2, this->get_relevant_file_name());
	}

	size_t updater_ui::get_total_size() const
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};

		size_t total_size = 0;
		for (const auto& file : this->total_files_)
		{
			total_size += file.size;
		}

		return total_size;
	}

	size_t updater_ui::get_downloaded_size() const
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};

		size_t downloaded_size = 0;
		for (const auto& file : this->downloaded_files_)
		{
			downloaded_size += file.size;
		}

		for (const auto& file : this->downloading_files_)
		{
			downloaded_size += file.second.first;
		}

		return downloaded_size;
	}

	size_t updater_ui::get_total_files() const
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};
		return this->total_files_.size();
	}

	size_t updater_ui::get_downloaded_files() const
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};
		return this->downloaded_files_.size();
	}

	std::string updater_ui::get_relevant_file_name() const
	{
		std::lock_guard<std::recursive_mutex> _{this->mutex_};

		std::string name{};
		auto smallest = std::numeric_limits<size_t>::max();

		for (const auto& file : this->downloading_files_)
		{
			const auto max_size = file.second.second;
			if (max_size < smallest)
			{
				smallest = max_size;
				name = file.first;
			}
		}

		if (name.empty() && !this->downloaded_files_.empty())
		{
			name = this->downloaded_files_.back().name;
		}

		return name;
	}
}
