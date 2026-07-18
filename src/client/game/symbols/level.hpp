#pragma once

#include <game/symbols/sym_include.hpp>

#include <cstdint>
#include <atomic>

namespace game {
namespace level {
WEAK symbol<level::gentity_pool> g_entities{0x0, 0x1471031B0};

/*
  Statically allocated on server, heap allocated with `malloc(0x27C000u);` on
  client.

   Notes:
     - 0x27C000 == sizeof(level::gentity_t) * 2048
     - **DO NOT** use `g_entities_cl` directly. Use `g_entities_cl_allocation`
      instead. See note above `store_g_entities_cl_allocation` in
      src/client/component/client_patches.cpp for context.
*/
WEAK symbol<level::gentity_pool *> g_entities_cl{0x14A5F25F8};
extern std::atomic<level::gentity_pool *> g_entities_cl_allocation;
inline gentity_pool *get_g_entities() {
  if (game::is_client()) {
    /*
      In the client, for each function where g_entities is accessed,
      in each of its calling functions, prior to its callsite,
      Treyarch's anticheat (TAC) will copy the pointer stored in g_entities
      into another memory address or stack pointer and set g_entities to
      0xFFFFFFFFFFFFFFFF. In the called function, g_entities will then be
      accessed through the pointer stored at the destination of the copy,
      using a series of highly obfuscated operations. After the call is
      completed, TAC will copy the g_entities pointer back into the global
      g_entities memory address through, again, a series of highly obfuscated
      operations.

      This obviously makes naive usage of the g_entities global impractical,
      as it will often be set to 0xFFFFFFFFFFFFFFFF during or before access,
      causing unexpected memory access exceptions.

      To circumvent this, and to avoid having to find some way of
      deterministically computing where the correct g_entities pointer was
      copied, we can instead store the g_entities pointer at time of
      allocation into our own global, and use this identically and reliably
      within boiii's code.
    */
    gentity_pool *stored = g_entities_cl_allocation.load();
    if (stored) {
      return stored;
    }
    return *g_entities_cl;
  }

  return g_entities.get();
}

template <typename T, typename = typename std::enable_if<
                          std::is_convertible<T, uint32_t>::value>::type>
inline gentity_t *entity(T input_index) {
  uint32_t index = static_cast<uint32_t>(input_index);
  if (index < GENTITY_POOL_LEN) {
    gentity_pool *pool = get_g_entities();
    if (pool) {
      return &pool->pool[index];
    }
  }
  return nullptr;
}

WEAK symbol<int32_t> level_time{0x14A5502C4, 0x1474FDC94};
WEAK symbol<int32_t> level_rounds_played{0x14A55BDEC, 0x1475097BC};
} // namespace level
} // namespace game
