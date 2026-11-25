#include <std_include.hpp>

#include "zombies/companion.hpp"
#include "zombies/diagnostics.hpp"
#include "zombies/ui_schema.hpp"
#include "zombies/ui_schema_policy.hpp"

namespace zombies::coop {
namespace {
namespace hks = game::lua::hks;
// Exact client bindings already used by Ezz's HKS API and our LobbyVM loader.
// Only called for the primary client UI VM; these are not new engine hooks.
const game::symbol<int(hks::lua_State*, int, int, int)> protected_call{0x141D41900};
const game::symbol<void(hks::lua_State*, int)> push_value{0x1414295F0};

bool function(const hks::HksObject& object) noexcept {
  return object.t == hks::HksObjectType::TIFUNCTION || object.t == hks::HksObjectType::TCFUNCTION;
}
std::string_view chunk_name(const char* name) noexcept {
  if (!name) return {};
  for (std::size_t i = 0; i <= 256; ++i)
    if (!name[i]) return {name, i};
  return {};
}
void mark_failure() noexcept {
  diagnostic_event("schema-ui-wrap-failure");
  try { schema_asset_failure("Could not wrap the early score UI schema chunk."); } catch (...) {}
}

int error_result(hks::lua_State* state, std::ptrdiff_t original, int status,
    bool has_error) {
  mark_failure();
  if (has_error && state->m_apistack.top > state->m_apistack.bottom + original + 1) {
    // Preserve the actual compiler/pcall error object, replacing the original
    // chunk as an ordinary failed load would. Never fall back to stock code.
    state->m_apistack.bottom[original] = state->m_apistack.top[-1];
    state->m_apistack.top = state->m_apistack.bottom + original + 1;
  } else {
    state->m_apistack.top = state->m_apistack.bottom + original;
    constexpr char error[] = "Experimental Zombies UI schema wrapper failed";
    hks::hksi_lua_pushlstring(state, error, sizeof(error) - 1);
  }
  return status ? status : static_cast<int>(hks::HksError::ERRRUN);
}
} // namespace

int wrap_score_model_chunk(hks::lua_State* state, const char* name, int compiler_status) noexcept {
  if (compiler_status || !game::is_client() || !state || state != *hks::primary_luaVM ||
      !score_model_chunk(chunk_name(name))) return compiler_status;
  refresh_schema_session();
  const auto slots = frozen_schema_capacity();
  if (slots < 5 || slots > 8) return compiler_status;
  if (!state->m_global || !state->m_apistack.bottom ||
      state->m_apistack.top <= state->m_apistack.base) {
    mark_failure();
    return static_cast<int>(hks::HksError::ERRRUN);
  }
  // Indices survive API-stack growth; never retain a raw stack pointer across
  // compilation or protected calls. The original closure remains GC-rooted.
  const auto original = state->m_apistack.top - state->m_apistack.bottom - 1;
  const auto previous_mode = state->m_global->m_bytecodeSharingMode;
  try {
    if (!function(state->m_apistack.top[-1]) || schema_assets_failed())
      return error_result(state, original, 0, false);
    const auto source = score_schema_wrapper();
    if (source.empty()) return error_result(state, original, 0, false);
    state->m_global->m_bytecodeSharingMode = hks::HksBytecodeSharingMode::ON;
    hks::HksCompilerSettings settings{};
    const int loaded = hks::hksi_hksL_loadbuffer(state, &settings, source.data(), source.size(),
        "@eroded_coop/score_models_constructor");
    state->m_global->m_bytecodeSharingMode = previous_mode;
    if (loaded) return error_result(state, original, loaded, true);
    if (state->m_apistack.top != state->m_apistack.bottom + original + 2 ||
        !function(state->m_apistack.top[-1])) return error_result(state, original, 0, false);
    // Execute only the pinned constructor chunk, then constructor(original).
    // The game executes the returned wrapper later at its normal load point.
    int status = protected_call(state, 0, 1, 0);
    if (status) return error_result(state, original, status, true);
    if (state->m_apistack.top != state->m_apistack.bottom + original + 2 ||
        !function(state->m_apistack.top[-1])) return error_result(state, original, 0, false);
    push_value(state, -2); // [original, constructor, original copy]
    status = protected_call(state, 1, 1, 0);
    if (status) return error_result(state, original, status, true);
    if (state->m_apistack.top != state->m_apistack.bottom + original + 2 ||
        !function(state->m_apistack.top[-1])) return error_result(state, original, 0, false);
    state->m_apistack.bottom[original] = state->m_apistack.top[-1];
    state->m_apistack.top = state->m_apistack.bottom + original + 1;
    diagnostic_event("schema-ui-chunk-wrapped", slots);
    return 0;
  } catch (...) {
    state->m_global->m_bytecodeSharingMode = previous_mode;
    try { return error_result(state, original, 0, false); }
    catch (...) { return static_cast<int>(hks::HksError::ERRMEM); }
  }
}
} // namespace zombies::coop
