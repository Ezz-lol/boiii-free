#pragma once

#include <structs/func.hpp>

#include <cstdint>

#include "core.hpp"

#include "game/structs/scr/primitives.hpp"

namespace game {

namespace level {
namespace sv {
struct gclient_s;
typedef gclient_s gclient_t;
} // namespace sv
} // namespace level

namespace scr {
namespace field {
namespace clientscr {

struct client_fields_s;
typedef client_fields_s client_fields_t;

typedef fastcall_t<void(level::sv::gclient_t *client,
                        const client_fields_t *field)>
    ScriptCallbackClient;

struct client_fields_s {
  ScrVarCanonicalName_t canonId;
  int32_t ofs;
  int32_t size[1];
  fieldtype_t type;
  uint64_t whichbits;
  ScriptCallbackClient *setter;
  ScriptCallbackClient *getter;
};
ASSERT_SIZE(client_fields_s, 0x28);

} // namespace clientscr
} // namespace field
} // namespace scr
} // namespace game
