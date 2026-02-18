#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "workshop.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"
#include "command.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include <utils/thread.hpp>
#include "steamcmd.hpp"
#include "fastdl.hpp"
#include "party.hpp"
#include "scheduler.hpp"

#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <shellapi.h>


namespace workshop
{
	namespace
	{
		std::thread download_thread{};
		std::atomic_bool downloading{false};

		utils::hook::detour setup_server_map_hook;
		utils::hook::detour load_usermap_hook;

		static const std::unordered_map<std::string, std::string> dlc_links = {
			{"zm_zod", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_castle", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_island", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_stalingrad", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_genesis", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_cosmodrome", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_theater", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_moon", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_prototype", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_tomb", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_temple", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_sumpf", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_factory", "https://forum.ezz.lol/topic/6/bo3-dlc"},
			{"zm_asylum", "https://forum.ezz.lol/topic/6/bo3-dlc"}
		};
		std::mutex dlc_mutex;
		std::condition_variable dlc_cv;
		std::string pending_dlc_map;
		std::atomic<bool> dlc_thread_shutdown{false};
		std::thread dlc_popup_thread_obj;

		void dlc_popup_thread_func()
		{
			while (true)
			{
				std::unique_lock lock(dlc_mutex);
				dlc_cv.wait_for(lock, std::chrono::milliseconds(200), []
				{
					return dlc_thread_shutdown.load() || !pending_dlc_map.empty();
				});
				if (dlc_thread_shutdown.load())
					break;
				if (pending_dlc_map.empty())
					continue;
				std::string map = std::move(pending_dlc_map);
				pending_dlc_map.clear();
				lock.unlock();

				const auto it = dlc_links.find(map);
				if (it != dlc_links.end())
				{
					const auto* msg = utils::string::va(
						"Missing DLC map: %s\n\nYou can download it from:\n%s\n\nWould you like to open the download page?",
						map.c_str(), it->second.c_str());
					const int result = MessageBoxA(nullptr, msg, "DLC Required",
						MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL);
					if (result == IDYES)
						ShellExecuteA(nullptr, "open", it->second.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
				}
			}
		}

		void queue_dlc_popup(const std::string& mapname)
		{
			std::lock_guard lock(dlc_mutex);
			pending_dlc_map = mapname;
			dlc_cv.notify_one();
		}

		bool has_mod(const std::string& pub_id)
		{
			for (unsigned int i = 0; i < *game::modsCount; ++i)
			{
				const auto& mod_data = game::modsPool[i];
				if (mod_data.publisherId == pub_id || mod_data.folderName == pub_id)
				{
					return true;
				}
			}

			return false;
		}

		std::string resolve_mod_workshop_id(const std::string& mod_name)
		{
			for (unsigned int i = 0; i < *game::modsCount; ++i)
			{
				const auto& mod_data = game::modsPool[i];
				if (mod_data.folderName == mod_name &&
					utils::string::is_numeric(mod_data.publisherId))
				{
					return mod_data.publisherId;
				}
			}

			std::error_code ec;
			std::filesystem::path mods_dir("mods");
			if (std::filesystem::exists(mods_dir, ec))
			{
				for (const auto& entry : std::filesystem::directory_iterator(mods_dir, ec))
				{
					if (!entry.is_directory(ec)) continue;

					auto ws_json = entry.path() / "zone" / "workshop.json";
					if (!std::filesystem::exists(ws_json, ec)) continue;

					const auto json_str = utils::io::read_file(ws_json.string());
					if (json_str.empty()) continue;

					rapidjson::Document doc;
					if (doc.Parse(json_str.c_str()).HasParseError() || !doc.IsObject()) continue;

					auto folder_it = doc.FindMember("FolderName");
					if (folder_it != doc.MemberEnd() && folder_it->value.IsString())
					{
						if (std::string(folder_it->value.GetString()) == mod_name)
						{
							auto pub_it = doc.FindMember("PublishedFileId");
							if (pub_it != doc.MemberEnd() && pub_it->value.IsString())
							{
								std::string pfid = pub_it->value.GetString();
								if (utils::string::is_numeric(pfid.data()))
									return pfid;
							}
							auto pubid_it = doc.FindMember("PublisherID");
							if (pubid_it != doc.MemberEnd() && pubid_it->value.IsString())
							{
								std::string pid = pubid_it->value.GetString();
								if (utils::string::is_numeric(pid.data()))
									return pid;
							}
						}
					}
				}
			}

			return {};
		}

		void load_usermap_mod_if_needed()
		{
			if (!game::isModLoaded())
			{
				game::loadMod(0, "usermaps", false);
			}
		}

		void setup_server_map_stub(int localClientNum, const char* map, const char* gametype)
		{
			if (utils::string::is_numeric(map) ||
				!get_usermap_publisher_id(map).empty())
			{
				load_usermap_mod_if_needed();
			}

			setup_server_map_hook.invoke(localClientNum, map, gametype);
		}

		void load_workshop_data(game::workshop_data& item)
		{
			const auto base_path = item.absolutePathZoneFiles;
			const auto path = utils::string::va("%s/workshop.json", base_path);
			const auto json_str = utils::io::read_file(path);

			if (json_str.empty())
			{
				printf("[ Workshop ] workshop.json has not been found in folder:\n%s\n", path);
				return;
			}

			rapidjson::Document doc;
			const rapidjson::ParseResult parse_result = doc.Parse(json_str);

			if (parse_result.IsError() || !doc.IsObject())
			{
				printf("[ Workshop ] Unable to parse workshop.json from folder:\n%s\n", path);
				return;
			}

			if (!doc.HasMember("Title") ||
				!doc.HasMember("Description") ||
				!doc.HasMember("FolderName") ||
				!doc.HasMember("PublisherID"))
			{
				printf("[ Workshop ] workshop.json is invalid:\n%s\n", path);
				return;
			}

			utils::string::copy(item.title, doc["Title"].GetString());
			utils::string::copy(item.description, doc["Description"].GetString());
			utils::string::copy(item.folderName, doc["FolderName"].GetString());
			utils::string::copy(item.publisherId, doc["PublisherID"].GetString());
			item.publisherIdInteger = std::strtoul(item.publisherId, nullptr, 10);
		}

		void load_usermap_content_stub(void* usermaps_count, int type)
		{
			utils::hook::invoke<void>(game::select(0x1420D6430, 0x1404E2360), usermaps_count, type);

			for (unsigned int i = 0; i < *game::usermapsCount; ++i)
			{
				auto& usermap_data = game::usermapsPool[i];

				if (std::strcmp(usermap_data.folderName, usermap_data.title) != 0)
				{
					continue;
				}

				load_workshop_data(usermap_data);
			}
		}

		void load_mod_content_stub(void* mods_count, int type)
		{
			utils::hook::invoke<void>(game::select(0x1420D6430, 0x1404E2360), mods_count, type);

			for (unsigned int i = 0; i < *game::modsCount; ++i)
			{
				auto& mod_data = game::modsPool[i];

				if (std::strcmp(mod_data.folderName, mod_data.title) != 0)
				{
					continue;
				}

				load_workshop_data(mod_data);
			}
		}

		game::workshop_data* load_usermap_stub(const char* map_arg)
		{
			std::string pub_id = map_arg;
			if (!utils::string::is_numeric(map_arg))
			{
				pub_id = get_usermap_publisher_id(map_arg);
			}

			return load_usermap_hook.invoke<game::workshop_data*>(pub_id.data());
		}

		bool has_workshop_item_stub(int type, const char* map, int a3)
		{
			std::string pub_id = map;
			if (!utils::string::is_numeric(map))
			{
				pub_id = get_usermap_publisher_id(map);
			}

			return utils::hook::invoke<bool>(0x1420D6380_g, type, pub_id.data(), a3);
		}

		const char* va_mods_path_stub(const char* fmt, const char* root_dir, const char* mods_dir, const char* dir_name)
		{
			const auto original_path = utils::string::va(fmt, root_dir, mods_dir, dir_name);

			if (utils::io::directory_exists(original_path))
			{
				return original_path;
			}

			return utils::string::va("%s/%s/%s", root_dir, mods_dir, dir_name);
		}

		const char* va_user_content_path_stub(const char* fmt, const char* root_dir, const char* user_content_dir)
		{
			const auto original_path = utils::string::va(fmt, root_dir, user_content_dir);

			if (utils::io::directory_exists(original_path))
			{
				return original_path;
			}

			return utils::string::va("%s/%s", root_dir, user_content_dir);
		}
	}

	std::string get_mod_resized_name()
	{
		const std::string loaded_mod_id = game::getPublisherIdFromLoadedMod();

		if (loaded_mod_id == "usermaps" || loaded_mod_id.empty())
		{
			return loaded_mod_id;
		}

		std::string mod_name = loaded_mod_id;

		for (unsigned int i = 0; i < *game::modsCount; ++i)
		{
			const auto& mod_data = game::modsPool[i];

			if (mod_data.publisherId == loaded_mod_id)
			{
				mod_name = mod_data.title;
				break;
			}
		}

		if (mod_name.size() > 31)
		{
			mod_name.resize(31);
		}

		return mod_name;
	}

	std::string get_usermap_publisher_id(const std::string& zone_name)
	{
		for (unsigned int i = 0; i < *game::usermapsCount; ++i)
		{
			const auto& usermap_data = game::usermapsPool[i];
			if (usermap_data.folderName == zone_name)
			{
				if (!utils::string::is_numeric(usermap_data.publisherId))
				{
					printf(
						"[ Workshop ] WARNING: The publisherId is not numerical you might have set your usermap folder incorrectly!\n%s\n",
						usermap_data.absolutePathZoneFiles);
				}

				return usermap_data.publisherId;
			}
		}

		return {};
	}

	int get_workshop_retry_attempts()
	{
		const int val = game::get_dvar_int("workshop_retry_attempts");
		if (val < 1) return 1;
		if (val > 1000) return 1000;
		return val;
	}

	std::string get_mod_publisher_id()
	{
		const std::string loaded_mod_id = game::getPublisherIdFromLoadedMod();

		if (loaded_mod_id == "usermaps" || loaded_mod_id.empty())
		{
			return loaded_mod_id;
		}

		if (!utils::string::is_numeric(loaded_mod_id))
		{
			printf(
				"[ Workshop ] WARNING: The publisherId: %s, is not numerical you might have set your mod folder incorrectly!\n",
				loaded_mod_id.data());
		}

		return loaded_mod_id;
	}

	bool is_dlc_map(const std::string& mapname)
	{
		return mapname == "zm_zod" ||
			mapname == "zm_castle" ||
			mapname == "zm_island" ||
			mapname == "zm_stalingrad" ||
			mapname == "zm_genesis" ||
			mapname == "zm_cosmodrome" ||
			mapname == "zm_theater" ||
			mapname == "zm_moon" ||
			mapname == "zm_prototype" ||
			mapname == "zm_tomb" ||
			mapname == "zm_temple" ||
			mapname == "zm_sumpf" ||
			mapname == "zm_factory" ||
			mapname == "zm_asylum";
	}

	extern bool downloading_workshop_item = false;

	bool check_valid_usermap_id(const std::string& mapname, const std::string& pub_id, const std::string& workshop_id, const std::string& base_url)
	{
		if (!game::DB_FileExists(mapname.data(), 0) && pub_id.empty())
		{
			if (is_dlc_map(mapname.data()))
			{
				queue_dlc_popup(mapname);
				return false;
			}

			if (downloading_workshop_item || fastdl::is_downloading())
			{
				game::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
					"You are already downloading a map in the background. You can download only one item at a time.");
				return false;
			}

			if (!base_url.empty())
			{
				fastdl::download_context context{};
				context.mapname = mapname;
				context.pub_id = workshop_id.empty() ? mapname : workshop_id;
				context.map_path = "./usermaps/" + mapname;
				context.base_url = base_url;
				context.success_callback = []()
				{
					scheduler::once([]
					{
						game::reloadUserContent();
					}, scheduler::main);
				};
				printf("[ Workshop ] Server has FastDL, attempting download for %s from %s\n", mapname.data(), base_url.data());
				fastdl::start_map_download(context);
				return false;
			}

			if (utils::string::is_numeric(mapname.data()))
			{
				const int result = MessageBoxA(nullptr,
					utils::string::va("Usermap '%s' not found.\n\nDo you want to download it from the Workshop now?", mapname.data()),
					"Missing Usermap", MB_OKCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
				if (result == IDOK)
				{
					download_thread = utils::thread::create_named_thread(
						"workshop_download", steamcmd::initialize_download, mapname, "Map");
					download_thread.detach();
				}
			}
			else if (!workshop_id.empty() && utils::string::is_numeric(workshop_id.data()))
			{
				const int result = MessageBoxA(nullptr,
					utils::string::va("Usermap '%s' not found.\n\nDo you want to download it from the Workshop now?", mapname.data()),
					"Missing Usermap", MB_OKCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
				if (result == IDOK)
				{
					download_thread = utils::thread::create_named_thread(
						"workshop_download", steamcmd::initialize_download, workshop_id, "Map");
					download_thread.detach();
				}
			}
			else
			{
				game::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
					utils::string::va(
						"Missing usermap: %s\n\nThis server did not provide FastDL (sv_wwwBaseURL/sv_wwwBaseUrl) and did not set workshop_id.\n\nSubscribe to the map on Steam Workshop, or ask the server to set sv_wwwBaseURL for FastDL or workshop_id in their config.",
						mapname.data()));
			}
			return false;
		}
		return true;
	}

	bool check_valid_mod_id(const std::string& mod, const std::string& workshop_id)
	{
		if (mod.empty() || mod == "usermaps")
		{
			return true;
		}

		if (!has_mod(mod))
		{
			if (downloading_workshop_item)
			{
				game::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
					"You are already downloading a mod in the background. You can download only one item at a time.");
				return false;
			}

			if (utils::string::is_numeric(mod.data()))
			{
				const int result = MessageBoxA(nullptr,
					utils::string::va("Mod '%s' not found.\n\nDo you want to download it from the Workshop now?", mod.data()),
					"Missing Mod", MB_OKCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
				if (result == IDOK)
				{
					download_thread = utils::thread::create_named_thread(
						"workshop_download", steamcmd::initialize_download, mod, "Mod");
					download_thread.detach();
				}
			}
			else if (!workshop_id.empty() && utils::string::is_numeric(workshop_id.data()))
			{
				const int result = MessageBoxA(nullptr,
					utils::string::va("Mod '%s' not found.\n\nDo you want to download it from the Workshop now?", mod.data()),
					"Missing Mod", MB_OKCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
				if (result == IDOK)
				{
					download_thread = utils::thread::create_named_thread(
						"workshop_download", steamcmd::initialize_download, workshop_id, "Mod");
					download_thread.detach();
				}
			}
			else
			{
				std::string resolved_id = resolve_mod_workshop_id(mod);
				if (!resolved_id.empty())
				{
					const int result = MessageBoxA(nullptr,
						utils::string::va("Mod '%s' not found.\n\nResolved workshop ID: %s\nDo you want to download it from the Workshop now?", mod.data(), resolved_id.data()),
						"Missing Mod", MB_OKCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL);
					if (result == IDOK)
					{
						download_thread = utils::thread::create_named_thread(
							"workshop_download", steamcmd::initialize_download, resolved_id, "Mod");
						download_thread.detach();
					}
				}
				else
				{
					game::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
						utils::string::va(
							"Could not download: folder name is not numeric and 'workshop_id' dvar is empty.\nMod: %s\nSet workshop_id or subscribe on Steam Workshop.",
							mod.data()));
				}
			}
			return false;
		}

		return true;
	}

	void setup_same_mod_as_host(const std::string& usermap, const std::string& mod)
	{
		if (game::getPublisherIdFromLoadedMod() == mod)
		{
			return;
		}

		if (!usermap.empty() || mod != "usermaps")
		{
			game::loadMod(0, mod.data(), true);
			return;
		}

		if (game::isModLoaded())
		{
			game::loadMod(0, "", true);
		}
	}
	void com_error_missing_map_stub(const char* file, int line, int code, const char* fmt, ...)
	{
		game::Com_Error_(file, line, code, "%s", "Missing map! Trying to reconnect to server...");
	}

	class component final : public generic_component
	{
	public:
		void post_unpack() override
		{
			[[maybe_unused]] const auto* dvar_retry = game::register_dvar_int("workshop_retry_attempts", 15, 1, 1000, game::DVAR_ARCHIVE,
				"Number of connection retry attempts for workshop downloads (default 15, increase for slow connections)");
			[[maybe_unused]] const auto* dvar_timeout = game::register_dvar_int("workshop_timeout", 300, 60, 3600, game::DVAR_ARCHIVE,
				"Download timeout in seconds for workshop items (reserved for future use)");

			dlc_popup_thread_obj = std::thread(dlc_popup_thread_func);

			command::add("userContentReload", [](const command::params& params)
			{
				game::reloadUserContent();
			});
			command::add("workshop_config", [](const command::params& params)
			{
				printf("[ Workshop ] workshop_retry_attempts: %d (set in game or config)\n",
					get_workshop_retry_attempts());
				printf("[ Workshop ] workshop_timeout: %d\n", game::get_dvar_int("workshop_timeout"));
			});
			command::add("workshop_download", [](const command::params& params)
			{
				if (params.size() < 2)
				{
					printf("[ Workshop ] Usage: workshop_download <id> [Map|Mod]\n");
					return;
				}
				const std::string id = params.get(1);
				std::string type_str = params.size() >= 3 ? params.get(2) : "Map";
				if (id.empty())
					return;
				if (downloading_workshop_item)
				{
					game::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI,
						"A workshop download is already in progress. Wait for it to finish.");
					return;
				}
				if (type_str != "Map" && type_str != "Mod")
					type_str = "Map";
				printf("[ Workshop ] Starting download: %s (%s)\n", id.c_str(), type_str.c_str());
				download_thread = utils::thread::create_named_thread(
					"workshop_download", steamcmd::initialize_download, id, type_str);
				download_thread.detach();
			});

			utils::hook::call(game::select(0x1420D6AA6, 0x1404E2936), va_mods_path_stub);
			utils::hook::call(game::select(0x1420D6577, 0x1404E24A7), va_user_content_path_stub);

			load_usermap_hook.create(game::select(0x1420D5700, 0x1404E18B0), load_usermap_stub);
			utils::hook::call(game::select(0x1420D67F5, 0x1404E25F2), load_usermap_content_stub);

			if (game::is_server())
			{
				utils::hook::jump(0x1404E2635_g, load_mod_content_stub);
				return;
			}

			utils::hook::call(0x1420D6745_g, load_mod_content_stub);
			utils::hook::call(0x14135CD84_g, has_workshop_item_stub);
			setup_server_map_hook.create(0x14135CD20_g, setup_server_map_stub);

			if (game::is_client())
			{
				utils::hook::call(0x14135CDA1_g, com_error_missing_map_stub);
			}
		}


		void pre_destroy() override
		{
			downloading_workshop_item = false;
			dlc_thread_shutdown = true;
			dlc_cv.notify_one();
			if (dlc_popup_thread_obj.joinable())
				dlc_popup_thread_obj.join();
			if (download_thread.joinable())
				download_thread.join();
		}
	};
}

REGISTER_COMPONENT(workshop::component)
