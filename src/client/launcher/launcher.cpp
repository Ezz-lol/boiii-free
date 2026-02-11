#include <std_include.hpp>
#include <utils/nt.hpp>
#include <utils/properties.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/http.hpp>

#include "launcher.hpp"
#include "launcher_workshop.hpp"
#include "html/html_window.hpp"
#include "../component/steamcmd.hpp"

#include <game/game.hpp>
#include <version.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <thread>
#include <atomic>
#include <mutex>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <TlHelp32.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

namespace launcher
{
	namespace
	{
		std::string human_readable_size(std::uint64_t bytes);
		std::filesystem::path get_steam_workshop_path();

		bool is_game_process_running()
		{
			const auto self_pid = GetCurrentProcessId();
			const HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (snap == INVALID_HANDLE_VALUE) return false;

			PROCESSENTRY32W pe{};
			pe.dwSize = sizeof(pe);

			if (Process32FirstW(snap, &pe))
			{
				do
				{
					if (pe.th32ProcessID == self_pid) continue;

					std::wstring name(pe.szExeFile);
					if (_wcsicmp(name.c_str(), L"BlackOps3.exe") == 0 ||
						_wcsicmp(name.c_str(), L"boiii.exe") == 0)
					{
						CloseHandle(snap);
						return true;
					}
				} while (Process32NextW(snap, &pe));
			}

			CloseHandle(snap);
			return false;
		}

		void workshop_remove_one(const std::string& folder_name)
		{
			std::string name = folder_name;
			utils::string::trim(name);
			if (name.empty()) return;
			name = std::filesystem::path(name).filename().string();
			if (name.empty()) return;

			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			std::error_code ec;
			std::filesystem::path mods_dir = base / "mods" / name;
			if (std::filesystem::exists(mods_dir))
			{
				std::filesystem::remove_all(mods_dir, ec);
			}
			std::filesystem::path usermaps_dir = base / "usermaps" / name;
			if (std::filesystem::exists(usermaps_dir))
			{
				std::filesystem::remove_all(usermaps_dir, ec);
			}
		}

		void workshop_remove_by_path(const std::string& path_str)
		{
			std::string p = path_str;
			utils::string::trim(p);
			if (p.empty()) return;
			std::filesystem::path target(p);
			std::error_code ec;
			if (std::filesystem::exists(target) && std::filesystem::is_directory(target))
			{
				std::filesystem::remove_all(target, ec);
			}
		}

		void workshop_remove_all_folders()
		{
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			std::error_code ec;
			auto clear_subdirs = [&ec](const std::filesystem::path& dir) {
				if (!std::filesystem::exists(dir)) return;
				for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
					if (entry.is_directory())
						std::filesystem::remove_all(entry.path(), ec);
			};
			clear_subdirs(base / "mods");
			clear_subdirs(base / "usermaps");

			std::filesystem::path steam_ws = get_steam_workshop_path();
			if (!steam_ws.empty())
			{
				std::error_code ec2;
				if (std::filesystem::exists(steam_ws, ec2))
				{
					clear_subdirs(steam_ws);
				}
			}
		}

		static const std::vector<std::string> IMAGE_EXTENSIONS = { ".jpg", ".jpeg", ".png", ".webp", ".gif", ".bmp" };

		bool path_starts_with_ignore_case(const std::string& stem, const std::string& prefix)
		{
			if (prefix.size() > stem.size()) return false;
			for (size_t i = 0; i < prefix.size(); ++i)
			{
				if (std::tolower(static_cast<unsigned char>(stem[i])) != std::tolower(static_cast<unsigned char>(prefix[i])))
					return false;
			}
			return true;
		}

		bool has_image_extension(const std::filesystem::path& p)
		{
			std::string ext = p.extension().string();
			utils::string::to_lower(ext);
			for (const auto& e : IMAGE_EXTENSIONS)
				if (ext == e) return true;
			return false;
		}

		std::string find_mod_image_path(const std::filesystem::path& mod_root)
		{
			std::filesystem::path zone_dir = mod_root / "zone";
			std::vector<std::filesystem::path> dirs_to_scan = { mod_root };
			if (std::filesystem::exists(zone_dir)) dirs_to_scan.push_back(zone_dir);

			std::string found_preview, found_loading, found_any;
			for (const auto& dir : dirs_to_scan)
			{
				std::error_code ec;
				for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
				{
					if (!entry.is_regular_file(ec)) continue;
					std::string stem = entry.path().stem().string();
					std::string stem_lower = utils::string::to_lower(stem);
					if (!has_image_extension(entry.path())) continue;
					std::string abs_path = std::filesystem::absolute(entry.path(), ec).generic_string();
					if (path_starts_with_ignore_case(stem_lower, "previewimage"))
					{
						if (found_preview.empty()) found_preview = abs_path;
					}
					else if (path_starts_with_ignore_case(stem_lower, "loadingimage"))
					{
						if (found_loading.empty()) found_loading = abs_path;
					}
					else if (found_any.empty())
						found_any = abs_path;
				}
			}
			if (!found_preview.empty()) return found_preview;
			if (!found_loading.empty()) return found_loading;
			return found_any;
		}

		std::string path_to_file_url(const std::string& path)
		{
			if (path.empty()) return {};
			std::string s = path;
			for (auto& c : s) if (c == '\\') c = '/';
			if (s.size() >= 2 && s[1] == ':')
				return "file:///" + s;
			if (s[0] != '/') return "file:///" + s;
			return "file://" + s;
		}

		std::string get_mode_files_info()
		{
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			struct mode_info {
				const char* prefix;
				const char* key;
				std::uint64_t total_size = 0;
				std::uint32_t file_count = 0;
			};

			mode_info modes[] = {
				{"cp_", "campaign", 0, 0},
				{"mp_", "multiplayer", 0, 0},
				{"zm_", "zombies", 0, 0}
			};

			const char* dirs_to_scan[] = { "zone", "video" };

			for (const auto& dir_name : dirs_to_scan)
			{
				std::filesystem::path dir = base / dir_name;
				if (!std::filesystem::exists(dir)) continue;

				std::error_code ec;
				for (const auto& entry : std::filesystem::recursive_directory_iterator(dir, ec))
				{
					if (!entry.is_regular_file(ec)) continue;
					auto filename = utils::string::to_lower(entry.path().filename().string());

					for (auto& m : modes)
					{
						if (filename.find(m.prefix) != std::string::npos)
						{
							m.total_size += entry.file_size(ec);
							m.file_count++;
							break;
						}
					}
				}
			}

			rapidjson::StringBuffer buf;
			rapidjson::Writer<rapidjson::StringBuffer> w(buf);
			w.StartObject();
			for (const auto& m : modes)
			{
				w.Key(m.key);
				w.StartObject();
				w.Key("count"); w.Uint(m.file_count);
				w.Key("size"); w.Uint64(m.total_size);
				w.Key("sizeHuman"); w.String(human_readable_size(m.total_size).c_str());
				w.EndObject();
			}
			w.EndObject();
			return std::string(buf.GetString(), buf.GetSize());
		}

		std::string remove_mode_files(const std::string& prefixes_csv)
		{
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			auto prefixes = utils::string::split(prefixes_csv, ',');
			for (auto& p : prefixes) utils::string::trim(p);

			// Collect files first to avoid modifying directory while iterating
			std::vector<std::filesystem::path> to_delete;
			const char* dirs_to_scan[] = { "zone", "video" };

			for (const auto& dir_name : dirs_to_scan)
			{
				std::filesystem::path dir = base / dir_name;
				if (!std::filesystem::exists(dir)) continue;

				std::error_code ec;
				for (const auto& entry : std::filesystem::recursive_directory_iterator(dir, ec))
				{
					if (!entry.is_regular_file(ec)) continue;
					auto filename = utils::string::to_lower(entry.path().filename().string());

					for (const auto& prefix : prefixes)
					{
						if (!prefix.empty() && filename.find(prefix) != std::string::npos)
						{
							to_delete.push_back(entry.path());
							break;
						}
					}
				}
			}

			std::uint64_t removed_size = 0;
			std::uint32_t removed_count = 0;
			std::uint32_t failed_count = 0;

			for (const auto& file : to_delete)
			{
				std::error_code ec;
				auto fsize = std::filesystem::file_size(file, ec);
				if (std::filesystem::remove(file, ec))
				{
					removed_size += fsize;
					removed_count++;
				}
				else
				{
					failed_count++;
				}
			}

			rapidjson::StringBuffer buf;
			rapidjson::Writer<rapidjson::StringBuffer> w(buf);
			w.StartObject();
			w.Key("removedCount"); w.Uint(removed_count);
			w.Key("removedSize"); w.String(human_readable_size(removed_size).c_str());
			w.Key("failedCount"); w.Uint(failed_count);
			w.EndObject();
			return std::string(buf.GetString(), buf.GetSize());
		}

		constexpr const char* STEAM_WORKSHOP_API = "https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/";
		constexpr int BO3_APP_ID = 311210;

		std::string get_steam_workshop_preview_url(const std::string& workshop_id)
		{
			if (workshop_id.empty()) return {};
			try
			{
				std::string body = "itemcount=1&publishedfileids[0]=" + workshop_id;
				std::optional<std::string> resp = utils::http::post_data(STEAM_WORKSHOP_API, body, 3);
				if (!resp || resp->empty()) return {};

				rapidjson::Document doc;
				if (doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject()) return {};
				auto resp_it = doc.FindMember("response");
				if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject()) return {};
				auto details_it = resp_it->value.FindMember("publishedfiledetails");
				if (details_it == resp_it->value.MemberEnd() || !details_it->value.IsArray() || details_it->value.Empty()) return {};

				const rapidjson::Value& first = details_it->value[0];
				if (!first.IsObject()) return {};
				auto consumer_it = first.FindMember("consumer_app_id");
				if (consumer_it == first.MemberEnd()) return {};
				int app_id = 0;
				if (consumer_it->value.IsInt()) app_id = consumer_it->value.GetInt();
				else if (consumer_it->value.IsInt64()) app_id = static_cast<int>(consumer_it->value.GetInt64());
				else if (consumer_it->value.IsString()) app_id = std::atoi(consumer_it->value.GetString());
				if (app_id != BO3_APP_ID) return {};

				auto preview_it = first.FindMember("preview_url");
				if (preview_it == first.MemberEnd() || !preview_it->value.IsString()) return {};
				return std::string(preview_it->value.GetString(), preview_it->value.GetStringLength());
			}
			catch (...)
			{
				return {};
			}
		}

		std::string human_readable_size(std::uint64_t bytes)
		{
			const char* suffixes[] = { "B", "KB", "MB", "GB", "TB" };
			double value = static_cast<double>(bytes);
			int idx = 0;

			while (value >= 1024.0 && idx < 4)
			{
				value /= 1024.0;
				++idx;
			}

			char buf[64]{};
			std::snprintf(buf, sizeof(buf), "%.2f %s", value, suffixes[idx]);
			return buf;
		}

		std::filesystem::path get_steam_workshop_path()
		{
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			// Game is typically at: X:\Steam\steamapps\common\Call of Duty Black Ops III
			// Workshop content is at: X:\Steam\steamapps\workshop\content\311210
			auto steamapps = base.parent_path().parent_path(); // up from common/<game>
			auto workshop_path = steamapps / "workshop" / "content" / "311210";
			if (std::filesystem::exists(workshop_path))
			{
				return workshop_path;
			}
			return {};
		}

		std::string workshop_list_json()
		{
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			rapidjson::StringBuffer buf;
			rapidjson::Writer<rapidjson::StringBuffer> w(buf);
			w.StartArray();
			auto scan = [&](const std::filesystem::path& parent, const char* type_label) {
				if (!std::filesystem::exists(parent)) return;
				for (const auto& entry : std::filesystem::directory_iterator(parent))
				{
					if (!entry.is_directory()) continue;
					std::filesystem::path zone_json = entry.path() / "zone" / "workshop.json";
					if (!utils::io::file_exists(zone_json.string())) continue;
					std::string data;
					if (!utils::io::read_file(zone_json.string(), &data)) continue;
					rapidjson::Document doc;
					if (doc.Parse(data).HasParseError() || !doc.IsObject() || !doc.HasMember("Title") || !doc.HasMember("FolderName")) continue;
					const char* name = doc["Title"].GetString();
					std::string id_str;
					if (doc.HasMember("PublisherID"))
					{
						const auto& pid = doc["PublisherID"];
						if (pid.IsString()) id_str = pid.GetString();
						else if (pid.IsInt64()) id_str = std::to_string(pid.GetInt64());
						else if (pid.IsUint64()) id_str = std::to_string(pid.GetUint64());
					}
					std::string image_url;
					std::string image_path = find_mod_image_path(entry.path());
					if (!image_path.empty())
						image_url = path_to_file_url(image_path);
					else if (!id_str.empty())
						image_url = get_steam_workshop_preview_url(id_str);

					w.StartObject();
					w.Key("name"); w.String(name);
					w.Key("folder"); w.String(entry.path().filename().string());
					w.Key("type"); w.String(type_label);
					w.Key("id"); w.String(id_str.c_str());
					if (!image_url.empty()) { w.Key("image"); w.String(image_url.c_str()); }
					w.EndObject();
				}
			};
			scan(base / "usermaps", "map");
			scan(base / "mods", "mod");

			// Also scan Steam's native workshop download path
			std::filesystem::path steam_ws = get_steam_workshop_path();
			if (!steam_ws.empty())
			{
				std::error_code ws_ec;
				if (std::filesystem::exists(steam_ws, ws_ec))
				{
				std::error_code ec;
				for (const auto& ws_entry : std::filesystem::directory_iterator(steam_ws, ec))
				{
					if (!ws_entry.is_directory()) continue;
					// Each workshop ID folder may contain the mod directly or have subfolders
					// Check for zone/workshop.json directly
					std::filesystem::path zone_json = ws_entry.path() / "zone" / "workshop.json";
					if (utils::io::file_exists(zone_json.string()))
					{
						std::string data;
						if (!utils::io::read_file(zone_json.string(), &data)) continue;
						rapidjson::Document doc;
						if (doc.Parse(data).HasParseError() || !doc.IsObject() || !doc.HasMember("Title") || !doc.HasMember("FolderName")) continue;
						const char* name = doc["Title"].GetString();
						std::string id_str;
						if (doc.HasMember("PublisherID"))
						{
							const auto& pid = doc["PublisherID"];
							if (pid.IsString()) id_str = pid.GetString();
							else if (pid.IsInt64()) id_str = std::to_string(pid.GetInt64());
							else if (pid.IsUint64()) id_str = std::to_string(pid.GetUint64());
						}
						std::string image_url;
						std::string image_path = find_mod_image_path(ws_entry.path());
						if (!image_path.empty())
							image_url = path_to_file_url(image_path);
						else if (!id_str.empty())
							image_url = get_steam_workshop_preview_url(id_str);

						w.StartObject();
						w.Key("name"); w.String(name);
						w.Key("folder"); w.String(ws_entry.path().filename().string());
						w.Key("type"); w.String("map");
						w.Key("id"); w.String(id_str.c_str());
						w.Key("source"); w.String("steam");
						w.Key("path"); w.String(ws_entry.path().string().c_str());
						if (!image_url.empty()) { w.Key("image"); w.String(image_url.c_str()); }
						w.EndObject();
						continue;
					}
					// Also check one level deeper (subfolder inside the workshop ID folder)
					for (const auto& sub : std::filesystem::directory_iterator(ws_entry.path(), ec))
					{
						if (!sub.is_directory()) continue;
						zone_json = sub.path() / "zone" / "workshop.json";
						if (!utils::io::file_exists(zone_json.string())) continue;
						std::string data;
						if (!utils::io::read_file(zone_json.string(), &data)) continue;
						rapidjson::Document doc;
						if (doc.Parse(data).HasParseError() || !doc.IsObject() || !doc.HasMember("Title") || !doc.HasMember("FolderName")) continue;
						const char* name = doc["Title"].GetString();
						std::string id_str;
						if (doc.HasMember("PublisherID"))
						{
							const auto& pid = doc["PublisherID"];
							if (pid.IsString()) id_str = pid.GetString();
							else if (pid.IsInt64()) id_str = std::to_string(pid.GetInt64());
							else if (pid.IsUint64()) id_str = std::to_string(pid.GetUint64());
						}
						std::string image_url;
						std::string image_path = find_mod_image_path(sub.path());
						if (!image_path.empty())
							image_url = path_to_file_url(image_path);
						else if (!id_str.empty())
							image_url = get_steam_workshop_preview_url(id_str);

						w.StartObject();
						w.Key("name"); w.String(name);
						w.Key("folder"); w.String(sub.path().filename().string());
						w.Key("type"); w.String("map");
						w.Key("id"); w.String(id_str.c_str());
						w.Key("source"); w.String("steam");
						w.Key("path"); w.String(sub.path().string().c_str());
						if (!image_url.empty()) { w.Key("image"); w.String(image_url.c_str()); }
						w.EndObject();
					}
				}
				}
			}

			w.EndArray();
			return std::string(buf.GetString(), buf.GetSize());
		}

		std::string normalize_option_token(std::string token)
		{
			utils::string::trim(token);

			if (token.empty())
			{
				return {};
			}

			while (!token.empty() && token.front() == '-')
			{
				token.erase(token.begin());
			}

			token = utils::string::to_lower(std::move(token));

			if (token == "safe-mode")
			{
				token = "safe";
			}
			return token;
		}

		void relaunch_with_launch_options(const std::vector<std::string>& options)
		{
			const auto self = utils::nt::library::get_by_address(relaunch_with_launch_options);
			const auto exe_path = self.get_path().generic_string();

			STARTUPINFOA startup_info;
			PROCESS_INFORMATION process_info;
			ZeroMemory(&startup_info, sizeof(startup_info));
			ZeroMemory(&process_info, sizeof(process_info));
			startup_info.cb = sizeof(startup_info);

			char current_dir[MAX_PATH];
			GetCurrentDirectoryA(sizeof(current_dir), current_dir);

			std::string command_line = "\"" + exe_path + "\"";

			int num_args = 0;
			auto* const argv = CommandLineToArgvW(GetCommandLineW(), &num_args);
			if (argv)
			{
				for (auto i = 1; i < num_args; ++i)
				{
					std::wstring wide_arg(argv[i]);
					std::string arg = utils::string::convert(wide_arg);

					const auto lower = utils::string::to_lower(arg);
					if (lower == "norelaunch" || lower == "update")
					{
						continue;
					}

					command_line += " \"" + arg + "\"";
				}

				LocalFree(argv);
			}

			command_line += " \"-launch\"";
			for (const auto& raw : options)
			{
				auto token = normalize_option_token(raw);
				if (token.empty())
				{
					continue;
				}

				command_line += " \"-" + token + "\"";
			}

			if (CreateProcessA(exe_path.data(), command_line.data(), nullptr, nullptr, false,
				CREATE_NEW_CONSOLE, nullptr, current_dir, &startup_info, &process_info))
			{
				if (process_info.hThread && process_info.hThread != INVALID_HANDLE_VALUE)
				{
					CloseHandle(process_info.hThread);
				}

				if (process_info.hProcess && process_info.hProcess != INVALID_HANDLE_VALUE)
				{
					CloseHandle(process_info.hProcess);
				}
			}
		}
	}

	bool run()
	{
		bool run_game = false;
		std::vector<std::string> launch_options{};
		html_window window("EZZ BOIII", 1260, 680);

		window.get_html_frame()->register_callback(
			"getVersion", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				return CComVariant(SHORTVERSION);
			});

		window.get_html_frame()->register_callback(
			"openUrl", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.empty()) return {};

				const auto& param = params[0];
				if (!param.is_string()) return {};

				const auto url = param.get_string();
				ShellExecuteA(nullptr, "open", url.data(), nullptr, nullptr, SW_SHOWNORMAL);

				return {};
			});

		window.get_html_frame()->register_callback(
			"workshopRemove", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.empty() || !params[0].is_string()) return {};
				workshop_remove_one(params[0].get_string());
				return {};
			});

		window.get_html_frame()->register_callback(
			"workshopRemoveAll", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				workshop_remove_all_folders();
				return {};
			});

		window.get_html_frame()->register_callback(
			"workshopRemoveByPath", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.empty() || !params[0].is_string()) return {};
				workshop_remove_by_path(params[0].get_string());
				return {};
			});

		window.get_html_frame()->register_callback(
			"workshopList", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				auto json = workshop_list_json();
				return CComVariant(json.c_str());
			});

		window.get_html_frame()->register_callback(
			"getModeFilesInfo", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				return CComVariant(get_mode_files_info().c_str());
			});

		window.get_html_frame()->register_callback(
			"removeModeFiles", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.empty() || !params[0].is_string()) return CComVariant("{}");
				auto result = remove_mode_files(params[0].get_string());
				return CComVariant(result.c_str());
			});

		launcher::workshop::register_callbacks(window.get_html_frame());

		window.get_html_frame()->register_callback(
			"invoke", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.size() < 1 || !params[0].is_string()) return {};
				const std::string method = params[0].get_string();
				const std::string arg = (params.size() >= 2 && params[1].is_string()) ? params[1].get_string() : "";
				if (method == "workshopRemove")
				{
					workshop_remove_one(arg);
					return CComVariant("ok");
				}
				if (method == "workshopRemoveAll")
				{
					workshop_remove_all_folders();
					return CComVariant("ok");
				}
				if (method == "workshopRemoveByPath")
				{
					workshop_remove_by_path(arg);
					return CComVariant("ok");
				}
				return {};
			});

		window.get_html_frame()->register_callback(
			"readLaunchOptions", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				const auto stored = utils::properties::load("launchOptions");
				if (!stored)
				{
					return CComVariant("");
				}
				return CComVariant(stored->c_str());
			});

		window.get_html_frame()->register_callback(
			"readPlayerName", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				const auto stored_name = utils::properties::load("playerName");
				if (!stored_name)
				{
					auto fallback = utils::nt::get_user_name();
					if (fallback.empty())
					{
						fallback = "Unknown Soldier";
					}
					return CComVariant(fallback.c_str());
				}
				return CComVariant(stored_name->c_str());
			});

		window.get_html_frame()->register_callback(
			"isGameRunning", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				return CComVariant(is_game_process_running() ? "1" : "0");
			});

		window.get_html_frame()->register_callback(
			"launchGame", [&](const std::vector<html_argument>& params) -> CComVariant
			{
				std::string new_name{};
				if (!params.empty() && params[0].is_string())
				{
					new_name = params[0].get_string();
					utils::string::trim(new_name);
				}

				if (new_name.empty())
				{
					new_name = utils::nt::get_user_name();
					if (new_name.empty()) new_name = "Unknown Soldier";
				}
				if (new_name.size() > 16) new_name.resize(16);
				utils::properties::store("playerName", new_name);

				std::string option_list{};
				if (params.size() >= 2 && params[1].is_string())
				{
					option_list = params[1].get_string();
					utils::string::trim(option_list);
				}
				utils::properties::store("launchOptions", option_list);

				std::vector<std::string> opts;
				if (!option_list.empty())
				{
					for (auto& part : utils::string::split(option_list, ' '))
					{
						auto token = normalize_option_token(std::move(part));
						if (!token.empty()) opts.emplace_back(std::move(token));
					}
				}

				relaunch_with_launch_options(opts);
				return CComVariant("ok");
			});

		window.get_html_frame()->register_callback(
			"runGame", [&](const std::vector<html_argument>& params) -> CComVariant
			{
				std::string new_name{};
				if (!params.empty() && params[0].is_string())
				{
					new_name = params[0].get_string();
					utils::string::trim(new_name);
				}

				if (new_name.empty())
				{
					new_name = utils::nt::get_user_name();
					if (new_name.empty())
					{
						new_name = "Unknown Soldier";
					}
				}

				if (new_name.size() > 16)
				{
					new_name.resize(16);
				}

				utils::properties::store("playerName", new_name);

				std::string option_list{};
				if (params.size() >= 2 && params[1].is_string())
				{
					option_list = params[1].get_string();
					utils::string::trim(option_list);
				}

				utils::properties::store("launchOptions", option_list);

				launch_options.clear();
				if (!option_list.empty())
				{
					for (auto& part : utils::string::split(option_list, ' '))
					{
						auto token = normalize_option_token(std::move(part));
						if (!token.empty())
						{
							launch_options.emplace_back(std::move(token));
						}
					}
				}

				if (!launch_options.empty())
				{
					run_game = false;
				}
				else
				{
					run_game = true;
				}

				window.get_window()->close();
				return {};
			});

		//window.get_html_frame()->load_html(utils::nt::load_resource(MENU_MAIN));
		window.get_html_frame()->load_url(
			utils::string::va("file:///%ls", get_launcher_ui_file().wstring().c_str()));

		window::run();
		if (!launch_options.empty())
		{
			relaunch_with_launch_options(launch_options);
			return false;
		}
		return run_game;
	}

	std::filesystem::path get_launcher_ui_file()
	{
		return game::get_appdata_path() / "data/launcher/main.html";
	}
}