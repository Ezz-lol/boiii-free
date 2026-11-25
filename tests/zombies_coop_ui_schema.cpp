#include "../src/client/component/zombies/ui_schema_policy.hpp"
#include <cassert>
#include <string>

int main() {
  using zombies::coop::score_model_chunk;
  assert(score_model_chunk("ui/uieditor/clientfieldmodels.lua"));
  assert(score_model_chunk("@ui/uieditor/clientfieldmodels.lua"));
  assert(score_model_chunk("@UI\\UIEDITOR\\CLIENTFIELDMODELS.LUA"));
  for (const auto name : {"clientfieldmodels.lua", "@@ui/uieditor/clientfieldmodels.lua",
      "/ui/uieditor/clientfieldmodels.lua", "../ui/uieditor/clientfieldmodels.lua",
      "ui/uieditor/../uieditor/clientfieldmodels.lua", "ui/uieditor/clientfieldmodels.lua.bak",
      "ui/uieditor/clientfieldmodels.lua/", "ui/uieditor/clientfieldmodels.luac",
      "x/ui/uieditor/clientfieldmodels.lua", "ui/uieditor//clientfieldmodels.lua", ""})
    assert(!score_model_chunk(name));
  auto embedded = std::string("ui/uieditor/clientfieldmodels.lua");
  embedded.push_back('\0'); embedded += "other";
  assert(!score_model_chunk(embedded));
  assert(!score_model_chunk(std::string(65536, 'a')));
}
