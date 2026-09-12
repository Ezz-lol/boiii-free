#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <game/game.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

namespace gamesettings {
namespace {
// <name, path>
std::unordered_map<std::string, std::string> game_settings_files;

std::string
get_game_settings_name(const std::vector<std::string_view> &sub_strings) {
  if (sub_strings.size() > 2) {
    return std::string(sub_strings[sub_strings.size() - 2]) + '/' +
           std::string(sub_strings[sub_strings.size() - 1]);
  }

  return {};
}

std::string get_game_settings_path(const std::string &name) {
  const std::unordered_map<std::string, std::string>::iterator itr =
      game_settings_files.find(name);
  return (itr == game_settings_files.end()) ? std::string() : itr->second;
}

void search_game_settings_folder(const std::string &game_settings_dir) {
  if (!utils::io::directory_exists(game_settings_dir)) {
    return;
  }

  const std::vector<std::filesystem::path> files =
      utils::io::list_files(game_settings_dir, true);

  for (const std::filesystem::path &path : files) {
    if (!std::filesystem::is_directory(path)) {
      const std::string path_str = path.generic_string();
      const std::vector<std::string_view> sub_strings =
          utils::string::split(std::string_view(path_str), '/');
      game_settings_files.insert_or_assign(get_game_settings_name(sub_strings),
                                           path.generic_string());
    }
  }
}

bool has_game_settings_file_on_disk(const char *path) {
  if (!path) {
    return false;
  }

  const std::vector<std::string_view> sub_strings =
      utils::string::split(std::string_view(path), '/');
  const std::string game_settings_name = get_game_settings_name(sub_strings);

  return !get_game_settings_path(game_settings_name).empty();
}

void cmd_exec_stub(utils::hook::assembler &a) {
  const asmjit::Label exec_from_fastfile = a.newLabel();
  const asmjit::Label exec_from_disk = a.newLabel();

  a.pushad64();

  a.mov(rcx, r10);
  a.call_aligned(has_game_settings_file_on_disk);
  a.cmp(rax, 1);
  a.popad64();

  a.jnz(exec_from_fastfile);

  a.bind(exec_from_disk);
  a.jmp(game::select(0x1420ED087, 0x1404F855E));

  a.bind(exec_from_fastfile);
  a.lea(rdx, ptr(rsp, (game::is_server() ? 0x30 : 0x40)));
  a.jmp(game::select(0x1420ED007, 0x1404F853F));
}

int read_file_stub(const char *qpath, void **buffer) {
  const std::vector<std::string_view> sub_strings =
      utils::string::split(std::string_view(qpath), '/');
  const std::string game_settings_name = get_game_settings_name(sub_strings);

  std::string gamesettings_data;
  utils::io::read_file(get_game_settings_path(game_settings_name),
                       &gamesettings_data);

  if (!gamesettings_data.empty()) {
    ++(*game::fs::fs_loadStack);

    int32_t len = static_cast<int32_t>(gamesettings_data.length());
    char *buf = game::fs::FS_AllocMem(len + 1);

    *buffer = buf;
    gamesettings_data.copy(reinterpret_cast<char *>(*buffer), len);
    buf[len] = '\0';

    return len;
  }

  return utils::hook::invoke<int>(game::select(0x1422A48D0, 0x140564F70), qpath,
                                  buffer);
}

void search_gamesettings_files_on_disk() {
  const utils::nt::library host{};

  search_game_settings_folder(
      (game::get_appdata_path() / "data/gamesettings").string());
  search_game_settings_folder(
      (host.get_folder() / "boiii/gamesettings").string());
}
} // namespace

struct component final : generic_component {
  void post_unpack() override {
    search_gamesettings_files_on_disk();

    utils::hook::call(game::select(0x1420ED0A1, 0x1404F857D), read_file_stub);
    utils::hook::jump(game::select(0x1420ED002, 0x1404F853A),
                      utils::hook::assemble(cmd_exec_stub));
  }
};
}; // namespace gamesettings

REGISTER_COMPONENT(gamesettings::component)