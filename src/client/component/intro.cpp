#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include <utils/hook.hpp>

namespace intro
{
	namespace
	{
		utils::hook::detour cinematic_start_playback_hook;

		void ccc(const char* name, const char* key, const unsigned int playback_flags, const float volume,
		         void* callback_info, const int id)
		{
			if (name == "BO3_Global_Logo_LogoSequence"s)
			{
				return;
			}

			cinematic_start_playback_hook.invoke(name, key, playback_flags, volume, callback_info, id);
		}
	}

	class component final : public client_component
	{
	public:
		void post_unpack() override
		{
			cinematic_start_playback_hook.create(game::Cinematic_StartPlayback, ccc);
		}
	};
}

#ifdef DEV_BUILD
REGISTER_COMPONENT(intro::component)
#endif
