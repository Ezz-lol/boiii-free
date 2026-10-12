#include <std_include.hpp>

#include "func.hpp"

#include "io/fs.hpp"

#include <game/impl/scr/scr.hpp>
#include <game/impl/scr/var.hpp>
#include <game/impl/sv/sv.hpp>
#include <game/utils.hpp>

#include <component/command.hpp>
#include <component/game_event.hpp>
#include <component/name.hpp>
#include <component/toast.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <rapidjson/writer.h>

using namespace game;
using namespace game::scr;

namespace gsc {
namespace func {
namespace json {
// =====================================================
// JSON builtins, simple string-based operations
// =====================================================

void gscr_jsonvalid(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 0);
  if (!json_str) {
    push(inst, 0);
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  push(inst, doc.HasParseError());
}

// jsonparse(json_string, key), returns value as string
void gscr_jsonparse(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 0);
  const char *key = Scr_GetString(inst, 1);
  if (!json_str || !key) {
    push_string(inst, "");
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  if (doc.HasParseError() || !doc.IsObject() || !doc.HasMember(key)) {
    push_string(inst, "");
    return;
  }
  const auto &val = doc[key];
  if (val.IsString())
    push_string(inst, val.GetString());
  else if (val.IsInt())
    push_string(inst, std::to_string(val.GetInt()).c_str());
  else if (val.IsInt64())
    push_string(inst, std::to_string(val.GetInt64()).c_str());
  else if (val.IsDouble())
    push_string(inst, std::to_string(val.GetDouble()).c_str());
  else if (val.IsBool())
    push_string(inst, val.GetBool() ? "true" : "false");
  else if (val.IsNull())
    push_string(inst, "null");
  else {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    val.Accept(writer);
    push_string(inst, buf.GetString());
  }
}

// jsonset(json_string, key, value_string), sets key, returns modified json
void gscr_jsonset(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 0);
  const char *key = Scr_GetString(inst, 1);
  const char *val_str = Scr_GetString(inst, 2);
  if (!json_str || !key || !val_str) {
    push_string(inst, json_str ? json_str : "{}");
    return;
  }
  rapidjson::Document doc;
  doc.Parse(json_str);
  if (doc.HasParseError() || !doc.IsObject())
    doc.SetObject();

  auto &alloc = doc.GetAllocator();

  // try to parse the value as JSON first
  rapidjson::Document val_doc;
  val_doc.Parse(val_str);

  rapidjson::Value k(key, alloc);

  if (doc.HasMember(key))
    doc.RemoveMember(key);

  if (val_doc.HasParseError()) {
    rapidjson::Value v(val_str, alloc);
    doc.AddMember(k, v, alloc);
  } else {
    rapidjson::Value v;
    v.CopyFrom(val_doc, alloc);
    doc.AddMember(k, v, alloc);
  }

  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
  doc.Accept(writer);
  push_string(inst, buf.GetString());
}

// jsondump(filepath, json_string), writes json to file
void gscr_jsondump(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  const char *json_str = Scr_GetString(inst, 1);
  if (!path || !json_str || !io::fs::is_safe_path(path)) {
    push(inst, false);
    return;
  }
  const std::filesystem::path full = io::fs::resolve_path(path);
  const std::filesystem::path parent = full.parent_path();
  if (!parent.empty())
    utils::io::create_directory(parent);
  push(inst, utils::io::write_file(full.string(), json_str));
}

struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/json");

  void post_unpack() override {

    // JSON
    register_builtin("jsonvalid", gscr_jsonvalid, 1);
    register_builtin("jsonparse", gscr_jsonparse, 2);
    register_builtin("jsonset", gscr_jsonset, 3);
    register_builtin("jsondump", gscr_jsondump, 2);
  }
};
} // namespace json
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::json::component)
