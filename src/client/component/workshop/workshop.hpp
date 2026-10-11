#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <game/game.hpp>
#include <string>
#include <vector>

namespace workshop {
extern std::atomic<bool> downloading_workshop_item;

extern std::atomic<bool> launcher_downloading;

bool is_any_download_active();

bool is_xzone_loaded(const char *zone_name);
bool are_xzones_loading();

int get_workshop_retry_attempts();

std::string get_usermap_publisher_id(const std::string &folder_name);
std::string get_mod_publisher_id();
std::string get_mod_resized_name();
bool check_valid_usermap_id(const std::string &mapname,
                            const std::string &pub_id,
                            const std::string &workshop_id,
                            const std::string &base_uri = {});
bool check_valid_mod_id(const std::string &pub_id,
                        const std::string &workshop_id);
bool mod_switch_requires_fs_reinitialization(const std::string &current_mod,
                                             const std::string &new_mod);
bool mod_load_requires_fs_reinitialization(std::string &mod_name);
void setup_same_mod_as_host(game::LocalClientNum_t localClientNum,
                            const std::string &usermap, const std::string &mod,
                            bool force_fs_reinit = false);

void set_pending_mod_reconnect(const std::string &address);
std::string get_pending_mod_reconnect();

void set_pending_download_reconnect(const std::string &address);
std::string get_pending_download_reconnect();

std::uint64_t compute_folder_size_bytes(const std::filesystem::path &folder);
std::string human_readable_size(std::uint64_t bytes);
std::uint64_t scrape_workshop_file_size_bytes(const std::string &workshop_id);

struct item_details {
  std::string id;
  std::string title;
  std::string description;
  std::string preview_url;
  std::string kind;
  std::vector<std::string> tags;
  std::uint64_t file_size{};
  std::uint64_t time_updated{};
  std::int64_t subs{};
  std::int64_t favorites{};
};
std::vector<item_details> get_details(const std::vector<std::string> &ids);

struct workshop_info {
  std::uint64_t file_size = 0;
  std::string title;
};
workshop_info get_steam_workshop_info(const std::string &workshop_id);

void load_workshop_data(game::ugc::WorkshopData *item);
void supplement_mods_from_disk();
void supplement_usermaps_from_disk();
void supplement_ugc_from_workshop(game::ZoneType zoneType);

const char *va_mods_path(const char *fmt, const char *root_dir,
                         const char *mods_dir, const char *dir_name);

const char *va_user_content_path(const char *fmt, const char *root_dir,
                                 const char *user_content_dir);

std::string normalize_kind(const std::string &kind);

struct installed_item {
  std::string id;
  std::string folder;
  std::string title;
  std::string description;
  std::string kind;
  std::string path;
  std::uint64_t file_size{};
};
std::vector<installed_item> list_installed();
std::filesystem::path find_installed(const std::string &id);
bool delete_installed(const std::string &id, std::string &error);
std::filesystem::path install_folder(const std::filesystem::path &download,
                                     const std::string &id,
                                     const std::string &kind_hint);
void request_content_reload(const std::string &finished_title = {});

struct download_status {
  std::string message;
  std::uint64_t downloaded{};
  std::uint64_t total{};
  double speed{};
  std::int64_t eta{-1};
};

enum class download_result {
  success,
  cancelled,
  failed,
  no_space,
  setup_failed,
  install_failed,
  missing_libraries,
};

struct download_job {
  std::string id;
  std::string kind;
  const std::atomic<bool> *cancel{};
  const std::atomic<bool> *pause{};
  std::function<void(const download_status &)> report;
};

download_result download_item(const download_job &job,
                              std::filesystem::path *installed = nullptr);
const char *describe(download_result result);

struct cdn_progress {
  std::atomic<std::uint64_t> written{};
  std::atomic<std::uint64_t> received{};
  std::atomic<std::uint64_t> total{};
  std::atomic<bool> retrying{};
};
bool download_from_steam(const std::string &workshop_id,
                         const std::filesystem::path &destination,
                         const download_job &job, cdn_progress &progress);

void start_download(const std::string &id, const std::string &kind = {});
bool cancel_download();

namespace queue {
enum class item_state { queued, downloading, done, failed, cancelled };

struct entry {
  std::string id;
  std::string kind;
  std::string title;
  std::uint64_t file_size{};
  item_state state{item_state::queued};
  int position{-1};
  bool show_overlay{true};
};

const char *state_name(item_state state);
bool add(const std::string &id, const std::string &kind,
         const std::string &title = {}, std::uint64_t file_size = 0,
         bool show_overlay = true);
bool remove(const std::string &id);
bool move(const std::string &id, int delta);
void clear();
std::vector<entry> list();
int position(const std::string &id);
std::string current_id();
std::uint32_t generation();
} // namespace queue

struct catalog_status {
  bool loading{};
  bool has_more{};
  int page{1};
  std::uint64_t generation{};
  std::string error;
};
void request_catalog_page(const std::string &query, int page);
std::vector<item_details> get_catalog_page();
catalog_status get_catalog_status();
void request_preview(const std::string &id, const std::string &url);
void set_preview_cover(const std::string &id);
} // namespace workshop
