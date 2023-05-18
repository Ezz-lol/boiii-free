#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <mmeapi.h>

namespace client_patches
{
	namespace
	{
		utils::hook::detour preload_map_hook;

		const game::dvar_t* cl_yaw_speed;
		const game::dvar_t* cl_pitch_speed;

		void stop_intro_if_needed()
		{
			if (game::Com_SessionMode_GetMode() != game::MODE_ZOMBIES &&
			    game::Com_SessionMode_GetMode() != game::MODE_CAMPAIGN)
			{
				return;
			}

			scheduler::once([]
			{
				scheduler::schedule([]
				{
					if (!game::Sys_IsDatabaseReady())
					{
						return scheduler::cond_continue;
					}

					game::Cinematic_StopPlayback(0, true);
					return scheduler::cond_end;
				}, scheduler::main);
			}, scheduler::main, 15s);
		}

		void preload_map_stub(int local_client_num, const char* mapname, const char* gametype)
		{
			game::Com_GametypeSettings_SetGametype(gametype, true);
			stop_intro_if_needed();
			preload_map_hook.invoke(local_client_num, mapname, gametype);
		}

		void reduce_process_affinity()
		{
			const DWORD_PTR affinity = (1ULL << (std::min(std::thread::hardware_concurrency(), 4U))) - 1;
			SetProcessAffinityMask(GetCurrentProcess(), affinity);
		}

		void reset_process_affinity()
		{
			DWORD_PTR affinity_proc, affinity_sys;
			GetProcessAffinityMask(GetCurrentProcess(), &affinity_proc, &affinity_sys);
			SetProcessAffinityMask(GetCurrentProcess(), affinity_sys);
		}

		void fix_amd_cpu_stuttering()
		{
			scheduler::once([]
			{
				reduce_process_affinity();
				scheduler::once(reset_process_affinity, scheduler::pipeline::main, 1s);
			}, scheduler::pipeline::main);
		}

		MMRESULT mixer_open_stub()
		{
			return MMSYSERR_NODRIVER;
		}

		bool is_mod_loaded_stub()
		{
			return false;
		}

		void patch_is_mod_loaded_checks()
		{
			const std::vector<uintptr_t> is_mod_loaded_addresses =
			{
				{ 0x1420F7484_g },
				{ 0x1420F74A4_g },
				{ 0x1420F73E4_g },
				{ 0x1420F73B4_g },
				{ 0x1420F6E57_g },
				{ 0x1413E6A54_g },
				{ 0x1415E7EBB_g },
				{ 0x1415E87BB_g },
				{ 0x1415EBAC9_g },
				{ 0x1415F1F09_g },
				{ 0x1415F1FB9_g },
				{ 0x1415F2080_g },
				{ 0x1415F7F40_g },
				{ 0x141A8D0ED_g },
				{ 0x141AA70F9_g },
				{ 0x141EA06FB_g },
				{ 0x141EA8C7E_g },
				{ 0x141EB1A39_g },
				{ 0x141ECBA9D_g },
				{ 0x1420F6E1D_g },
			};

			for (const auto& address : is_mod_loaded_addresses)
			{
				utils::hook::call(address, is_mod_loaded_stub);
			}
		}

		float cl_key_state_yaw_speed_stub(void* key)
		{
			return game::CL_KeyState(key) * cl_yaw_speed->current.value.value;
		}

		float cl_key_state_pitch_speed_stub(void* key)
		{
			return game::CL_KeyState(key) * cl_pitch_speed->current.value.value;
		}

		game::fileHandle_t fs_f_open_file_write_to_dir_stub(const char* filename, [[maybe_unused]] const char* dir,
		                                                    const char* os_base_path)
		{
			return game::FS_FOpenFileWriteToDir(filename, "boiii_players", os_base_path);
		}

		game::fileHandle_t fs_f_open_file_read_from_dir_stub(const char* filename, [[maybe_unused]] const char* dir,
		                                                     const char* os_base_path)
		{
			return game::FS_FOpenFileReadFromDir(filename, "boiii_players", os_base_path);
		}

		int i_stricmp_stub(const char* s0, [[maybe_unused]] const char* s1)
		{
			return game::I_stricmp(s0, "boiii_players");
		}

		void fs_add_game_directory_stub(const char* path, [[maybe_unused]] const char* dir)
		{
			utils::hook::invoke<void>(0x1422A2AF0_g, path, "boiii_players");
		}

		// TODO: Remove me after some time
		extern "C" void migrate_if_needed()
		{
			std::error_code e;

			// Folder does not exist. Nothing to migrate
			if (!std::filesystem::is_directory("players", e))
			{
				return;
			}

			// Folder does exist. Already migrated
			if (std::filesystem::is_directory("boiii_players", e))
			{
				return;
			}

			utils::io::create_directory("boiii_players");

			std::filesystem::copy("players", "boiii_players", std::filesystem::copy_options::recursive, e);
		}

		void patch_players_folder_name()
		{
			// Override 'players' folder
			utils::hook::call(0x14134764F_g, fs_f_open_file_write_to_dir_stub); // ??
			utils::hook::set<uint8_t>(0x14134762E_g, 0xEB); // ^^

			utils::hook::call(0x1413477EE_g, fs_f_open_file_write_to_dir_stub); // ??
			utils::hook::set<uint8_t>(0x1413477CD_g, 0xEB); // ^^

			utils::hook::call(0x141C20A1F_g, fs_f_open_file_write_to_dir_stub); // ??
			utils::hook::set<uint8_t>(0x141C209FE_g, 0xEB); // ^^

			utils::hook::call(0x1422F391E_g, fs_f_open_file_write_to_dir_stub); // ??

			utils::hook::call(0x141C2090F_g, fs_f_open_file_read_from_dir_stub); // ??
			utils::hook::set<uint8_t>(0x141C208EE_g, 0xEB); // ^^

			utils::hook::call(0x1422F3773_g, fs_f_open_file_read_from_dir_stub); // ??

			utils::hook::call(0x1422A2A61_g, i_stricmp_stub); // ??
			utils::hook::call(0x1422A2C82_g, i_stricmp_stub); // FS_AddGameDirectory

			utils::hook::call(0x1422A45A4_g, fs_add_game_directory_stub); // FS_Startup
		}
	}

	class component final : public client_component
	{
	public:
		static_assert(offsetof(game::clientActive_t, viewangles) == 0xB8C8);

		component()
		{
			migrate_if_needed(); // TODO: Remove me after some time
		}

		void post_unpack() override
		{
			fix_amd_cpu_stuttering();

			// Don't modify process priority
			utils::hook::nop(0x142334C98_g, 6);

			// Kill microphones for now
			utils::hook::set(0x15AAE9254_g, mixer_open_stub);

			preload_map_hook.create(0x14135A1E0_g, preload_map_stub);

			// Keep client ranked when mod loaded
			utils::hook::jump(0x1420D5BA0_g, is_mod_loaded_stub);
			patch_is_mod_loaded_checks();

			// Kill Client/Server Index Mismatch error
			utils::hook::set<uint8_t>(0x1400A7588_g, 0xEB);

			// Always get loadscreen gametype from s_gametype
			utils::hook::set<uint8_t>(0x14228F5DC_g, 0xEB);

			cl_yaw_speed = game::register_dvar_float("cl_yawspeed", 140.0f, std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
			                                         game::DVAR_NONE, "Max yaw speed in degrees for game pad and keyboard");
			cl_pitch_speed = game::register_dvar_float("cl_pitchspeed", 140.0f, std::numeric_limits<float>::min(), std::numeric_limits<float>::max(),
			                                           game::DVAR_NONE, "Max pitch speed in degrees for game pad");
			// CL_AdjustAngles
			utils::hook::call(0x1412F3324_g, cl_key_state_yaw_speed_stub); // cl_yawspeed
			utils::hook::call(0x1412F3344_g, cl_key_state_yaw_speed_stub); // ^^

			utils::hook::call(0x1412F3380_g, cl_key_state_pitch_speed_stub); // cl_pitchspeed
			utils::hook::call(0x1412F33A1_g, cl_key_state_pitch_speed_stub); // ^^

			patch_players_folder_name();
		}
	};
}

REGISTER_COMPONENT(client_patches::component)
