#include <std_include.hpp>
#include <game/game.hpp>
#include "core.hpp"

namespace game {
thread_local engineDependentForEachCallback convertee;
void Converted_ForEachCallback(const dvar_t *dvar, void *userData) {
  return convertee(dvar, userData);
}
} // namespace game