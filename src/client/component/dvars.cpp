#include "../std_include.hpp"
#include "../loader/component_loader.hpp"

#include "../game/game.hpp"
#include "../game/utils.hpp"
#include "scheduler.hpp"

#include "../../common/utils/hook.hpp"
#include "../../common/utils/io.hpp"
#include "../../common/utils/string.hpp"
#include <variant>
#include <atomic>

namespace dvars {
namespace {
std::atomic_bool dvar_write_scheduled{false};
bool initial_config_read = false;
utils::hook::detour dvar_set_variant_hook;

void dvar_for_each_name_stub(void (*callback)(const char *debugName)) {
  for (int i = 0; i < *game::g_dvarCount; ++i) {
    std::visit(
        [&callback, i](auto *dvar_pool) -> void {
          const auto *dvar = &dvar_pool->pool[i];

          if (dvar->debugName && (dvar->flags & 0x8000) == 0 &&
              (!game::com::Com_SessionMode_IsMode(game::eModes::COUNT) ||
               !game::Dvar_IsSessionModeBaseDvar(
                   reinterpret_cast<const game::dvar_t *>(dvar)))) {
            callback(dvar->debugName);
          }
        },
        game::dvar_pool());
  }
}

void dvar_for_each_name_client_num_stub(
    game::LocalClientNum_t localClientNum,
    void (*callback)(game::LocalClientNum_t localClientNum,
                     const char *debugName)) {
  for (int i = 0; i < *game::g_dvarCount; ++i) {
    std::visit(
        [&callback, localClientNum, i](auto *dvar_pool) -> void {
          const auto *dvar = &dvar_pool->pool[i];

          if (dvar->debugName && (dvar->flags & 0x8000) == 0 &&
              (!game::com::Com_SessionMode_IsMode(game::eModes::COUNT) ||
               !game::Dvar_IsSessionModeBaseDvar(
                   reinterpret_cast<const game::dvar_t *>(dvar)))) {
            callback(localClientNum, dvar->debugName);
          }
        },
        game::dvar_pool());
  }
}

void read_dvar_name_hashes_data(
    std::unordered_map<std::uint32_t, std::string> &map) {
  const std::filesystem::path path =
      game::get_appdata_path() / "data/lookup_tables/dvar_list.txt";
  std::string data;

  if (!utils::io::read_file(path, &data)) {
    printf("Failed to read Dvar lookup table\n");
    return;
  }

  const auto [beg, end] = std::ranges::remove(data, '\r');
  data.erase(beg, end);

  std::istringstream stream(data);
  std::string debug_name;

  while (std::getline(stream, debug_name, '\n')) {
    if (utils::string::starts_with(debug_name, "//")) {
      continue;
    }

    if (!debug_name.empty()) {
      map.emplace(game::Dvar_GenerateHash(debug_name.data()), debug_name);
    }
  }
}

void copy_dvar_names_to_pool() {
  std::unordered_map<std::uint32_t, std::string> dvar_hash_name_map;
  read_dvar_name_hashes_data(dvar_hash_name_map);

  for (int i = 0; i < *game::g_dvarCount; ++i) {
    std::visit(
        [&dvar_hash_name_map, i](auto *dvar_pool) -> void {
          auto *dvar = &dvar_pool->pool[i];

          if (!dvar->debugName) {
            if (dvar_hash_name_map.contains(dvar->name)) {
              dvar->debugName =
                  game::CopyString(dvar_hash_name_map[dvar->name].data());
            }
          }
        },
        game::dvar_pool());
  }
}

std::string get_config_file_path() { return "boiii_players/user/config.cfg"; }

bool is_archive_dvar(game::EngineDependentDvar dvar) {
  return std::visit(
      [](const auto *resolved) -> bool {
        if (!resolved->debugName) {
          return false;
        }

        return (resolved->flags & game::DVAR_ARCHIVE) != 0;
      },
      dvar);
}

void write_archive_dvars() {
  const std::string path = get_config_file_path();

  std::string config_buffer;

  for (int i = 0; i < *game::g_dvarCount; ++i) {
    std::visit(
        [&config_buffer, i](auto *dvar_pool) -> void {
          const auto *dvar = &dvar_pool->pool[i];
          if (is_archive_dvar(dvar)) {

            const char *name = dvar->debugName;
            const char *value = game::Dvar_DisplayableValue(
                reinterpret_cast<const game::dvar_t *>(dvar));

            config_buffer.append(
                utils::string::va("set %s \"%s\"\n", name, value));
          }
        },
        game::dvar_pool());
  }

  if (config_buffer.empty()) {
    return;
  }

  utils::io::write_file(path, config_buffer);
}

void schedule_dvar_write() {
  if (dvar_write_scheduled.exchange(true)) {
    return;
  }

  scheduler::once(
      [] {
        dvar_write_scheduled = false;
        write_archive_dvars();
      },
      scheduler::main, 10s);
}

void dvar_set_variant_stub(const game::dvar_t *dvar, game::DvarValue *value,
                           uint32_t source) {
  dvar_set_variant_hook.invoke(dvar, value, source);

  if (initial_config_read && is_archive_dvar(game::dvar_variant(dvar))) {
    schedule_dvar_write();
  }
}

void read_archive_dvars() {
  const std::string path = get_config_file_path();

  if (!utils::io::file_exists(path)) {
    initial_config_read = true;
    return;
  }

  std::string filedata;
  utils::io::read_file(path, &filedata);

  game::cbuf::Cbuf_ExecuteBuffer(0, game::ControllerIndex_t::CONTROLLER_INDEX_0,
                                 filedata.c_str());
  initial_config_read = true;
  scheduler::execute(scheduler::pipeline::dvars_loaded);
}
} // namespace

class component final : public generic_component {
public:
  void post_unpack() override {
    if (!game::is_server()) {
      scheduler::once(read_archive_dvars,
                      scheduler::pipeline::dvars_flags_patched);
      dvar_set_variant_hook.create(0x1422C9030_g, dvar_set_variant_stub);

      // Show all known dvars in console
      utils::hook::jump(0x1422BCE30_g, dvar_for_each_name_stub);
      utils::hook::jump(0x1422BCD80_g, dvar_for_each_name_client_num_stub);
    }

    scheduler::once(copy_dvar_names_to_pool, scheduler::pipeline::main);

    // All dvars are recognized as command
    utils::hook::nop(game::select(0x142151F1A, 0x14050949A), 2);
    // Show all dvars in dvarlist command
    utils::hook::nop(game::select(0x142152227, 0x140509797), 6);
    // Show all dvars in dvardump command
    utils::hook::nop(game::select(0x142151BF9, 0x140509179), 6);
    // Stops game from deleting debug names from archive dvars
    utils::hook::set<uint8_t>(game::select(0x1422C5DE0, 0x1405786D0), 0xC3);
  }
};
} // namespace dvars

REGISTER_COMPONENT(dvars::component)