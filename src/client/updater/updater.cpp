#include <std_include.hpp>

#include "updater.hpp"
#include "updater_ui.hpp"
#include "file_updater.hpp"

namespace updater
{
	void run(const std::filesystem::path& base)
	{
		const auto self = utils::nt::library::get_by_address(run);
		const auto self_file = self.get_path();

		updater_ui updater_ui{};
		const file_updater file_updater{updater_ui, base, self_file};

		file_updater.run();
	}
}
