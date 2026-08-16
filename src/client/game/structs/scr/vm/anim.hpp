#pragma once

#include <game/structs/scr/core.hpp>
#include <game/structs/macros.hpp>

namespace game {
namespace scr {
namespace vm {
namespace anim {

PACKED(struct scrAnimPub_t {
  uint32_t animtrees;
  uint32_t animtree_node;
  uint32_t animTreeNames;
  uint8_t _padding0C[4];
  ScrPool<scr_animtree_t> xanim_lookup[0x80];
  ScrPool<uint32_t> xanim_num;
  uint32_t animTreeIndex;
  bool animtree_loading;
  uint8_t _padding81D[3];
});
ASSERT_SIZE(scrAnimPub_t, 0x820);
} // namespace anim
} // namespace vm
} // namespace scr
} // namespace game