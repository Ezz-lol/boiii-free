#pragma once

#include <game/game.hpp>
namespace script {
using namespace game::db::xasset;
using namespace game::scr;

struct hash_info {
  std::string name;
  int32_t line;
  uint8_t params;
};

typedef concurrent_hash_map<std::string, ScriptParseTree *> loadedScriptMap_t;
extern loadedScriptMap_t loaded_scripts;

typedef concurrent_hash_map<std::string, XAsset> loadedAssetMap_t;
extern loadedAssetMap_t loaded_assets;

typedef concurrent_hash_map<uint32_t, std::vector<hash_info>>
    scriptHashNameMap_t;
extern scriptHashNameMap_t script_hash_names;

typedef concurrent_hash_map<std::string, std::string> scriptSourceMap_t;
extern scriptSourceMap_t script_sources;

typedef concurrent_hash_map<std::string, std::vector<uint8_t>> scriptGdbMap_t;
extern scriptGdbMap_t script_gdbs;

void load_assets();
void print_asset_log(const std::string_view& message);

ScriptParseTree *get_loaded_script(const std::string &name);
XAsset get_loaded_asset(const std::string &name);

template <typename F>
concept LoadedScriptCallback = std::is_invocable_v<F, const ScriptParseTree &>;
template <LoadedScriptCallback F>
void for_each_loaded_script(const F &callback) {
  loaded_scripts.for_each([&](const loadedScriptMap_t::value_type &v) {
    if (v.second && v.second->buffer) {
      callback(*v.second);
    }
  });
}
std::string script_path(const std::string &name);

std::string resolve_hash(ScrVarCanonicalName_t hash);
std::string get_source_line(const std::string &file, int32_t line_num);
struct script_frame {
  std::string file;
  int32_t line{-1};
  std::string source;
  std::string function;
};

uint8_t *find_export_address(const std::string &script_name,
                             const std::string &func_name,
                             int expected_params = -1);
std::vector<script_frame> get_script_frames(scriptInstance_t inst,
                                            const uint8_t *pos = nullptr);
std::vector<std::string> get_script_callstack(scriptInstance_t inst,
                                              const uint8_t *pos = nullptr);
} // namespace script
