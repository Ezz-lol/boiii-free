#include <std_include.hpp>
#include <utils/nt.hpp>
#include <utils/properties.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/http.hpp>
#include <utils/com.hpp>

#include "launcher.hpp"
#include "launcher_workshop.hpp"
#include "html/html_window.hpp"

#include <game/game.hpp>
#include <version.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <thread>
#include <atomic>
#include <mutex>
#include <map>
#include <set>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <TlHelp32.h>

// XXH3 via single-header xxhash library (must be at file scope)
#define XXH_INLINE_ALL
#include "xxhash.h"

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

namespace launcher
{
	namespace
	{
		std::string human_readable_size(std::uint64_t bytes);
		std::filesystem::path get_steam_workshop_path();

		std::mutex library_list_mutex;
		std::string library_list_cache;
		std::atomic<bool> library_list_loading{false};

		std::mutex remove_status_mutex;
		std::string remove_status_message;
		double remove_progress_percent = 0.0;
		std::string remove_progress_details;
		std::atomic<bool> remove_running{false};

		void set_remove_status(const std::string& msg, double pct, const std::string& details = "")
		{
			std::lock_guard lock(remove_status_mutex);
			remove_status_message = msg;
			remove_progress_percent = pct;
			remove_progress_details = details;
		}

		void reset_remove_status()
		{
			std::lock_guard lock(remove_status_mutex);
			remove_status_message.clear();
			remove_progress_percent = 0.0;
			remove_progress_details.clear();
		}

		std::uint64_t compute_folder_size(const std::filesystem::path& folder)
		{
			std::uint64_t total = 0;
			std::error_code ec;
			if (!std::filesystem::exists(folder, ec)) return 0;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(folder, ec)) {
				if (ec) break;
				if (entry.is_regular_file(ec))
					total += static_cast<std::uint64_t>(entry.file_size(ec));
			}
			return total;
		}

		std::mutex verify_mutex;
		std::string verify_status_message;
		double verify_progress_percent = 0.0;
		std::string verify_progress_details;
		std::vector<std::string> verify_changed_files;
		std::atomic<bool> verify_running{false};
		std::atomic<bool> verify_cancel_requested{false};

		void set_verify_status(const std::string& msg, double pct, const std::string& details)
		{
			std::lock_guard lock(verify_mutex);
			verify_status_message = msg;
			verify_progress_percent = pct;
			verify_progress_details = details;
		}

		void reset_verify_status()
		{
			std::lock_guard lock(verify_mutex);
			verify_status_message.clear();
			verify_progress_percent = 0.0;
			verify_progress_details.clear();
			verify_changed_files.clear();
		}

		std::string compute_file_xxh3(const std::filesystem::path& file_path)
		{
			std::ifstream file_stream(file_path, std::ios::binary);
			if (!file_stream.is_open()) return {};

			file_stream.seekg(0, std::ios::end);
			const auto file_size = static_cast<std::size_t>(file_stream.tellg());
			file_stream.seekg(0, std::ios::beg);

			if (file_size == 0) return {};

			XXH3_state_t* state = XXH3_createState();
			if (!state) return {};

			XXH3_64bits_reset(state);

			constexpr std::size_t read_buffer_size = 16ull * 1024ull * 1024ull; // 16MB
			std::string buffer;
			buffer.resize(read_buffer_size);

			auto bytes_to_read = file_size;
			while (bytes_to_read > 0)
			{
				const auto read_size = std::min(bytes_to_read, read_buffer_size);
				file_stream.read(buffer.data(), read_size);
				XXH3_64bits_update(state, buffer.data(), read_size);
				bytes_to_read -= read_size;
			}

			const auto hash_value = XXH3_64bits_digest(state);
			XXH3_freeState(state);

			// Output as uppercase hex in native (little-endian) byte order
			static const char hex[] = "0123456789ABCDEF";
			std::string result;
			result.reserve(16);
			const auto* bytes = reinterpret_cast<const std::uint8_t*>(&hash_value);
			for (int i = 0; i < 8; i++)
			{
				result += hex[bytes[i] >> 4];
				result += hex[bytes[i] & 0x0F];
			}
			return result;
		}

		void verify_game_thread(const std::string& modes_csv)
		{
			verify_cancel_requested = false;
			set_verify_status("Preparing verification...", 0.0, "");

			try {

			auto prefixes = utils::string::split(modes_csv, ',');
			for (auto& p : prefixes) utils::string::trim(p);

			if (prefixes.size() == 1 && prefixes[0] == "all")
			{
				prefixes = {"cp_", "mp_", "zm_"};
			}

			bool want_cp = false, want_mp = false, want_zm = false;
			for (const auto& p : prefixes)
			{
				if (p == "cp_") want_cp = true;
				else if (p == "mp_") want_mp = true;
				else if (p == "zm_") want_zm = true;
			}
			bool want_all = want_cp && want_mp && want_zm;

			std::string mode_label;
			if (want_all) mode_label = "All";
			else
			{
				if (want_zm) mode_label += "Zombies ";
				if (want_mp) mode_label += "Multiplayer ";
				if (want_cp) mode_label += "Campaign ";
				utils::string::trim(mode_label);
			}

			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			auto manifest_path = game::get_appdata_path() / "data" / "launcher" / "verification.json";
			if (!std::filesystem::exists(manifest_path))
			{
				set_verify_status("verification.json not found in " + manifest_path.string(), 0.0, "");
				verify_running = false;
				return;
			}

			set_verify_status("Loading manifest...", 0.0, "Mode: " + mode_label);
			std::ifstream mf(manifest_path, std::ios::binary);
			std::string manifest_str((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());
			mf.close();

			rapidjson::Document manifest;
			if (manifest.Parse(manifest_str.c_str()).HasParseError() || !manifest.IsObject())
			{
				set_verify_status("Failed to parse verification.json manifest", 0.0, "");
				verify_running = false;
				return;
			}

			auto files_it = manifest.FindMember("files");
			if (files_it == manifest.MemberEnd() || !files_it->value.IsArray())
			{
				set_verify_status("Invalid manifest: no files array", 0.0, "");
				verify_running = false;
				return;
			}


			auto should_include_component = [&](const std::string& comp) -> bool {
				if (comp == "redist") return false; // skip redistributables
				if (comp == "base") return true;
				if (comp == "sp") return want_cp;
				if (comp == "zc") return want_zm;
				if (comp == "dlc") return want_mp || want_zm;
				return want_all;
			};

			std::vector<std::string> exclude_prefixes;
			if (!want_cp) { exclude_prefixes.push_back("cp_"); exclude_prefixes.push_back("en_cp_"); }
			if (!want_mp) { exclude_prefixes.push_back("mp_"); exclude_prefixes.push_back("en_mp_"); }
			if (!want_zm) { exclude_prefixes.push_back("zm_"); exclude_prefixes.push_back("en_zm_"); }

			auto should_exclude_file = [&](const std::string& filepath) -> bool {
				if (want_all) return false;
				std::string fname = filepath;
				auto slash = fname.rfind('/');
				if (slash != std::string::npos) fname = fname.substr(slash + 1);
				fname = utils::string::to_lower(fname);
				for (const auto& ex : exclude_prefixes)
				{
					if (fname.rfind(ex, 0) == 0) return true;
				}
				return false;
			};

			struct manifest_entry {
				std::string path;
				std::uint64_t expected_size;
				std::string expected_hash;
				std::string component;
			};
			std::vector<manifest_entry> files_to_check;

			set_verify_status("Filtering manifest files...", 0.0, "Mode: " + mode_label);

			const auto& files_array = files_it->value;
			for (rapidjson::SizeType i = 0; i < files_array.Size(); i++)
			{
				const auto& entry = files_array[i];
				if (!entry.IsArray() || entry.Size() < 4) continue;
				if (!entry[0].IsString() || !entry[3].IsString()) continue;

				std::string path = entry[0].GetString();
				std::uint64_t size = 0;
				if (entry[1].IsUint64()) size = entry[1].GetUint64();
				else if (entry[1].IsInt64()) size = static_cast<std::uint64_t>(entry[1].GetInt64());
				else if (entry[1].IsUint()) size = entry[1].GetUint();
				std::string hash_str;
				if (entry.Size() >= 3 && entry[2].IsString())
					hash_str = entry[2].GetString();
				std::string comp = entry[3].GetString();

				if (!should_include_component(comp)) continue;
				if (should_exclude_file(path)) continue;

				files_to_check.push_back({path, size, hash_str, comp});
			}

			struct component_stats {
				std::string display_name;
				std::uint32_t total = 0;
				std::uint32_t ok = 0;
				std::uint32_t missing = 0;
				std::uint32_t size_mismatch = 0;
				std::uint32_t hash_mismatch = 0;
				std::vector<std::string> missing_files;
			};
			std::map<std::string, component_stats> comp_stats;

			auto friendly_comp_name = [&](const std::string& key) -> std::string {
				if (key == "base") return "Base Game";
				if (key == "sp") return "Campaign";
				if (key == "zc") return "Zombie Chronicles";
				if (key == "dlc")
				{
					if (want_mp && !want_zm) return "MP DLC";
					if (want_zm && !want_mp) return "ZM DLC";
					return "DLC";
				}
				return key;
			};
			auto comps_it = manifest.FindMember("components");
			if (comps_it != manifest.MemberEnd() && comps_it->value.IsObject())
			{
				for (auto it = comps_it->value.MemberBegin(); it != comps_it->value.MemberEnd(); ++it)
				{
					std::string key(it->name.GetString(), it->name.GetStringLength());
					comp_stats[key].display_name = friendly_comp_name(key);
				}
			}

			// Verify each file against disk
			std::uint32_t total = static_cast<std::uint32_t>(files_to_check.size());
			std::uint32_t ok_count = 0;
			std::uint32_t missing_count = 0;
			std::uint32_t size_mismatch_count = 0;
			std::uint32_t hash_mismatch_count = 0;
			std::uint32_t warn_missing_count = 0;
			std::uint32_t warn_size_count = 0;
			std::uint32_t warn_hash_count = 0;
			std::vector<std::string> problematic_files;

			for (std::uint32_t i = 0; i < total; i++)
			{
				if (verify_cancel_requested)
				{
					set_verify_status("Cancelled", 0.0, "");
					verify_running = false;
					return;
				}

				const auto& fe = files_to_check[i];
				double pct = static_cast<double>(i) / static_cast<double>(total) * 100.0;

				std::string fname = fe.path;
				auto slash = fname.rfind('/');
				if (slash != std::string::npos) fname = fname.substr(slash + 1);
				set_verify_status("Verifying (" + mode_label + ")...", pct,
					fname + " (" + std::to_string(i + 1) + "/" + std::to_string(total) + ")");

				auto& cs = comp_stats[fe.component];
				if (cs.display_name.empty()) cs.display_name = fe.component;
				cs.total++;

				std::filesystem::path full_path = base / fe.path;
				std::error_code ec;

				bool is_dlc_or_zc = (fe.component == "dlc" || fe.component == "zc");
				std::string severity = is_dlc_or_zc ? "warning" : "error";

				if (!std::filesystem::exists(full_path, ec))
				{
					if (is_dlc_or_zc)
					{
						warn_missing_count++;
					}
					else
					{
						missing_count++;
					}
					cs.missing++;
					cs.missing_files.push_back(fe.path);
					problematic_files.push_back("[" + severity + "] " + fe.path + " (missing - " + cs.display_name + ")");
				}
				else
				{
					auto actual_size = std::filesystem::file_size(full_path, ec);
					if (actual_size != fe.expected_size)
					{
						if (is_dlc_or_zc)
						{
							warn_size_count++;
						}
						else
						{
							size_mismatch_count++;
						}
						cs.size_mismatch++;
						problematic_files.push_back("[" + severity + "] " + fe.path + " (wrong size - " + cs.display_name + ")");
					}
					else if (!fe.expected_hash.empty())
					{
						auto actual_hash = compute_file_xxh3(full_path);
						if (!actual_hash.empty() && actual_hash != fe.expected_hash)
						{
							if (is_dlc_or_zc)
							{
								warn_hash_count++;
							}
							else
							{
								hash_mismatch_count++;
							}
							cs.hash_mismatch++;
							problematic_files.push_back("[" + severity + "] " + fe.path + " (corrupt - " + cs.display_name + ")");
						}
						else
						{
							ok_count++;
							cs.ok++;
						}
					}
					else
					{
						ok_count++;
						cs.ok++;
					}
				}
			}

			const std::uint32_t error_total = missing_count + size_mismatch_count + hash_mismatch_count;
			const std::uint32_t warn_total = warn_missing_count + warn_size_count + warn_hash_count;

			std::string result_msg = mode_label + ": Verified " + std::to_string(total) + " files: " +
				std::to_string(ok_count) + " OK";
			if (error_total > 0)
			{
				result_msg += " | ERRORS:";
				if (missing_count > 0)
					result_msg += " " + std::to_string(missing_count) + " missing";
				if (size_mismatch_count > 0)
					result_msg += " " + std::to_string(size_mismatch_count) + " wrong size";
				if (hash_mismatch_count > 0)
					result_msg += " " + std::to_string(hash_mismatch_count) + " corrupt";
			}
			if (warn_total > 0)
			{
				result_msg += " | DLC warnings:";
				if (warn_missing_count > 0)
					result_msg += " " + std::to_string(warn_missing_count) + " missing";
				if (warn_size_count > 0)
					result_msg += " " + std::to_string(warn_size_count) + " wrong size";
				if (warn_hash_count > 0)
					result_msg += " " + std::to_string(warn_hash_count) + " corrupt";
			}
			if (error_total == 0 && warn_total == 0)
				result_msg += " - all good!";

			std::vector<std::string> comp_issues;
			for (const auto& [comp_key, cs] : comp_stats)
			{
				if (cs.missing > 0 || cs.size_mismatch > 0 || cs.hash_mismatch > 0)
				{
					std::string issue = cs.display_name + ": ";
					std::vector<std::string> parts;
					if (cs.missing > 0)
						parts.push_back(std::to_string(cs.missing) + " missing");
					if (cs.size_mismatch > 0)
						parts.push_back(std::to_string(cs.size_mismatch) + " wrong size");
					if (cs.hash_mismatch > 0)
						parts.push_back(std::to_string(cs.hash_mismatch) + " corrupt");
					for (size_t j = 0; j < parts.size(); j++)
					{
						if (j > 0) issue += ", ";
						issue += parts[j];
					}
					comp_issues.push_back(issue);
				}
			}
			if (!comp_issues.empty())
			{
				result_msg += " | ";
				for (size_t i = 0; i < comp_issues.size(); i++)
				{
					if (i > 0) result_msg += " ; ";
					result_msg += comp_issues[i];
				}
			}

			{
				std::lock_guard lock(verify_mutex);
				verify_changed_files = std::move(problematic_files);
			}
			set_verify_status(result_msg, 100.0, "Done");

			} catch (...) {
				set_verify_status("Verification failed (try again)", 0.0, "");
			}
			verify_running = false;
		}

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
			launcher::workshop::try_refresh_workshop_content();
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
			launcher::workshop::try_refresh_workshop_content();
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
			launcher::workshop::try_refresh_workshop_content();
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
						if (filename.rfind(m.prefix, 0) == 0)
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
for (auto& p : prefixes) utils::string::trim(p); std::vector<std::filesystem::path> to_delete; const char* dirs_to_scan[] = { "zone", "video" }; for (const auto& dir_name : dirs_to_scan) { std::filesystem::path dir = base / dir_name;
				if (!std::filesystem::exists(dir)) continue;

				std::error_code ec;
				for (const auto& entry : std::filesystem::recursive_directory_iterator(dir, ec))
				{
					if (!entry.is_regular_file(ec)) continue;
					auto filename = utils::string::to_lower(entry.path().filename().string());

					for (const auto& prefix : prefixes)
					{
						if (!prefix.empty() && filename.rfind(prefix, 0) == 0)
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

			auto steamapps = base.parent_path().parent_path();
			auto workshop_path = steamapps / "workshop" / "content" / "311210";
			if (std::filesystem::exists(workshop_path))
			{
				return workshop_path;
			}
			return {};
		}

		std::uint64_t get_folder_mtime_epoch(const std::filesystem::path& folder)
		{
			try {
				std::error_code ec;
				auto lwt = std::filesystem::last_write_time(folder, ec);
				if (ec) return 0;
				auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
					lwt - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
				return static_cast<std::uint64_t>(std::chrono::system_clock::to_time_t(sctp));
			} catch (...) { return 0; }
		}

		struct mod_item_info {
			std::string name;
			std::string folder;
			std::string type;
			std::string id;
			std::string image;
			std::string source;
			std::string path;
			std::string description;
			std::filesystem::path dir_path;
			std::uint64_t local_size = 0;
		};

		bool folder_has_zone_content(const std::filesystem::path& dir)
		{
			std::error_code ec;
			auto zone_dir = dir / "zone";
			if (std::filesystem::exists(zone_dir, ec))
			{
				for (const auto& f : std::filesystem::directory_iterator(zone_dir, ec))
				{
					if (f.is_regular_file(ec) && f.path().extension() == ".ff")
						return true;
				}
			}
			for (const auto& f : std::filesystem::directory_iterator(dir, ec))
			{
				if (f.is_regular_file(ec) && f.path().extension() == ".ff")
					return true;
			}
			return false;
		}

		bool try_parse_workshop_json(const std::filesystem::path& dir, mod_item_info& item)
		{
			std::filesystem::path zone_json = dir / "zone" / "workshop.json";
			if (!utils::io::file_exists(zone_json.string())) return false;
			std::string data;
			if (!utils::io::read_file(zone_json.string(), &data)) return false;
			rapidjson::Document doc;
			if (doc.Parse(data).HasParseError() || !doc.IsObject()) return false;
			if (!doc.HasMember("Title") || !doc.HasMember("FolderName")) return false;
			item.name = doc["Title"].GetString();
			if (doc.HasMember("Description") && doc["Description"].IsString())
			{
				item.description = doc["Description"].GetString();
				if (item.description.size() > 300)
					item.description = item.description.substr(0, 300) + "...";
			}
			if (doc.HasMember("PublisherID"))
			{
				const auto& pid = doc["PublisherID"];
				if (pid.IsString()) item.id = pid.GetString();
				else if (pid.IsInt64()) item.id = std::to_string(pid.GetInt64());
				else if (pid.IsUint64()) item.id = std::to_string(pid.GetUint64());
			}
			return true;
		}

		std::string workshop_list_json()
		{
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			std::vector<mod_item_info> items;
			std::set<std::string> seen_paths;

			auto scan = [&](const std::filesystem::path& parent, const char* type_label) {
				if (!std::filesystem::exists(parent)) return;
				std::error_code ec;
				for (const auto& entry : std::filesystem::directory_iterator(parent, ec))
				{
					if (!entry.is_directory()) continue;
					auto abs = std::filesystem::absolute(entry.path(), ec).string();
					if (seen_paths.count(abs)) continue;

					mod_item_info item;
					item.folder = entry.path().filename().string();
					item.type = type_label;
					item.dir_path = entry.path();
					item.local_size = compute_folder_size(entry.path());

					if (!try_parse_workshop_json(entry.path(), item))
					{
						if (!folder_has_zone_content(entry.path())) continue;
						item.name = item.folder;
						if (utils::string::is_numeric(item.folder))
							item.id = item.folder;
					}

					std::string image_path = find_mod_image_path(entry.path());
					if (!image_path.empty())
						item.image = path_to_file_url(image_path);
					else if (!item.id.empty())
						item.image = get_steam_workshop_preview_url(item.id);

					seen_paths.insert(abs);
					items.push_back(std::move(item));
				}
			};
			scan(base / "usermaps", "map");
			scan(base / "mods", "mod");

			std::filesystem::path steam_ws = get_steam_workshop_path();
			if (!steam_ws.empty())
			{
				std::error_code ws_ec;
				if (std::filesystem::exists(steam_ws, ws_ec))
				{
					auto scan_steam = [&](const std::filesystem::path& dir) {
						auto abs = std::filesystem::absolute(dir, ws_ec).string();
						if (seen_paths.count(abs)) return;

						mod_item_info item;
						item.folder = dir.filename().string();
						item.type = "map";
						item.source = "steam";
						item.path = dir.string();
						item.dir_path = dir;
						item.local_size = compute_folder_size(dir);

						if (!try_parse_workshop_json(dir, item))
						{
							if (!folder_has_zone_content(dir)) return;
							item.name = item.folder;
							if (utils::string::is_numeric(item.folder))
								item.id = item.folder;
						}

						std::string image_path = find_mod_image_path(dir);
						if (!image_path.empty())
							item.image = path_to_file_url(image_path);
						else if (!item.id.empty())
							item.image = get_steam_workshop_preview_url(item.id);

						seen_paths.insert(abs);
						items.push_back(std::move(item));
					};

					std::error_code ec;
					for (const auto& ws_entry : std::filesystem::directory_iterator(steam_ws, ec))
					{
						if (!ws_entry.is_directory()) continue;

						if (try_parse_workshop_json(ws_entry.path(), mod_item_info{}) || folder_has_zone_content(ws_entry.path()))
						{
							scan_steam(ws_entry.path());
							continue;
						}

						std::error_code ec2;
						for (const auto& sub : std::filesystem::directory_iterator(ws_entry.path(), ec2))
						{
							if (!sub.is_directory()) continue;
							scan_steam(sub.path());
						}
					}
				}
			}

			std::vector<std::string> all_ids;
			for (const auto& it : items) {
				if (!it.id.empty()) all_ids.push_back(it.id);
			}
			auto meta_map = workshop::batch_get_workshop_meta(all_ids);

			rapidjson::StringBuffer buf;
			rapidjson::Writer<rapidjson::StringBuffer> w(buf);
			w.StartArray();
			for (const auto& it : items) {
				w.StartObject();
				w.Key("name"); w.String(it.name.c_str());
				w.Key("folder"); w.String(it.folder.c_str());
				w.Key("type"); w.String(it.type.c_str());
				w.Key("id"); w.String(it.id.c_str());
				if (!it.image.empty()) { w.Key("image"); w.String(it.image.c_str()); }
				if (!it.source.empty()) { w.Key("source"); w.String(it.source.c_str()); }
				{
					w.Key("path");
					if (!it.path.empty()) w.String(it.path.c_str());
					else w.String(it.dir_path.string().c_str());
				}

				w.Key("localSize"); w.Uint64(it.local_size);
				if (!it.description.empty()) {
					w.Key("description"); w.String(it.description.c_str());
				}

				if (!it.id.empty()) {
					auto mi = meta_map.find(it.id);
					if (mi != meta_map.end()) {
						const auto& m = mi->second;
						if (m.time_updated > 0) {
							auto local_mtime = get_folder_mtime_epoch(it.dir_path);
							if (local_mtime > 0 && m.time_updated > local_mtime) {
								w.Key("needsUpdate"); w.Bool(true);
							}
						}
						if (m.file_size > 0) { w.Key("file_size"); w.Uint64(m.file_size); }
						if (m.subs > 0) { w.Key("subs"); w.Int64(m.subs); }
						if (m.favorites > 0) { w.Key("favorites"); w.Int64(m.favorites); }
						if (m.star_rating > 0) { w.Key("starRating"); w.Int(m.star_rating); }
						if (!m.description.empty() && it.description.empty()) {
							w.Key("description"); w.String(m.description.c_str());
						}
						if (it.image.empty() && !m.preview_url.empty()) {
							w.Key("image"); w.String(m.preview_url.c_str());
						}
					}
				}
				w.EndObject();
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
			"getGamePath", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				char cwd[MAX_PATH];
				GetCurrentDirectoryA(sizeof(cwd), cwd);
				return CComVariant(cwd);
			});

		window.get_html_frame()->register_callback(
			"selectGameFolder", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				std::string selected_str;
				try
				{
					if (!utils::com::select_folder(selected_str, "Select your Black Ops 3 installation folder"))
					{
						return CComVariant("cancelled");
					}
				}
				catch (...)
				{
					return CComVariant("error");
				}

				const std::filesystem::path selected(selected_str);
				const bool has_client = std::filesystem::exists(selected / "BlackOps3.exe");
				const bool has_server = std::filesystem::exists(selected / "BlackOps3_UnrankedDedicatedServer.exe");
				if (!has_client && !has_server)
				{
					return CComVariant("invalid");
				}

				const auto path_file = game::get_appdata_path() / "user" / "game_path.txt";
				std::error_code ec;
				std::filesystem::create_directories(path_file.parent_path(), ec);
				utils::io::write_file(path_file.string(), selected_str);

				SetCurrentDirectoryA(selected_str.c_str());

				return CComVariant(selected_str.c_str());
			});

		window.get_html_frame()->register_callback(
			"workshopRemove", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.empty() || !params[0].is_string()) return {};
				if (remove_running.load()) return CComVariant("already_running");
				remove_running = true;
				std::string folder = params[0].get_string();
				std::thread([folder]() {
					set_remove_status("Removing mod...", -1.0, folder);
					workshop_remove_one(folder);
					{
						std::lock_guard lock(library_list_mutex);
						library_list_cache.clear();
					}
					set_remove_status("Removal complete", 100.0);
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					reset_remove_status();
					remove_running = false;
				}).detach();
				return CComVariant("started");
			});

		window.get_html_frame()->register_callback(
			"workshopRemoveAll", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				if (remove_running.load()) return CComVariant("already_running");
				remove_running = true;
				std::thread([]() {
					set_remove_status("Preparing removal...", -1.0);

					char cwd[MAX_PATH];
					GetCurrentDirectoryA(sizeof(cwd), cwd);
					std::filesystem::path base(cwd);

					// Collect all directories to remove
					std::vector<std::filesystem::path> dirs_to_remove;
					std::error_code ec;
					auto collect = [&](const std::filesystem::path& dir) {
						if (!std::filesystem::exists(dir, ec)) return;
						for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
							if (entry.is_directory())
								dirs_to_remove.push_back(entry.path());
					};
					collect(base / "mods");
					collect(base / "usermaps");
					std::filesystem::path steam_ws = get_steam_workshop_path();
					if (!steam_ws.empty() && std::filesystem::exists(steam_ws, ec))
						collect(steam_ws);

					const int total = static_cast<int>(dirs_to_remove.size());
					if (total == 0)
					{
						set_remove_status("Nothing to remove", 100.0);
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						reset_remove_status();
						remove_running = false;
						return;
					}

					for (int i = 0; i < total; ++i)
					{
						const double pct = (static_cast<double>(i) / total) * 100.0;
						const auto name = dirs_to_remove[i].filename().string();
						set_remove_status("Removing mods...", pct,
							name + "  (" + std::to_string(i + 1) + "/" + std::to_string(total) + ")");
						std::error_code ec2;
						std::filesystem::remove_all(dirs_to_remove[i], ec2);
					}

					{
						std::lock_guard lock(library_list_mutex);
						library_list_cache.clear();
					}
					set_remove_status("Removal complete", 100.0,
						std::to_string(total) + " item" + (total != 1 ? "s" : "") + " removed");
					launcher::workshop::try_refresh_workshop_content();
					std::this_thread::sleep_for(std::chrono::milliseconds(800));
					reset_remove_status();
					remove_running = false;
				}).detach();
				return CComVariant("started");
			});

		window.get_html_frame()->register_callback(
			"workshopRemoveByPath", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.empty() || !params[0].is_string()) return {};
				if (remove_running.load()) return CComVariant("already_running");
				remove_running = true;
				std::string path = params[0].get_string();
				std::thread([path]() {
					set_remove_status("Removing mod...", -1.0, path);
					workshop_remove_by_path(path);
					{
						std::lock_guard lock(library_list_mutex);
						library_list_cache.clear();
					}
					set_remove_status("Removal complete", 100.0);
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					reset_remove_status();
					remove_running = false;
				}).detach();
				return CComVariant("started");
			});

		window.get_html_frame()->register_callback(
			"workshopList", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				std::lock_guard lock(library_list_mutex);
				if (!library_list_cache.empty())
					return CComVariant(library_list_cache.c_str());
				return CComVariant("[]");
			});

		window.get_html_frame()->register_callback(
			"workshopListAsync", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				if (library_list_loading.load())
					return CComVariant("already_loading");
				library_list_loading = true;
				std::thread([]() {
					try {
						auto json = workshop_list_json();
						{
							std::lock_guard lock(library_list_mutex);
							library_list_cache = std::move(json);
						}
					} catch (...) {}
					library_list_loading = false;
				}).detach();
				return CComVariant("started");
			});

		window.get_html_frame()->register_callback(
			"workshopListIsLoading", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				return CComVariant(library_list_loading.load() ? "true" : "false");
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
			"verifyGameFiles", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.empty() || !params[0].is_string()) return CComVariant("");
				std::string modes = params[0].get_string();
				utils::string::trim(modes);
				if (modes.empty()) return CComVariant("");
				if (verify_running.exchange(true)) return CComVariant("already_running");
				reset_verify_status();
				std::thread(verify_game_thread, modes).detach();
				return CComVariant("started");
			});

		window.get_html_frame()->register_callback(
			"getVerifyStatus", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				std::lock_guard lock(verify_mutex);
				rapidjson::StringBuffer buf;
				rapidjson::Writer<rapidjson::StringBuffer> w(buf);
				w.StartObject();
				w.Key("message"); w.String(verify_status_message.c_str());
				w.Key("progress"); w.Double(verify_progress_percent);
				w.Key("details"); w.String(verify_progress_details.c_str());
				w.Key("running"); w.Bool(verify_running.load());
				w.Key("changedFiles");
				w.StartArray();
				for (const auto& f : verify_changed_files)
					w.String(f.c_str());
				w.EndArray();
				w.EndObject();
				return CComVariant(std::string(buf.GetString(), buf.GetSize()).c_str());
			});

		window.get_html_frame()->register_callback(
			"cancelVerify", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				verify_cancel_requested = true;
				return CComVariant("cancel_requested");
			});

		window.get_html_frame()->register_callback(
			"getRemoveStatus", [](const std::vector<html_argument>& /*params*/) -> CComVariant
			{
				std::lock_guard lock(remove_status_mutex);
				rapidjson::StringBuffer buf;
				rapidjson::Writer<rapidjson::StringBuffer> w(buf);
				w.StartObject();
				w.Key("message"); w.String(remove_status_message.c_str());
				w.Key("progress"); w.Double(remove_progress_percent);
				w.Key("details"); w.String(remove_progress_details.c_str());
				w.Key("running"); w.Bool(remove_running.load());
				w.EndObject();
				return CComVariant(std::string(buf.GetString(), buf.GetSize()).c_str());
			});

		window.get_html_frame()->register_callback(
			"invoke", [](const std::vector<html_argument>& params) -> CComVariant
			{
				if (params.size() < 1 || !params[0].is_string()) return {};
				const std::string method = params[0].get_string();
				const std::string arg = (params.size() >= 2 && params[1].is_string()) ? params[1].get_string() : "";
				if (method == "workshopRemove")
				{
					if (remove_running.load()) return CComVariant("already_running");
					remove_running = true;
					std::thread([a = arg]() {
						set_remove_status("Removing mod...", -1.0, a);
						workshop_remove_one(a);
						{
							std::lock_guard lock(library_list_mutex);
							library_list_cache.clear();
						}
						set_remove_status("Removal complete", 100.0);
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						reset_remove_status();
						remove_running = false;
					}).detach();
					return CComVariant("started");
				}
				if (method == "workshopRemoveAll")
				{
					// Delegate to the workshopRemoveAll callback logic
					// (handled by the dedicated callback now)
					return CComVariant("use_callback");
				}
				if (method == "workshopRemoveByPath")
				{
					if (remove_running.load()) return CComVariant("already_running");
					remove_running = true;
					std::thread([a = arg]() {
						set_remove_status("Removing mod...", -1.0, a);
						workshop_remove_by_path(a);
						{
							std::lock_guard lock(library_list_mutex);
							library_list_cache.clear();
						}
						set_remove_status("Removal complete", 100.0);
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						reset_remove_status();
						remove_running = false;
					}).detach();
					return CComVariant("started");
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