#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace live {
namespace qos {
WEAK symbol<int32_t(const qosServer_t *server)> Live_Qos_GetPingForDatacenter{
    0x141E89EB0, 0x141E96940, 0x140460F50};
WEAK symbol<const qosServer_t *(uint32_t id)> Live_Qos_GetDatacenterById{
    0x141E89DE0, 0x141E96870, 0x140460E80};
} // namespace qos
} // namespace live
} // namespace game
