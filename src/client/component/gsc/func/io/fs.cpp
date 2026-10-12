#include <std_include.hpp>

#include "fs.hpp"

#include <component/gsc/func/func.hpp>

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/name.hpp>
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace io {
namespace fs {

const std::filesystem::path &scriptdata_path() {
  static const std::filesystem::path result = []() -> std::filesystem::path {
    const utils::nt::library host{};
    return host.get_folder() / "boiii" / "scriptdata";
  }();

  return result;
}

// =====================================================
// File I/O builtins, paths relative to boiii/scriptdata/
// =====================================================

void gscr_writefile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  const char *data = Scr_GetString(inst, 1);

  if (path && data && is_safe_path(path)) {
    const std::filesystem::path full = resolve_path(path);
    const std::filesystem::path parent = full.parent_path();
    if (!parent.empty()) {
      utils::io::create_directory(parent);
    }
    bool append = Scr_GetBoolOptional(inst, 2, false);

    qboolean result =
        qboolean::from(utils::io::write_file(full.string(), data, append));
    push(inst, result);
  } else {
    push(inst, 0);
  }
}

void gscr_readfile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (path && is_safe_path(path)) {
    const std::filesystem::path full = resolve_path(path);
    std::string data;
    if (utils::io::read_file(full.string(), &data)) {
      push_string(inst, data.c_str());
    } else {
      push_string(inst, "");
    }
  } else {
    push_string(inst, "");
  }
}

void gscr_appendfile(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  const char *data = Scr_GetString(inst, 1);
  if (path && data && is_safe_path(path)) {
    const std::filesystem::path full = resolve_path(path);
    const std::filesystem::path parent = full.parent_path();
    if (!parent.empty()) {
      utils::io::create_directory(parent);
    }
    push(inst, utils::io::write_file(full.string(), data, true));
  } else {
    push(inst, 0);
  }
}

void gscr_fileexists(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (path && is_safe_path(path)) {
    push(inst, utils::io::file_exists(resolve_path(path).string()));
  } else {
    push(inst, 0);
  }
}

void gscr_removedirectory(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (path && is_safe_path(path)) {
    push(inst, utils::io::remove_directory(resolve_path(path), true));
  } else {
    push(inst, 0);
  }
}

void gscr_rm(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (path && is_safe_path(path)) {
    bool recurse = Scr_GetBoolOptional(inst, 1, false);

    if (utils::io::directory_exists(resolve_path(path))) {
      push(inst, utils::io::remove_directory(resolve_path(path), recurse));
    } else {
      push(inst, utils::io::remove_file(resolve_path(path)));
    }
  } else {
    push(inst, 0);
  }
}

void gscr_filesize(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (path && is_safe_path(path)) {
    push(inst,
         static_cast<int>(utils::io::file_size(resolve_path(path).string())));
  } else {
    push(inst, 0);
  }
}

void gscr_createdirectory(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (path && is_safe_path(path)) {
    bool result = utils::io::create_directory(resolve_path(path));
    push(inst, result);
  } else {
    push(inst, 0);
  }
}

void gscr_directoryexists(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (path && is_safe_path(path)) {
    push(inst, utils::io::directory_exists(resolve_path(path)));
  } else {
    push(inst, 0);
  }
}

/*
 ls(path, recurse = false, include_directories = false)
 Lists files in a directory, optionally recursively and including
 directories. Returns an array of file/directory paths.
*/
void gscr_ls(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (path && is_safe_path(path)) {
    bool recurse = Scr_GetBoolOptional(inst, 1, false);
    bool include_directories = Scr_GetBoolOptional(inst, 2, false);

    const std::filesystem::path full = resolve_path(path);
    if (utils::io::directory_exists(full)) {
      const std::vector<std::filesystem::path> entries =
          utils::io::list_files(full, recurse, include_directories);

      std::vector<std::string> str_entries;
      str_entries.reserve(entries.size());
      std::transform(entries.begin(), entries.end(),
                     std::back_inserter(str_entries),
                     [](const std::filesystem::path &p) -> std::string {
                       return relative_path(p).string();
                     });
      push(inst, std::move(str_entries));
      return;
    }
  }

  push(inst);
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/io/fs");

  void post_unpack() override {

    // File I/O
    register_builtin("writefile", gscr_writefile, 2, 3);
    register_builtin("readfile", gscr_readfile, 1);
    register_builtin("appendfile", gscr_appendfile, 2);
    register_builtin("fileexists", gscr_fileexists, 1);
    register_builtin<2>({"removefile", "rm"}, gscr_rm, 1, 2);
    register_builtin<2>({"rmdir", "removedirectory"}, gscr_removedirectory, 1);
    register_builtin("filesize", gscr_filesize, 1);
    register_builtin<2>({"mkdir", "createdirectory"}, gscr_createdirectory, 1);
    register_builtin("directoryexists", gscr_directoryexists, 1);
    register_builtin("ls", gscr_ls, 1, 3);
  }
};
} // namespace fs
} // namespace io
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::io::fs::component)
