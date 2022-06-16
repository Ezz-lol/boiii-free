#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "splash.hpp"

#include <version.hpp>

#include <utils/io.hpp>
#include <utils/http.hpp>
#include <utils/compression.hpp>
#include <utils/progress_ui.hpp>

#define VERSION_URL "https://nightly.link/momo5502/boiii/workflows/build/" GIT_BRANCH "/Version.zip"
#define BINARY_URL "https://nightly.link/momo5502/boiii/workflows/build/" GIT_BRANCH "/Release%20Binary.zip"

namespace updater
{
	namespace
	{
		std::string get_version_zip()
		{
			const auto version_zip = utils::http::get_data(VERSION_URL);
			if (!version_zip || version_zip->empty())
			{
				throw std::runtime_error("Invalid version data");
			}

			return *version_zip;
		}

		std::string get_version()
		{
			const auto zip = get_version_zip();
			auto res = utils::compression::zip::extract(zip);
			return res["version.txt"];
		}

		bool requires_update()
		{
			return get_version() != GIT_HASH;
		}

		std::string get_self_file()
		{
			const auto self = utils::nt::library::get_by_address(get_self_file);
			return self.get_path();
		}

		std::string get_leftover_file()
		{
			return get_self_file() + ".old";
		}

		std::string download_update()
		{
			const auto data = utils::http::get_data(BINARY_URL);

			if (!data)
			{
				throw std::runtime_error("Invalid binary");
			}

			return *data;
		}

		void activate_update()
		{
			utils::nt::relaunch_self();
			TerminateProcess(GetCurrentProcess(), 0);
		}

		std::string get_binary(const std::string& data)
		{
			auto res = utils::compression::zip::extract(data);
			if (res.size() == 1)
			{
				for (auto& file : res)
				{
					return std::move(file.second);
				}
			}

			throw std::runtime_error("Invalid data");
		}

		void cleanup_update()
		{
			const auto leftover_file = get_leftover_file();
			for (size_t i = 0; i < 3; ++i)
			{
				if (utils::io::remove_file(leftover_file))
				{
					break;
				}

				std::this_thread::sleep_for(1s);
			}
		}

		void perform_update()
		{
			utils::progress_ui progress_ui{};
			progress_ui.set_title("Updating BOIII");
			progress_ui.set_line(1, "Downloading update...");
			progress_ui.show(true);

			const auto update_data = download_update();

			if (progress_ui.is_cancelled())
			{
				return;
			}

			// Is it good to add artificial sleeps?
			// Makes the ui nice, for sure.
			std::this_thread::sleep_for(2s);

			progress_ui.set_line(1, "Installing update...");
			progress_ui.set_progress(1, 1);

			const auto self_file = get_self_file();
			const auto leftover_file = get_leftover_file();

			const auto binary = get_binary(update_data);

			cleanup_update();
			utils::io::move_file(self_file, leftover_file);
			utils::io::write_file(self_file, binary);

			std::this_thread::sleep_for(2s);
		}
	}

	class component final : public component_interface
	{
	public:
		component()
		{
			cleanup_update();

			this->update_thread_ = std::thread([this]
			{
				this->update();
			});
		}

		~component() override
		{
			if (this->update_thread_.joinable())
			{
				this->update_thread_.detach();
			}
		}

		void pre_destroy() override
		{
			join();
		}

		void post_unpack() override
		{
			join();
		}

		int priority() override
		{
			return 999;
		}

	private:
		std::thread update_thread_{};

		void join()
		{
			if (this->update_thread_.joinable())
			{
				this->update_thread_.join();
			}
		}

		void update()
		{
			try
			{
				if (requires_update())
				{
					splash::hide();
					perform_update();
					activate_update();
				}
			}
			catch (...)
			{
			}
		}
	};
}

#if !defined(DEBUG) && defined(CI)
REGISTER_COMPONENT(updater::component)
#endif
