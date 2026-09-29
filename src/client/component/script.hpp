#pragma once

#include <game/game.hpp>
namespace script {
using namespace game::db::xasset;
using namespace game::scr;
void load_rawfiles();

ScriptParseTree *get_loaded_script(const std::string &name);
RawFile *get_loaded_rawfile(const std::string &name);

void for_each_loaded_script(
    const std::function<void(const ScriptParseTree &)> &callback);

std::string resolve_hash(ScrVarCanonicalName_t hash);
std::string get_source_line(const std::string &file, int32_t line_num);
std::vector<std::string> get_script_callstack(scriptInstance_t inst);
} // namespace script
