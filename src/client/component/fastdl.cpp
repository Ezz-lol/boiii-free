#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include "command.hpp"
#include "download_overlay.hpp"
#include "fastdl.hpp"
#include "scheduler.hpp"
#include "workshop/workshop.hpp"

#include <game/game.hpp>
#include <game/impl/ugc/ugc.hpp>

#include <utils/concurrency.hpp>
#include <utils/cryptography.hpp>
#include <utils/finally.hpp>
#include <utils/http.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <curl/curl.h>
#include <fstream>
#include <future>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace fastdl {
namespace {
std::atomic_bool download_active{false};
std::atomic_bool download_cancelled{false};

struct download_stream_context {
  fastdl_ui *ui{};
  updater::file_info file{};
  std::ofstream *stream{};
};

int download_progress_cb(void *clientp, const curl_off_t /*dltotal*/,
                         const curl_off_t dlnow, const curl_off_t /*ultotal*/,
                         const curl_off_t /*ulnow*/) {
  download_stream_context *ctx =
      static_cast<download_stream_context *>(clientp);
  if (download_cancelled.load()) {
    return 1;
  }
  if (ctx && ctx->ui) {
    try {
      ctx->ui->file_progress(ctx->file, static_cast<size_t>(dlnow));
    } catch (...) {
      download_cancelled.store(true);
      return 1;
    }
  }
  return 0;
}

size_t download_write_cb(void *contents, const size_t size, const size_t nmemb,
                         void *userp) {
  if (download_cancelled.load()) {
    return 0;
  }
  const size_t total_size = size * nmemb;
  download_stream_context *ctx = static_cast<download_stream_context *>(userp);
  if (ctx && ctx->stream && contents && total_size > 0) {
    ctx->stream->write(static_cast<const char *>(contents),
                       static_cast<std::streamsize>(total_size));
    ctx->stream->flush();
    if (!*ctx->stream) {
      return 0;
    }
  }
  return total_size;
}

std::string get_hash(std::ifstream &data) {
  return utils::cryptography::sha1::compute(data, true);
}

std::string get_hash(const std::string &data) {
  return utils::cryptography::sha1::compute(data, true);
}

bool is_map_file(const std::string &file_name) {
  constexpr const char *asset_file_exts[] = {".dll",  ".xpak", ".ff",
                                             ".json", ".sabs", ".sabl"};
  const std::string file_name_lower = utils::string::to_lower(file_name);
  for (const char *ext : asset_file_exts) {
    if (file_name_lower.ends_with(ext)) {
      return true;
    }
  }
  return false;
}

bool is_safe_relative_path(const std::string &path_string) {
  if (path_string.empty()) {
    return false;
  }

  const std::filesystem::path path{path_string};
  if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) {
    return false;
  }

  for (const auto &part : path) {
    if (part == "." || part.empty()) {
      continue;
    }

    if (part == "..") {
      return false;
    }
  }

  return true;
}

void show_ingame_error(const std::string &error) {
  scheduler::once(
      [error] {
        game::ui::UI_OpenErrorPopupWithMessage(
            game::LOCAL_CLIENT_0, game::errorCode::UI, error.data());
      },
      scheduler::main);
}

std::vector<updater::file_info> parse_manifest(const std::string &json) {
  rapidjson::Document doc{};
  doc.Parse(json.data(), json.size());

  if (!doc.IsArray()) {
    return {};
  }

  std::vector<updater::file_info> files{};

  for (const auto &element : doc.GetArray()) {
    if (!element.IsArray()) {
      continue;
    }

    auto array = element.GetArray();
    if (array.Size() < 3) {
      continue;
    }

    updater::file_info info{};
    info.name.assign(array[0].GetString(), array[0].GetStringLength());
    info.size = static_cast<std::size_t>(array[1].GetInt64());
    info.hash.assign(array[2].GetString(), array[2].GetStringLength());

    files.emplace_back(std::move(info));
  }

  return files;
}

std::pair<std::vector<updater::file_info>, updater::file_info>
get_manifest_files(const download_context &context) {
  const std::string manifest_json_uri = context.map_tree_uri + "/manifest.json";

  const std::optional<std::string> json =
      utils::http::get_data(manifest_json_uri);
  if (!json) {
    return {};
  }

  std::vector<updater::file_info> all_files = parse_manifest(*json);
  std::vector<updater::file_info> other_files;
  updater::file_info workshop_json;
  bool found_workshop_json = false;

  for (const updater::file_info &file : all_files) {
    if (file.name == "workshop.json") {
      workshop_json = file;
      found_workshop_json = true;
    } else if (file.name == "zone/workshop.json") {
      workshop_json = file;
      found_workshop_json = true;
    } else {
      other_files.push_back(file);
    }
  }

  return {other_files,
          found_workshop_json ? workshop_json : updater::file_info{}};
}

void download_file(const updater::file_info &file,
                   const download_context &context, fastdl_ui &ui) {
  if (download_cancelled.load()) {
    throw download_is_cancelled();
  }

  if (!is_safe_relative_path(context.mapname) ||
      !is_safe_relative_path(file.name)) {
    throw std::runtime_error("Invalid download path");
  }

  const std::string file_url = context.map_tree_uri + "/" + file.name;
  const std::string out_relative =
      file.name.starts_with("zone/") ? file.name : ("zone/" + file.name);
  const std::filesystem::path local_path =
      std::filesystem::path(context.map_path) / file.name;
  bool completed = false;

  try {
    std::string empty{};
    if (!utils::io::directory_exists(local_path.parent_path())) {
      utils::io::create_directory(local_path.parent_path());
    }
    if (!utils::io::write_file(local_path, empty, false)) {
      throw std::runtime_error(
          utils::string::va("Failed to write file: %s", local_path.c_str()));
    }

    std::ofstream ofs(local_path, std::ios::binary);
    if (!ofs) {
      throw std::runtime_error(
          utils::string::va("Failed to open file: %s", local_path.c_str()));
    }

    auto *curl = curl_easy_init();
    if (!curl) {
      throw std::runtime_error("Failed to initialize curl");
    }

    auto curl_cleanup = utils::finally([&]() { curl_easy_cleanup(curl); });

    curl_easy_setopt(curl, CURLOPT_URL, file_url.data());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "fastdl/1.0");
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING,
                     utils::http::PREFERRED_ACCEPT_ENCODING_HEADER);
    download_stream_context stream_ctx{&ui, file, &ofs};
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream_ctx);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, download_progress_cb);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &stream_ctx);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 0L);

    const auto result_code = curl_easy_perform(curl);

    ofs.close();

    if (download_cancelled.load()) {
      throw download_is_cancelled();
    }

    if (result_code != CURLE_OK) {
      throw std::runtime_error(
          utils::string::va("Failed to download: %s - curl error %d",
                            file_url.data(), result_code));
    }

    if (utils::io::file_size(local_path) != file.size) {
      throw std::runtime_error(utils::string::va(
          "Downloaded file size mismatch: %s", local_path.c_str()));
    }

    {
      std::ifstream ifs(local_path, std::ios::binary);
      const std::string hash = get_hash(ifs);
      ifs.close();

      if (utils::string::to_lower(hash) != utils::string::to_lower(file.hash)) {
        throw std::runtime_error(utils::string::va(
            "Downloaded file hash mismatch for %s:\n %s != %s",
            local_path.c_str(), hash.c_str(), file.hash.c_str()));
      }
    }

    completed = true;
  } catch (...) {
    if (!completed) {
      utils::io::remove_file(local_path);
    }
    throw;
  }
}

void download_workshop_file(const updater::file_info &file,
                            const download_context &context) {
  if (download_cancelled.load()) {
    throw download_is_cancelled();
  }

  if (!is_safe_relative_path(context.mapname) ||
      !is_safe_relative_path(file.name)) {
    throw std::runtime_error("Invalid download path");
  }

  const std::string file_url = context.map_tree_uri + "/" + file.name;
  const std::optional<std::string> data = utils::http::get_data(file_url);

  if (!data ||
      (data->size() != file.size || utils::string::to_lower(get_hash(*data)) !=
                                        utils::string::to_lower(file.hash))) {
    throw std::runtime_error(
        utils::string::va("Failed to download: %s", file_url.data()));
  }

  const std::string out_relative =
      file.name.starts_with("zone/") ? file.name : ("zone/" + file.name);
  const std::filesystem::path local_path =
      std::filesystem::path(context.map_path) / file.name;
  if (!utils::io::write_file(local_path, *data, false)) {
    throw std::runtime_error(
        utils::string::va("Failed to write: %s", local_path.c_str()));
  }
}

void download_files(const std::vector<updater::file_info> &files,
                    const download_context &context, fastdl_ui &ui) {
  /* Note: don't multi-thread these downloads.
     For large usermaps, in combination with a slow hard drive (writes buffered
     in memory for extended period), this results in catastrophically massive
     memory overhead.

     It will also throttle the dedicated server's available network
     bandwidth if the fastDL HTTP server is hosted on the same machine,
     especially if more than one player is concurrently downloading the map.
  */

  for (size_t index = 0; index < files.size(); ++index) {
    const updater::file_info &file = files[index];
    ui.begin_file(file);
    download_file(file, context, ui);
    ui.end_file(file);
  }
}

bool is_file_outdated(const updater::file_info &file,
                      const std::filesystem::path &map_path,
                      const bool verify_hash) {
  const std::filesystem::path local_path = map_path / file.name;

  if (!utils::io::file_exists(local_path)) {
    return true;
  }

  if (utils::io::file_size(local_path) != file.size) {
    return true;
  }

  if (verify_hash || !is_map_file(file.name)) {
    std::ifstream ifs(local_path, std::ios::binary);
    const std::string hash = get_hash(ifs);
    ifs.close();

    if (utils::string::to_lower(hash) != utils::string::to_lower(file.hash)) {
      return true;
    }
  }

  return false;
}

std::vector<updater::file_info>
get_outdated_files(const std::vector<updater::file_info> &files,
                   const std::filesystem::path &map_path,
                   const bool verify_hashes) {
  std::vector<updater::file_info> outdated_files{};

  for (const updater::file_info &file : files) {
    if (is_file_outdated(file, map_path, verify_hashes)) {
      outdated_files.emplace_back(file);
    }
  }

  return outdated_files;
}

void perform_download(const download_context &context) {
  try {
    download_active = true;
    download_cancelled.store(false);

    const std::string kind = utils::string::to_lower(context.kind);
    const auto [files, workshop_json] = get_manifest_files(context);
    if (files.empty() || (workshop_json.name.empty() && kind == "map")) {
      download_active = false;
      if (context.missing_manifest_callback) {
        context.missing_manifest_callback();
      } else {
        show_ingame_error(utils::string::va(
            "This server has FastDL but no manifest.json for this %s.\n%s",
            kind.c_str(), context.mapname.c_str()));
      }
      return;
    }

    const std::vector<updater::file_info> outdated_files =
        get_outdated_files(files, context.map_path, context.verify_hashes);
    if (outdated_files.empty()) {
      if (!workshop_json.name.empty()) {
        download_workshop_file(workshop_json, context);
      }
      download_active = false;

      if (context.success_callback) {
        context.success_callback();
      }
      return;
    }

    size_t total_download_size = 0;
    for (const auto &file : outdated_files) {
      total_download_size += file.size;
    }
    const double size_gb =
        static_cast<double>(total_download_size) / (1024.0 * 1024.0 * 1024.0);

    const auto answer = std::make_shared<std::promise<bool>>();
    std::future<bool> accepted = answer->get_future();
    const bool installed = utils::io::directory_exists(context.map_path);
    download_overlay::show_confirmation(
        "FastDL - " + context.kind + " Download",
        std::format("{} Download it from the server?\n{}: {}\nDownload size: "
                    "{:.2f} GB",
                    installed ? std::format("Your copy of this {} doesn't "
                                            "match the server's.",
                                            kind)
                              : std::format("You don't have this {}.", kind),
                    context.kind, context.mapname, size_gb),
        [answer] { answer->set_value(true); },
        [answer] { answer->set_value(false); });

    if (!accepted.get()) {
      throw download_is_cancelled();
    }

    if (!utils::io::directory_exists(context.map_path)) {
      utils::io::create_directory(context.map_path);
    }

    fastdl_ui ui{};
    ui.update_files(outdated_files);
    download_files(outdated_files, context, ui);
    if (!workshop_json.name.empty()) {
      download_workshop_file(workshop_json, context);
    }

    ui.done_update();

    download_active = false;

    if (context.success_callback) {
      context.success_callback();
    }
  } catch (const download_is_cancelled &) {
    download_active = false;
    show_ingame_error(context.kind + " download was cancelled.");
  } catch (const std::exception &e) {
    download_active = false;
    show_ingame_error(e.what());
  }
}
} // namespace

void start_map_download(const download_context &context) {
  bool expected = false;
  if (download_active.compare_exchange_strong(expected, true)) {
    scheduler::once([context]() { perform_download(context); },
                    scheduler::async);
  }
}

void cancel_download() { download_cancelled.store(true); }

bool is_downloading() { return download_active; }

fastdl_ui::fastdl_ui() = default;
fastdl_ui::~fastdl_ui() = default;

void fastdl_ui::update_files(const std::vector<updater::file_info> &files) {
  this->handle_cancellation();

  std::lock_guard<std::recursive_mutex> _{this->mutex_};
  this->total_files_ = files;
  this->downloaded_files_.clear();
  this->downloading_files_.clear();

  this->progress_ui_.set_title("FastDL");
  this->progress_ui_.show(false);

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void fastdl_ui::done_update() {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};

  const auto total_size = this->get_total_size();

  this->update_file_name();
  this->progress_ui_.set_progress(total_size > 1 ? total_size - 1 : 0,
                                  total_size);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  this->progress_ui_.set_progress(total_size, total_size);

  this->total_files_.clear();
  this->downloaded_files_.clear();
  this->downloading_files_.clear();

  download_overlay::clear();

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void fastdl_ui::begin_file(const updater::file_info &file) {
  this->handle_cancellation();

  std::lock_guard<std::recursive_mutex> _{this->mutex_};

  this->file_progress(file, 0);
  this->update_file_name();
}

void fastdl_ui::end_file(const updater::file_info &file) {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};

  this->downloaded_files_.emplace_back(file);
  const auto entry = this->downloading_files_.find(file.name);
  if (entry != this->downloading_files_.end()) {
    this->downloading_files_.erase(entry);
  }

  this->update_progress();
  this->update_file_name();
}

void fastdl_ui::file_progress(const updater::file_info &file,
                              const size_t progress) {
  this->handle_cancellation();

  std::lock_guard<std::recursive_mutex> _{this->mutex_};

  this->downloading_files_[file.name] = {progress, file.size};
  this->update_progress();
}

void fastdl_ui::handle_cancellation() const {
  if (this->progress_ui_.is_cancelled()) {
    download_cancelled.store(true);
    throw download_is_cancelled();
  }
}

void fastdl_ui::update_progress() {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};
  const auto downloaded = this->get_downloaded_size();
  const auto tot = this->get_total_size();
  this->progress_ui_.set_progress(downloaded, tot);

  const float spd = this->speed_.update(downloaded);
  download_overlay::download_state s;
  s.active = true;
  s.item_name = "FastDL";
  s.downloaded_bytes = downloaded;
  s.total_bytes = tot;
  s.speed_bps = spd;
  s.eta_seconds = this->speed_.eta(tot, downloaded);
  s.status_line = utils::string::va(
      "File %zu/%zu: %s", this->get_downloaded_files() + 1,
      this->get_total_files(), this->current_filename_.c_str());
  s.on_cancel = [] { cancel_download(); };
  download_overlay::update(s);
}

void fastdl_ui::update_file_name() {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};

  const auto downloaded_file_count = this->get_downloaded_files();
  const auto total_file_count = this->get_total_files();

  if (downloaded_file_count == total_file_count) {
    this->progress_ui_.set_line(1, "Download completed.");
  } else {
    this->progress_ui_.set_line(
        1, utils::string::va("Downloading files... (%zu/%zu)",
                             downloaded_file_count, total_file_count));
  }

  this->current_filename_ = this->get_relevant_file_name();
  this->progress_ui_.set_line(2, this->current_filename_);
}

size_t fastdl_ui::get_total_size() const {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};

  size_t total_size = 0;
  for (const auto &file : this->total_files_) {
    total_size += file.size;
  }

  return total_size;
}

size_t fastdl_ui::get_downloaded_size() const {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};

  size_t downloaded_size = 0;
  for (const auto &file : this->downloaded_files_) {
    downloaded_size += file.size;
  }

  for (const auto &file : this->downloading_files_) {
    downloaded_size += file.second.first;
  }

  return downloaded_size;
}

size_t fastdl_ui::get_total_files() const {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};
  return this->total_files_.size();
}

size_t fastdl_ui::get_downloaded_files() const {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};
  return this->downloaded_files_.size();
}

std::string fastdl_ui::get_relevant_file_name() const {
  std::lock_guard<std::recursive_mutex> _{this->mutex_};

  std::string name{};
  auto smallest = std::numeric_limits<size_t>::max();

  for (const auto &file : this->downloading_files_) {
    const auto max_size = file.second.second;
    if (max_size < smallest) {
      smallest = max_size;
      name = file.first;
    }
  }

  if (name.empty() && !this->downloaded_files_.empty()) {
    name = this->downloaded_files_.back().name;
  }

  return name;
}

namespace {
std::filesystem::path content_folder(const char *zone_path) {
  if (!zone_path || !zone_path[0]) {
    return {};
  }
  std::filesystem::path folder(zone_path);
  if (utils::string::to_lower(folder.filename().string()) == "zone") {
    folder = folder.parent_path();
  }
  std::error_code ec;
  return std::filesystem::is_directory(folder, ec) ? folder
                                                   : std::filesystem::path{};
}

void write_manifest(const std::filesystem::path &folder,
                    const std::string &url_path) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  writer.StartArray();
  size_t count = 0;
  std::error_code ec;
  for (std::filesystem::recursive_directory_iterator it(folder, ec), end;
       !ec && it != end; it.increment(ec)) {
    std::error_code file_ec;
    if (!it->is_regular_file(file_ec)) {
      continue;
    }
    const std::string name =
        std::filesystem::relative(it->path(), folder, file_ec).generic_string();
    if (file_ec || utils::string::to_lower(name) == "manifest.json") {
      continue;
    }
    std::ifstream file(it->path(), std::ios::binary);
    writer.StartArray();
    writer.String(name.c_str());
    writer.Int64(static_cast<int64_t>(it->file_size(file_ec)));
    writer.String(get_hash(file).c_str());
    writer.EndArray();
    ++count;
  }
  writer.EndArray();

  const std::filesystem::path manifest =
      (folder / "manifest.json").make_preferred();
  if (!utils::io::write_file(manifest, buffer.GetString(), false)) {
    printf("FastDL: failed to write %s\n", manifest.string().c_str());
    return;
  }
  printf("FastDL: wrote %s (%zu files)\nUpload that folder so it is reachable "
         "at <sv_wwwBaseURL>/%s/\n",
         manifest.string().c_str(), count, url_path.c_str());
}

void generate_manifests() {
  bool wrote = false;
  const std::string mod_id = workshop::get_mod_publisher_id();
  if (!mod_id.empty() && mod_id != "usermaps") {
    for (uint32_t i = 0; i < game::ugc::modsPool.count; ++i) {
      const game::ugc::UGCItem &mod = game::ugc::modsPool.data[i];
      if (mod_id == mod.publisherId) {
        const std::filesystem::path folder =
            content_folder(mod.absolutePathZoneFiles);
        if (!folder.empty()) {
          write_manifest(folder, "mods/" + mod_id);
          wrote = true;
        }
        break;
      }
    }
  }

  const game::ugc::UGCItem &usermap = *game::ugc::active_usermap;
  const std::filesystem::path map_folder =
      content_folder(usermap.absolutePathZoneFiles);
  std::error_code ec;
  if (usermap.internalName[0] && !map_folder.empty() &&
      !std::filesystem::equivalent(map_folder, game::get_game_path(), ec)) {
    write_manifest(map_folder, std::string("usermaps/") + usermap.internalName);
    wrote = true;
  }

  if (!wrote) {
    printf("FastDL: no mod or custom map is loaded\n");
  }
}
} // namespace

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("fastdl");

public:
  void post_unpack() override {
    command::add("fastdl_manifest", generate_manifests);
  }

  void pre_destroy() override { cancel_download(); }
};
} // namespace fastdl

REGISTER_COMPONENT(fastdl::component)
