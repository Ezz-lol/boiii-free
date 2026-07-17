#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>

#include "fastdl.hpp"
#include "scheduler.hpp"
#include "download_overlay.hpp"

#include <utils/string.hpp>
#include <utils/io.hpp>
#include <utils/http.hpp>
#include <utils/cryptography.hpp>
#include <utils/concurrency.hpp>
#include <utils/finally.hpp>

#include <curl/curl.h>
#include <fstream>

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
        game::ui::UI_OpenErrorPopupWithMessage(0, game::errorCode::UI,
                                               error.data());
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
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
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
                      const std::filesystem::path &map_path) {
  const std::filesystem::path local_path = map_path / file.name;

  if (!utils::io::file_exists(local_path)) {
    return true;
  }

  if (utils::io::file_size(local_path) != file.size) {
    return true;
  }

  if (!is_map_file(file.name)) {
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
                   const std::filesystem::path &map_path) {
  std::vector<updater::file_info> outdated_files{};

  for (const updater::file_info &file : files) {
    if (is_file_outdated(file, map_path)) {
      outdated_files.emplace_back(file);
    }
  }

  return outdated_files;
}

void perform_download(const download_context &context) {
  try {
    download_active = true;
    download_cancelled.store(false);

    const auto [files, workshop_json] = get_manifest_files(context);
    if (files.empty() || workshop_json.name.empty()) {
      download_active = false;
      show_ingame_error(
          "Failed to download usermap manifest file or manifest is empty.");
      return;
    }

    const std::vector<updater::file_info> outdated_files =
        get_outdated_files(files, context.map_path);
    if (outdated_files.empty()) {
      download_workshop_file(workshop_json, context);
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

    const auto result = MessageBoxA(
        nullptr,
        utils::string::va("You don't have this map. Would you like to download "
                          "it?\nMap: %s\nDownload size: %.2f GB",
                          context.mapname.data(), size_gb),
        "FastDL - Map Download", MB_YESNO | MB_ICONQUESTION);

    if (result != IDYES) {
      throw download_is_cancelled();
    }

    if (!utils::io::directory_exists(context.map_path)) {
      utils::io::create_directory(context.map_path);
    }

    fastdl_ui ui{};
    ui.update_files(outdated_files);
    download_files(outdated_files, context, ui);
    download_workshop_file(workshop_json, context);

    ui.done_update();

    download_active = false;

    if (context.success_callback) {
      context.success_callback();
    }
  } catch (const download_is_cancelled &) {
    download_active = false;
    show_ingame_error("Map download was cancelled.");
  } catch (const std::exception &e) {
    download_active = false;
    show_ingame_error(e.what());
  }
}
} // namespace

void start_map_download(const download_context &context) {
  bool expected = false;
  if (!download_active.compare_exchange_strong(expected, true)) {
    return;
  }

  scheduler::once([context]() { perform_download(context); }, scheduler::async);
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

class component final : public generic_component {
public:
  void pre_destroy() override { cancel_download(); }
};
} // namespace fastdl

REGISTER_COMPONENT(fastdl::component)