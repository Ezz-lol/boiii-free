#include <std_include.hpp>

#include "steamcmd.hpp"
#include "workshop.hpp"
#include <loader/component_loader.hpp>

#include <component/download_overlay.hpp>
#include <component/scheduler.hpp>
#include <component/toast.hpp>

#include <utils/nt.hpp>
#include <utils/string.hpp>

#include <deque>

namespace workshop {
namespace {
std::atomic<bool> cancel_requested{false};

bool cancelled(const download_job &job) {
  return job.cancel && job.cancel->load();
}

download_result
download_with_steamcmd(const download_job &job,
                       const std::filesystem::path &destination) {
  const auto dir = game::get_game_path() / "steamcmd";
  const bool wine = utils::nt::is_wine();
  const auto report = [&](download_status status) {
    if (job.report)
      job.report(status);
  };

  std::string error;
  if (!steamcmd::ensure_installed(
          dir, wine, [&](const std::string &status) { report({status}); },
          error)) {
    printf("[ Workshop ] %s\n", error.c_str());
    return download_result::setup_failed;
  }

  const auto find_content = [&]() -> std::filesystem::path {
    std::error_code ec;
    for (const auto &root : steamcmd::workshop_roots(dir, wine)) {
      const auto folder = root / "content" / game::APP_ID_STR / job.id;
      if (std::filesystem::exists(folder, ec))
        return folder;
    }
    return {};
  };

  steamcmd::clear_downloads(dir);
  const int max_tries = get_workshop_retry_attempts();
  int fast_fails = 0;
  for (int tries = 1; find_content().empty(); ++tries) {
    if (cancelled(job))
      return download_result::cancelled;
    if (tries > max_tries)
      return download_result::failed;
    if (fast_fails >= 5) {
      std::error_code ec;
      for (const char *name : {"steamapps", "dumps", "logs", "depotcache",
                               "appcache", "userdata", "Steam"}) {
        std::filesystem::remove_all(dir / name, ec);
      }
      fast_fails = 0;
    }

    steamcmd::process steamcmd(dir, wine);
    if (!steamcmd.start("+login anonymous +workshop_download_item " +
                        std::string(game::APP_ID_STR) + " " + job.id +
                        " +quit")) {
      ++fast_fails;
      std::this_thread::sleep_for(2s);
      continue;
    }

    const auto started = std::chrono::steady_clock::now();
    steamcmd::progress tracker(dir);
    while (!steamcmd.wait(1s)) {
      if (cancelled(job) || (job.pause && job.pause->load())) {
        steamcmd.terminate();
        while (!cancelled(job) && job.pause && job.pause->load())
          std::this_thread::sleep_for(300ms);
        --tries;
        break;
      }
      tracker.update(steamcmd);
      download_status status{tracker.committing()
                                 ? "Finishing download..."
                                 : "Downloading (SteamCMD)..."};
      if (tracker.started()) {
        status.downloaded = tracker.installed_bytes();
        status.total = tracker.install_total();
        status.speed = tracker.speed();
        status.eta = tracker.eta_seconds();
      }
      report(status);
    }

    if (steamcmd.missing_libraries())
      return download_result::missing_libraries;
    if (std::chrono::steady_clock::now() - started < 15s &&
        find_content().empty())
      ++fast_fails;
  }

  std::error_code ec;
  std::filesystem::rename(find_content(), destination, ec);
  steamcmd::clear_downloads(dir);
  return ec ? download_result::install_failed : download_result::success;
}

download_result run_download(const std::string &id, const std::string &kind,
                             const bool show_overlay) {
  if (launcher_downloading.load()) {
    scheduler::once(
        [] {
          toast::warn("Workshop", "Wait for the launcher download to finish.");
        },
        scheduler::main);
    return download_result::failed;
  }

  downloading_workshop_item = true;
  cancel_requested = false;
  const auto info = get_steam_workshop_info(id);
  const auto title = info.title.empty() ? "Workshop item " + id : info.title;

  download_job job{id, kind, &cancel_requested, nullptr, {}};
  job.report = [&](const download_status &status) {
    download_overlay::download_state state;
    state.active = true;
    state.hidden = !show_overlay;
    state.item_name = title;
    state.on_cancel = [] { cancel_requested = true; };
    state.downloaded_bytes = status.downloaded;
    state.total_bytes = status.total;
    state.speed_bps = static_cast<float>(status.speed);
    state.eta_seconds = static_cast<int>(status.eta);
    state.status_line = status.message;
    if (status.total) {
      state.status_line =
          human_readable_size(status.downloaded) + " / " +
          human_readable_size(status.total) +
          (status.message.empty() ? "" : " | " + status.message);
    }
    download_overlay::update(state);
  };
  job.report({"Connecting to Steam..."});

  const auto result = download_item(job);
  download_overlay::clear();
  downloading_workshop_item = false;

  if (result == download_result::success) {
    request_content_reload(title);
    const auto reconnect = get_pending_download_reconnect();
    scheduler::once(
        [title, reconnect] {
          if (!reconnect.empty()) {
            download_overlay::show_confirmation(
                "Download Complete",
                title + " downloaded successfully!\n\nDo you want to connect "
                        "to the server?",
                [reconnect] {
                  game::cbuf::Cbuf_AddText(
                      game::LOCAL_CLIENT_0,
                      utils::string::va("connect %s\n", reconnect.c_str()));
                });
          } else {
            toast::success("Download finished", title + " is ready.");
          }
        },
        scheduler::main);
  } else {
    scheduler::once(
        [result] {
          if (result == download_result::cancelled)
            toast::info("Workshop", describe(result));
          else
            toast::error("Workshop", describe(result));
        },
        scheduler::main);
  }
  return result;
}
} // namespace

const char *describe(const download_result result) {
  switch (result) {
  case download_result::success:
    return "Download complete.";
  case download_result::cancelled:
    return "Download cancelled.";
  case download_result::no_space:
    return "Not enough disk space for this download.";
  case download_result::setup_failed:
    return "SteamCMD could not be set up. Check your internet connection.";
  case download_result::install_failed:
    return "The download could not be moved into the game folder.";
  case download_result::missing_libraries:
    return "SteamCMD needs 32-bit libraries on Linux. Install lib32gcc-s1 "
           "(Debian/Ubuntu), glibc.i686 and libgcc.i686 (Fedora) or "
           "lib32-gcc-libs (Arch), then try again.";
  default:
    return "The workshop item could not be downloaded.";
  }
}

download_result download_item(const download_job &job,
                              std::filesystem::path *installed) {
  const auto game_path = game::get_game_path();
  const auto info = get_steam_workshop_info(job.id);

  ULARGE_INTEGER free_bytes{};
  if (info.file_size &&
      GetDiskFreeSpaceExW(game_path.c_str(), &free_bytes, nullptr, nullptr) &&
      free_bytes.QuadPart < info.file_size + (512ull << 20)) {
    return download_result::no_space;
  }

  const auto staging = game_path / "workshop_downloads" / job.id;
  std::error_code ec;
  std::filesystem::remove_all(staging, ec);
  std::filesystem::create_directories(staging, ec);

  cdn_progress progress;
  std::atomic<bool> done{false};
  std::thread reporter([&] {
    double speed = 0.0;
    uint64_t last = 0;
    while (!done) {
      std::this_thread::sleep_for(500ms);
      const uint64_t received = progress.received;
      speed = 0.3 * static_cast<double>(received - last) * 2.0 + 0.7 * speed;
      last = received;

      download_status status{};
      status.total = progress.total ? progress.total.load() : info.file_size;
      status.downloaded = std::min<uint64_t>(progress.written, status.total);
      status.speed = speed;
      if (job.pause && job.pause->load()) {
        status.message = "Paused";
      } else if (!status.downloaded) {
        status.message = "Preparing download...";
      } else if (progress.retrying) {
        status.message = "Retrying...";
      }
      if (speed > 1024.0 && status.total > status.downloaded) {
        status.eta = static_cast<int64_t>(
            static_cast<double>(status.total - status.downloaded) / speed);
      }
      if (job.report && !done)
        job.report(status);
    }
  });

  auto result = download_from_steam(job.id, staging, job, progress)
                    ? download_result::success
                    : download_result::failed;
  done = true;
  reporter.join();

  if (result != download_result::success && !cancelled(job)) {
    printf("[ Workshop ] Falling back to SteamCMD for %s\n", job.id.c_str());
    std::filesystem::remove_all(staging, ec);
    result = download_with_steamcmd(job, staging);
  }

  if (cancelled(job))
    result = download_result::cancelled;

  if (result == download_result::success) {
    if (job.report)
      job.report({"Installing..."});
    const auto target = install_folder(staging, job.id, job.kind);
    if (target.empty())
      result = download_result::install_failed;
    else if (installed)
      *installed = target;
  }

  std::filesystem::remove_all(staging, ec);
  return result;
}

void start_download(const std::string &id, const std::string &kind) {
  queue::add(id, kind);
}

bool cancel_download() {
  if (!downloading_workshop_item.load())
    return false;
  cancel_requested = true;
  return true;
}

namespace queue {
namespace {
constexpr size_t MAX_HISTORY = 20;

std::mutex mutex;
std::condition_variable cv;
std::deque<entry> pending;
std::optional<entry> current;
std::deque<entry> history;
std::atomic<uint32_t> generation_counter{1};

auto by_id(const std::string &id) {
  return [&id](const entry &e) { return e.id == id; };
}

void worker() {
  for (;;) {
    entry next{};
    {
      std::unique_lock lock(mutex);
      cv.wait_for(lock, 1s, [] { return !pending.empty(); });
      if (pending.empty() || is_any_download_active())
        continue;
      next = pending.front();
      pending.pop_front();
      next.state = item_state::downloading;
      current = next;
      ++generation_counter;
    }

    const auto result = run_download(next.id, next.kind, next.show_overlay);

    std::lock_guard lock(mutex);
    next.state = result == download_result::success     ? item_state::done
                 : result == download_result::cancelled ? item_state::cancelled
                                                        : item_state::failed;
    std::erase_if(history, by_id(next.id));
    history.push_front(next);
    if (history.size() > MAX_HISTORY)
      history.pop_back();
    current.reset();
    ++generation_counter;
  }
}
} // namespace

const char *state_name(const item_state state) {
  switch (state) {
  case item_state::queued:
    return "queued";
  case item_state::downloading:
    return "downloading";
  case item_state::done:
    return "done";
  case item_state::failed:
    return "failed";
  default:
    return "cancelled";
  }
}

bool add(const std::string &id, const std::string &kind,
         const std::string &title, const uint64_t file_size,
         const bool show_overlay) {
  if (id.empty() || !utils::string::is_numeric(id))
    return false;

  static std::once_flag started;
  std::call_once(started, [] { std::thread(worker).detach(); });

  {
    std::lock_guard lock(mutex);
    if ((current && current->id == id) ||
        std::ranges::any_of(pending, by_id(id)))
      return false;
    pending.push_back({id, kind, title.empty() ? id : title, file_size,
                       item_state::queued, -1, show_overlay});
    std::erase_if(history, by_id(id));
    ++generation_counter;
  }
  cv.notify_all();
  return true;
}

bool remove(const std::string &id) {
  std::lock_guard lock(mutex);
  if (!std::erase_if(pending, by_id(id)))
    return false;
  ++generation_counter;
  return true;
}

bool move(const std::string &id, const int delta) {
  std::lock_guard lock(mutex);
  const auto it = std::ranges::find_if(pending, by_id(id));
  if (it == pending.end())
    return false;
  const auto from = static_cast<int>(it - pending.begin());
  const auto to =
      std::clamp(from + delta, 0, static_cast<int>(pending.size()) - 1);
  if (from == to)
    return false;
  auto item = std::move(*it);
  pending.erase(it);
  pending.insert(pending.begin() + to, std::move(item));
  ++generation_counter;
  return true;
}

void clear() {
  std::lock_guard lock(mutex);
  pending.clear();
  history.clear();
  ++generation_counter;
}

std::vector<entry> list() {
  std::lock_guard lock(mutex);
  std::vector<entry> out;
  if (current) {
    out.push_back(*current);
    out.back().position = 0;
  }
  int position = 0;
  for (const auto &e : pending) {
    out.push_back(e);
    out.back().position = ++position;
  }
  for (const auto &e : history) {
    out.push_back(e);
  }
  return out;
}

int position(const std::string &id) {
  std::lock_guard lock(mutex);
  if (current && current->id == id)
    return 0;
  const auto it = std::ranges::find_if(pending, by_id(id));
  return it == pending.end() ? -1 : static_cast<int>(it - pending.begin()) + 1;
}

std::string current_id() {
  std::lock_guard lock(mutex);
  return current ? current->id : std::string{};
}

uint32_t generation() { return generation_counter; }
} // namespace queue
} // namespace workshop
