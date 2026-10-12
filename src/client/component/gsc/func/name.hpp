#pragma once

#include <game/game.hpp>

namespace gsc {
namespace func {
namespace name {
extern std::unordered_map<game::ClientNum_t, std::unordered_set<std::string>>
    client_dvar_changes;
}
} // namespace func
} // namespace gsc
