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
#include <set>
#include <map>
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

		std::mutex library_list_mutex;
		std::string library_list_cache;
		std::atomic<bool> library_list_loading{false};

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

		std::string compute_file_md5(const std::filesystem::path& file_path)
		{
			HANDLE hFile = CreateFileW(file_path.wstring().c_str(), GENERIC_READ, FILE_SHARE_READ,
				nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
			if (hFile == INVALID_HANDLE_VALUE) return {};

			HCRYPTPROV hProv = 0;
			HCRYPTHASH hHash = 0;
			std::string result;

			if (CryptAcquireContextA(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
			{
				if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
				{
					BYTE buffer[65536];
					DWORD bytesRead = 0;
					bool ok = true;

					while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0)
					{
						if (!CryptHashData(hHash, buffer, bytesRead, 0))
						{
							ok = false;
							break;
						}
					}

					if (ok)
					{
						BYTE hash[16];
						DWORD hashLen = sizeof(hash);
						if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0))
						{
							static const char hex[] = "0123456789abcdef";
							result.reserve(32);
							for (DWORD i = 0; i < hashLen; i++)
							{
								result += hex[hash[i] >> 4];
								result += hex[hash[i] & 0x0F];
							}
						}
					}
					CryptDestroyHash(hHash);
				}
				CryptReleaseContext(hProv, 0);
			}
			CloseHandle(hFile);
			return result;
		}

		std::filesystem::path get_verification_json_path()
		{
			return game::get_appdata_path() / "verification.json";
		}

		std::string load_verification_json()
		{
			auto path = get_verification_json_path();
			std::error_code ec;
			if (!std::filesystem::exists(path, ec)) return "{}";
			std::ifstream f(path, std::ios::binary);
			if (!f) return "{}";
			return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		}

		void save_verification_json(const std::string& json_str)
		{
			auto path = get_verification_json_path();
			std::error_code ec;
			std::filesystem::create_directories(path.parent_path(), ec);
			std::ofstream f(path, std::ios::binary | std::ios::trunc);
			if (f) f << json_str;
		}

		void verify_game_thread(const std::string& modes_csv)
		{
			verify_running = true;
			verify_cancel_requested = false;
			set_verify_status("Preparing verification...", 0.0, "");

			try {

			auto prefixes = utils::string::split(modes_csv, ',');
			for (auto& p : prefixes) utils::string::trim(p);

			if (prefixes.size() == 1 && prefixes[0] == "all")
			{
				prefixes = {"cp_", "mp_", "zm_"};
			}

			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			set_verify_status("Scanning game files...", 0.0, "");
			struct file_entry {
				std::filesystem::path full_path;
				std::string relative_path;
				std::uint64_t size;
			};
			std::vector<file_entry> files_to_verify;
			bool include_base_files = (prefixes.size() == 3); // only when verifying all modes
			std::map<std::string, std::uint32_t> prefix_file_counts;
			if (include_base_files) prefix_file_counts["base"] = 0;
			for (const auto& p : prefixes) prefix_file_counts[p] = 0;

			std::vector<std::string> active_prefixes;
			for (const auto& p : prefixes) {
				active_prefixes.push_back(p);
				active_prefixes.push_back("en_" + p);
			}

			static const std::vector<std::string> all_mode_markers = {
				"cp_", "mp_", "zm_", "en_cp_", "en_mp_", "en_zm_"
			};
			auto is_base_file = [&](const std::string& fname) -> bool {
				for (const auto& m : all_mode_markers) {
					if (fname.rfind(m, 0) == 0) return false;
				}
				return true;
			};

			const char* dirs_to_scan[] = {"zone", "video"};
			for (const auto& dir_name : dirs_to_scan)
			{
				if (verify_cancel_requested) break;
				std::filesystem::path dir = base / dir_name;
				if (!std::filesystem::exists(dir)) continue;
				std::error_code ec;
				for (const auto& entry : std::filesystem::recursive_directory_iterator(dir, ec))
				{
					if (verify_cancel_requested) break;
					if (!entry.is_regular_file(ec)) continue;
					auto filename = utils::string::to_lower(entry.path().filename().string());
					if (is_base_file(filename))
					{
						if (!include_base_files) continue;
						auto rel = std::filesystem::relative(entry.path(), base, ec).generic_string();
						files_to_verify.push_back({entry.path(), rel, entry.file_size(ec)});
						prefix_file_counts["base"]++;
					}
					else
					{
						for (const auto& prefix : active_prefixes)
						{
							if (!prefix.empty() && filename.rfind(prefix, 0) == 0)
							{
								auto rel = std::filesystem::relative(entry.path(), base, ec).generic_string();
								files_to_verify.push_back({entry.path(), rel, entry.file_size(ec)});
								for (const auto& p : prefixes)
								{
									if (prefix == p || prefix == "en_" + p)
									{
										prefix_file_counts[p]++;
										break;
									}
								}
								break;
							}
						}
					}
				}
			}

			if (verify_cancel_requested)
			{
				set_verify_status("Cancelled", 0.0, "");
				verify_running = false;
				return;
			}

			std::string existing_json = load_verification_json();
			rapidjson::Document existing_doc;
			if (existing_doc.Parse(existing_json.c_str()).HasParseError() || !existing_doc.IsObject())
			{
				existing_doc.SetObject();
			}

			std::uint32_t total_files = static_cast<std::uint32_t>(files_to_verify.size());
			std::uint32_t verified_ok = 0;
			std::uint32_t changed_files = 0;
			std::uint32_t new_files = 0;
			std::uint32_t failed_files = 0;
			std::vector<std::string> problematic_files;

			rapidjson::Document new_doc;
			new_doc.SetObject();
			auto& alloc = new_doc.GetAllocator();

			if (existing_doc.IsObject())
			{
				for (auto it = existing_doc.MemberBegin(); it != existing_doc.MemberEnd(); ++it)
				{
					std::string key(it->name.GetString(), it->name.GetStringLength());
					bool belongs_to_current = false;
					std::string key_lower = utils::string::to_lower(key);
					std::string fname = key_lower;
					auto slash = fname.rfind('/');
					if (slash != std::string::npos) fname = fname.substr(slash + 1);
					if (include_base_files && is_base_file(fname))
					{
						belongs_to_current = true;
					}
					else
					{
						for (const auto& prefix : active_prefixes)
						{
							if (fname.rfind(prefix, 0) == 0)
							{
								belongs_to_current = true;
								break;
							}
						}
					}
					if (!belongs_to_current)
					{
						rapidjson::Value k(key.c_str(), alloc);
						rapidjson::Value v(it->value, alloc);
						new_doc.AddMember(k, v, alloc);
					}
				}
			}

			for (std::uint32_t i = 0; i < total_files; i++)
			{
				if (verify_cancel_requested)
				{
					set_verify_status("Cancelled", 0.0, "");
					verify_running = false;
					return;
				}

				const auto& fe = files_to_verify[i];
				double pct = static_cast<double>(i) / static_cast<double>(total_files) * 100.0;
				auto fname = fe.full_path.filename().string();
				set_verify_status("Verifying...", pct,
					fname + " (" + std::to_string(i + 1) + "/" + std::to_string(total_files) + ")");

				std::string hash = compute_file_md5(fe.full_path);
				if (hash.empty())
				{
					failed_files++;
					problematic_files.push_back(fe.relative_path + " (read error)");
					continue;
				}

				auto existing_it = existing_doc.FindMember(fe.relative_path.c_str());
				if (existing_it != existing_doc.MemberEnd() && existing_it->value.IsObject())
				{
					auto hash_it = existing_it->value.FindMember("hash");
					if (hash_it != existing_it->value.MemberEnd() && hash_it->value.IsString())
					{
						std::string old_hash(hash_it->value.GetString(), hash_it->value.GetStringLength());
						if (old_hash == hash)
						{
							verified_ok++;
						}
						else
						{
							changed_files++;
							problematic_files.push_back(fe.relative_path + " (changed)");
						}
					}
					else
					{
						new_files++;
					}
				}
				else
				{
					new_files++;
				}

				rapidjson::Value entry_obj(rapidjson::kObjectType);
				rapidjson::Value hash_val(hash.c_str(), alloc);
				entry_obj.AddMember("hash", hash_val, alloc);
				entry_obj.AddMember("size", fe.size, alloc);
				rapidjson::Value path_key(fe.relative_path.c_str(), alloc);
				new_doc.AddMember(path_key, entry_obj, alloc);
			}

			rapidjson::StringBuffer buf;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> w(buf);
			new_doc.Accept(w);
			save_verification_json(std::string(buf.GetString(), buf.GetSize()));

			std::uint32_t missing_files = 0;
			if (existing_doc.IsObject() && existing_doc.MemberCount() > 0)
			{
				std::set<std::string> found_paths;
				for (const auto& fe : files_to_verify)
					found_paths.insert(fe.relative_path);

				for (auto it = existing_doc.MemberBegin(); it != existing_doc.MemberEnd(); ++it)
				{
					std::string key(it->name.GetString(), it->name.GetStringLength());
					std::string key_lower = utils::string::to_lower(key);
					std::string fname = key_lower;
					auto slash = fname.rfind('/');
					if (slash != std::string::npos) fname = fname.substr(slash + 1);

					bool belongs = false;
					if (include_base_files && is_base_file(fname))
					{
						belongs = true;
					}
					else
					{
						for (const auto& prefix : active_prefixes)
						{
							if (!prefix.empty() && fname.rfind(prefix, 0) == 0)
							{
								belongs = true;
								break;
							}
						}
					}
					if (belongs && found_paths.find(key) == found_paths.end())
					{
						missing_files++;
						problematic_files.push_back(key + " (missing)");
					}
				}
			}

			std::vector<std::string> missing_modes;
			for (const auto& [prefix, count] : prefix_file_counts)
			{
				if (count == 0)
				{
					std::string mode_name;
					if (prefix == "cp_") mode_name = "Campaign";
					else if (prefix == "mp_") mode_name = "Multiplayer";
					else if (prefix == "zm_") mode_name = "Zombies";
					else if (prefix == "base") mode_name = "Core/Common";
					else mode_name = prefix;
					missing_modes.push_back(mode_name);
					problematic_files.push_back(mode_name + " (not installed - 0 files found)");
				}
			}

			std::string result_msg;
			bool first_run = !existing_doc.IsObject() || existing_doc.MemberCount() == 0;

			bool all_new = (new_files == total_files - failed_files) && changed_files == 0 && verified_ok == 0;

			if (total_files == 0)
			{
				if (!missing_modes.empty())
				{
					result_msg = "No files found on disk.";
				}
				else if (missing_files > 0)
				{
					result_msg = std::to_string(missing_files) + " files missing from disk.";
				}
				else
				{
					result_msg = "No game files found for selected modes.";
				}
			}
			else if (first_run || all_new)
			{
				result_msg = "Baseline created: " + std::to_string(total_files) + " files indexed.";
			}
			else
			{
				result_msg = "Verified " + std::to_string(total_files) + " files: " +
					std::to_string(verified_ok) + " OK";
				if (changed_files > 0)
					result_msg += ", " + std::to_string(changed_files) + " changed";
				if (new_files > 0)
					result_msg += ", " + std::to_string(new_files) + " new";
				if (missing_files > 0)
					result_msg += ", " + std::to_string(missing_files) + " missing";
				if (failed_files > 0)
					result_msg += ", " + std::to_string(failed_files) + " failed";
			}
			if (!missing_modes.empty())
			{
				result_msg += " | Not installed: ";
				for (size_t i = 0; i < missing_modes.size(); i++)
				{
					if (i > 0) result_msg += ", ";
					result_msg += missing_modes[i];
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
			std::filesystem::path dir_path; // for mtime comparison
		};

		std::string workshop_list_json()
		{
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			std::filesystem::path base(cwd);

			std::vector<mod_item_info> items;

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
					mod_item_info item;
					item.name = doc["Title"].GetString();
					item.folder = entry.path().filename().string();
					item.type = type_label;
					item.dir_path = entry.path();
					if (doc.HasMember("PublisherID"))
					{
						const auto& pid = doc["PublisherID"];
						if (pid.IsString()) item.id = pid.GetString();
						else if (pid.IsInt64()) item.id = std::to_string(pid.GetInt64());
						else if (pid.IsUint64()) item.id = std::to_string(pid.GetUint64());
					}
					std::string image_path = find_mod_image_path(entry.path());
					if (!image_path.empty())
						item.image = path_to_file_url(image_path);
					else if (!item.id.empty())
						item.image = get_steam_workshop_preview_url(item.id);
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
				std::error_code ec;
				for (const auto& ws_entry : std::filesystem::directory_iterator(steam_ws, ec))
				{
					if (!ws_entry.is_directory()) continue;
					std::filesystem::path zone_json = ws_entry.path() / "zone" / "workshop.json";
					if (utils::io::file_exists(zone_json.string()))
					{
						std::string data;
						if (!utils::io::read_file(zone_json.string(), &data)) continue;
						rapidjson::Document doc;
						if (doc.Parse(data).HasParseError() || !doc.IsObject() || !doc.HasMember("Title") || !doc.HasMember("FolderName")) continue;
						mod_item_info item;
						item.name = doc["Title"].GetString();
						item.folder = ws_entry.path().filename().string();
						item.type = "map";
						item.source = "steam";
						item.path = ws_entry.path().string();
						item.dir_path = ws_entry.path();
						if (doc.HasMember("PublisherID"))
						{
							const auto& pid = doc["PublisherID"];
							if (pid.IsString()) item.id = pid.GetString();
							else if (pid.IsInt64()) item.id = std::to_string(pid.GetInt64());
							else if (pid.IsUint64()) item.id = std::to_string(pid.GetUint64());
						}
						std::string image_path = find_mod_image_path(ws_entry.path());
						if (!image_path.empty())
							item.image = path_to_file_url(image_path);
						else if (!item.id.empty())
							item.image = get_steam_workshop_preview_url(item.id);
						items.push_back(std::move(item));
						continue;
					}
					for (const auto& sub : std::filesystem::directory_iterator(ws_entry.path(), ec))
					{
						if (!sub.is_directory()) continue;
						zone_json = sub.path() / "zone" / "workshop.json";
						if (!utils::io::file_exists(zone_json.string())) continue;
						std::string data;
						if (!utils::io::read_file(zone_json.string(), &data)) continue;
						rapidjson::Document doc;
						if (doc.Parse(data).HasParseError() || !doc.IsObject() || !doc.HasMember("Title") || !doc.HasMember("FolderName")) continue;
						mod_item_info item;
						item.name = doc["Title"].GetString();
						item.folder = sub.path().filename().string();
						item.type = "map";
						item.source = "steam";
						item.path = sub.path().string();
						item.dir_path = sub.path();
						if (doc.HasMember("PublisherID"))
						{
							const auto& pid = doc["PublisherID"];
							if (pid.IsString()) item.id = pid.GetString();
							else if (pid.IsInt64()) item.id = std::to_string(pid.GetInt64());
							else if (pid.IsUint64()) item.id = std::to_string(pid.GetUint64());
						}
						std::string image_path = find_mod_image_path(sub.path());
						if (!image_path.empty())
							item.image = path_to_file_url(image_path);
						else if (!item.id.empty())
							item.image = get_steam_workshop_preview_url(item.id);
						items.push_back(std::move(item));
					}
				}
				}
			}

			std::vector<std::string> all_ids;
			for (const auto& it : items) {
				if (!it.id.empty()) all_ids.push_back(it.id);
			}
			auto update_times = workshop::batch_get_time_updated(all_ids);

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
				if (!it.path.empty()) { w.Key("path"); w.String(it.path.c_str()); }

				if (!it.id.empty()) {
					auto tu = update_times.find(it.id);
					if (tu != update_times.end() && tu->second > 0) {
						auto local_mtime = get_folder_mtime_epoch(it.dir_path);
						if (local_mtime > 0 && tu->second > local_mtime) {
							w.Key("needsUpdate"); w.Bool(true);
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
				if (verify_running.load()) return CComVariant("already_running");
				std::string modes = "all";
				if (!params.empty() && params[0].is_string())
				{
					modes = params[0].get_string();
					utils::string::trim(modes);
					if (modes.empty()) modes = "all";
				}
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