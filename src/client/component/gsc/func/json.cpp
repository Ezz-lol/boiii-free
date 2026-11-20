#include <std_include.hpp>

#include "func.hpp"

#include "io/fs.hpp"
#include "script_value.hpp"

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

#include <rapidjson/prettywriter.h>
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
  push(inst, !doc.HasParseError());
}

std::string serialize(const rapidjson::Value &value, const int indent) {
  rapidjson::StringBuffer buffer;
  if (indent > 0) {
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    writer.SetIndent(' ', static_cast<uint32_t>(std::min(indent, 16)));
    value.Accept(writer);
  } else {
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
  }
  return std::string(buffer.GetString(), buffer.GetSize());
}

int indent_arg(scriptInstance_t inst, const uint32_t index) {
  return Scr_GetNumParam(inst) > index ? Scr_GetInt(inst, index) : 0;
}

void gscr_jsonserialize(scriptInstance_t inst) {
  rapidjson::Document document;
  const rapidjson::Value value =
      script_value::read(inst, 0, document.GetAllocator());
  push_string(inst, serialize(value, indent_arg(inst, 1)));
}

void gscr_jsonprint(scriptInstance_t inst) {
  rapidjson::Document document;
  std::string line;
  for (uint32_t i = 0; i < Scr_GetNumParam(inst); ++i) {
    if (i > 0) {
      line += ' ';
    }
    const rapidjson::Value value =
        script_value::read(inst, i, document.GetAllocator());
    line +=
        value.IsString() ? script_value::to_string(value) : serialize(value, 0);
  }
  game::com::Com_Printf(consoleChannel_e::CHANNEL_DONT_FILTER,
                        consoleLabel_e::DEFAULT, "%s\n", line.c_str());
}

void gscr_createmap(scriptInstance_t inst) {
  rapidjson::Document document(rapidjson::kObjectType);
  auto &allocator = document.GetAllocator();
  for (uint32_t i = 0; i + 1 < Scr_GetNumParam(inst); i += 2) {
    const std::string key =
        script_value::to_string(script_value::read(inst, i, allocator));
    document.RemoveMember(key.c_str());
    document.AddMember(rapidjson::Value(key.c_str(), allocator),
                       script_value::read(inst, i + 1, allocator), allocator);
  }
  script_value::push(inst, document);
}

void gscr_jsonparse_value(scriptInstance_t inst) {
  const char *json_str = Scr_GetString(inst, 0);
  rapidjson::Document document;
  document.Parse<rapidjson::kParseIterativeFlag>(json_str ? json_str : "");
  if (document.HasParseError()) {
    Scr_AddUndefined(inst);
    return;
  }
  script_value::push(inst, document);
}

// jsonparse(json_string, key), returns value as string
void gscr_jsonparse(scriptInstance_t inst) {
  if (Scr_GetNumParam(inst) == 1) {
    gscr_jsonparse_value(inst);
    return;
  }
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

// jsondump(filepath, value[, indent]), writes json to file
void gscr_jsondump(scriptInstance_t inst) {
  const char *path = Scr_GetString(inst, 0);
  if (!path || !io::fs::is_safe_path(path)) {
    push(inst, false);
    return;
  }
  std::string json_str;
  if (var::ScrVar_StringLike(Scr_GetType(inst, 1))) {
    json_str = Scr_GetString(inst, 1);
  } else {
    rapidjson::Document document;
    json_str = serialize(script_value::read(inst, 1, document.GetAllocator()),
                         indent_arg(inst, 2));
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
    register_builtin("jsonparse", gscr_jsonparse, 1, 2);
    register_builtin("jsonset", gscr_jsonset, 3);
    register_builtin("jsondump", gscr_jsondump, 2, 3);
    register_builtin("jsonserialize", gscr_jsonserialize, 1, 2);
    register_variadic_builtin("jsonprint", gscr_jsonprint);
    register_variadic_builtin("createmap", gscr_createmap, 0);
  }
};
} // namespace json
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::json::component)
