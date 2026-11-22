#include <std_include.hpp>

#include "theater.hpp"

#include "name.hpp"
#include "scheduler.hpp"

#include <loader/component_loader.hpp>

#include <game/utils.hpp>

#include <utils/hook.hpp>
#include <utils/info_string.hpp>
#include <utils/io.hpp>

// Theater films normally stream to and from Demonware fileshare. Recorded films
// are written to disk instead and played back from there.
namespace theater {
namespace {
using game::live::storage::FileShareCategory;
using game::live::storage::FileShareMetadataUpdate;
using game::live::storage::FileShareStreamDownload;
using game::live::storage::FileShareStreamUpload;

constexpr std::string_view FILMS_PATH = "boiii_players/user/films";
constexpr size_t MAX_FILMS = 20;

utils::hook::detour stream_upload_hook;
utils::hook::detour stream_download_hook;
utils::hook::detour is_stream_upload_in_progress_hook;
utils::hook::detour update_metadata_hook;

game::EngineDependentDvarMut auto_record;

// The engine only checks that a transfer task was created.
game::dw::TaskRecord film_task{};
std::atomic_bool recording{false};
std::atomic_uint32_t download_serial{0};

std::filesystem::path film_path(const uint64_t id,
                                const std::string_view extension) {
  return std::filesystem::path(FILMS_PATH) /
         std::format("{}.{}", id, extension);
}

void remove_film(const uint64_t id) {
  utils::io::remove_file(film_path(id, "demo"));
  utils::io::remove_file(film_path(id, "info"));
  utils::io::remove_file(film_path(id, "thumb"));
}

uint64_t new_film_id() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

utils::info_string film_info() {
  utils::info_string info;
  info.set("map", game::get_mapname().value_or(""));
  info.set("gametype", game::gametype().value_or(""));
  info.set("mode",
           std::to_string(static_cast<int32_t>(
               game::com::Com_SessionMode_GetMode())));
  info.set("author", name::get_player_name());
  return info;
}

void save_clip(const game::ControllerIndex_t controller,
               const FileShareStreamUpload *upload) {
  const uint64_t id = new_film_id();
  utils::info_string info = film_info();
  info.set("clip", "1");
  info.set("duration",
           std::to_string((*game::demo::s_demoPlayback)->clipDuration));

  utils::io::write_file(film_path(id, "demo"),
                        std::string(upload->buffer, upload->bufferSize));
  utils::io::write_file(film_path(id, "info"), info.build());

  scheduler::once([controller, callback = upload->onSuccess,
                   id] { callback(controller, id); },
                  scheduler::pipeline::main);
}

void record_film(const game::ControllerIndex_t controller,
                 const FileShareStreamUpload upload) {
  const uint64_t id = new_film_id();
  utils::info_string info = film_info();

  utils::io::create_directory(FILMS_PATH);
  std::ofstream file(film_path(id, "demo"), std::ios::binary);

  // Streaming starts once this hook has returned to the engine.
  while (!*game::demo::demo_isStreamingUpload) {
    std::this_thread::sleep_for(10ms);
  }

  const auto start = std::chrono::steady_clock::now();
  std::array<char, 0x10000> buffer;
  for (;;) {
    const bool streaming = *game::demo::demo_isStreamingUpload;
    while (const uint32_t size =
               upload.streamData(buffer.data(), buffer.size())) {
      file.write(buffer.data(), size);
    }
    if (!streaming) {
      break;
    }
    std::this_thread::sleep_for(50ms);
  }
  file.close();

  info.set("duration",
           std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::steady_clock::now() - start)
                              .count()));
  utils::io::write_file(film_path(id, "info"), info.build());
  recording = false;

  scheduler::once([controller, upload, id] { upload.onSuccess(controller, id); },
                  scheduler::pipeline::main);
}

game::dw::TaskRecord *
stream_upload_stub(const game::ControllerIndex_t controller,
                   const FileShareStreamUpload *upload) {
  if (upload->category == FileShareCategory::CLIP && upload->buffer) {
    save_clip(controller, upload);
    return &film_task;
  }

  // Only the theater recording streams its upload.
  if (!upload->streamData || !auto_record.get_bool()) {
    return stream_upload_hook.invoke<game::dw::TaskRecord *>(controller,
                                                             upload);
  }

  size_t kept = 0;
  for (const film &entry : get_films()) {
    if (!entry.clip && ++kept >= MAX_FILMS) {
      remove_film(entry.id);
    }
  }

  recording = true;
  std::thread(record_film, controller, *upload).detach();
  return &film_task;
}

game::dw::TaskRecord *
update_metadata_stub(const game::ControllerIndex_t controller,
                     const FileShareMetadataUpdate *update, const bool unk) {
  if (!utils::io::file_exists(film_path(update->fileId, "demo"))) {
    return update_metadata_hook.invoke<game::dw::TaskRecord *>(controller,
                                                               update, unk);
  }

  if (update->summary && update->summarySize) {
    utils::io::write_file(film_path(update->fileId, "thumb"),
                          std::string(update->summary, update->summarySize));
  }
  scheduler::once([controller,
                   callback = update->onSuccess] { callback(controller); },
                  scheduler::pipeline::main);
  return &film_task;
}

bool is_stream_upload_in_progress_stub(
    const game::ControllerIndex_t controller) {
  return recording ||
         is_stream_upload_in_progress_hook.invoke<bool>(controller);
}

void play_film(const game::ControllerIndex_t controller,
               const std::filesystem::path path,
               const FileShareStreamDownload download, const uint32_t serial) {
  std::ifstream file(path, std::ios::binary);
  file.seekg(download.rangeStart);

  const uint32_t end = download.rangeEnd
                           ? std::min(download.rangeEnd + 1, download.fileSize)
                           : download.fileSize;
  uint32_t received = 0;
  std::array<char, 0x10000> buffer;
  while (serial == download_serial && download.rangeStart + received < end) {
    const uint32_t size = std::min<uint32_t>(
        buffer.size(), end - download.rangeStart - received);
    if (!file.read(buffer.data(), size)) {
      break;
    }
    received += size;
    download.onData(buffer.data(), size, received, download.fileSize);
  }

  const bool complete = download.rangeStart + received == end;
  scheduler::once(
      [controller, download, serial, complete] {
        if (serial != download_serial) {
          return;
        }
        if (complete) {
          download.onComplete(controller);
        } else {
          download.onFailure(controller);
        }
      },
      scheduler::pipeline::main);
}

game::dw::TaskRecord *
stream_download_stub(const game::ControllerIndex_t controller,
                     const FileShareStreamDownload *download) {
  for (const char *extension : {"demo", "thumb"}) {
    const std::filesystem::path path = film_path(download->fileId, extension);
    if (utils::io::file_exists(path) &&
        utils::io::file_size(path) == download->fileSize) {
      std::thread(play_film, controller, path, *download, ++download_serial)
          .detach();
      return &film_task;
    }
  }
  return stream_download_hook.invoke<game::dw::TaskRecord *>(controller,
                                                             download);
}
} // namespace

std::vector<film> get_films() {
  std::vector<film> films;
  for (const std::filesystem::path &path :
       utils::io::list_files(FILMS_PATH, false, false)) {
    if (path.extension() != ".info") {
      continue;
    }

    film entry{};
    try {
      entry.id = std::stoull(path.stem().string());
    } catch (...) {
      continue;
    }

    const std::filesystem::path demo = film_path(entry.id, "demo");
    if (!utils::io::file_exists(demo)) {
      continue;
    }

    const utils::info_string info{utils::io::read_file(path)};
    entry.size = static_cast<uint32_t>(utils::io::file_size(demo));
    entry.mode = std::atoi(info.get("mode").c_str());
    entry.duration = std::atoi(info.get("duration").c_str());
    entry.map = info.get("map");
    entry.gametype = info.get("gametype");
    entry.author = info.get("author");
    entry.clip = info.get("clip") == "1";

    const std::filesystem::path thumb = film_path(entry.id, "thumb");
    if (utils::io::file_exists(thumb)) {
      entry.summary_size = static_cast<uint32_t>(utils::io::file_size(thumb));
    }

    const std::time_t time = static_cast<std::time_t>(entry.id / 1000);
    std::tm local{};
    localtime_s(&local, &time);
    char created[32]{};
    std::strftime(created, sizeof(created), "%d/%m/%Y %H:%M", &local);
    entry.created = created;

    films.push_back(std::move(entry));
  }

  std::ranges::sort(films, std::greater{}, &film::id);
  return films;
}

struct component final : client_component {
  DEFINE_COMPONENT_NAME("theater");

  void post_unpack() override {
    auto_record = game::register_dvar_bool(
        "theater_autoRecord", false, game::dvarFlags_e::ARCHIVE,
        "Record theater films of the games you host");

    stream_upload_hook.create(
        game::live::storage::LiveStorage_FileShare_StreamUpload.get(),
        stream_upload_stub);
    stream_download_hook.create(
        game::live::storage::LiveStorage_FileShare_StreamDownload.get(),
        stream_download_stub);
    is_stream_upload_in_progress_hook.create(
        game::live::storage::LiveStorage_FileShare_IsStreamUploadInProgress
            .get(),
        is_stream_upload_in_progress_stub);
    update_metadata_hook.create(
        game::live::storage::LiveStorage_FileShare_UpdateMetadata.get(),
        update_metadata_stub);
  }
};
} // namespace theater

REGISTER_COMPONENT(theater::component)
