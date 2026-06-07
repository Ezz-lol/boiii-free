#include <std_include.hpp>
#include "level.hpp"
#include "../structs/structs.hpp"
#include <atomic>

namespace game {
namespace level {
std::atomic<gentity_pool *> g_entities_cl_allocation = nullptr;
} // namespace level
} // namespace game