#include <std_include.hpp>
#include <iomanip>
#include <utils/io.hpp>
#include <utils/http.hpp>
#include <utils/string.hpp>
#include "game/game.hpp"
#include "workshop.hpp"
#include "steamcmd.hpp"
#include "download_overlay.hpp"
#include "fastdl.hpp"
#include "scheduler.hpp"
#include <curl/curl.h>
#include "unzip.h"
#include <zlib.h>
#include <zip.h>

namespace steamcmd {
namespace {
std::atomic<HANDLE> active_process{nullptr};

bool parse_steam_timestamp(const std::string &line,
                           std::chrono::system_clock::time_point &out_time) {
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

bool clear_directory_contents(const std::filesystem::path &dir) {
  std::error_code ec;
  if (!std::filesystem::exists(dir, ec) ||
      !std::filesystem::is_directory(dir, ec))
    return false;

  for (const auto &entry : std::filesystem::directory_iterator(dir, ec)) {
    std::filesystem::remove_all(entry.path(), ec);
    if (ec) {
      std::filesystem::permissions(entry.path(),
                                   std::filesystem::perms::owner_all,
                                   std::filesystem::perm_options::add, ec);
      std::filesystem::remove_all(entry.path(), ec);
    }
  }
  return true;
}

void monitor_initial_dump_phase(
    const std::string &workshop_id,
    std::chrono::steady_clock::time_point download_start) {
  const std::filesystem::path content_log_path =
      "./steamcmd/logs/content_log.txt";

  // Wait for the log file to appear (up to 120 seconds)
  std::ifstream file;
  for (int i = 0; i < 600 && workshop::downloading_workshop_item.load(); ++i) {
    file.open(content_log_path, std::ios::in);
    if (file.is_open())
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  if (!file.is_open())
    return;

  file.seekg(0, std::ios::end);

  const std::string base_pattern = "AppID 311210 update started";
  std::string line;

  while (workshop::downloading_workshop_item.load()) {
    std::streampos pos = file.tellg();
    if (!std::getline(file, line)) {
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
    auto log_time_steady = now_steady + (log_time - now_sys);

    if (log_time_steady >= download_start) {
      clear_directory_contents(
          "./steamcmd/steamapps/workshop/downloads/311210/" + workshop_id);
      printf("[ Workshop ] Cleared pre-allocated dump files for %s\n",
             workshop_id.c_str());
      return;
    }
  }
}
} // namespace

int start_new_process(const char *exePath, bool Hide_Window, bool waittill_done,
                      const char *arguments) {
  std::string commandLine = std::string(exePath) + " " + std::string(arguments);

  STARTUPINFO startupInfo;
  PROCESS_INFORMATION pi;

  ZeroMemory(&startupInfo, sizeof(startupInfo));
  startupInfo.cb = sizeof(startupInfo);
  if (Hide_Window) {
    startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;
  }

  if (CreateProcess(nullptr, (LPSTR)commandLine.c_str(), nullptr, nullptr,
                    FALSE, 0, nullptr, nullptr, &startupInfo, &pi)) {
    if (waittill_done) {
      active_process.store(pi.hProcess);
      WaitForSingleObject(pi.hProcess, INFINITE);
      active_process.store(nullptr);
    }

    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
      if (exitCode == -1073741510) {
        return 1;
      }

      if (exitCode == 0) {
        printf("SteamCMD process completed successfully.\n");
      } else {
        printf("SteamCMD process exited with code: %ld\n", exitCode);
      }
    } else {
      printf("Error - %lu\n", GetLastError());
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  } else {
    printf("Error - %lu\n", GetLastError());
  }

  return 0;
}

int extract_steamcmd() {
  auto zipFileName = "steamcmd.zip";
  auto extractDir = "";

  unzFile zipFile = unzOpen(zipFileName);
  if (zipFile == nullptr) {
    printf("Failed to open ZIP file: %s\n", zipFileName);
    return 1;
  }

  if (unzGoToFirstFile(zipFile) != UNZ_OK) {
    printf("No files found in the ZIP archive.\n");
    unzClose(zipFile);
    return 1;
  }

  do {
    char fileName[256];
    unz_file_info fileInfo;
    if (unzGetCurrentFileInfo(zipFile, &fileInfo, fileName, sizeof(fileName),
                              nullptr, 0, nullptr, 0) != UNZ_OK) {
      printf("Failed to get file info.\n");
      unzClose(zipFile);
      return 1;
    }

    printf("Extracting: %s\n", fileName);

    if (unzOpenCurrentFile(zipFile) != UNZ_OK) {
      printf("Failed to open file inside ZIP: %s\n", fileName);
      unzClose(zipFile);
      return 1;
    }

    char buffer[1024];
    FILE *outFile = nullptr;
    if (extractDir[0] != '\0') {
      char outputPath[512];
      snprintf(outputPath, sizeof(outputPath), "%s/%s", extractDir, fileName);

      if (fopen_s(&outFile, outputPath, "wb") != 0) {
        printf("Failed to open output file: %s\n", outputPath);
        unzCloseCurrentFile(zipFile);
        unzClose(zipFile);
        return 1;
      }
    } else {
      if (fopen_s(&outFile, fileName, "wb") != 0) {
        printf("Failed to open output file: %s\n", fileName);
        unzCloseCurrentFile(zipFile);
        unzClose(zipFile);
        return 1;
      }
    }

    int bytesRead;
    do {
      bytesRead = unzReadCurrentFile(zipFile, buffer, sizeof(buffer));
      if (bytesRead > 0) {
        fwrite(buffer, 1, bytesRead, outFile);
      }
    } while (bytesRead > 0);

    fclose(outFile);
    unzCloseCurrentFile(zipFile);
  } while (unzGoToNextFile(zipFile) == UNZ_OK);

  unzClose(zipFile);
  return 0;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  return fwrite(ptr, size, nmemb, stream);
}

int setup_steamcmd() {
  int tries = 0;
  int max_tries = 3;
  if (!std::filesystem::exists("steamcmd")) {
    std::filesystem::create_directory("steamcmd");
  }

  const bool is_wine = utils::nt::is_wine();
  const std::string steamcmd_bin =
      is_wine ? "steamcmd/steamcmd.sh" : "steamcmd/steamcmd.exe";

  if (!utils::io::file_exists("steamcmd.zip") &&
      !utils::io::file_exists("steamcmd.tar.gz") &&
      !utils::io::file_exists(steamcmd_bin)) {
    const std::string url =
        is_wine
            ? "https://steamcdn-a.akamaihd.net/client/installer/"
              "steamcmd_linux.tar.gz"
            : "https://steamcdn-a.akamaihd.net/client/installer/steamcmd.zip";

    const std::string outfilename =
        is_wine ? "./steamcmd.tar.gz" : "./steamcmd.zip";

    for (; tries < max_tries; ++tries) {
      CURL *curl = curl_easy_init();
      if (!curl) {
        printf("CURL initialization failed.\n");
        return 1;
      }

      FILE *outFile = nullptr;
      errno_t err = fopen_s(&outFile, outfilename.c_str(), "wb");
      if (err != 0 || outFile == nullptr) {
        printf("Error opening file for writing.\n");
        curl_easy_cleanup(curl);
        return 1;
      }

      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_CAINFO, "./ca-bundle.crt");
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, outFile);
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

      CURLcode res = curl_easy_perform(curl);
      fclose(outFile);
      curl_easy_cleanup(curl);

      if (res == CURLE_OK) {
        break;
      }

      printf("CURL failed: %s\n", curl_easy_strerror(res));
      remove(outfilename.c_str());
      if (tries + 1 < max_tries) {
        Sleep(2000);
      }
    }

    if (tries >= max_tries) {
      printf("[ERROR] Could not download steamcmd! \nMax tries used.");
      return 1;
    }
  }

  if (is_wine) {
    if (utils::io::file_exists("steamcmd.tar.gz") &&
        !utils::io::file_exists("steamcmd/steamcmd.sh")) {
      printf("Extracting steamcmd_linux.tar.gz...\n");
      system("tar -xzf steamcmd.tar.gz -C steamcmd");
    }
  } else {
    if (utils::io::file_exists("steamcmd.zip") &&
        !utils::io::file_exists("steamcmd/steamcmd.exe")) {
      if (extract_steamcmd() != 0) {
        return 1;
      }
    }
  }

  if (!is_wine) {
    if (!utils::io::file_exists("steamcmd/steamcmd.exe") &&
        utils::io::file_exists("steamcmd.exe")) {
      std::filesystem::rename("steamcmd.exe", "steamcmd/steamcmd.exe");
    }

    try {
      if (utils::io::file_exists("steamcmd/steamcmd.exe") &&
          std::filesystem::is_regular_file("steamcmd/steamcmd.exe")) {
        std::uintmax_t file_size =
            std::filesystem::file_size("steamcmd/steamcmd.exe");
        if (file_size < 3 * 1024 * 1024) {
          printf("Install / update SteamCMD \n");
          start_new_process("./steamcmd/steamcmd.exe", false, true, "+quit");
        }
      }
    } catch (std::filesystem::filesystem_error &e) {
      printf("%s\n", e.what());
    }
  }

  return 0;
}

void move_downloaded_folder(const std::string &sourceFolderPath,
                            const std::string &destinationFolderPath) {
  try {
    if (std::filesystem::exists(sourceFolderPath) &&
        std::filesystem::is_directory(sourceFolderPath)) {
      if (!std::filesystem::exists(destinationFolderPath)) {
        std::filesystem::create_directories(destinationFolderPath);
      }

      for (const auto &entry :
           std::filesystem::directory_iterator(sourceFolderPath)) {
        std::string sourcePath = entry.path().string();
        std::string filename = entry.path().filename().string();
        std::filesystem::path destinationPath =
            std::filesystem::path(destinationFolderPath) / filename;
        std::filesystem::rename(sourcePath, destinationPath);
      }

      std::filesystem::remove(sourceFolderPath);
      printf("Folder and its contents moved successfully.\n");
    } else {
      fprintf(stderr, "Source folder does not exist or is not a directory.\n");
    }
  } catch (const std::filesystem::filesystem_error &ex) {
    fprintf(stderr, "Error: %s\n", ex.what());
  }
}

int download_workshop_item(std::string workshop_id, std::string modtype);

void initialize_download(std::string workshop_id, std::string modtype) {
  if (workshop::launcher_downloading.load()) {
    scheduler::once(
        [] {
          game::UI_OpenErrorPopupWithMessage(
              0, game::ERROR_UI,
              "A download is already in progress from the launcher. Wait for "
              "it to finish.");
        },
        scheduler::main);
    return;
  }

  workshop::downloading_workshop_item = true;

  // Fetch workshop info (title, file size) for detailed progress
  const auto ws_info = workshop::get_steam_workshop_info(workshop_id);
  const std::string workshop_title =
      ws_info.title.empty()
          ? ((modtype == "Map" ? "Map: " : "Mod: ") + workshop_id)
          : ws_info.title;
  const std::uint64_t expected_size = ws_info.file_size;

  {
    download_overlay::download_state s;
    s.active = true;
    s.item_name = workshop_title;
    s.status_line = "Setting up SteamCMD...";
    if (expected_size > 0) {
      s.total_bytes = expected_size;
    }
    download_overlay::update(s);
  }

  // Set up cancel callback
  auto cancel_cb = std::make_shared<std::function<void()>>([]() {
    HANDLE h = active_process.load();
    if (h)
      TerminateProcess(h, 1);
    workshop::downloading_workshop_item = false;
  });

  // Set up detailed progress tracking with network speed
  const std::string dl_folder =
      "./steamcmd/steamapps/workshop/downloads/311210/" + workshop_id;
  const std::string content_folder =
      "./steamcmd/steamapps/workshop/content/311210/" + workshop_id;

  auto net_bytes_prev = std::make_shared<std::uint64_t>(0);
  auto net_baseline_set = std::make_shared<bool>(false);
  auto last_tick = std::make_shared<std::chrono::steady_clock::time_point>(
      std::chrono::steady_clock::now());
  auto download_start = std::make_shared<std::chrono::steady_clock::time_point>(
      std::chrono::steady_clock::now());
  auto warmup_phase = std::make_shared<bool>(true);
  auto download_phase_start =
      std::make_shared<std::chrono::steady_clock::time_point>();
  auto smoothed_speed = std::make_shared<double>(0.0);
  auto speed_str = std::make_shared<std::string>();

  scheduler::schedule(
      [=]() -> bool {
        if (!workshop::downloading_workshop_item.load())
          return scheduler::cond_end;

        // Check multiple possible download paths
        std::uint64_t current_size = 0;
        {
          current_size = workshop::compute_folder_size_bytes(dl_folder);
          if (current_size == 0)
            current_size = workshop::compute_folder_size_bytes(content_folder);
        }

        // Track when files first appear (start of dump/download phase)
        if (*warmup_phase && current_size > 0 &&
            *download_phase_start == std::chrono::steady_clock::time_point{}) {
          *download_phase_start = std::chrono::steady_clock::now();
        }

        if (*warmup_phase &&
            *download_phase_start != std::chrono::steady_clock::time_point{}) {
          const auto wp_elapsed =
              std::chrono::steady_clock::now() - *download_phase_start;
          if (std::chrono::duration_cast<std::chrono::seconds>(wp_elapsed)
                  .count() >= 10)
            *warmup_phase = false;
        }

        if (*warmup_phase) {
          // Reset network baseline each warmup tick so the first real reading
          // is clean
          *net_baseline_set = false;
          *last_tick = std::chrono::steady_clock::now();

          auto elapsed = std::chrono::steady_clock::now() - *download_start;
          auto elapsed_sec =
              std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
          int eh = static_cast<int>(elapsed_sec / 3600);
          int em = static_cast<int>((elapsed_sec % 3600) / 60);
          int es = static_cast<int>(elapsed_sec % 60);
          char time_str[32];
          snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", eh, em, es);

          download_overlay::download_state s;
          s.active = true;
          s.item_name = workshop_title;
          s.status_line =
              std::string("Preparing download... | Elapsed: ") + time_str;
          s.on_cancel = *cancel_cb;
          download_overlay::update(s);
          return scheduler::cond_continue;
        }

        std::uint64_t display_size = current_size;
        if (expected_size > 0 && display_size > expected_size)
          display_size = expected_size;

        // Network speed via GetIfTable2
        std::uint64_t net_bytes_now = 0;
        {
          MIB_IF_TABLE2 *if_table = nullptr;
          if (GetIfTable2(&if_table) == NO_ERROR && if_table) {
            for (ULONG i = 0; i < if_table->NumEntries; i++)
              net_bytes_now += if_table->Table[i].InOctets;
            FreeMibTable(if_table);
          }
        }

        if (!*net_baseline_set) {
          *net_bytes_prev = net_bytes_now;
          *net_baseline_set = true;
        }

        std::uint64_t net_delta = 0;
        if (net_bytes_now >= *net_bytes_prev)
          net_delta = net_bytes_now - *net_bytes_prev;
        *net_bytes_prev = net_bytes_now;

        const auto now = std::chrono::steady_clock::now();
        const auto dt_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                                  *last_tick)
                .count();
        if (dt_ms > 0 && net_delta > 0) {
          const double raw_speed = (static_cast<double>(net_delta) * 1000.0) /
                                   static_cast<double>(dt_ms);
          // EMA smoothing (alpha ~0.3) to prevent spiky readings
          if (*smoothed_speed <= 0.0)
            *smoothed_speed = raw_speed;
          else
            *smoothed_speed = 0.3 * raw_speed + 0.7 * (*smoothed_speed);

          if (*smoothed_speed > 0.0)
            *speed_str = workshop::human_readable_size(
                             static_cast<std::uint64_t>(*smoothed_speed)) +
                         "/s";
        }
        *last_tick = now;
        const float speed = static_cast<float>(*smoothed_speed);

        // ETA calculation
        int eta = -1;
        if (expected_size > 0 && display_size > 0 && speed > 1024.f) {
          const auto remaining = (expected_size > display_size)
                                     ? (expected_size - display_size)
                                     : 0ULL;
          eta = static_cast<int>(static_cast<double>(remaining) /
                                 static_cast<double>(speed));
        }

        // Elapsed time string
        auto elapsed = std::chrono::steady_clock::now() - *download_start;
        auto elapsed_sec =
            std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
        int eh = static_cast<int>(elapsed_sec / 3600);
        int em = static_cast<int>((elapsed_sec % 3600) / 60);
        int es = static_cast<int>(elapsed_sec % 60);
        char time_str[32];
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", eh, em, es);

        // Build status line
        std::string details;
        if (display_size > 0) {
          details = workshop::human_readable_size(display_size);
          if (expected_size > 0)
            details += " / " + workshop::human_readable_size(expected_size);
        } else if (expected_size > 0) {
          details = "0 / " + workshop::human_readable_size(expected_size);
        }
        details += " | Elapsed: " + std::string(time_str);
        if (!speed_str->empty()) {
          details += " | " + *speed_str;
        }

        download_overlay::download_state s;
        s.active = true;
        s.item_name = workshop_title;
        s.downloaded_bytes = display_size;
        s.total_bytes = expected_size;
        s.speed_bps = speed;
        s.eta_seconds = eta;
        s.status_line = details;
        s.on_cancel = *cancel_cb;
        download_overlay::update(s);

        return scheduler::cond_continue;
      },
      scheduler::async, 500ms);

  const auto dl_start_time = std::chrono::steady_clock::now();
  std::thread dump_monitor([workshop_id, dl_start_time] {
    monitor_initial_dump_phase(workshop_id, dl_start_time);
  });
  dump_monitor.detach();

  int result = download_workshop_item(workshop_id.data(), modtype.data());

  // Clean up partial download files on cancel
  if (result == 1) {
    std::error_code cleanup_ec;
    std::filesystem::remove_all(dl_folder, cleanup_ec);
    std::filesystem::remove_all(content_folder, cleanup_ec);
  }

  const char *error_msg = nullptr;
  if (result == 4)
    error_msg = "Problem downloading the workshop item. Max tries used.";
  else if (result == 3)
    error_msg = "Cannot install SteamCMD. Please try again.";
  else if (result == 2)
    error_msg =
        "There was a problem moving the workshop item to the correct "
        "folder.\nYou can try moving it manually and joining the server again.";
  else if (result == 1)
    error_msg = "Download cancelled.";

  if (error_msg) {
    scheduler::once(
        [error_msg] {
          game::UI_OpenErrorPopupWithMessage(0, game::ERROR_UI, error_msg);
        },
        scheduler::main);
  } else if (result == 0) {
    const auto reconnect_addr = workshop::get_pending_download_reconnect();
    if (!reconnect_addr.empty()) {
      const auto addr_copy = reconnect_addr;
      scheduler::once(
          [addr_copy] {
            download_overlay::show_confirmation(
                "Download Complete",
                "Workshop item downloaded successfully!\n\nDo you want to "
                "connect to the server?",
                [addr_copy] {
                  game::Cbuf_AddText(
                      0, utils::string::va("connect %s\n", addr_copy.c_str()));
                });
          },
          scheduler::main);
    } else {
      scheduler::once(
          [] {
            game::UI_OpenErrorPopupWithMessage(
                0, game::ERROR_UI, "Workshop item downloaded successfully!");
          },
          scheduler::main);
    }
  }

  download_overlay::clear();

  // Refresh steam workshop items with command
  game::Cbuf_AddText(0, "userContentReload\n");
  printf("Workshop items refreshed\n");
  workshop::downloading_workshop_item = false;
}

int download_workshop_item(std::string workshop_id, std::string modtype) {
  if (setup_steamcmd() == 1) {
    printf("[ERROR] Could not setup steamcmd! \n");
    return 3;
  }

  const char *workshop_id_char = workshop_id.c_str();
  std::string content_folder =
      "./steamcmd/steamapps/workshop/content/311210/" + workshop_id;
  std::string steamapps_folder = "./steamcmd/steamapps";

  std::string destination_folder;
  if (modtype == "Mod") {
    destination_folder = "./mods/" + workshop_id;
  } else if (modtype == "Map") {
    destination_folder = "./usermaps/" + workshop_id;
  }

  // remove old folder before each download to prevent any issues with steamcmd
  // download
  if (std::filesystem::exists(steamapps_folder)) {
    std::filesystem::remove_all(steamapps_folder);
    printf("Old steamapps folder removed successfully.\n");
  }

  bool continue_download = false;
  const int max_tries = workshop::get_workshop_retry_attempts();
  printf("[ Workshop ] Download started (max retries: %d, set via "
         "workshop_retry_attempts dvar).\n",
         max_tries);
  int tries = 0;
  int fast_fail_count = 0;
  constexpr int FAST_FAIL_THRESHOLD = 5;

  while (workshop::downloading_workshop_item.load() &&
         !std::filesystem::exists(content_folder)) {
    if (tries >= max_tries) {
      // Ask the user if they want to keep trying instead of giving up
      if (download_overlay::show_confirmation_blocking(
              "Retry Limit Reached",
              "Download has used all " + std::to_string(max_tries) +
                  " retry attempts without completing.\n\n"
                  "Do you want to continue downloading?\n"
                  "(Your progress will be preserved)")) {
        printf("[ Workshop ] User chose to continue downloading, resetting "
               "retry counter.\n");
        tries = 0;
        continue;
      } else {
        printf("[ Workshop ] User chose to stop after retry limit.\n");
        break;
      }
    }

    tries++;

    if (fast_fail_count >= FAST_FAIL_THRESHOLD) {
      printf(
          "[ Workshop ] Too many quick failures (%d), resetting SteamCMD...\n",
          fast_fail_count);
      std::error_code ec;
      for (const auto &dir_name : {"steamapps", "dumps", "logs", "depotcache",
                                   "appcache", "userdata"}) {
        auto dir = std::filesystem::path("./steamcmd") / dir_name;
        if (std::filesystem::exists(dir, ec))
          std::filesystem::remove_all(dir, ec);
      }
      fast_fail_count = 0;
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    if (continue_download) {
      printf("[ Workshop ] Resuming download (attempt %d/%d)...\n", tries,
             max_tries);
    } else {
      printf("[ Workshop ] Downloading (attempt %d/%d)...\n", tries, max_tries);
    }

    const bool is_wine = utils::nt::is_wine();
    const std::string steamcmd_bin =
        is_wine ? "./steamcmd/steamcmd.sh" : "./steamcmd/steamcmd.exe";

    auto attempt_start = std::chrono::steady_clock::now();

    // SteamCMD will resume from where it left off if the download was
    // interrupted
    int result = start_new_process(
        steamcmd_bin.c_str(), true, true,
        ("+login anonymous app_update 311210 +workshop_download_item 311210 " +
         std::string(workshop_id_char) + " validate +quit")
            .c_str());

    if (result == 1) {
      printf("User interrupted download \n");
      return 1;
    }

    // Detect rapid failures (SteamCMD exiting in under 15 seconds without
    // producing content)
    auto attempt_elapsed = std::chrono::steady_clock::now() - attempt_start;
    if (std::chrono::duration_cast<std::chrono::seconds>(attempt_elapsed)
                .count() < 15 &&
        !std::filesystem::exists(content_folder)) {
      fast_fail_count++;
    } else {
      fast_fail_count = 0;
    }

    continue_download = true;
  }

  if (!workshop::downloading_workshop_item.load()) {
    printf("[ Workshop ] Download cancelled by user.\n");
    return 1;
  }

  // Final check, content_folder must exist regardless of try count
  if (!std::filesystem::exists(content_folder)) {
    printf(
        "Problem downloading the workshop item. Max tries used exiting.. \n");
    return 4;
  }

  try {
    move_downloaded_folder(content_folder, destination_folder);
    printf("Mod moved to correct folder \n");
  } catch (const std::filesystem::filesystem_error &ex) {
    printf("%s\n", ex.what());
    return 2;
  }

  return 0;
}
} // namespace steamcmd