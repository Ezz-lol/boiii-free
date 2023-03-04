#include <std_include.hpp>

#include "game.hpp"

#include <utils/finally.hpp>

namespace game
{
	namespace
	{
		const utils::nt::library& get_host_library()
		{
			static const auto host_library = []
			{
				utils::nt::library host{};
				if (!host || host == utils::nt::library::get_by_address(get_base))
				{
					throw std::runtime_error("Invalid host application");
				}

				return host;
			}();

			return host_library;
		}
	}

	size_t get_base()
	{
		static const auto base = reinterpret_cast<size_t>(get_host_library().get_ptr());
		return base;
	}

	bool is_server()
	{
		static const auto server = get_host_library().get_optional_header()->CheckSum == 0x14C28B4;
		return server;
	}

	bool is_client()
	{
		static const auto server = get_host_library().get_optional_header()->CheckSum == 0x888C368;
		return server;
	}

	bool is_legacy_client()
	{
		static const auto server = get_host_library().get_optional_header()->CheckSum == 0x8880704;
		return server;
	}

	std::filesystem::path get_appdata_path()
	{
		static const auto appdata_path = []
		{
			PWSTR path;
			if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path)))
			{
				throw std::runtime_error("Failed to read APPDATA path!");
			}

			auto _ = utils::finally([&path]
			{
				CoTaskMemFree(path);
			});

			static auto appdata = std::filesystem::path(path) / "boiii";
			return appdata;
		}();

		return appdata_path;
	}
}
