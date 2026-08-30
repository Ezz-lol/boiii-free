#pragma once

#include <game/game.hpp>
namespace script {
using namespace game::db::xasset;
using namespace game::scr;
void load_rawfiles();

ScriptParseTree *get_loaded_script(const std::string &name);
RawFile *get_loaded_rawfile(const std::string &name);
} // namespace script