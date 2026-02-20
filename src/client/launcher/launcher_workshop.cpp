#include <std_include.hpp>
#include "html/html_frame.hpp"
#include "launcher_workshop.hpp"
#include "../game/game.hpp"
#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <regex>
#include <set>
#include <thread>
#include <utils/compression.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/nt.hpp>
#include <utils/string.hpp>
#include <sstream>
#include <iomanip>

namespace launcher::workshop
{
	std::chrono::steady_clock::time_point download_start_time;
	double mod_size = 0.0;

	bool clear_directory_contents(const std::filesystem::path& dir)
	{
		std::error_code ec;

		if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec))
		{
			return false;
		}

		for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
		{
			std::filesystem::remove_all(entry.path(), ec);

			if (ec)
			{
#ifdef _WIN32
				std::filesystem::permissions(
					entry.path(),
					std::filesystem::perms::owner_all,
					std::filesystem::perm_options::add,
					ec);

				std::filesystem::remove_all(entry.path(), ec);
#endif
			}
		}

		return true;
	}


	bool parse_steam_timestamp(	const std::string& line, std::chrono::system_clock::time_point& out_time)
	{
		if (line.size() < 21 || line[0] != '[')
			return false;

		std::tm tm{};
		std::istringstream ss(line.substr(1, 19));
		ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

		if (ss.fail())
			return false;

		std::time_t tt = std::mktime(&tm);
		if (tt == -1)
			return false;

		out_time = std::chrono::system_clock::from_time_t(tt);
		return true;
	}

	void monitor_initial_dump_phase(std::string workshop_id)
	{
		std::filesystem::path content_log_path = "./steamcmd/logs/content_log.txt";
		bool running = true;

		std::ifstream file(content_log_path, std::ios::in);
		if (!file.is_open())
			return;

		file.seekg(0, std::ios::end);

		const std::string base_pattern = "AppID 311210 update started";

		std::string line;

		while ( running)
		{
			std::streampos pos = file.tellg();

			if (!std::getline(file, line))
			{
				file.clear();
				file.seekg(pos);
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				continue;
			}

			if (line.find(base_pattern) == std::string::npos)
				continue;

			if (line.find("download 0/") == std::string::npos)
				continue;

			std::chrono::system_clock::time_point log_time;
			if (!parse_steam_timestamp(line, log_time))
				continue;

			auto now_sys = std::chrono::system_clock::now();
			auto now_steady = std::chrono::steady_clock::now();
			auto log_duration_since_now = log_time - now_sys;
			auto log_time_steady = now_steady + log_duration_since_now;

			if (log_time_steady >= download_start_time)
			{
				running = false;

				//mod_size = //save mod size before remove

				clear_directory_contents("./steamcmd/steamapps/workshop/downloads/311210/" + workshop_id);
				return;
			}
		}
	}

	void try_refresh_workshop_content()
	{
		try
		{
			game::Cbuf_AddText(0, "userContentReload\n");
			printf("Workshop items refreshed in-game.\n");
		}
		catch (...)
		{
			// Game not running yet, nothing to refresh
		}
	}

	namespace
	{
		constexpr const char* WORKSHOP_STATUS_IDLE = "";

		std::mutex workshop_status_mutex;
		std::string workshop_status_message = WORKSHOP_STATUS_IDLE;
		double workshop_progress_percent = 0.0;
		std::string workshop_progress_details = "";
		std::string workshop_download_folder = "";

		std::mutex workshop_download_mutex;
		PROCESS_INFORMATION workshop_download_process{};
		std::atomic<bool> workshop_cancel_requested{ false };
		std::atomic<bool> workshop_paused{ false };

		std::mutex workshop_browse_mutex;
		std::vector<std::string> workshop_browse_cache;
		std::atomic<bool> workshop_browse_loading{ false };

		std::string human_readable_size(std::uint64_t bytes);
		void save_workshop_backup(const std::string& json_data);

		std::string extract_workshop_id(const std::string& input)
		{
			std::string s = input;
			utils::string::trim(s);
			bool all_digits = true;
			for (char c : s) {
				if (!std::isdigit(static_cast<unsigned char>(c))) {
					all_digits = false;
					break;
				}
			}
			if (all_digits && !s.empty())
				return s;
			auto pos = s.find("id=");
			if (pos != std::string::npos) {
				pos += 3;
				auto end = s.find_first_not_of("0123456789", pos);
				if (end == std::string::npos)
					end = s.size();
				if (end > pos)
					return s.substr(pos, end - pos);
			}
			size_t last_num_start = std::string::npos;
			for (size_t i = 0; i < s.size(); ++i) {
				if (std::isdigit(static_cast<unsigned char>(s[i]))) {
					if (last_num_start == std::string::npos || (i > 0 && !std::isdigit(static_cast<unsigned char>(s[i - 1]))))
						last_num_start = i;
				}
			}
			if (last_num_start != std::string::npos) {
				size_t end = last_num_start;
				while (end < s.size() && std::isdigit(static_cast<unsigned char>(s[end])))
					++end;
				return s.substr(last_num_start, end - last_num_start);
			}
			return {};
		}

		void set_workshop_status(const std::string& msg, double progress = -1.0, const std::string& details = "")
		{
			std::lock_guard lock(workshop_status_mutex);
			workshop_status_message = msg;
			if (progress >= 0.0)
				workshop_progress_percent = progress;
			workshop_progress_details = details;
		}

		void reset_workshop_status()
		{
			std::lock_guard lock(workshop_status_mutex);
			workshop_status_message = WORKSHOP_STATUS_IDLE;
			workshop_progress_percent = 0.0;
			workshop_progress_details.clear();
			workshop_download_folder.clear();
		}

		constexpr const char* STEAM_WORKSHOP_API = "https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/";
		constexpr int BO3_APP_ID = 311210;

		std::uint64_t parse_human_size_to_bytes(const std::string& text)
		{
			std::smatch m;
			std::regex re(R"((\d+(?:\.\d+)?)\s*(B|KB|MB|GB|TB))", std::regex::icase);
			if (!std::regex_search(text, m, re) || m.size() < 3)
				return 0;
			const double value = std::stod(m[1].str());
			std::string unit = m[2].str();
			for (auto& c : unit)
				c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

			double mul = 1.0;
			if (unit == "KB")
				mul = 1024.0;
			else if (unit == "MB")
				mul = 1024.0 * 1024.0;
			else if (unit == "GB")
				mul = 1024.0 * 1024.0 * 1024.0;
			else if (unit == "TB")
				mul = 1024.0 * 1024.0 * 1024.0 * 1024.0;

			const auto bytes = value * mul;
			if (bytes <= 0.0)
				return 0;
			return static_cast<std::uint64_t>(bytes);
		}

		std::uint64_t scrape_workshop_file_size_bytes(const std::string& workshop_id)
		{
			try {
				utils::http::headers h;
				h["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
				h["Accept"] = "text/html";
				h["Accept-Language"] = "en-US,en;q=0.9";
				h["Referer"] = "https://steamcommunity.com/app/311210/workshop/";

				const auto url = "https://steamcommunity.com/sharedfiles/filedetails/?id=" + workshop_id + "&searchtext=";
				const auto resp = utils::http::get_data(url, h, {}, 2);
				if (!resp || resp->empty())
					return 0;

				const std::string& html = *resp;

				{
					std::regex re(R"(detailsStatRight[^>]*>\s*([\d,\.]+\s*(?:B|KB|MB|GB|TB))\s*<)", std::regex::icase);
					std::smatch m;
					if (std::regex_search(html, m, re) && m.size() >= 2) {
						std::string size_text = m[1].str();
						size_text.erase(std::remove(size_text.begin(), size_text.end(), ','), size_text.end());
						const auto bytes = parse_human_size_to_bytes(size_text);
						if (bytes > 0) return bytes;
					}
				}

				{
					std::regex re(R"(File\s*Size\s*<\/div>\s*<div[^>]*>([^<]+)<)", std::regex::icase);
					std::smatch m;
					if (std::regex_search(html, m, re) && m.size() >= 2) {
						std::string size_text = m[1].str();
						size_text.erase(std::remove(size_text.begin(), size_text.end(), ','), size_text.end());
						const auto bytes = parse_human_size_to_bytes(size_text);
						if (bytes > 0) return bytes;
					}
				}

				{
					std::regex re(R"(File\s*Size[^\d]*(\d+(?:[,.]\d+)?)\s*(B|KB|MB|GB|TB))", std::regex::icase);
					std::smatch m;
					if (std::regex_search(html, m, re) && m.size() >= 3) {
						std::string num = m[1].str();
						num.erase(std::remove(num.begin(), num.end(), ','), num.end());
						const auto bytes = parse_human_size_to_bytes(num + " " + m[2].str());
						if (bytes > 0) return bytes;
					}
				}

				return 0;
			}
			catch (...) {
				return 0;
			}
		}

		struct workshop_info {
			std::uint64_t file_size = 0;
			std::string title;
		};

		workshop_info get_steam_workshop_info(const std::string& workshop_id)
		{
			workshop_info info{};
			if (workshop_id.empty())
				return info;
			try {
				std::string body = "itemcount=1&publishedfileids[0]=" + workshop_id;
				std::optional<std::string> resp = utils::http::post_data(STEAM_WORKSHOP_API, body, 10);
				if (!resp || resp->empty())
					return info;

				rapidjson::Document doc;
				if (doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject())
					return info;
				auto resp_it = doc.FindMember("response");
				if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject())
					return info;
				auto details_it = resp_it->value.FindMember("publishedfiledetails");
				if (details_it == resp_it->value.MemberEnd() || !details_it->value.IsArray() || details_it->value.Empty())
					return info;
				const rapidjson::Value& first = details_it->value[0];
				if (!first.IsObject())
					return info;

				auto title_it = first.FindMember("title");
				if (title_it != first.MemberEnd() && title_it->value.IsString())
					info.title = title_it->value.GetString();

				auto size_it = first.FindMember("file_size");
				if (size_it != first.MemberEnd()) {
					if (size_it->value.IsUint64())
						info.file_size = size_it->value.GetUint64();
					else if (size_it->value.IsInt64())
						info.file_size = static_cast<std::uint64_t>(size_it->value.GetInt64());
					else if (size_it->value.IsUint())
						info.file_size = size_it->value.GetUint();
					else if (size_it->value.IsInt())
						info.file_size = static_cast<std::uint64_t>(size_it->value.GetInt());
					else if (size_it->value.IsString())
						info.file_size = static_cast<std::uint64_t>(std::strtoull(size_it->value.GetString(), nullptr, 10));
				}

				if (info.file_size == 0) {
					const auto scraped = scrape_workshop_file_size_bytes(workshop_id);
					if (scraped > 0)
						info.file_size = scraped;
				}
				return info;
			}
			catch (...) {
				const auto scraped = scrape_workshop_file_size_bytes(workshop_id);
				if (scraped > 0)
					info.file_size = scraped;
				return info;
			}
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
			}
			catch (...) { return 0; }
		}

		std::uint64_t compute_folder_size_bytes(const std::filesystem::path& folder)
		{
			std::error_code ec;
			if (!std::filesystem::exists(folder, ec))
				return 0;
			std::uint64_t total = 0;
			for (const auto& entry : std::filesystem::recursive_directory_iterator(folder, ec)) {
				if (ec)
					break;
				if (!entry.is_regular_file(ec))
					continue;
				total += static_cast<std::uint64_t>(std::filesystem::file_size(entry.path(), ec));
				if (ec)
					break;
			}
			return total;
		}

		bool copy_directory_recursive(const std::filesystem::path& from, const std::filesystem::path& to)
		{
			std::error_code ec;
			if (!std::filesystem::exists(from, ec))
				return false;
			std::filesystem::create_directories(to, ec);
			if (ec)
				return false;

			for (const auto& entry : std::filesystem::recursive_directory_iterator(from, ec)) {
				if (ec)
					return false;
				const auto rel = std::filesystem::relative(entry.path(), from, ec);
				if (ec)
					return false;
				const auto dest_path = to / rel;

				if (entry.is_directory(ec)) {
					std::filesystem::create_directories(dest_path, ec);
					if (ec)
						return false;
					continue;
				}
				if (entry.is_regular_file(ec)) {
					std::filesystem::create_directories(dest_path.parent_path(), ec);
					if (ec)
						return false;
					std::filesystem::copy_file(entry.path(), dest_path, std::filesystem::copy_options::overwrite_existing, ec);
					if (ec)
						return false;
				}
			}
			return true;
		}

		bool copy_directory_recursive_with_progress(const std::filesystem::path& from, const std::filesystem::path& to,
			const std::uint64_t total_bytes,
			const std::function<void(std::uint64_t)>& on_bytes_copied)
		{
			std::error_code ec;
			if (!std::filesystem::exists(from, ec))
				return false;
			std::filesystem::create_directories(to, ec);
			if (ec) {
				if (ec == std::errc::no_space_on_device) {
					set_workshop_status("Error: Disk full - cannot create destination folder.", 0.0,
						"Free up disk space and try again.");
				}
				return false;
			}

			std::uint64_t copied_bytes = 0;
			auto last_update = std::chrono::steady_clock::now();
			for (const auto& entry : std::filesystem::recursive_directory_iterator(from, ec)) {
				if (ec)
					return false;
				if (workshop_cancel_requested.load())
					return false;

				const auto rel = std::filesystem::relative(entry.path(), from, ec);
				if (ec)
					return false;
				const auto dest_path = to / rel;

				if (entry.is_directory(ec)) {
					std::filesystem::create_directories(dest_path, ec);
					if (ec)
						return false;
					continue;
				}
				if (entry.is_regular_file(ec)) {
					std::filesystem::create_directories(dest_path.parent_path(), ec);
					if (ec)
						return false;

					const auto file_size = static_cast<std::uint64_t>(std::filesystem::file_size(entry.path(), ec));
					if (ec)
						return false;
					std::filesystem::copy_file(entry.path(), dest_path, std::filesystem::copy_options::overwrite_existing, ec);
					if (ec) {
						if (ec == std::errc::no_space_on_device) {
							set_workshop_status("Error: Disk full during file copy.", 0.0,
								"Ran out of disk space while copying files. Free up space and try again.");
						}
						return false;
					}

					copied_bytes += file_size;
					const auto now = std::chrono::steady_clock::now();
					if (on_bytes_copied && (now - last_update) > std::chrono::milliseconds(250)) {
						last_update = now;
						on_bytes_copied(std::min(copied_bytes, total_bytes));
					}
				}
			}
			if (on_bytes_copied)
				on_bytes_copied(std::min(copied_bytes, total_bytes));
			return true;
		}

		std::string human_readable_size(std::uint64_t bytes)
		{
			const char* suffixes[] = { "B", "KB", "MB", "GB", "TB" };
			double value = static_cast<double>(bytes);
			int idx = 0;

			while (value >= 1024.0 && idx < 4) {
				value /= 1024.0;
				++idx;
			}

			char buf[64]{};
			std::snprintf(buf, sizeof(buf), "%.2f %s", value, suffixes[idx]);
			return buf;
		}

		std::string html_decode(const std::string& input)
		{
			std::string result = input;
			std::regex re("&#(\\d+);");
			std::smatch m;
			std::string::const_iterator searchStart(result.cbegin());
			std::string output;
			while (std::regex_search(searchStart, result.cend(), m, re)) {
				output.append(searchStart, m[0].first);
				int code = std::stoi(m[1].str());
				if (code >= 32 && code < 127) {
					output.push_back(static_cast<char>(code));
				}
				else {
					output.append(m[0].str());
				}
				searchStart = m[0].second;
			}
			output.append(searchStart, result.cend());
			result = output;

			size_t pos = 0;
			while ((pos = result.find("&quot;", pos)) != std::string::npos) {
				result.replace(pos, 6, "\"");
				pos += 1;
			}
			pos = 0;
			while ((pos = result.find("&amp;", pos)) != std::string::npos) {
				result.replace(pos, 5, "&");
				pos += 1;
			}
			pos = 0;
			while ((pos = result.find("&lt;", pos)) != std::string::npos) {
				result.replace(pos, 4, "<");
				pos += 1;
			}
			pos = 0;
			while ((pos = result.find("&gt;", pos)) != std::string::npos) {
				result.replace(pos, 4, ">");
				pos += 1;
			}
			return result;
		}

		std::string get_workshop_backup_path()
		{
			char cwd[MAX_PATH];
			GetCurrentDirectoryA(sizeof(cwd), cwd);
			return std::string(cwd) + "\\workshop_cache.json";
		}

		std::string get_default_workshop_cache()
		{
			return R"DEFCACHE([
{"id":"3662223649","title":"ZM_FATAL_ERROR:TOWER","imageUrl":"https://images.steamusercontent.com/ugc/14179339278271119524/2DF5D01901B5D9BEF6DDD1E647C22608939B5169/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"ZM_FATAL_ERROR:TOWER Difficulty: High. Panzer first appearance: Round 12."},
{"id":"3661484631","title":"Quadinfin 2 - Perkaholic Boogaloo","imageUrl":"https://images.steamusercontent.com/ugc/10776649294626077998/16BDBF8F64ACF11FA468563E948E11AE2C8493E9/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"An updated version of my original Quadinfin, a classic 4-window survival experience. Featuring: 4 windows, 60 Perks, Ultimis Crew, Timed Gameplay."},
{"id":"3660826489","title":"TORRENTE TOWER V1","imageUrl":"https://images.steamusercontent.com/ugc/17418759426979053188/54D8BAC3FC2D4C00DD6E154D4C7D24C8626600D3/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"TORRENTE TOWER - xkrauser7. Bienvenido a Torrente Tower, un mapa estilo torre de alta dificultad."},
{"id":"3660476314","title":"THE LAST CHIME","imageUrl":"https://images.steamusercontent.com/ugc/11594178117164307986/DCA415507CBF347F7FBD28C88D8DA7589FD1A00D/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"An old Western town once filled with life now filled with evil and destruction."},
{"id":"3659265140","title":"DROP TOWER[HARD]","imageUrl":"https://images.steamusercontent.com/ugc/11027918728522681235/3117DC469515C7BC384EBFC533212FC1839640E5/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Difficulty Rating: 5/8. FEATURES: BO2 Weapons, Buyable Ending, Timed Gameplay, Death Ceiling, Custom Weapons."},
{"id":"3659176950","title":"School of the Zombies","imageUrl":"https://community.akamai.steamstatic.com/public/images/sharedfiles/steam_workshop_default_image.png","description":"Welcome to the School of Zombies. Fight your way through the school."},
{"id":"3658478726","title":"RURAL FARM","imageUrl":"https://images.steamusercontent.com/ugc/14111859186847883556/A318E997E28889D2E13CC22A70E27205A679074B/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"A zombie map featuring BO6 & 7 perks, BO2 weapons."},
{"id":"3657900170","title":"Sol Badguy Character Mod","imageUrl":"https://images.steamusercontent.com/ugc/12589509590737188680/94B10B318EA8CE00591F5693E548A21FBA567FA5/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Sol Badguy character mod for BO3."},
{"id":"3657886915","title":"FACING'S FLAT MAP","imageUrl":"https://images.steamusercontent.com/ugc/12018625100457671518/44BDB2FE748DA8DB9BFCE21F7228C8B319A96E21/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Facing's Flat Map - 40 PERKS ON ONE MAP!"},
{"id":"3657549400","title":"Black Ops 6 + Black Ops 7 Characters mod","imageUrl":"https://images.steamusercontent.com/ugc/16216719299880445495/7D307C7D1930D0E94FC143C9F94A204DAE49F468/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Characters from Black Ops 6 and Black Ops 7."},
{"id":"3656779213","title":"Classic Viewmodel FOV (Console-Style) V1.1","imageUrl":"https://images.steamusercontent.com/ugc/11325191810539314643/2978155577D8FC00C85F175412BE5A41346ED978/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Adjusts viewmodel FOV to match console / other CoD titles."},
{"id":"3656583092","title":"BRODES' APARTMENT","imageUrl":"https://images.steamusercontent.com/ugc/12518569369083293956/A64E56DD1DF545F064DE3AEDEAAC6E9250F2AC2F/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"A small survival map based on a real life apartment."},
{"id":"3656248258","title":"THE WAITING ROOM ZOMBIES","imageUrl":"https://images.steamusercontent.com/ugc/9809675098278553771/5AB87DDAF33FF9D719B02E5CFBF6A861B61FCB9D/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Map based on the horror game THE WAITING ROOM."},
{"id":"3654882086","title":"GUN RANGE","imageUrl":"https://images.steamusercontent.com/ugc/9655108334957386121/B28AD6F81EBE204A6163585539ED8FD987902259/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"M9K & BF4 Weapons, Buyable Ending, Dogs, Ammomatic."},
{"id":"3653611984","title":"YELLOW BEAR 36","imageUrl":"https://images.steamusercontent.com/ugc/17056600832089406645/3DED6582B0083DCCD7DBA62E79B262D731628C79/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"BO4 Weapons, BO7 Perks, Fast Gameplay, Buyable Ending."},
{"id":"3653516165","title":"Grimsel","imageUrl":"https://community.akamai.steamstatic.com/public/images/sharedfiles/steam_workshop_default_image.png","description":"Fill the monkeys in front of the perks to unlock them. Survive as long as you can."},
{"id":"3653336011","title":"NUKETOWN 1925","imageUrl":"https://images.steamusercontent.com/ugc/9316046992592864423/346237A0877E77825E456BE391FDE570CD4E32E1/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Experience Nuketown 100 years before the earth was nuked."},
{"id":"3652765825","title":"the evil within - abandoned hospital challenge map","imageUrl":"https://images.steamusercontent.com/ugc/11917762213855472942/57418C1707A629A75E84CACDC8D47546D230CF0E/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"The evil within abandoned hospital challenge map."},
{"id":"3651819567","title":"1 Room Challenge","imageUrl":"https://images.steamusercontent.com/ugc/10061471386946354590/2C6C02235EDC10F5F731AA60BBCB3AD56F7F7B2A/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Limited Perks, Limited Box Spins, Limited playable area."},
{"id":"3650570722","title":"MELON 2","imageUrl":"https://images.steamusercontent.com/ugc/12435539783903330593/AE02D66C5B73447AEF87C09F6C97E36F767DB4E4/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"The great continuation of MELON with new and improved features."},
{"id":"3650197721","title":"STORMGATE FORTRESS","imageUrl":"https://images.steamusercontent.com/ugc/15043592501945790701/65896EBB850009B61053C2A4F0A2053A64E0207F/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"Explore the massive Stormgate Fortress. A very large Zombies experience."},
{"id":"3650139418","title":"Zombie Bodies Instant-Despawn","imageUrl":"https://images.steamusercontent.com/ugc/10562545484082102394/217C97EB583672FD71C2F158BD2A36C8609C04A3/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"A mod to remove corpses instantly."},
{"id":"3649285026","title":"HOGZMEADE ZOMBIES","imageUrl":"https://images.steamusercontent.com/ugc/9770236674819273014/98341B495C30B4D9B4A779C34939019E8A0D333A/?imw=200&imh=200&ima=fit&impolicy=Letterbox&imcolor=%23000000&letterbox=true","description":"The All Wizarding Village of Hogsmeade. 4 PERK LIMIT."}
])DEFCACHE";
		}

		std::string load_workshop_backup()
		{
			try {
				const auto backup_path = get_workshop_backup_path();
				if (!utils::io::file_exists(backup_path)) {
					auto default_cache = get_default_workshop_cache();
					save_workshop_backup(default_cache);
					return default_cache;
				}
				const auto content = utils::io::read_file(backup_path);
				if (!content.empty()) {
					rapidjson::Document doc;
					if (!doc.Parse(content.c_str()).HasParseError() && doc.IsArray()) {
						return content;
					}
				}
			}
			catch (...) {
			}
			return get_default_workshop_cache();
		}

		void save_workshop_backup(const std::string& json_data)
		{
			try {
				const auto backup_path = get_workshop_backup_path();
				utils::io::write_file(backup_path, json_data);
			}
			catch (...) {
			}
		}

		bool is_archive_item(const rapidjson::Value& item)
		{
			if (!item.IsObject())
				return false;
			auto title_it = item.FindMember("title");
			if (title_it != item.MemberEnd() && title_it->value.IsString()) {
				std::string title = title_it->value.GetString();
				if (title.find("**ARCHIVE**") != std::string::npos || title.find("ARCHIVE") == 0)
					return true;
			}
			auto desc_it = item.FindMember("description");
			if (desc_it != item.MemberEnd() && desc_it->value.IsString()) {
				std::string desc = desc_it->value.GetString();
				if (desc.find("THIS MAP IS AN ARCHIVE") != std::string::npos)
					return true;
			}
			return false;
		}

		std::string merge_workshop_items(const std::string& fetched_json, const std::string& backup_json)
		{
			try {
				rapidjson::Document fetched_doc;
				rapidjson::Document backup_doc;
				if (fetched_doc.Parse(fetched_json.c_str()).HasParseError() || !fetched_doc.IsArray()) {
					if (!backup_doc.Parse(backup_json.c_str()).HasParseError() && backup_doc.IsArray()) {
						return backup_json;
					}
					return "[]";
				}
				if (backup_doc.Parse(backup_json.c_str()).HasParseError() || !backup_doc.IsArray()) {
					rapidjson::StringBuffer buf;
					rapidjson::Writer<rapidjson::StringBuffer> w(buf);
					fetched_doc.Accept(w);
					return std::string(buf.GetString(), buf.GetSize());
				}

				std::set<std::string> seen_ids;
				rapidjson::StringBuffer buf;
				rapidjson::Writer<rapidjson::StringBuffer> w(buf);
				w.StartArray();

				for (auto& item : fetched_doc.GetArray()) {
					if (item.IsObject() && item.HasMember("id") && item["id"].IsString()) {
						if (is_archive_item(item))
							continue;
						std::string id = item["id"].GetString();
						if (seen_ids.find(id) == seen_ids.end()) {
							seen_ids.insert(id);
							item.Accept(w);
						}
					}
				}

				for (auto& item : backup_doc.GetArray()) {
					if (item.IsObject() && item.HasMember("id") && item["id"].IsString()) {
						if (is_archive_item(item))
							continue;
						std::string id = item["id"].GetString();
						if (seen_ids.find(id) == seen_ids.end()) {
							seen_ids.insert(id);
							item.Accept(w);
						}
					}
				}

				w.EndArray();
				return std::string(buf.GetString(), buf.GetSize());
			}
			catch (...) {
				return fetched_json;
			}
		}

		std::string extract_image_url_from_description(const std::string& description)
		{
			try {
				std::smatch m;
				std::regex url_regex(R"((https?://[^\s\"\'<>\)]+\.(?:jpg|png|jpeg|gif|webp)))", std::regex::icase);
				if (std::regex_search(description, m, url_regex)) {
					return m[1].str();
				}
			}
			catch (...) {
			}
			return "";
		}

		std::string url_encode(const std::string& value)
		{
			std::string result;
			result.reserve(value.size() * 3);
			for (unsigned char c : value) {
				if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
					result += static_cast<char>(c);
				else {
					char buf[4];
					std::snprintf(buf, sizeof(buf), "%%%02X", c);
					result += buf;
				}
			}
			return result;
		}

		std::vector<std::pair<std::string, int>> scrape_ids_and_ratings(const std::string& html)
		{
			std::vector<std::pair<std::string, int>> items;
			std::regex id_re(R"(sharedfile_(\d+))");
			std::regex star_re(R"((\d)-star)");

			struct id_pos { std::string id; size_t pos; };
			std::vector<id_pos> matches;
			std::set<std::string> page_seen;

			auto begin = std::sregex_iterator(html.begin(), html.end(), id_re);
			auto end = std::sregex_iterator();
			for (auto it = begin; it != end; ++it) {
				std::string id = (*it)[1].str();
				if (!id.empty() && !page_seen.count(id)) {
					page_seen.insert(id);
					matches.push_back({ id, static_cast<size_t>(it->position()) });
				}
			}

			for (size_t i = 0; i < matches.size(); ++i) {
				size_t start = matches[i].pos;
				size_t block_end = (i + 1 < matches.size()) ? matches[i + 1].pos : std::min(start + 5000, html.size());
				std::string block = html.substr(start, block_end - start);
				std::smatch star_match;
				int stars = 0;
				if (std::regex_search(block, star_match, star_re)) {
					stars = std::stoi(star_match[1].str());
				}
				items.push_back({ matches[i].id, stars });
			}

			return items;
		}

		std::int64_t parse_json_int64(const rapidjson::Value& v)
		{
			if (v.IsInt64()) return v.GetInt64();
			if (v.IsInt()) return v.GetInt();
			if (v.IsUint64()) return static_cast<std::int64_t>(v.GetUint64());
			if (v.IsUint()) return v.GetUint();
			if (v.IsString()) return static_cast<std::int64_t>(std::atoll(v.GetString()));
			return 0;
		}

		std::string search_workshop_by_name(const std::string& search_text)
		{
			std::string backup_json = load_workshop_backup();
			try {
				utils::http::headers h;
				h["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
				h["Accept"] = "text/html";
				h["Accept-Language"] = "en-US,en;q=0.9";
				h["Referer"] = "https://steamcommunity.com/app/311210/workshop/";

				std::vector<std::string> all_ids;
				std::set<std::string> seen_ids;
				std::map<std::string, int> item_ratings;
				const int max_pages = 5;
				const std::string encoded_query = url_encode(search_text);

				for (int page = 1; page <= max_pages && workshop_browse_loading; ++page) {
					try {
						std::string url = "https://steamcommunity.com/workshop/browse/?appid=311210&searchtext=" + encoded_query
							+ "&browsesort=textsearch&section=readytouseitems&actualsort=textsearch&p=" + std::to_string(page);
						const auto resp = utils::http::get_data(url, h, {}, 3);
						if (!resp || resp->empty())
							break;

						auto page_results = scrape_ids_and_ratings(*resp);
						int page_items = 0;
						for (const auto& pr : page_results) {
							if (pr.first.empty() || seen_ids.count(pr.first))
								continue;
							seen_ids.insert(pr.first);
							all_ids.push_back(pr.first);
							item_ratings[pr.first] = pr.second;
							page_items++;
						}
						if (page_items == 0)
							break;
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
					}
					catch (...) {
						break;
					}
				}

				if (all_ids.empty()) {
					return "[]";
				}

				rapidjson::StringBuffer buf;
				rapidjson::Writer<rapidjson::StringBuffer> w(buf);
				w.StartArray();

				const size_t batch_size = 20;
				for (size_t i = 0; i < all_ids.size() && workshop_browse_loading; i += batch_size) {
					try {
						const size_t batch_end = std::min(i + batch_size, all_ids.size());
						const int count = static_cast<int>(batch_end - i);

						std::string body = "itemcount=" + std::to_string(count);
						for (int j = 0; j < count; ++j)
							body += "&publishedfileids[" + std::to_string(j) + "]=" + all_ids[i + j];

						auto api_resp = utils::http::post_data(STEAM_WORKSHOP_API, body, 15);
						if (!api_resp || api_resp->empty())
							continue;

						rapidjson::Document doc;
						if (doc.Parse(api_resp->c_str()).HasParseError() || !doc.IsObject())
							continue;
						auto resp_it = doc.FindMember("response");
						if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject())
							continue;
						auto details_it = resp_it->value.FindMember("publishedfiledetails");
						if (details_it == resp_it->value.MemberEnd() || !details_it->value.IsArray())
							continue;

						for (auto& item : details_it->value.GetArray()) {
							if (!item.IsObject())
								continue;
							auto app_it = item.FindMember("consumer_app_id");
							if (app_it != item.MemberEnd()) {
								int app_id = 0;
								if (app_it->value.IsInt())
									app_id = app_it->value.GetInt();
								else if (app_it->value.IsInt64())
									app_id = static_cast<int>(app_it->value.GetInt64());
								else if (app_it->value.IsString())
									app_id = std::atoi(app_it->value.GetString());
								if (app_id != 0 && app_id != BO3_APP_ID)
									continue;
							}

							std::string id, title, description, imageUrl;
							auto id_it = item.FindMember("publishedfileid");
							if (id_it != item.MemberEnd() && id_it->value.IsString())
								id = id_it->value.GetString();
							if (id.empty())
								continue;

							auto title_it = item.FindMember("title");
							if (title_it != item.MemberEnd() && title_it->value.IsString())
								title = html_decode(title_it->value.GetString());

							auto desc_it = item.FindMember("description");
							if (desc_it != item.MemberEnd() && desc_it->value.IsString()) {
								description = desc_it->value.GetString();
								if (description.size() > 2000)
									description = description.substr(0, 2000) + "...";
							}

							auto img_it = item.FindMember("preview_url");
							if (img_it != item.MemberEnd() && img_it->value.IsString())
								imageUrl = img_it->value.GetString();
							if (imageUrl.empty())
								imageUrl = extract_image_url_from_description(description);

							std::int64_t subs = 0, favorites = 0;
							auto subs_it = item.FindMember("lifetime_subscriptions");
							if (subs_it != item.MemberEnd())
								subs = parse_json_int64(subs_it->value);
							auto fav_it = item.FindMember("lifetime_favorited");
							if (fav_it != item.MemberEnd())
								favorites = parse_json_int64(fav_it->value);

							std::uint64_t file_size = 0;
							auto fs_it = item.FindMember("file_size");
							if (fs_it != item.MemberEnd()) {
								if (fs_it->value.IsUint64()) file_size = fs_it->value.GetUint64();
								else if (fs_it->value.IsUint()) file_size = fs_it->value.GetUint();
								else if (fs_it->value.IsString()) file_size = std::strtoull(fs_it->value.GetString(), nullptr, 10);
							}

							int star_rating = item_ratings.count(id) ? item_ratings[id] : 0;

							w.StartObject();
							w.Key("id");
							w.String(id.c_str());
							w.Key("title");
							w.String(title.c_str());
							w.Key("description");
							w.String(description.c_str());
							w.Key("imageUrl");
							w.String(imageUrl.c_str());
							w.Key("starRating");
							w.Int(star_rating);
							w.Key("subs");
							w.Int64(subs);
							w.Key("favorites");
							w.Int64(favorites);
							if (file_size > 0) {
								w.Key("file_size");
								w.Uint64(file_size);
							}
							w.EndObject();
						}
						if (i + batch_size < all_ids.size())
							std::this_thread::sleep_for(std::chrono::milliseconds(50));
					}
					catch (...) {
						continue;
					}
				}

				w.EndArray();
				std::string fetched_json = std::string(buf.GetString(), buf.GetSize());

				std::string merged_json = merge_workshop_items(fetched_json, backup_json);
				save_workshop_backup(merged_json);

				return fetched_json;
			}
			catch (...) {
				return "[]";
			}
		}

		void workshop_search_fetch_thread(const std::string& search_text)
		{
			try {
				workshop_browse_loading = true;

				try {
					std::string cached = load_workshop_backup();
					if (!cached.empty() && cached != "[]") {
						std::lock_guard lock(workshop_browse_mutex);
						workshop_browse_cache.clear();
						workshop_browse_cache.push_back(cached);
					}
				}
				catch (...) {
				}

				std::string result = search_workshop_by_name(search_text);
				if (!result.empty() && result != "[]") {
					std::lock_guard lock(workshop_browse_mutex);
					workshop_browse_cache.clear();
					workshop_browse_cache.push_back(result);
				}
				workshop_browse_loading = false;
			}
			catch (...) {
				workshop_browse_loading = false;
			}
		}

		std::string fetch_all_workshop_items()
		{
			std::string backup_json = load_workshop_backup();
			try {
				utils::http::headers h;
				h["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36";
				h["Accept"] = "text/html";
				h["Accept-Language"] = "en-US,en;q=0.9";
				h["Referer"] = "https://steamcommunity.com/app/311210/workshop/";

				std::vector<std::string> all_ids;
				std::set<std::string> seen_ids;
				std::map<std::string, int> item_ratings;
				const int max_pages = 20;

				for (int page = 1; page <= max_pages && workshop_browse_loading; ++page) {
					try {
						std::string url = "https://steamcommunity.com/workshop/browse/?appid=311210&browsesort=mostrecent&section=readytouseitems&actualsort=mostrecent&p=" + std::to_string(page);
						const auto resp = utils::http::get_data(url, h, {}, 3);
						if (!resp || resp->empty())
							break;

						auto page_results = scrape_ids_and_ratings(*resp);
						int page_items = 0;
						for (const auto& pr : page_results) {
							if (pr.first.empty() || seen_ids.count(pr.first))
								continue;
							seen_ids.insert(pr.first);
							all_ids.push_back(pr.first);
							item_ratings[pr.first] = pr.second;
							page_items++;
						}
						if (page_items == 0)
							break;
						std::this_thread::sleep_for(std::chrono::milliseconds(300));
					}
					catch (...) {
						break;
					}
				}

				if (all_ids.empty()) {
					if (!backup_json.empty() && backup_json != "[]")
						return backup_json;
					return get_default_workshop_cache();
				}

				rapidjson::StringBuffer buf;
				rapidjson::Writer<rapidjson::StringBuffer> w(buf);
				w.StartArray();

				const size_t batch_size = 20;
				for (size_t i = 0; i < all_ids.size() && workshop_browse_loading; i += batch_size) {
					try {
						const size_t batch_end = std::min(i + batch_size, all_ids.size());
						const int count = static_cast<int>(batch_end - i);

						std::string body = "itemcount=" + std::to_string(count);
						for (int j = 0; j < count; ++j) {
							body += "&publishedfileids[" + std::to_string(j) + "]=" + all_ids[i + j];
						}

						auto api_resp = utils::http::post_data(STEAM_WORKSHOP_API, body, 15);
						if (!api_resp || api_resp->empty())
							continue;

						rapidjson::Document doc;
						if (doc.Parse(api_resp->c_str()).HasParseError() || !doc.IsObject())
							continue;

						auto resp_it = doc.FindMember("response");
						if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject())
							continue;

						auto details_it = resp_it->value.FindMember("publishedfiledetails");
						if (details_it == resp_it->value.MemberEnd() || !details_it->value.IsArray())
							continue;

						for (auto& item : details_it->value.GetArray()) {
							if (!item.IsObject())
								continue;

							auto app_it = item.FindMember("consumer_app_id");
							if (app_it != item.MemberEnd()) {
								int app_id = 0;
								if (app_it->value.IsInt())
									app_id = app_it->value.GetInt();
								else if (app_it->value.IsInt64())
									app_id = static_cast<int>(app_it->value.GetInt64());
								else if (app_it->value.IsString())
									app_id = std::atoi(app_it->value.GetString());
								if (app_id != 0 && app_id != BO3_APP_ID)
									continue;
							}

							std::string id, title, description, imageUrl;

							auto id_it = item.FindMember("publishedfileid");
							if (id_it != item.MemberEnd() && id_it->value.IsString())
								id = id_it->value.GetString();
							if (id.empty())
								continue;

							auto title_it = item.FindMember("title");
							if (title_it != item.MemberEnd() && title_it->value.IsString())
								title = html_decode(title_it->value.GetString());

							auto desc_it = item.FindMember("description");
							if (desc_it != item.MemberEnd() && desc_it->value.IsString()) {
								description = desc_it->value.GetString();
								if (description.size() > 2000)
									description = description.substr(0, 2000) + "...";
							}

							auto img_it = item.FindMember("preview_url");
							if (img_it != item.MemberEnd() && img_it->value.IsString())
								imageUrl = img_it->value.GetString();

							if (imageUrl.empty())
								imageUrl = extract_image_url_from_description(description);

							std::int64_t subs = 0, favorites = 0;
							auto subs_it = item.FindMember("lifetime_subscriptions");
							if (subs_it != item.MemberEnd())
								subs = parse_json_int64(subs_it->value);
							auto fav_it = item.FindMember("lifetime_favorited");
							if (fav_it != item.MemberEnd())
								favorites = parse_json_int64(fav_it->value);

							std::uint64_t file_size = 0;
							auto fs_it = item.FindMember("file_size");
							if (fs_it != item.MemberEnd()) {
								if (fs_it->value.IsUint64()) file_size = fs_it->value.GetUint64();
								else if (fs_it->value.IsUint()) file_size = fs_it->value.GetUint();
								else if (fs_it->value.IsString()) file_size = std::strtoull(fs_it->value.GetString(), nullptr, 10);
							}

							int star_rating = item_ratings.count(id) ? item_ratings[id] : 0;

							w.StartObject();
							w.Key("id");
							w.String(id.c_str());
							w.Key("title");
							w.String(title.c_str());
							w.Key("description");
							w.String(description.c_str());
							w.Key("imageUrl");
							w.String(imageUrl.c_str());
							w.Key("starRating");
							w.Int(star_rating);
							w.Key("subs");
							w.Int64(subs);
							w.Key("favorites");
							w.Int64(favorites);
							if (file_size > 0) {
								w.Key("file_size");
								w.Uint64(file_size);
							}
							w.EndObject();
						}

						if (i + batch_size < all_ids.size())
							std::this_thread::sleep_for(std::chrono::milliseconds(200));
					}
					catch (...) {
						continue;
					}
				}

				w.EndArray();
				std::string fetched_json = std::string(buf.GetString(), buf.GetSize());

				std::string merged_json = merge_workshop_items(fetched_json, backup_json);
				save_workshop_backup(merged_json);

				return merged_json;
			}
			catch (...) {
				if (!backup_json.empty() && backup_json != "[]")
					return backup_json;
				return get_default_workshop_cache();
			}
		}

		void workshop_browse_fetch_thread(int /*page_num*/)
		{
			try {
				workshop_browse_loading = true;

				try {
					std::string cached = load_workshop_backup();
					if (!cached.empty() && cached != "[]") {
						std::lock_guard lock(workshop_browse_mutex);
						workshop_browse_cache.clear();
						workshop_browse_cache.push_back(cached);
					}
				}
				catch (...) {
				}

				std::string result = fetch_all_workshop_items();
				{
					std::lock_guard lock(workshop_browse_mutex);
					workshop_browse_cache.clear();
					workshop_browse_cache.push_back(result);
				}
				workshop_browse_loading = false;
			}
			catch (...) {
				workshop_browse_loading = false;
			}
		}

		bool has_zone_content(const std::filesystem::path& dir)
		{
			std::error_code ec;
			if (!std::filesystem::exists(dir, ec)) return false;
			return compute_folder_size_bytes(dir) > 1024;
		}

		std::filesystem::path get_steam_workshop_content_path(const std::filesystem::path& game_path)
		{
			auto steamapps = game_path.parent_path().parent_path();
			auto ws_path = steamapps / "workshop" / "content" / "311210";
			std::error_code ec;
			if (std::filesystem::exists(ws_path, ec)) return ws_path;
			return {};
		}

		bool check_steam_ws_folder(const std::filesystem::path& steam_ws, const std::string& workshop_id)
		{
			if (steam_ws.empty()) return false;
			std::error_code ec;
			auto item_dir = steam_ws / workshop_id;
			if (!std::filesystem::exists(item_dir, ec)) return false;
			if (has_zone_content(item_dir / "zone")) return true;
			for (const auto& sub : std::filesystem::directory_iterator(item_dir, ec))
			{
				if (sub.is_directory(ec) && has_zone_content(sub.path() / "zone"))
					return true;
			}
			return false;
		}

		std::string find_installed_workshop_item(const std::filesystem::path& game_path, const std::string& workshop_id)
		{
			std::error_code ec;
			const char* parent_dirs[] = { "mods", "usermaps" };

			for (const auto& parent_name : parent_dirs)
			{
				std::filesystem::path candidate = game_path / parent_name / workshop_id / "zone";
				if (has_zone_content(candidate))
					return candidate.parent_path().string();
			}

			auto steam_ws = get_steam_workshop_content_path(game_path);
			if (check_steam_ws_folder(steam_ws, workshop_id))
				return (steam_ws / workshop_id).string();

			for (const auto& parent_name : parent_dirs)
			{
				std::filesystem::path parent = game_path / parent_name;
				if (!std::filesystem::exists(parent, ec)) continue;
				for (const auto& entry : std::filesystem::directory_iterator(parent, ec))
				{
					if (!entry.is_directory(ec)) continue;
					std::filesystem::path zone_dir = entry.path() / "zone";
					if (!std::filesystem::exists(zone_dir, ec)) continue;

					std::filesystem::path wsjson = zone_dir / "workshop.json";
					if (!std::filesystem::exists(wsjson, ec)) continue;
					std::string json_str;
					if (!utils::io::read_file(wsjson.string(), &json_str) || json_str.empty()) continue;
					rapidjson::Document doc;
					if (doc.Parse(json_str.c_str()).HasParseError() || !doc.IsObject()) continue;

					auto pfid = doc.FindMember("PublishedFileId");
					if (pfid != doc.MemberEnd() && pfid->value.IsString())
					{
						if (std::string(pfid->value.GetString()) == workshop_id)
						{
							if (has_zone_content(zone_dir))
								return entry.path().string();
						}
					}
					auto pubid = doc.FindMember("PublisherID");
					if (pubid != doc.MemberEnd() && pubid->value.IsString())
					{
						if (std::string(pubid->value.GetString()) == workshop_id)
						{
							if (has_zone_content(zone_dir))
								return entry.path().string();
						}
					}
				}
			}


			try {
				std::string body = "itemcount=1&publishedfileids[0]=" + workshop_id;
				auto api_resp = utils::http::post_data(STEAM_WORKSHOP_API, body, 8);
				if (api_resp && !api_resp->empty()) {
					rapidjson::Document api_doc;
					if (!api_doc.Parse(api_resp->c_str()).HasParseError() && api_doc.IsObject()) {
						auto ri = api_doc.FindMember("response");
						if (ri != api_doc.MemberEnd() && ri->value.IsObject()) {
							auto di = ri->value.FindMember("publishedfiledetails");
							if (di != ri->value.MemberEnd() && di->value.IsArray() && !di->value.Empty()) {
								const auto& item = di->value[0];

								std::string mod_type = "mod";
								auto ti = item.FindMember("tags");
								if (ti != item.MemberEnd() && ti->value.IsArray()) {
									for (auto it = ti->value.Begin(); it != ti->value.End(); ++it) {
										if (it->IsObject()) {
											auto tag_it = it->FindMember("tag");
											if (tag_it != it->MemberEnd() && tag_it->value.IsString()) {
												std::string t = tag_it->value.GetString();
												for (auto& c : t) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
												if (t == "map" || t == "maps" || t.find("map") != std::string::npos) {
													mod_type = "map";
													break;
												}
											}
										}
									}
								}

								std::string folder_name = workshop_id;

								auto title_it = item.FindMember("title");
								if (title_it != item.MemberEnd() && title_it->value.IsString()) {
									std::string title = title_it->value.GetString();
									std::string safe_name;
									for (char c : title) {
										if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '-' || c == '_')
											safe_name += c;
									}
									while (!safe_name.empty() && safe_name.back() == ' ') safe_name.pop_back();
									while (!safe_name.empty() && safe_name.front() == ' ') safe_name.erase(safe_name.begin());
									for (auto& c : safe_name) { if (c == ' ') c = '_'; }
									if (safe_name.length() > 60) safe_name = safe_name.substr(0, 60);
									if (!safe_name.empty()) {
										folder_name = safe_name;
									}
								}

								std::vector<std::string> names_to_check = { folder_name };
								if (folder_name != workshop_id) names_to_check.push_back(workshop_id);

								for (const auto& name : names_to_check) {
									for (const auto& parent_name : parent_dirs) {
										std::filesystem::path candidate = game_path / parent_name / name / "zone";
										if (has_zone_content(candidate))
											return candidate.parent_path().string();
									}
								}
							}
						}
					}
				}
			}
			catch (...) {}

			return "";
		}

		void workshop_download_thread(std::string workshop_id)
		{
			try
			{
				reset_workshop_status();
				workshop_cancel_requested = false;
				workshop_paused = false;
				set_workshop_status("Initializing...", -1.0, "Workshop ID: " + workshop_id);

				char cwd[MAX_PATH];
				GetCurrentDirectoryA(sizeof(cwd), cwd);
				std::filesystem::path game_path(cwd);

				{
					auto existing = find_installed_workshop_item(game_path, workshop_id);
					if (!existing.empty())
					{
						set_workshop_status("Already installed.", 100.0, "This workshop item is already installed at:\n" + existing + "\nRemove it first if you want to reinstall.");
						return;
					}
				}

				std::filesystem::path steamcmd_dir = game_path / "steamcmd";
				std::filesystem::path steamcmd_exe = steamcmd_dir / "steamcmd.exe";
				std::string steamcmd_dir_str = steamcmd_dir.string();

				if (!std::filesystem::exists(steamcmd_exe))
				{
					set_workshop_status("Downloading SteamCMD...", -1.0, "First-time setup - downloading from Steam CDN");

					std::error_code ec;
					std::filesystem::create_directories(steamcmd_dir, ec);
					if (ec)
					{
						set_workshop_status("Error: Cannot create steamcmd folder.", 0.0, ec.message());
						return;
					}

					const auto zip_data = utils::http::get_data("https://steamcdn-a.akamaihd.net/client/installer/steamcmd.zip", {}, {}, 3);
					if (!zip_data || zip_data->empty())
					{
						set_workshop_status("Error: Failed to download SteamCMD.", 0.0,	"Could not reach steamcdn-a.akamaihd.net. Check your internet connection.");
						return;
					}

					set_workshop_status("Extracting SteamCMD...", -1.0, "");
					try
					{
						auto files = utils::compression::zip::extract(*zip_data);
						if (files.empty())
						{
							set_workshop_status("Error: SteamCMD zip is empty or corrupt.", 0.0, "");
							return;
						}
						for (const auto& [name, data] : files)
						{
							auto dest = steamcmd_dir / name;
							std::filesystem::create_directories(dest.parent_path(), ec);
							utils::io::write_file(dest.string(), data, false);
						}
					}
					catch (const std::exception& ex)
					{
						set_workshop_status("Error: Failed to extract SteamCMD.", 0.0, ex.what());
						return;
					}

					if (!std::filesystem::exists(steamcmd_exe))
					{
						set_workshop_status("Error: SteamCMD extraction failed - steamcmd.exe not found.", 0.0, "");
						return;
					}
				}

				{
					std::error_code ec;
					const auto exe_size = std::filesystem::file_size(steamcmd_exe, ec);
					if (!ec && exe_size < 3 * 1024 * 1024)
					{
						set_workshop_status("Initializing SteamCMD (first run)...", -1.0, "SteamCMD is updating itself, this may take a minute");
						std::string init_cmd = "\"" + steamcmd_exe.string() + "\" +quit";
						STARTUPINFOA si{};
						PROCESS_INFORMATION pi{};
						si.cb = sizeof(si);
						si.dwFlags = STARTF_USESHOWWINDOW;
						si.wShowWindow = SW_HIDE;
						if (CreateProcessA(nullptr, const_cast<char*>(init_cmd.c_str()), nullptr, nullptr,	FALSE, 0, nullptr, steamcmd_dir_str.c_str(), &si, &pi))
						{
							WaitForSingleObject(pi.hProcess, 180000);
							TerminateProcess(pi.hProcess, 0);
							CloseHandle(pi.hProcess);
							CloseHandle(pi.hThread);
						}

						const auto new_size = std::filesystem::file_size(steamcmd_exe, ec);
						if (!ec && new_size < 3 * 1024 * 1024)
						{
							set_workshop_status("Error: SteamCMD failed to initialize.", 0.0, "The self-update may have failed. Try again or check your internet connection.");
							return;
						}
					}
				}

				set_workshop_status("Fetching file info...", -1.0, "Workshop ID: " + workshop_id);
				const auto ws_info = get_steam_workshop_info(workshop_id);
				const std::uint64_t expected_size = ws_info.file_size;
				const std::string workshop_title = ws_info.title.empty() ? ("Workshop #" + workshop_id) : ws_info.title;

				if (expected_size > 0)
				{
					ULARGE_INTEGER free_bytes_available{};
					ULARGE_INTEGER total_bytes{};
					ULARGE_INTEGER total_free_bytes{};
					const std::uint64_t required_space = expected_size * 2 + (512ULL * 1024 * 1024);
					if (GetDiskFreeSpaceExA(cwd, &free_bytes_available, &total_bytes, &total_free_bytes))
					{
						if (free_bytes_available.QuadPart < required_space)
						{
							set_workshop_status("Error: Not enough disk space.", 0.0, "Need ~" + human_readable_size(required_space) + " free, but only " + human_readable_size(free_bytes_available.QuadPart) + " available. Free up space and try again.");
							return;
						}
					}
				}

				std::filesystem::path content_path = steamcmd_dir / "steamapps" / "workshop" / "content" / "311210" / workshop_id;
				std::filesystem::path download_path = steamcmd_dir / "steamapps" / "workshop" / "downloads" / "311210" / workshop_id;
				std::filesystem::path alt_content_path = game_path / "steamapps" / "workshop" / "content" / "311210" / workshop_id;
				std::filesystem::path alt_download_path = game_path / "steamapps" / "workshop" / "downloads" / "311210" / workshop_id;

				std::string steamapps_folder = "./steamcmd/steamapps";

				if (std::filesystem::exists(steamapps_folder))
				{
					std::filesystem::remove_all(steamapps_folder);
					printf("Old steamapps folder removed successfully.\n");
				}

				std::string cmd_args = "+login anonymous +workshop_download_item 311210 " + workshop_id + " validate +quit";

				constexpr int MAX_ATTEMPTS = 20;
				constexpr int FAIL_THRESHOLD = 5;
				int attempt = 0;
				int fast_fail_count = 0;

				download_start_time = std::chrono::steady_clock::now();

				std::thread log_thread(monitor_initial_dump_phase, workshop_id);
				log_thread.detach();

				while (!std::filesystem::exists(content_path) && !std::filesystem::exists(alt_content_path) && !workshop_cancel_requested.load())
				{
					attempt++;
					if (attempt > MAX_ATTEMPTS)
					{
						set_workshop_status("Error: Download failed after " + std::to_string(MAX_ATTEMPTS) + " attempts.", 0.0,
							"SteamCMD could not complete the download.\n"
							"Checked: " + content_path.string() + "\n"
							"    and: " + alt_content_path.string() + "\n"
							"Try again or check your connection.");
						return;
					}

					if (fast_fail_count >= FAIL_THRESHOLD)
					{
						set_workshop_status("Resetting SteamCMD...", -1.0, "Too many quick failures (" + std::to_string(fast_fail_count) + "), resetting and retrying");
						std::error_code ec;
						for (const auto& dir_name : { "steamapps", "dumps", "logs", "depotcache", "appcache", "userdata" })
						{
							auto dir = steamcmd_dir / dir_name;
							if (std::filesystem::exists(dir, ec))
							{
								std::filesystem::remove_all(dir, ec);
							}
						}
						fast_fail_count = 0;
						std::this_thread::sleep_for(std::chrono::seconds(2));
					}

					std::string attempt_str = (attempt > 1) ? (" (attempt " + std::to_string(attempt) + ")") : "";
					set_workshop_status("Starting download: " + workshop_title + attempt_str, -1.0, expected_size > 0 ? ("File size: " + human_readable_size(expected_size)) : "Workshop ID: " + workshop_id);

					std::string full_cmd = "\"" + steamcmd_exe.string() + "\" " + cmd_args;

					HANDLE h_pipe_read = nullptr, h_pipe_write = nullptr;
					{
						SECURITY_ATTRIBUTES sa_pipe{};
						sa_pipe.nLength = sizeof(SECURITY_ATTRIBUTES);
						sa_pipe.bInheritHandle = TRUE;
						sa_pipe.lpSecurityDescriptor = nullptr;
						CreatePipe(&h_pipe_read, &h_pipe_write, &sa_pipe, 0);
						SetHandleInformation(h_pipe_read, HANDLE_FLAG_INHERIT, 0);
					}

					STARTUPINFOA si{};
					PROCESS_INFORMATION pi{};
					si.cb = sizeof(si);
					si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
					si.wShowWindow = SW_HIDE;
					si.hStdOutput = h_pipe_write;
					si.hStdError = h_pipe_write;

					if (!CreateProcessA(nullptr, const_cast<char*>(full_cmd.c_str()), nullptr, nullptr, TRUE, 0, nullptr, steamcmd_dir_str.c_str(), &si, &pi))
					{
						if (h_pipe_read) CloseHandle(h_pipe_read);
						if (h_pipe_write) CloseHandle(h_pipe_write);
						set_workshop_status("Error: Failed to start SteamCMD.", 0.0, "Attempt " + std::to_string(attempt));
						std::this_thread::sleep_for(std::chrono::seconds(2));
						continue;
					}
					if (h_pipe_write) { CloseHandle(h_pipe_write); h_pipe_write = nullptr; }
					{
						std::lock_guard plock(workshop_download_mutex);
						workshop_download_process = pi;
					}

					auto attempt_start = std::chrono::steady_clock::now();
					bool is_downloading = false;
					bool is_steamcmd_updating = false;
					auto last_tick = std::chrono::steady_clock::now();
					std::string last_speed_str;

					std::uint64_t net_bytes_prev = 0;
					bool net_baseline_set = false;
					auto download_phase_start = std::chrono::steady_clock::time_point{};
					bool warmup_phase = true;

					std::filesystem::path log_path = steamcmd_dir / "logs" / "workshop_log.txt";
					{
						std::error_code ec;
						std::filesystem::create_directories(log_path.parent_path(), ec);
						try
						{
							std::ofstream ofs(log_path.string(), std::ios::trunc);
						}
						catch (...)
						{
						}
					}

					for (;;)
					{
						if (workshop_cancel_requested.load())
						{
							TerminateProcess(pi.hProcess, 1);
							break;
						}

						if (workshop_paused.load())
						{
							set_workshop_status("Paused - " + workshop_title, -1.0, "Download paused. Click Resume to continue.");
							TerminateProcess(pi.hProcess, 1);
							// Wait for unpause or cancel
							while (workshop_paused.load() && !workshop_cancel_requested.load())
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(300));
							}
							break; // Break to outer loop which will restart SteamCMD
						}

						const DWORD wait = WaitForSingleObject(pi.hProcess, 1000);

						if (!is_downloading)
						{
							std::filesystem::path bootstrap_log = steamcmd_dir / "logs" / "bootstrap_log.txt";
							try
							{
								std::error_code ec;
								if (std::filesystem::exists(bootstrap_log, ec))
								{
									std::ifstream bsf(bootstrap_log.string());
									if (bsf.is_open())
									{
										std::string last_line;
										std::string line;
										while (std::getline(bsf, line))
										{
											if (!line.empty()) last_line = line;
										}
										if (last_line.find("Downloading update") != std::string::npos
											|| last_line.find("downloading") != std::string::npos)
										{
											is_steamcmd_updating = true;
											set_workshop_status("Updating SteamCMD..." + attempt_str, 0.0, "Please wait");
										}
										else if (is_steamcmd_updating)
										{
											is_steamcmd_updating = false;
										}
									}
								}
							}
							catch (...)
							{
							}
						}

						if (!is_downloading && !is_steamcmd_updating)
						{
							try
							{
								std::error_code ec;
								if (std::filesystem::exists(log_path, ec))
								{
									std::ifstream lf(log_path.string());
									if (lf.is_open())
									{
										std::string line;
										while (std::getline(lf, line))
										{
											std::string lower = line;
											for (auto& c : lower)
												c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

											if (lower.find("download item " + workshop_id) != std::string::npos)
											{
												is_downloading = true;
												last_tick = std::chrono::steady_clock::now();
												break;
											}
										}
									}
								}
							}
							catch (...)
							{
							}

							if (!is_downloading)
							{
								std::uint64_t early_bytes = 0;
								try
								{
									std::error_code fec;
									if (std::filesystem::exists(download_path, fec))
										early_bytes = compute_folder_size_bytes(download_path);
									if (early_bytes == 0 && std::filesystem::exists(alt_download_path, fec))
										early_bytes = compute_folder_size_bytes(alt_download_path);
								}
								catch (...) {}

								if (early_bytes > 4096)
								{
									is_downloading = true;
									last_tick = std::chrono::steady_clock::now();
								}
								else
								{
									auto elapsed = std::chrono::steady_clock::now() - download_start_time;
									auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
									int h = static_cast<int>(elapsed_sec / 3600);
									int m = static_cast<int>((elapsed_sec % 3600) / 60);
									int s = static_cast<int>(elapsed_sec % 60);
									char time_str[32];
									snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", h, m, s);

									set_workshop_status(
										is_steamcmd_updating ? ("Updating SteamCMD..." + attempt_str) : ("Waiting for SteamCMD..." + attempt_str),
										0.0,
										std::string("Elapsed: ") + time_str);
								}
							}
						}

						if (is_downloading)
						{

							if (warmup_phase && download_phase_start == std::chrono::steady_clock::time_point{})
							{
								download_phase_start = std::chrono::steady_clock::now();
							}

							std::uint64_t current_size = 0;
							std::string active_folder;
							{
								std::error_code fec;
								if (std::filesystem::exists(download_path, fec))
								{
									current_size = compute_folder_size_bytes(download_path);
									if (current_size > 0) active_folder = download_path.string();
								}
								if (current_size == 0 && std::filesystem::exists(content_path, fec))
								{
									current_size = compute_folder_size_bytes(content_path);
									if (current_size > 0) active_folder = content_path.string();
								}
								if (current_size == 0 && std::filesystem::exists(alt_download_path, fec))
								{
									current_size = compute_folder_size_bytes(alt_download_path);
									if (current_size > 0) active_folder = alt_download_path.string();
								}
								if (current_size == 0 && std::filesystem::exists(alt_content_path, fec))
								{
									current_size = compute_folder_size_bytes(alt_content_path);
									if (current_size > 0) active_folder = alt_content_path.string();
								}
							}

							{
								ULARGE_INTEGER free_avail{};
								if (GetDiskFreeSpaceExA(cwd, &free_avail, nullptr, nullptr))
								{
									if (free_avail.QuadPart < 100ULL * 1024 * 1024)
									{
										TerminateProcess(pi.hProcess, 1);
										set_workshop_status("Error: Disk nearly full during download.", 0.0,
											"Only " + human_readable_size(free_avail.QuadPart)
											+ " remaining. Free up disk space and try again.");
										CloseHandle(pi.hProcess);
										CloseHandle(pi.hThread);
										return;
									}
								}
							}


							std::uint64_t net_bytes_now = 0;
							{
								MIB_IF_TABLE2* if_table = nullptr;
								if (GetIfTable2(&if_table) == NO_ERROR && if_table)
								{
									for (ULONG i = 0; i < if_table->NumEntries; i++)
									{
										net_bytes_now += if_table->Table[i].InOctets;
									}
									FreeMibTable(if_table);
								}
							}

							if (!net_baseline_set)
							{
								net_bytes_prev = net_bytes_now;
								net_baseline_set = true;
							}

							std::uint64_t net_delta = 0;
							if (net_bytes_now >= net_bytes_prev)
							{
								net_delta = net_bytes_now - net_bytes_prev;
							}
							net_bytes_prev = net_bytes_now;

							auto dl_elapsed = std::chrono::steady_clock::now() - download_phase_start;
							auto dl_elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(dl_elapsed).count();
							if (warmup_phase && dl_elapsed_sec >= 10) warmup_phase = false;

							double percent = -1.0;
							std::uint64_t display_size = current_size;
							const std::uint64_t effective_expected = expected_size > 0 ? expected_size : 0;

							if (effective_expected > 0 && display_size > effective_expected)
								display_size = effective_expected;

							if (!warmup_phase)
							{
								if (effective_expected > 0 && current_size > 0)
								{
									percent = (static_cast<double>(display_size) / static_cast<double>(effective_expected)) * 100.0;
								}
								else if (current_size > 0)
								{
									const double mb = static_cast<double>(current_size) / (1024.0 * 1024.0);
									percent = (mb / (mb + 2000.0)) * 95.0;
								}
								else
								{
									percent = 0.1;
								}
								if (percent > 99.0) percent = 99.0;
								if (percent < 0.1 && current_size > 0) percent = 0.1;
							}


							const auto now = std::chrono::steady_clock::now();
							const auto dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick).count();
							if (dt_ms > 0 && net_delta > 0)
							{
								const double bytes_per_sec = (static_cast<double>(net_delta) * 1000.0) / static_cast<double>(dt_ms);
								if (bytes_per_sec > 0.0)
								{
									last_speed_str = human_readable_size(static_cast<std::uint64_t>(bytes_per_sec)) + "/s";
								}
							}
							last_tick = now;

							auto elapsed = std::chrono::steady_clock::now() - download_start_time;
							auto elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
							int eh = static_cast<int>(elapsed_sec / 3600);
							int em = static_cast<int>((elapsed_sec % 3600) / 60);
							int es = static_cast<int>(elapsed_sec % 60);
							char time_str[32];
							snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", eh, em, es);

							std::string details;
							if (display_size > 0)
							{
								details = human_readable_size(display_size);
								if (expected_size > 0)
									details += " / " + human_readable_size(expected_size);
							}
							else if (expected_size > 0)
							{
								details = "0 / " + human_readable_size(expected_size);
							}
							if (!last_speed_str.empty())
							{
								if (!details.empty()) details += " | ";
								details += last_speed_str;
							}
							details += " | Elapsed: " + std::string(time_str);
							if (attempt > 1) {
								details += " | Attempt " + std::to_string(attempt);
							}

							set_workshop_status("Downloading " + workshop_title + "...", percent, details);

							if (!active_folder.empty()) {
								std::lock_guard lock(workshop_status_mutex);
								workshop_download_folder = active_folder;
							}
						}

						if (wait == WAIT_OBJECT_0 || wait == WAIT_FAILED) {
							break;
						}
					}

					DWORD exit_code = 0;
					GetExitCodeProcess(pi.hProcess, &exit_code);
					{
						std::lock_guard plock(workshop_download_mutex);
						if (workshop_download_process.hProcess == pi.hProcess) {
							workshop_download_process = {};
						}
					}
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);

					if (workshop_cancel_requested.load()) {
						set_workshop_status("Canceled.", 0.0, "");
						return;
					}

					// If paused, don't count this as a failed attempt - just loop back
					if (workshop_paused.load()) {
						attempt--; // Don't consume an attempt for pause
						continue;
					}

					if (std::filesystem::exists(content_path)) {
						const auto content_size = compute_folder_size_bytes(content_path);
						if (content_size > 1024) {
							break;
						}

						std::error_code rm_ec;
						std::filesystem::remove_all(content_path, rm_ec);
					}

					{
						std::error_code ec;
						if (std::filesystem::exists(alt_content_path, ec) && compute_folder_size_bytes(alt_content_path) > 1024) {
							content_path = alt_content_path;
							break;
						}
					}

					auto attempt_elapsed = std::chrono::steady_clock::now() - attempt_start;
					auto attempt_sec = std::chrono::duration_cast<std::chrono::seconds>(attempt_elapsed).count();
					if (attempt_sec < 15) {
						fast_fail_count++;
					}

					set_workshop_status("SteamCMD exited without completing" + std::string(attempt < MAX_ATTEMPTS ? ", retrying..." : ""),
						0.0,
						"Exit code: " + std::to_string(exit_code) + " | Attempt " + std::to_string(attempt) + "/" + std::to_string(MAX_ATTEMPTS)
						+ " | Quick fails: " + std::to_string(fast_fail_count));

					std::this_thread::sleep_for(std::chrono::seconds(2));
				}

				if (workshop_cancel_requested.load()) {
					set_workshop_status("Canceled.", 0.0, "");
					return;
				}

				set_workshop_status("Verifying download...", 99.9, "");
				if (!std::filesystem::exists(content_path)) {
					std::error_code ec;
					if (std::filesystem::exists(alt_content_path, ec) && compute_folder_size_bytes(alt_content_path) > 1024) {
						content_path = alt_content_path;
					}
					else {
						set_workshop_status("Error: Download failed - content folder not found.", 0.0,
							"SteamCMD did not download to the expected path.\n"
							"Checked: " + content_path.string() + "\n"
							"    and: " + alt_content_path.string());
						return;
					}
				}
				{
					const auto dl_size = compute_folder_size_bytes(content_path);
					if (dl_size < 1024) {
						set_workshop_status("Error: Download is empty or incomplete.", 0.0,
							"SteamCMD created the folder but no files were downloaded (" + human_readable_size(dl_size) + ").\n"
							"Path: " + content_path.string() + "\n"
							"Try again or check your internet connection.");
						return;
					}
				}

				std::string mod_type = "mod";
				std::string folder_name = workshop_id;

				std::string json_path = (content_path / "workshop.json").string();
				std::string json_str;
				bool has_workshop_json = false;
				if (utils::io::read_file(json_path, &json_str) && !json_str.empty()) {
					rapidjson::Document doc;
					if (!doc.Parse(json_str).HasParseError() && doc.IsObject()) {
						if (doc.HasMember("Type") && doc["Type"].IsString()) {
							mod_type = doc["Type"].GetString();
						}
						if (doc.HasMember("FolderName") && doc["FolderName"].IsString()) {
							std::string fn = doc["FolderName"].GetString();
							if (!fn.empty()) {
								folder_name = fn;
							}
						}
						has_workshop_json = true;
					}
				}

				for (auto& c : mod_type) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
				if (mod_type != "mod" && mod_type != "map") {
					mod_type = "mod";
				}

				if (!has_workshop_json) {
					try {
						std::string body = "itemcount=1&publishedfileids[0]=" + workshop_id;
						auto api_resp = utils::http::post_data(STEAM_WORKSHOP_API, body, 10);
						if (api_resp && !api_resp->empty()) {
							rapidjson::Document api_doc;
							if (!api_doc.Parse(api_resp->c_str()).HasParseError() && api_doc.IsObject()) {
								auto ri = api_doc.FindMember("response");
								if (ri != api_doc.MemberEnd() && ri->value.IsObject()) {
									auto di = ri->value.FindMember("publishedfiledetails");
									if (di != ri->value.MemberEnd() && di->value.IsArray() && !di->value.Empty()) {
										const auto& item = di->value[0];
										auto ti = item.FindMember("tags");
										if (ti != item.MemberEnd() && ti->value.IsArray()) {
											for (auto it = ti->value.Begin(); it != ti->value.End(); ++it) {
												if (it->IsObject()) {
													auto tag_it = it->FindMember("tag");
													if (tag_it != it->MemberEnd() && tag_it->value.IsString()) {
														std::string t = tag_it->value.GetString();
														for (auto& c : t) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
														if (t == "map" || t == "maps" || t.find("map") != std::string::npos) {
															mod_type = "map";
															break;
														}
													}
												}
											}
										}

										if (mod_type != "map") {
											auto title_it = item.FindMember("title");
											if (title_it != item.MemberEnd() && title_it->value.IsString()) {
												std::string title = title_it->value.GetString();
												std::string safe_name;
												for (char c : title) {
													if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '-' || c == '_') {
														safe_name += c;
													}
												}
												while (!safe_name.empty() && safe_name.back() == ' ') safe_name.pop_back();
												while (!safe_name.empty() && safe_name.front() == ' ') safe_name.erase(safe_name.begin());
												for (auto& c : safe_name) { if (c == ' ') c = '_'; }
												if (safe_name.length() > 60) safe_name = safe_name.substr(0, 60);
												if (!safe_name.empty()) {
													folder_name = safe_name;
												}
											}
										}
									}
								}
							}
						}
					}
					catch (...) {}
				}

				set_workshop_status("Installing files...", 99.9,
					"Type: " + mod_type + " | Folder: " + folder_name);

				std::filesystem::path dest_parent = (mod_type == "mod") ? (game_path / "mods") : (game_path / "usermaps");
				std::filesystem::path dest = dest_parent / folder_name / "zone";
				std::error_code ec;
				std::filesystem::create_directories(dest, ec);
				if (ec) {
					set_workshop_status("Error: Cannot create destination folder.", 0.0,
						"Failed to create: " + dest.string() + "\n" + ec.message());
					return;
				}

				const std::uint64_t install_total = compute_folder_size_bytes(content_path);
				set_workshop_status("Copying files...", 99.0,
					install_total > 0 ? ("0 / " + human_readable_size(install_total)) : std::string("Preparing..."));
				const bool copied = copy_directory_recursive_with_progress(
					content_path, dest, install_total,
					[&](std::uint64_t copied_bytes) {
						if (install_total > 0) {
							double p = 99.0 + (static_cast<double>(copied_bytes) / static_cast<double>(install_total)) * 0.99;
							if (p > 99.99)
								p = 99.99;
							set_workshop_status("Copying files...", p,
								human_readable_size(copied_bytes) + " / " + human_readable_size(install_total));
						}
						else {
							set_workshop_status("Copying files...", 99.5, human_readable_size(copied_bytes));
						}
					});
				if (!copied) {
					if (workshop_cancel_requested.load()) {
						set_workshop_status("Canceled.", 0.0, "");
						return;
					}
					set_workshop_status("Error: Failed to copy downloaded files.", 0.0,
						"Source: " + content_path.string() + "\n"
						"Destination: " + dest.string());
					return;
				}

				{
					const auto dest_size = compute_folder_size_bytes(dest);
					if (dest_size < 1024) {
						set_workshop_status("Error: Copy completed but destination is empty.", 0.0,
							"Source had files but nothing was copied to: " + dest.string() + "\n"
							"Source size: " + human_readable_size(install_total));
						return;
					}
				}

				{
					std::error_code cleanup_ec;
					if (std::filesystem::exists(content_path, cleanup_ec))
						std::filesystem::remove_all(content_path, cleanup_ec);
					if (std::filesystem::exists(download_path, cleanup_ec))
						std::filesystem::remove_all(download_path, cleanup_ec);
					if (std::filesystem::exists(alt_content_path, cleanup_ec))
						std::filesystem::remove_all(alt_content_path, cleanup_ec);
					if (std::filesystem::exists(alt_download_path, cleanup_ec))
						std::filesystem::remove_all(alt_download_path, cleanup_ec);
				}

				try {
					std::filesystem::path ws_json_path = dest / "workshop.json";
					std::string ws_json_str;
					rapidjson::Document ws_doc;
					bool need_write = false;

					if (utils::io::read_file(ws_json_path.string(), &ws_json_str) && !ws_json_str.empty()) {
						if (!ws_doc.Parse(ws_json_str.c_str()).HasParseError() && ws_doc.IsObject()) {
							auto pfid = ws_doc.FindMember("PublishedFileId");
							if (pfid == ws_doc.MemberEnd() || !pfid->value.IsString()
								|| std::string(pfid->value.GetString()) != workshop_id) {
								if (pfid != ws_doc.MemberEnd()) {
									pfid->value.SetString(workshop_id.c_str(),
										static_cast<rapidjson::SizeType>(workshop_id.size()), ws_doc.GetAllocator());
								}
								else {
									ws_doc.AddMember(
										rapidjson::Value("PublishedFileId", ws_doc.GetAllocator()),
										rapidjson::Value(workshop_id.c_str(),
											static_cast<rapidjson::SizeType>(workshop_id.size()), ws_doc.GetAllocator()),
										ws_doc.GetAllocator());
								}
								need_write = true;
							}
						}
						else {
							ws_doc.SetObject();
							ws_doc.AddMember(
								rapidjson::Value("PublishedFileId", ws_doc.GetAllocator()),
								rapidjson::Value(workshop_id.c_str(),
									static_cast<rapidjson::SizeType>(workshop_id.size()), ws_doc.GetAllocator()),
								ws_doc.GetAllocator());
							need_write = true;
						}
					}
					else {
						ws_doc.SetObject();
						ws_doc.AddMember(
							rapidjson::Value("PublishedFileId", ws_doc.GetAllocator()),
							rapidjson::Value(workshop_id.c_str(),
								static_cast<rapidjson::SizeType>(workshop_id.size()), ws_doc.GetAllocator()),
							ws_doc.GetAllocator());
						need_write = true;
					}

					if (need_write) {
						rapidjson::StringBuffer sb;
						rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
						ws_doc.Accept(writer);
						utils::io::write_file(ws_json_path.string(), std::string(sb.GetString(), sb.GetSize()), false);
					}
				}
				catch (...) {
				}

				{
					bool has_zone_files = false;
					std::error_code verify_ec;
					for (const auto& entry : std::filesystem::recursive_directory_iterator(dest, verify_ec)) {
						if (verify_ec) break;
						if (!entry.is_regular_file(verify_ec)) continue;
						const auto ext = entry.path().extension().string();
						if (ext == ".ff" || ext == ".sabl" || ext == ".sabs" || ext == ".pak" || ext == ".xpak") {
							has_zone_files = true;
							break;
						}
					}
					if (!has_zone_files) {
						std::uint64_t dest_size = compute_folder_size_bytes(dest);
						if (dest_size < 1024) {
							set_workshop_status("Error: No game files found in download.", 0.0,
								"The workshop item downloaded but contained no usable zone files.\n"
								"Installed to: " + dest.string() + "\n"
								"This item may be corrupt or incompatible.");
							return;
						}
						set_workshop_status("Done! Workshop item installed.", 100.0,
							mod_type + " \"" + folder_name + "\" installed (" + human_readable_size(dest_size) + ") - no .ff files found, may need manual setup.\n"
							"Installed to: " + dest.string());
						try_refresh_workshop_content();
						return;
					}
				}

				{
					std::uint64_t final_size = compute_folder_size_bytes(dest);
					set_workshop_status("Done! Workshop item installed successfully.", 100.0,
						mod_type + " \"" + folder_name + "\" is ready to use (" + human_readable_size(final_size) + ").\n"
						"Installed to: " + dest.string());
					try_refresh_workshop_content();
				}
			}
			catch (const std::filesystem::filesystem_error& fse) {
				std::string detail = fse.what();
				if (detail.find("space") != std::string::npos || detail.find("disk") != std::string::npos
					|| fse.code() == std::errc::no_space_on_device) {
					set_workshop_status("Error: Disk full.", 0.0,
						"Ran out of disk space. Free up space and try again.");
				}
				else {
					set_workshop_status("Error: File system error.", 0.0, detail);
				}
			}
			catch (const std::exception& ex) {
				set_workshop_status("Error: Workshop download failed.", 0.0, ex.what());
			}
			catch (...) {
				set_workshop_status("Error: Workshop download crashed.", 0.0, "");
			}
		}
	}

	std::map<std::string, std::uint64_t> batch_get_time_updated(const std::vector<std::string>& ids)
	{
		std::map<std::string, std::uint64_t> result;
		if (ids.empty()) return result;
		try {
			std::string body = "itemcount=" + std::to_string(ids.size());
			for (size_t i = 0; i < ids.size(); i++)
				body += "&publishedfileids[" + std::to_string(i) + "]=" + ids[i];

			auto resp = utils::http::post_data(
				"https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/", body, 15);
			if (!resp || resp->empty()) return result;

			rapidjson::Document doc;
			if (doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject()) return result;
			auto resp_it = doc.FindMember("response");
			if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject()) return result;
			auto details_it = resp_it->value.FindMember("publishedfiledetails");
			if (details_it == resp_it->value.MemberEnd() || !details_it->value.IsArray()) return result;

			for (rapidjson::SizeType i = 0; i < details_it->value.Size(); i++) {
				const auto& item = details_it->value[i];
				if (!item.IsObject()) continue;
				std::string item_id;
				auto id_it = item.FindMember("publishedfileid");
				if (id_it != item.MemberEnd() && id_it->value.IsString())
					item_id = id_it->value.GetString();
				if (item_id.empty()) continue;

				auto tu_it = item.FindMember("time_updated");
				if (tu_it != item.MemberEnd()) {
					std::uint64_t ts = 0;
					if (tu_it->value.IsUint64()) ts = tu_it->value.GetUint64();
					else if (tu_it->value.IsInt64()) ts = static_cast<std::uint64_t>(tu_it->value.GetInt64());
					else if (tu_it->value.IsUint()) ts = tu_it->value.GetUint();
					else if (tu_it->value.IsInt()) ts = static_cast<std::uint64_t>(tu_it->value.GetInt());
					if (ts > 0) result[item_id] = ts;
				}
			}
		}
		catch (...) {}
		return result;
	}

	std::map<std::string, workshop_item_meta> batch_get_workshop_meta(const std::vector<std::string>& ids)
	{
		std::map<std::string, workshop_item_meta> result;
		if (ids.empty()) return result;
		try {
			std::string body = "itemcount=" + std::to_string(ids.size());
			for (size_t i = 0; i < ids.size(); i++)
				body += "&publishedfileids[" + std::to_string(i) + "]=" + ids[i];

			auto resp = utils::http::post_data(
				"https://api.steampowered.com/ISteamRemoteStorage/GetPublishedFileDetails/v1/", body, 15);
			if (!resp || resp->empty()) return result;

			rapidjson::Document doc;
			if (doc.Parse(resp->c_str()).HasParseError() || !doc.IsObject()) return result;
			auto resp_it = doc.FindMember("response");
			if (resp_it == doc.MemberEnd() || !resp_it->value.IsObject()) return result;
			auto details_it = resp_it->value.FindMember("publishedfiledetails");
			if (details_it == resp_it->value.MemberEnd() || !details_it->value.IsArray()) return result;

			for (rapidjson::SizeType i = 0; i < details_it->value.Size(); i++) {
				const auto& item = details_it->value[i];
				if (!item.IsObject()) continue;
				std::string item_id;
				auto id_it = item.FindMember("publishedfileid");
				if (id_it != item.MemberEnd() && id_it->value.IsString())
					item_id = id_it->value.GetString();
				if (item_id.empty()) continue;

				workshop_item_meta meta{};

				auto tu_it = item.FindMember("time_updated");
				if (tu_it != item.MemberEnd()) {
					if (tu_it->value.IsUint64()) meta.time_updated = tu_it->value.GetUint64();
					else if (tu_it->value.IsUint()) meta.time_updated = tu_it->value.GetUint();
				}

				auto fs_it = item.FindMember("file_size");
				if (fs_it != item.MemberEnd()) {
					if (fs_it->value.IsUint64()) meta.file_size = fs_it->value.GetUint64();
					else if (fs_it->value.IsUint()) meta.file_size = fs_it->value.GetUint();
					else if (fs_it->value.IsString()) meta.file_size = std::strtoull(fs_it->value.GetString(), nullptr, 10);
				}

				auto desc_it = item.FindMember("description");
				if (desc_it != item.MemberEnd() && desc_it->value.IsString())
					meta.description = desc_it->value.GetString();

				auto prev_it = item.FindMember("preview_url");
				if (prev_it != item.MemberEnd() && prev_it->value.IsString())
					meta.preview_url = prev_it->value.GetString();

				auto subs_it = item.FindMember("subscriptions");
				if (subs_it != item.MemberEnd()) {
					if (subs_it->value.IsInt64()) meta.subs = subs_it->value.GetInt64();
					else if (subs_it->value.IsInt()) meta.subs = subs_it->value.GetInt();
				}

				auto fav_it = item.FindMember("favorited");
				if (fav_it != item.MemberEnd()) {
					if (fav_it->value.IsInt64()) meta.favorites = fav_it->value.GetInt64();
					else if (fav_it->value.IsInt()) meta.favorites = fav_it->value.GetInt();
				}

				result[item_id] = std::move(meta);
			}
		}
		catch (...) {}
		return result;
	}

	void register_callbacks(html_frame* frame)
	{
		frame->register_callback(
			"workshopGetStatus", [](const std::vector<html_argument>& /*params*/) -> CComVariant {
				std::lock_guard lock(workshop_status_mutex);
				rapidjson::StringBuffer buf;
				rapidjson::Writer<rapidjson::StringBuffer> w(buf);
				w.StartObject();
				w.Key("message");
				w.String(workshop_status_message.c_str());
				w.Key("progress");
				w.Double(workshop_progress_percent);
				w.Key("details");
				w.String(workshop_progress_details.c_str());
				w.Key("downloadFolder");
				w.String(workshop_download_folder.c_str());
				w.Key("paused");
				w.Bool(workshop_paused.load());
				w.EndObject();
				return CComVariant(std::string(buf.GetString(), buf.GetSize()).c_str());
			});

		frame->register_callback(
			"workshopCheckInstalled", [](const std::vector<html_argument>& params) -> CComVariant {
				if (params.empty() || !params[0].is_string())
					return CComVariant("");
				auto id = extract_workshop_id(params[0].get_string());
				if (id.empty()) return CComVariant("");
				char cwd[MAX_PATH];
				GetCurrentDirectoryA(sizeof(cwd), cwd);
				std::filesystem::path game_path(cwd);
				auto existing = find_installed_workshop_item(game_path, id);
				return CComVariant(existing.c_str());
			});

		frame->register_callback(
			"workshopDownload", [](const std::vector<html_argument>& params) -> CComVariant {
				if (params.empty() || !params[0].is_string())
					return CComVariant("Error: no ID");
				auto id = extract_workshop_id(params[0].get_string());
				if (id.empty())
					return CComVariant("Error: Invalid Workshop ID or link.");
				workshop_cancel_requested = false;
				reset_workshop_status();
				std::thread(workshop_download_thread, id).detach();
				return CComVariant("Download started");
			});

		frame->register_callback(
			"workshopCancelDownload", [](const std::vector<html_argument>& /*params*/) -> CComVariant {
				workshop_cancel_requested = true;
				workshop_paused = false;
				{
					std::lock_guard plock(workshop_download_mutex);
					if (workshop_download_process.hProcess) {
						TerminateProcess(workshop_download_process.hProcess, 1);
					}
				}
				reset_workshop_status();
				return CComVariant("Cancel requested");
			});

		frame->register_callback(
			"workshopPauseDownload", [](const std::vector<html_argument>& /*params*/) -> CComVariant {
				workshop_paused = true;
				return CComVariant("Pause requested");
			});

		frame->register_callback(
			"workshopResumeDownload", [](const std::vector<html_argument>& /*params*/) -> CComVariant {
				workshop_paused = false;
				return CComVariant("Resume requested");
			});

		frame->register_callback(
			"workshopIsPaused", [](const std::vector<html_argument>& /*params*/) -> CComVariant {
				return CComVariant(workshop_paused.load() ? "true" : "false");
			});

		frame->register_callback(
			"workshopBrowse", [](const std::vector<html_argument>& params) -> CComVariant {
				int page = 1;
				if (!params.empty() && params[0].is_string()) {
					try {
						page = std::stoi(params[0].get_string());
					}
					catch (...) {
						page = 1;
					}
				}
				if (page < 1)
					page = 1;

				std::thread(workshop_browse_fetch_thread, page).detach();
				return CComVariant("Fetching...");
			});

		frame->register_callback(
			"workshopSearch", [](const std::vector<html_argument>& params) -> CComVariant {
				std::string query;
				if (!params.empty() && params[0].is_string())
					query = params[0].get_string();
				utils::string::trim(query);
				if (query.empty())
					return CComVariant("Error: empty search query");

				std::thread(workshop_search_fetch_thread, query).detach();
				return CComVariant("Searching...");
			});

		frame->register_callback(
			"workshopGetBrowseData", [](const std::vector<html_argument>& /*params*/) -> CComVariant {
				std::lock_guard lock(workshop_browse_mutex);
				if (workshop_browse_cache.empty()) {
					try {
						auto file_cache = load_workshop_backup();
						if (!file_cache.empty() && file_cache != "[]") {
							workshop_browse_cache.push_back(file_cache);
							return CComVariant(file_cache.c_str());
						}
					}
					catch (...) {
					}
					return CComVariant("[]");
				}
				return CComVariant(workshop_browse_cache[0].c_str());
			});

		frame->register_callback(
			"workshopIsBrowseLoading", [](const std::vector<html_argument>& /*params*/) -> CComVariant {
				return CComVariant(workshop_browse_loading.load() ? "true" : "false");
			});
	}
}
