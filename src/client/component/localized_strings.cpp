#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "localized_strings.hpp"
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/concurrency.hpp>
#include "game/game.hpp"

namespace localized_strings
{
	namespace
	{
		utils::hook::detour seh_string_ed_get_string_hook;

		using localized_map = std::unordered_map<std::string, std::string>;
		utils::concurrency::container<localized_map> localized_overrides;

		const char* seh_string_ed_get_string(const char* reference)
		{
			return localized_overrides.access<const char*>([&](const localized_map& map)
			{
				const auto entry = map.find(reference);
				if (entry != map.end())
				{
					return utils::string::va("%s", entry->second.data());
				}

				return seh_string_ed_get_string_hook.invoke<const char*>(reference);
			});
		}
	}

	void override(const std::string& key, const std::string& value)
	{
		localized_overrides.access([&](localized_map& map)
		{
			map[key] = value;
		});
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			// Change some localized strings
			seh_string_ed_get_string_hook.create(0x1422796E0_g, &seh_string_ed_get_string);
		}
	};
}

REGISTER_COMPONENT(localized_strings::component)
