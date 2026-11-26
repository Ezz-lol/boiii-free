#include <std_include.hpp>

#include "game/game.hpp"
#include "game/utils.hpp"
#include "game_event.hpp"
#include "loader/component_loader.hpp"
#include "scheduler.hpp"
#include "zombies/companion.hpp"
#include "zombies/diagnostic_ring.hpp"
#include "zombies/diagnostics.hpp"
#include <utils/flags.hpp>

#if __has_include(<version.hpp>)
#include <version.hpp>
#endif
#ifndef VERSION
#define VERSION "unknown"
#endif
#ifndef GIT_HASH
#define GIT_HASH "unknown"
#endif
#ifndef GIT_DIRTY
#define GIT_DIRTY 1
#endif

namespace zombies::coop {
namespace {
constexpr std::size_t cached_limit = 8192;
diagnostics_detail::event_buffer events;
diagnostics_detail::snapshot_text<cached_limit> cached_state;
std::atomic<int> image_role{0};
std::atomic_bool debug_output{false};

std::string safe_token(std::string_view value, std::size_t limit = 64) {
  std::string result;
  result.reserve(std::min(value.size(), limit));
  for (std::size_t i = 0; i < value.size() && i < limit; ++i)
    result.push_back(
        static_cast<char>(diagnostics_detail::label_char(value[i])));
  return result;
}

game::EngineDependentDvarMut diagnostic_dvar(const char *name) {
  auto dvar = game::get_dvar(name);
  return dvar ? dvar.resolve() : dvar;
}
int read_integer(const char *name) {
  const auto dvar = diagnostic_dvar(name);
  if (!dvar)
    return 0;
  if (dvar.type() == game::dvarType_t::BOOL)
    return dvar.get_bool() ? 1 : 0;
  if (dvar.type() == game::dvarType_t::INT ||
      dvar.type() == game::dvarType_t::ENUM)
    return dvar.get_int();
  if (dvar.type() == game::dvarType_t::STRING) {
    const char *value = dvar.get_cstring();
    return value ? diagnostics_detail::parse_integer(
                       std::string_view(value, strnlen(value, 32)))
                 : 0;
  }
  return 0;
}
std::string read_token(const char *name) {
  const auto dvar = diagnostic_dvar(name);
  if (!dvar || dvar.type() != game::dvarType_t::STRING)
    return {};
  const char *value = dvar.get_cstring();
  return value ? safe_token(std::string_view(value, strnlen(value, 64)))
               : std::string{};
}

void capture_snapshot() {
  try {
    // This explicit list is sampled only on the normal main-thread pipeline.
    // Never enumerate dvars, command lines, userinfo, credentials or endpoints.
    constexpr const char *integers[] = {"sv_running",
                                        "com_maxclients",
                                        "zm_coopAppliedLimit",
                                        "zm_coopStartupEpoch",
                                        "zm_coopReadyEpoch",
                                        "zm_coopHeartbeat",
                                        "zm_coopSchemaSlots",
                                        "zm_coopSchemaServerRevision",
                                        "zm_coopSchemaClientRevision",
                                        "zm_coopSchemaServerMask",
                                        "zm_coopSchemaClientMask",
                                        "zm_coopPlayers",
                                        "zm_coopRoundNumber",
                                        "zm_coopRoundTarget",
                                        "zm_coopRoundConsumed",
                                        "zm_coopRoundPending"};
    constexpr const char *strings[] = {"mapname",
                                       "g_gametype",
                                       "zm_coopStatus",
                                       "zm_coopReadyMap",
                                       "zm_coopRoundStatus",
                                       "zm_coopMapAdapter",
                                       "zm_coopOwnerStatus",
                                       "zm_coopSpawnStatus",
                                       "zm_coopGumStatus",
                                       "zm_coopLastEvent",
                                       "zm_coopSchemaServerContract",
                                       "zm_coopSchemaClientContract", "zm_coopIslandTrialsStatus"};
    std::string snapshot =
        "{\"sample_tick_ms\":" + std::to_string(GetTickCount64());
    snapshot +=
        ",\"mode\":" +
        std::to_string(static_cast<int>(game::com::Com_SessionMode_GetMode()));
    snapshot += ",\"dvars\":{";
    bool first = true;
    for (const auto *name : integers) {
      if (!first)
        snapshot += ',';
      first = false;
      snapshot +=
          "\"" + std::string(name) + "\":" + std::to_string(read_integer(name));
    }
    for (const auto *name : strings)
      snapshot += ",\"" + std::string(name) + "\":\"" + read_token(name) + "\"";
    snapshot += "}}";
    if (snapshot.size() >= cached_limit) {
      diagnostic_event("snapshot_overflow",
                       static_cast<std::int64_t>(snapshot.size()));
      cached_state.publish("{\"status\":\"overflow\"}");
      return;
    }
    cached_state.publish(snapshot);
    if (debug_output.load()) {
      static std::uint64_t printed = 0;
      std::array<diagnostics_detail::event, diagnostics_detail::event_limit>
          history;
      const auto count = events.copy(history);
      for (std::size_t i = 0; i < count; ++i)
        if (history[i].sequence > printed) {
          const auto &event = history[i];
          printf("[coop_debug] tick=%llu phase=%s a=%lld b=%lld\n",
                 static_cast<unsigned long long>(event.tick_ms),
                 event.phase.data(), static_cast<long long>(event.a),
                 static_cast<long long>(event.b));
          printed = event.sequence;
        }
      printf("[coop_debug] snapshot=%s\n", snapshot.c_str());
    }
  } catch (...) {
    diagnostic_event("snapshot_failed");
  }
}

void map_initialized() { diagnostic_event("map_init"); }
void map_shutdown() { diagnostic_event("map_shutdown"); }
} // namespace

void diagnostic_event(std::string_view phase, std::int64_t a,
                      std::int64_t b) noexcept {
  events.record(phase, a, b, GetTickCount64());
}

std::string crash_diagnostics() {
  // All engine/dvar reads belong to capture_snapshot(). A crashed thread uses
  // bounded atomics even if another thread faulted while publishing an update.
  try {
    const auto role = image_role.load();
    std::string result;
    result.reserve(16384);
    result = "{\"family\":\"Ezz\",\"role\":\"";
    result += role == 1 ? "dedicated" : role == 2 ? "client" : "unknown";
    result += "\",\"version\":\"" + safe_token(VERSION) + "\",\"commit\":\"" +
              safe_token(GIT_HASH);
    result += "\",\"source_dirty\":" + std::to_string(GIT_DIRTY);
    result += ",\"manifest_contract_sha256\":\"" +
              std::string(companion_contract) + "\"";
    std::array<char, cached_limit> snapshot;
    std::size_t length = 0;
    result += ",\"capture_tick_ms\":" + std::to_string(GetTickCount64());
    result += ",\"cached_state\":";
    if (cached_state.copy(snapshot, length))
      result.append(snapshot.data(), length);
    else
      result += "null";
    result += ",\"events_dropped\":" + std::to_string(events.dropped()) +
              ",\"events\":[";
    std::array<diagnostics_detail::event, diagnostics_detail::event_limit>
        history;
    const auto count = events.copy(history);
    for (std::size_t i = 0; i < count; ++i) {
      const auto &event = history[i];
      if (i)
        result += ',';
      result += "{\"sequence\":" + std::to_string(event.sequence) +
                ",\"tick_ms\":" + std::to_string(event.tick_ms);
      result += ",\"phase\":\"" + std::string(event.phase.data()) +
                "\",\"a\":" + std::to_string(event.a);
      result += ",\"b\":" + std::to_string(event.b) + "}";
    }
    result += "]}";
    return result;
  } catch (...) {
    return "{}";
  }
}

struct diagnostics_component final : generic_component {
  DEFINE_COMPONENT_NAME("zombies_diagnostics");
  void post_unpack() override {
    image_role.store(game::is_server() ? 1 : 2);
    debug_output.store(utils::flags::has_flag("coop-debug"));
    diagnostic_event("diagnostics_start", image_role.load());
    game_event::on_g_init_game(map_initialized);
    game_event::on_g_shutdown_game(map_shutdown);
    scheduler::loop(capture_snapshot, scheduler::pipeline::main,
                    std::chrono::milliseconds(1000));
  }
};
} // namespace zombies::coop

REGISTER_COMPONENT(zombies::coop::diagnostics_component)
