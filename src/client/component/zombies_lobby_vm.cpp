#include <std_include.hpp>

#include "zombies_lobby_vm.hpp"
#include "game/game.hpp"
#include "loader/component_loader.hpp"
#include <atomic>
#include <fstream>
#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/nt.hpp>

namespace zombies::coop {
namespace {
std::atomic_bool client_hooks_ready{false};
using namespace game::lua::hks;
utils::hook::detour lobby_vm_init_hook;
constexpr const char *module_path = "lobby_scripts/zombies_coop/__init__.lua";

void log_failure(const char *phase, lua_State *state) {
  const char *error = "no Lua error string";
  if (state->m_apistack.top > state->m_apistack.bottom &&
      state->m_apistack.top[-1].t == HksObjectType::TSTRING &&
      state->m_apistack.top[-1].v.str)
    error = state->m_apistack.top[-1].v.str->m_data;
  printf("[zombies_coop_lobby_vm] role=client event=%s error=%s\n", phase,
         error);
}

void load_client_lobby_hooks() {
  auto *state = game::lua::lobby::vm::LobbyVM_GetVM();
  // Never route Lobby hooks through primary UI objects, references or globals.
  if (!state || state == *primary_luaVM || !state->m_global ||
      !state->m_apistack.bottom || state->globals.t != HksObjectType::TTABLE) {
    printf("[zombies_coop_lobby_vm] role=client event=invalid_vm\n");
    return;
  }
  std::string source, path;
  const std::filesystem::path roots[] = {game::get_data_path(),
                                         game::get_boiii_dir_path()};
  for (const auto &root : roots) {
    const auto candidate = (root / module_path).string();
    std::ifstream input(candidate, std::ios::binary);
    if (!input.is_open())
      continue;
    // Read one byte beyond the accepted limit to detect oversized input;
    // never size an allocation from a corrupt companion's full file length.
    std::string bytes(65537, '\0');
    input.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    const auto count = input.gcount();
    if (input.bad() || (!input.eof() && input.fail()) || count <= 0 ||
        count > 65536) {
      printf("[zombies_coop_lobby_vm] role=client event=read_failed\n");
      return; // An invalid first opened file must not fall through to another
              // root.
    }
    source.assign(bytes.data(), static_cast<std::size_t>(count));
    path = candidate;
    break;
  }
  if (path.empty() || source.empty() || source.size() > 65536) {
    printf("[zombies_coop_lobby_vm] role=client event=read_failed\n");
    return;
  }
  // This bounded companion has no imports. Leave the primary UI relative-
  // require loader untouched rather than swapping its global VM pointer.
  const std::string code =
      source +
      "\nreturn type(LobbyVM)=='table' and LobbyVM.ZombiesCoopModuleVersion==4 "
      "and "
      "type(Lobby)=='table' and type(Lobby.Actions)=='table' and "
      "type(Lobby.Actions.LobbyHostStart)=='function' and "
      "Lobby.Actions.LobbyHostStart==Lobby.Actions.ZombiesCoopHostStart and "
      "type(LobbyVM.LaunchGame)=='function' and "
      "LobbyVM.LaunchGame==LobbyVM.ZombiesCoopLaunchGame and "
      "type(LobbyVM.CanLobbyCanMigrate)=='function' and "
      "LobbyVM.CanLobbyCanMigrate==LobbyVM.ZombiesCoopCanMigrate\n";
  const auto top = state->m_apistack.top - state->m_apistack.bottom;
  const auto mode = state->m_global->m_bytecodeSharingMode;
  const auto restore = gsl_lite::finally([&] {
    state->m_apistack.top = state->m_apistack.bottom + top;
    state->m_global->m_bytecodeSharingMode = mode;
  });
  state->m_global->m_bytecodeSharingMode = HksBytecodeSharingMode::ON;
  HksCompilerSettings settings{};
  if (hksi_hksL_loadbuffer(state, &settings, code.data(), code.size(),
                           path.c_str()) != 0) {
    log_failure("compile_failed", state);
    return;
  }
  state->m_global->m_bytecodeSharingMode = mode;
  if (hksi_lua_pcall(state, 0, 1, 0) != 0) {
    log_failure("execute_failed", state);
    return;
  }
  const bool installed =
      state->m_apistack.top > state->m_apistack.bottom + top &&
      state->m_apistack.top[-1].t == HksObjectType::TBOOLEAN &&
      state->m_apistack.top[-1].v.boolean;
  client_hooks_ready.store(installed, std::memory_order_release);
  printf("[zombies_coop_lobby_vm] role=client event=%s path=%s\n",
         installed ? "installed" : "incomplete", path.c_str());
}

void lobby_vm_init_stub() {
  client_hooks_ready.store(false, std::memory_order_release);
  lobby_vm_init_hook.invoke<void>();
  try {
    load_client_lobby_hooks();
  } catch (const std::exception &error) {
    printf("[zombies_coop_lobby_vm] role=client event=load_failed error=%s\n",
           error.what());
  }
}
} // namespace

bool client_lobby_hooks_ready() noexcept {
  return client_hooks_ready.load(std::memory_order_acquire);
}

struct lobby_vm_component final : generic_component {
  DEFINE_COMPONENT_NAME("zombies/lobby_vm");
  void post_unpack() override {
    if (!game::is_client() ||
        !utils::flags::has_flag("experimental-zombies-coop"))
      return;
    lobby_vm_init_hook.create(game::lua::lobby::vm::LobbyVM_Init.get(),
                              lobby_vm_init_stub);
    printf("[zombies_coop_lobby_vm] role=client event=hook_armed\n");
  }
};
} // namespace zombies::coop
REGISTER_COMPONENT(zombies::coop::lobby_vm_component)
