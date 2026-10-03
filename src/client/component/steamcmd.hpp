#pragma once

#include <std_include.hpp>

namespace steamcmd {
class process {
public:
  process(std::filesystem::path dir, bool wine);
  ~process();

  process(const process &) = delete;
  process &operator=(const process &) = delete;

  bool start(const std::string &args);
  bool wait(std::chrono::milliseconds timeout);
  void terminate();
  uint32_t exit_code();
  uint64_t transferred_bytes();
  bool counts_disk_writes() const;
  bool missing_libraries() const;

private:
  std::filesystem::path dir_;
  bool wine_;
  PROCESS_INFORMATION pi_{};
};

class progress {
public:
  explicit progress(const std::filesystem::path &dir);

  void update(process &steamcmd);
  bool started() const;
  bool committing() const;
  uint64_t installed_bytes() const;
  uint64_t install_total() const;
  double fraction() const;
  double speed() const;
  int64_t eta_seconds() const;

private:
  std::vector<std::pair<std::filesystem::path, uint64_t>> logs_;
  bool committing_ = false;
  bool by_writes_ = false;
  uint64_t resumed_ = 0;
  uint64_t total_ = 0;
  uint64_t install_total_ = 0;
  uint64_t baseline_ = 0;
  uint64_t previous_ = 0;
  uint64_t received_ = 0;
  double speed_ = 0.0;
  std::chrono::steady_clock::time_point last_sample_{};
};

bool ensure_installed(const std::filesystem::path &dir, bool wine,
                      const std::function<void(const std::string &)> &status,
                      std::string &error);
void clear_downloads(const std::filesystem::path &dir);
std::vector<std::filesystem::path>
workshop_roots(const std::filesystem::path &dir, bool wine);

void initialize_download(std::string workshop_id, std::string modtype);
} // namespace steamcmd
