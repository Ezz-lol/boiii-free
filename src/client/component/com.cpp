#include <cstdint>
#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include <game/game.hpp>

#include "com.hpp"

#include <utils/concurrency.hpp>
#include <utils/hook.hpp>

namespace com {
namespace {
using LoadTasks = std::vector<LoadTask>;
utils::concurrency::container<LoadTasks> com_loadlevelfastfiles_tasks;
utils::concurrency::container<LoadTasks> com_unloadlevelfastfiles_tasks;

utils::hook::detour Com_LoadLevelFastFiles_hook;
void Com_LoadLevelFastFiles_stub(const char *level) {
  Com_LoadLevelFastFiles_hook.invoke(level);
  if (level) {
    com_loadlevelfastfiles_tasks.access([level](LoadTasks &tasks) {
      for (const auto &func : tasks) {
        func(level);
      }
    });
  }
}

void execute_unload_callbacks(const char *level) {
  if (level && level[0]) {
    com_unloadlevelfastfiles_tasks.access([level](LoadTasks &tasks) {
      for (const auto &func : tasks) {
        func(level);
      }
    });
  }
}

static const uint32_t LEVEL_ZONE_ALLOC_FLAGS_MASK = 0x64;
utils::hook::detour DB_LoadXAssets_hook;
void DB_LoadXAssets_stub(game::db::xzone::XZoneInfo *zoneInfo,
                         uint32_t zoneCount, game::qboolean sync,
                         game::qboolean suppressSync) {
  bool is_level_fastfile_unload = false;
  if (zoneInfo) {
    bool alloc_flags_include_level_zone =
        /* All zones*/ zoneInfo->allocFlags == 0 ||
        /* Only level zone */ (zoneInfo->allocFlags &
                               LEVEL_ZONE_ALLOC_FLAGS_MASK) ==
            LEVEL_ZONE_ALLOC_FLAGS_MASK;
    /*
      Usuallly, free flags are set to one of:
      - 0x2008A00
      - 0x2008000
      - 0x2000900
      However, this varies widely depending on the timing of the level zone
      unload, and is not required to uniquely identify a level zone unload
      operation, so we do not check free flags here.
    */
    is_level_fastfile_unload = zoneInfo->name == nullptr &&
                               alloc_flags_include_level_zone && zoneCount == 1;
  }
  DB_LoadXAssets_hook.invoke(zoneInfo, zoneCount, sync, suppressSync);
  if (is_level_fastfile_unload) {
    char *level = reinterpret_cast<char *>(*game::g_mapname);
    if (level[0]) {
      execute_unload_callbacks(level);
    }
  }
}
} // namespace

void on_level_load(const LoadTask &callback) {
  com_loadlevelfastfiles_tasks.access(
      [&callback](LoadTasks &tasks) { tasks.emplace_back(callback); });
}

void on_level_unload(const LoadTask &callback) {
  com_unloadlevelfastfiles_tasks.access(
      [&callback](LoadTasks &tasks) { tasks.emplace_back(callback); });
}

class component final : public generic_component {
public:
  void post_unpack() override {
    /*
      Com_UnloadLevelFastFiles and Com_TryUnloadLevelFastFiles
      are often inlined - hooking these functions will not allow callback
      execution on all instances of fastfile unload.

      Instead, we can detect fastfile unload by hooking DB_LoadXAssets and
      checking if the given `XZoneInfo` will trigger a level fastfile zone
      unload.
    */
    DB_LoadXAssets_hook.create(game::db::load::DB_LoadXAssets.get(),
                               DB_LoadXAssets_stub);
    Com_LoadLevelFastFiles_hook.create(game::com::Com_LoadLevelFastFiles.get(),
                                       Com_LoadLevelFastFiles_stub);
  }
};
} // namespace com

REGISTER_COMPONENT(com::component)