#include <std_include.hpp>

#include "updater.hpp"
#include "updater_ui.hpp"
#include "file_updater.hpp"

#include <utils/cryptography.hpp>
#include <utils/flags.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/compression.hpp>

#define UPDATE_SERVER "https://r2.ezz.lol/"

#define UPDATE_FILE_MAIN UPDATE_SERVER "b.json" //TODO: Remove on release
#define UPDATE_FOLDER_MAIN UPDATE_SERVER "b/"

#define UPDATE_HOST_BINARY "boiii.exe"

namespace updater
{
	namespace
	{
		std::string get_update_file()
		{
			return UPDATE_FILE_MAIN;
		}

		std::string get_update_folder()
		{
			return UPDATE_FOLDER_MAIN;
		}

		std::vector<file_info> parse_file_infos(const std::string& json)
		{
			rapidjson::Document doc{};
			doc.Parse(json.data(), json.size());

			if (!doc.IsArray())
			{
				return {};
			}

			std::vector<file_info> files{};

			for (const auto& element : doc.GetArray())
			{
				if (!element.IsArray())
				{
					continue;
				}

				auto array = element.GetArray();

				file_info info{};
				info.name.assign(array[0].GetString(), array[0].GetStringLength());
				info.size = array[1].GetInt64();
				info.hash.assign(array[2].GetString(), array[2].GetStringLength());

				files.emplace_back(std::move(info));
			}

			return files;
		}

		std::string get_cache_buster()
		{
			return "?" + std::to_string(
				std::chrono::duration_cast<std::chrono::nanoseconds>(
					std::chrono::system_clock::now().time_since_epoch()).count());
		}

		std::vector<file_info> get_file_infos()
		{
			const auto json = utils::http::get_data(get_update_file() + get_cache_buster());
			if (!json)
			{
				return {};
			}

			return parse_file_infos(*json);
		}

		std::string get_hash(const std::string& data)
		{
			return utils::cryptography::sha1::compute(data, true);
		}

		const file_info* find_host_file_info(const std::vector<file_info>& outdated_files)
		{
			for (const auto& file : outdated_files)
			{
				if (file.name == UPDATE_HOST_BINARY)
				{
					return &file;
				}
			}

			return nullptr;
		}

		size_t get_optimal_concurrent_download_count(const size_t file_count)
		{
			size_t cores = std::thread::hardware_concurrency();
			cores = (cores * 2) / 3;
			return std::max(1ull, std::min(cores, file_count));
		}

		bool is_inside_folder(const std::filesystem::path& file, const std::filesystem::path& folder)
		{
			const auto relative = std::filesystem::relative(file, folder);
			const auto start = relative.begin();
			return start != relative.end() && start->string() != "..";
		}
	}

	file_updater::file_updater(progress_listener& listener, std::filesystem::path base,
		std::filesystem::path process_file)
		: listener_(listener)
		, base_(std::move(base))
		, process_file_(std::move(process_file))
		, dead_process_file_(process_file_)
	{
		this->dead_process_file_.replace_extension(".exe.old");
		
		if (this->process_file_.extension() == ".old")
		{
			MessageBoxA(nullptr, 
				"You are running from a backup file (boiii.exe.old). This indicates a previous update failed.\n"
				"Please restore boiii.exe from the .old file and try again.",
				"Update Error", MB_OK | MB_ICONERROR);
		}
		
		this->delete_old_process_file();
	}

	void file_updater::create_config_file_if_not_exists() const
	{
		char* buffer = nullptr;
		size_t size = 0;

		// Use _dupenv_s instead of getenv
		if (_dupenv_s(&buffer, &size, "LOCALAPPDATA") != 0 || buffer == nullptr)
		{
			return;
		}

		// Automatically free the buffer when we're done
		std::unique_ptr<char, decltype(&free)> buffer_ptr(buffer, free);

		const auto config_path = std::filesystem::path(buffer) / "Activision" / "CoD" / "config.ini";

		std::error_code ec{};
		if (std::filesystem::exists(config_path, ec))
		{
			return;
		}

		std::filesystem::create_directories(config_path.parent_path(), ec);
		if (ec)
		{
			return;
		}

		utils::io::write_file(config_path.string(), {}, false);
	}


	void file_updater::run() const
	{
		this->create_config_file_if_not_exists();
		const auto files = get_file_infos();
		
		OutputDebugStringA(("Found " + std::to_string(files.size()) + " files in update manifest\n").c_str());

		if (!files.empty())
		{
			this->cleanup_directories(files);
		}

		const auto outdated_files = this->get_outdated_files(files);
		
		OutputDebugStringA(("Found " + std::to_string(outdated_files.size()) + " outdated files\n").c_str());

		for (const auto& file : outdated_files)
		{
			OutputDebugStringA(("  - " + file.name + "\n").c_str());
		}

#ifndef NDEBUG
		const auto* host_file = find_host_file_info(files);
		if (host_file)
		{
			std::string data{};
			const auto drive_name = this->get_drive_filename(*host_file);
			if (utils::io::read_file(drive_name, &data))
			{
				const auto hash = get_hash(data);
				if (hash != host_file->hash)
				{
					if (!utils::flags::has_flag("update"))
					{
						OutputDebugStringA("WARNING: Host binary is outdated but not updating in debug build\n");
						OutputDebugStringA("Run with -update flag to enable exe updates in debug mode\n");
					}
				}
			}
		}
#endif

		if (outdated_files.empty())
		{
			return;
		}

		this->update_host_binary(outdated_files);
		this->update_files(outdated_files);

		std::this_thread::sleep_for(1s);
	}

	void file_updater::update_file(const file_info& file) const
	{
		const auto url = get_update_folder() + file.name + "?" + file.hash;
		OutputDebugStringA(("Downloading: " + file.name + "\n").c_str());

		const auto data = utils::http::get_data(url, {}, [&](const size_t progress)
			{
				this->listener_.file_progress(file, progress);
			});

		if (!data)
		{
			throw std::runtime_error("Failed to download: " + file.name + " (no data received)");
		}

		if (data->size() != file.size)
		{
			throw std::runtime_error("Failed to download: " + file.name + 
				" (size mismatch: got " + std::to_string(data->size()) + 
				", expected " + std::to_string(file.size) + ")");
		}

		if (get_hash(*data) != file.hash)
		{
			throw std::runtime_error("Failed to download: " + file.name + " (hash mismatch)");
		}

		const auto out_file = this->get_drive_filename(file);
		const bool is_exe = file.name.ends_with(".exe") || file.name == UPDATE_HOST_BINARY;

		if (is_exe)
		{
			OutputDebugStringA(("Writing exe to temp file first: " + out_file.string() + "\n").c_str());

			auto temp_file = out_file;
			const auto temp_extension = temp_file.extension().string() + ".new";
			const auto temp_path = temp_file.replace_extension(temp_extension);

			if (!utils::io::write_file_executable(temp_path, *data))
			{
				throw std::runtime_error("Failed to write temp exe: " + file.name + " to " + temp_path.string());
			}

			OutputDebugStringA("Verifying temp exe size and hash...\n");

			std::string verify_data{};
			if (!utils::io::read_file(temp_path, &verify_data))
			{
				throw std::runtime_error("Failed to read back temp exe for verification: " + temp_path.string());
			}

			if (verify_data.size() != data->size())
			{
				throw std::runtime_error("Temp exe size mismatch after write: expected " + 
					std::to_string(data->size()) + ", got " + std::to_string(verify_data.size()));
			}

			if (get_hash(verify_data) != file.hash)
			{
				throw std::runtime_error("Temp exe hash mismatch after write");
			}

			OutputDebugStringA(("Moving temp exe to final location: " + out_file.string() + "\n").c_str());

			if (!MoveFileExW(temp_path.wstring().c_str(), out_file.wstring().c_str(), 
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
			{
				const DWORD error = GetLastError();
				throw std::runtime_error("Failed to replace exe: " + file.name + 
					" (MoveFileEx failed with error " + std::to_string(error) + ")");
			}

			OutputDebugStringA(("Successfully replaced executable: " + file.name + "\n").c_str());
		}
		else
		{
			OutputDebugStringA(("Writing to: " + out_file.string() + "\n").c_str());

			if (!utils::io::write_file(out_file, *data, false))
			{
				throw std::runtime_error("Failed to write: " + file.name + " to " + out_file.string() + 
					" (file may be locked or read-only)");
			}

			OutputDebugStringA(("Successfully updated: " + file.name + "\n").c_str());
		}
	}

	std::vector<file_info> file_updater::get_outdated_files(const std::vector<file_info>& files) const
	{
		std::vector<file_info> outdated_files{};

		for (const auto& info : files)
		{
			if (this->is_outdated_file(info))
			{
				outdated_files.emplace_back(info);
			}
		}

		return outdated_files;
	}

	void file_updater::update_host_binary(const std::vector<file_info>& outdated_files) const
	{
		const auto* host_file = find_host_file_info(outdated_files);
		if (!host_file)
		{
			return;
		}

		try
		{
			OutputDebugStringA("Starting exe update process...\n");
			this->move_current_process_file();

			OutputDebugStringA("Waiting for file system to settle...\n");
			std::this_thread::sleep_for(500ms);

			const auto max_retries = 3;
			std::exception_ptr last_exception;

			for (auto i = 0; i < max_retries; ++i)
			{
				try
				{
					OutputDebugStringA(("Attempting exe update, attempt " + std::to_string(i + 1) + "/" + std::to_string(max_retries) + "\n").c_str());
					this->update_files({ *host_file });
					OutputDebugStringA("Exe update successful!\n");

					OutputDebugStringA("Verifying final exe file...\n");
					std::string verify_data{};
					if (!utils::io::read_file(this->process_file_, &verify_data))
					{
						throw std::runtime_error("Failed to read updated exe for verification");
					}

					if (verify_data.size() != host_file->size)
					{
						throw std::runtime_error("Updated exe size mismatch: expected " + 
							std::to_string(host_file->size) + ", got " + std::to_string(verify_data.size()));
					}

					if (get_hash(verify_data) != host_file->hash)
					{
						throw std::runtime_error("Updated exe hash mismatch");
					}

					OutputDebugStringA("Final exe verification passed!\n");
					break;
				}
				catch (...)
				{
					last_exception = std::current_exception();
					if (i < max_retries - 1)
					{
						OutputDebugStringA("Exe update failed, retrying...\n");
						std::this_thread::sleep_for(1s);
					}
					else
					{
						OutputDebugStringA("Exe update failed after all retries\n");
						std::rethrow_exception(last_exception);
					}
				}
			}
		}
		catch (...)
		{
			OutputDebugStringA("Exe update failed, restoring old file...\n");
			this->restore_current_process_file();
			throw;
		}

		OutputDebugStringA("Exe update complete, preparing to relaunch...\n");

		if (!utils::flags::has_flag("norelaunch"))
		{
			utils::nt::relaunch_self();
		}

		throw update_cancelled();
	}

	void file_updater::update_files(const std::vector<file_info>& outdated_files) const
	{
		this->listener_.update_files(outdated_files);

		const auto thread_count = get_optimal_concurrent_download_count(outdated_files.size());

		std::vector<std::thread> threads{};
		std::atomic<size_t> current_index{ 0 };

		utils::concurrency::container<std::exception_ptr> exception{};

		for (size_t i = 0; i < thread_count; ++i)
		{
			threads.emplace_back([&]()
				{
					while (!exception.access<bool>([](const std::exception_ptr& ptr)
						{
							return static_cast<bool>(ptr);
						}))
					{
						const auto index = current_index++;
						if (index >= outdated_files.size())
						{
							break;
						}

						try
						{
							const auto& file = outdated_files[index];
							this->listener_.begin_file(file);
							this->update_file(file);
							this->listener_.end_file(file);
						}
						catch (...)
						{
							exception.access([](std::exception_ptr& ptr)
								{
									ptr = std::current_exception();
								});

							return;
						}
					}
				});
		}

		for (auto& thread : threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}

		exception.access([](const std::exception_ptr& ptr)
			{
				if (ptr)
				{
					std::rethrow_exception(ptr);
				}
			});

		this->listener_.done_update();
	}

	bool file_updater::is_outdated_file(const file_info& file) const
	{
#ifndef NDEBUG
		if (file.name == UPDATE_HOST_BINARY && !utils::flags::has_flag("update"))
		{
			OutputDebugStringA("Skipping host binary update in debug build (use -update flag to enable)\n");
			return false;
		}
#endif

		std::string data{};
		const auto drive_name = this->get_drive_filename(file);
		if (!utils::io::read_file(drive_name, &data))
		{
			OutputDebugStringA(("File not found, marking as outdated: " + file.name + "\n").c_str());
			return true;
		}

		if (data.size() != file.size)
		{
			OutputDebugStringA(("Size mismatch for " + file.name + ": local=" + std::to_string(data.size()) + 
				", remote=" + std::to_string(file.size) + "\n").c_str());
			return true;
		}

		const auto hash = get_hash(data);
		if (hash != file.hash)
		{
			OutputDebugStringA(("Hash mismatch for " + file.name + "\n").c_str());
			return true;
		}

		return false;
	}

	std::filesystem::path file_updater::get_drive_filename(const file_info& file) const
	{
		if (file.name == UPDATE_HOST_BINARY)
		{
			return this->process_file_;
		}

		return this->base_ / file.name;
	}

	void file_updater::move_current_process_file() const
	{
		OutputDebugStringA(("Moving exe from " + this->process_file_.string() + " to " + 
			this->dead_process_file_.string() + "\n").c_str());

		DWORD attrs = GetFileAttributesW(this->process_file_.wstring().c_str());
		if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_READONLY))
		{
			OutputDebugStringA("Removing read-only attribute from exe\n");
			SetFileAttributesW(this->process_file_.wstring().c_str(), attrs & ~FILE_ATTRIBUTE_READONLY);
		}

		for (auto i = 0; i < 5; ++i)
		{
			if (MoveFileExW(this->process_file_.wstring().c_str(), 
				this->dead_process_file_.wstring().c_str(), 
				MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED))
			{
				OutputDebugStringA("Successfully moved exe to .old\n");
				return;
			}

			const DWORD error = GetLastError();
			OutputDebugStringA(("Failed to move exe, attempt " + std::to_string(i + 1) + "/5, error: " + 
				std::to_string(error) + "\n").c_str());

			if (i < 4)
			{
				std::this_thread::sleep_for(500ms);
			}
		}

		throw std::runtime_error("Failed to move current process file after 5 attempts from " + 
			this->process_file_.string() + " to " + this->dead_process_file_.string());
	}

	void file_updater::restore_current_process_file() const
	{
		OutputDebugStringA(("Restoring exe from " + this->dead_process_file_.string() + " to " + 
			this->process_file_.string() + "\n").c_str());

		DWORD attrs = GetFileAttributesW(this->dead_process_file_.wstring().c_str());
		if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_READONLY))
		{
			OutputDebugStringA("Removing read-only attribute from .exe.old\n");
			SetFileAttributesW(this->dead_process_file_.wstring().c_str(), attrs & ~FILE_ATTRIBUTE_READONLY);
		}

		if (!MoveFileExW(this->dead_process_file_.wstring().c_str(), 
			this->process_file_.wstring().c_str(), 
			MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		{
			const DWORD error = GetLastError();
			throw std::runtime_error("Failed to restore process file from " + 
				this->dead_process_file_.string() + " to " + this->process_file_.string() + 
				" (error: " + std::to_string(error) + ")");
		}

		OutputDebugStringA("Successfully restored exe from .old\n");
	}

	void file_updater::delete_old_process_file() const
	{
		OutputDebugStringA(("Attempting to delete old process file: " + 
			this->dead_process_file_.string() + "\n").c_str());

		for (auto i = 0; i < 4; ++i)
		{
			utils::io::remove_file(this->dead_process_file_);
			if (!utils::io::file_exists(this->dead_process_file_))
			{
				OutputDebugStringA("Successfully deleted old process file\n");
				break;
			}

			OutputDebugStringA(("Failed to delete old process file, retrying (attempt " + 
				std::to_string(i + 1) + "/4)...\n").c_str());
			std::this_thread::sleep_for(2s);
		}

		if (utils::io::file_exists(this->dead_process_file_))
		{
			OutputDebugStringA("Warning: Could not delete old process file after 4 attempts\n");
		}
	}

	void file_updater::cleanup_directories(const std::vector<file_info>& files) const
	{
		if (!utils::io::directory_exists(this->base_))
		{
			return;
		}

		this->cleanup_root_directory(files);
		this->cleanup_data_directory(files);
	}

	void file_updater::cleanup_root_directory(const std::vector<file_info>& files) const
	{
		const auto existing_files = utils::io::list_files(this->base_);
		for (const auto& file : existing_files)
		{
			const auto entry = std::filesystem::relative(file, this->base_);
			if ((entry.string() == "user" || entry.string() == "data") && utils::io::directory_exists(file))
			{
				continue;
			}

			bool found = false;
			for (const auto& wantedFile : files)
			{
				if (wantedFile.name == entry)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				std::error_code code{};
				std::filesystem::remove_all(file, code);
			}
		}
	}

	void file_updater::cleanup_data_directory(const std::vector<file_info>& files) const
	{
		const auto base = std::filesystem::path(this->base_);
		if (!utils::io::directory_exists(base.string()))
		{
			return;
		}

		std::vector<std::filesystem::path> legal_files{};
		legal_files.reserve(files.size());
		for (const auto& file : files)
		{
			if (file.name.starts_with("data"))
			{
				legal_files.emplace_back(std::filesystem::absolute(base / file.name));
			}
		}

		const auto existing_files = utils::io::list_files(base / "data", true);
		for (auto& file : existing_files)
		{
			const auto is_file = std::filesystem::is_regular_file(file);
			const auto is_folder = std::filesystem::is_directory(file);

			if (is_file || is_folder)
			{
				bool is_legal = false;

				for (const auto& legal_file : legal_files)
				{
					if ((is_folder && is_inside_folder(legal_file, file)) ||
						(is_file && legal_file == file))
					{
						is_legal = true;
						break;
					}
				}

				if (is_legal)
				{
					continue;
				}
			}

			std::error_code code{};
			std::filesystem::remove_all(file, code);
		}
	}
}
