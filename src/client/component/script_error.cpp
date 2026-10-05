#include <std_include.hpp>

#include "scheduler.hpp"
#include "script.hpp"
#include "script_error.hpp"

#include <loader/component_loader.hpp>

#include <component/gsc/gsc_emitter.hpp>

#include <game/game.hpp>
#include <game/impl/scr/gdb.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace script_error {
namespace {
using namespace game;
using namespace game::scr;
using namespace gsc_compiler;

constexpr ScrVarCanonicalName_t EMPTY_NAMESPACE = builtin::fnv1a("");
constexpr ScrVarCanonicalName_t SYS_NAMESPACE = builtin::SYS_NS_HASH;
constexpr uint8_t IMPORT_TYPE_MASK = 0xF;
constexpr int32_t BUILTIN_DEV_ONLY = 1;

thread_local scriptInstance_t active_instance = SCRIPTINSTANCE_SERVER;

class instance_scope {
public:
  explicit instance_scope(const scriptInstance_t inst)
      : previous_(std::exchange(active_instance, inst)) {}
  ~instance_scope() { active_instance = previous_; }

  instance_scope(const instance_scope &) = delete;
  instance_scope &operator=(const instance_scope &) = delete;

private:
  scriptInstance_t previous_;
};

struct link_failure {
  std::string error;
  std::string script;
  std::vector<int32_t> lines;
  std::string call;
  std::string cause;
  std::string fix;
  std::vector<std::string> notes;
};

struct known_script {
  std::string name;
  std::string key;
  const GSC_OBJ *obj;
};

struct builtin_info {
  bool exists{};
  int32_t type{};
  int32_t min_args{};
  int32_t max_args{};
};

inline bool valid_object(const GSC_OBJ *obj) {
  return obj && obj->hasMagic(GSC_OBJ::T7_MAGIC);
}

std::string script_key(const std::string_view name) {
  std::string key = utils::string::to_lower(std::string(name));
  std::ranges::replace(key, '\\', '/');
  if (key.ends_with(".gsc") || key.ends_with(".csc")) {
    key.resize(key.size() - 4);
  }
  return key;
}

std::string using_path(const std::string_view name) {
  std::string path(name);
  std::ranges::replace(path, '/', '\\');
  if (path.ends_with(".gsc") || path.ends_with(".csc")) {
    path.resize(path.size() - 4);
  }
  return path;
}

std::string hash_name(const ScrVarCanonicalName_t hash,
                      const std::string_view unknown_prefix) {
  std::string name = script::resolve_hash(hash);
  if (name.empty()) {
    const char *canonical = sl::SL_LookupCanonicalString(hash);
    if (canonical && canonical[0] &&
        _stricmp(canonical, std::format("{:X}", hash).c_str()) != 0) {
      name = canonical;
    }
  }
  return name.empty() ? std::format("{}_{:08x}", unknown_prefix, hash) : name;
}

std::string function_name(const ScrVarCanonicalName_t hash) {
  return hash_name(hash, "function");
}

std::string namespace_name(const ScrVarCanonicalName_t hash) {
  return hash == EMPTY_NAMESPACE ? std::string() : hash_name(hash, "namespace");
}

std::string plural(const int32_t count, const std::string_view word) {
  return std::format("{} {}{}", count, word, count == 1 ? "" : "s");
}

std::vector<std::string> object_includes(const GSC_OBJ *obj) {
  std::vector<std::string> includes;
  for (const uint32_t offset : obj->includes()) {
    includes.emplace_back(reinterpret_cast<const char *>(obj) + offset);
  }
  return includes;
}

template <typename Callback>
void for_each_import(const GSC_OBJ *obj, Callback &&callback) {
  const uint8_t *cursor =
      reinterpret_cast<const uint8_t *>(obj) + obj->imports_offset;
  for (uint16_t i = 0; i < obj->imports_count; ++i) {
    const GSC_IMPORT_ITEM *import =
        reinterpret_cast<const GSC_IMPORT_ITEM *>(cursor);
    callback(*import);
    cursor += sizeof(GSC_IMPORT_ITEM) + import->num_address * sizeof(uint32_t);
  }
}

std::vector<known_script> collect_scripts(const scriptInstance_t inst) {
  const std::string_view extension =
      inst == SCRIPTINSTANCE_CLIENT ? ".csc" : ".gsc";
  std::vector<known_script> scripts;
  std::unordered_set<std::string> seen;

  const auto add = [&](const std::string_view name, const GSC_OBJ *obj) {
    if (valid_object(obj) &&
        utils::string::to_lower(std::string(name)).ends_with(extension)) {
      std::string key = script_key(name);
      if (seen.insert(key).second) {
        scripts.push_back({std::string(name), std::move(key), obj});
      }
    }
  };

  for (uint32_t i = 0; i < gObjFileInfoCount->instance[inst]; ++i) {
    const GSC_OBJ *obj = gObjFileInfo->instance[inst][i].activeVersion;
    if (valid_object(obj)) {
      add(obj->get_name(), obj);
    }
  }

  script::for_each_loaded_script(
      [&](const ScriptParseTree &tree) { add(tree.name, tree.buffer); });

  db::xasset::DB_EnumXAssets(
      db::xasset::XAssetType::SCRIPTPARSETREE,
      [](db::xasset::XAssetHeader header, void *data) {
        const ScriptParseTree *tree = header.scriptParseTree;
        if (tree && tree->name && tree->buffer) {
          (*static_cast<decltype(add) *>(data))(tree->name, tree->buffer);
        }
      },
      const_cast<void *>(static_cast<const void *>(&add)), true);

  return scripts;
}

const known_script *find_script(const std::vector<known_script> &scripts,
                                const std::string_view name) {
  const std::string key = script_key(name);
  const std::ranges::borrowed_iterator_t<
      const std::vector<known_script, std::allocator<known_script>> &>
      it = std::ranges::find(scripts, key, &known_script::key);
  return it == scripts.end() ? nullptr : &*it;
}

builtin_info find_builtin(const scriptInstance_t inst,
                          const ScrVarCanonicalName_t name, const bool method) {
  builtin_info info{};
  const void *handler = nullptr;
  if (inst == SCRIPTINSTANCE_SERVER) {
    handler = method ? reinterpret_cast<const void *>(builtin::Scr_GetMethod(
                           name, &info.type, &info.min_args, &info.max_args))
                     : reinterpret_cast<const void *>(builtin::Scr_GetFunction(
                           name, &info.type, &info.min_args, &info.max_args));
  } else {
    handler =
        method ? reinterpret_cast<const void *>(builtin::cscr::CScr_GetMethod(
                     name, &info.type, &info.min_args, &info.max_args))
               : reinterpret_cast<const void *>(builtin::cscr::CScr_GetFunction(
                     name, &info.type, &info.min_args, &info.max_args));
  }
  info.exists = handler != nullptr;
  return info;
}

bool is_method_import(const uint8_t type) {
  return type == IMPORT_FUNC_METHOD || type == IMPORT_FUNC_METHOD_THREAD;
}

std::string describe_call(const GSC_IMPORT_ITEM &import) {
  const uint8_t type = import.flags & IMPORT_TYPE_MASK;
  const std::string ns = namespace_name(import.name_space);
  std::string name = function_name(import.name);
  if (!(import.flags & IMPORT_CALL_LOCAL) && !ns.empty() &&
      import.name_space != SYS_NAMESPACE) {
    name = ns + "::" + name;
  }

  const std::string args =
      std::format("({})", plural(import.param_count, "arg"));
  switch (type) {
  case IMPORT_FUNC_GETFUNCTION:
    return "&" + name;
  case IMPORT_FUNC_THREAD:
    return "thread " + name + args;
  case IMPORT_FUNC_METHOD:
    return "<entity> " + name + args;
  case IMPORT_FUNC_METHOD_THREAD:
    return "<entity> thread " + name + args;
  default:
    return name + args;
  }
}

std::string describe_export(const known_script &script,
                            const GSC_EXPORT_ITEM &item) {
  const std::string ns = namespace_name(item.name_space);
  return std::format("{}{}{}({}{}) in {}{}", ns,
                     ns.empty() ? "" : "::", function_name(item.name),
                     plural(item.param_count, "param"),
                     item.flags & EXPORT_VARARG ? ", vararg" : "", script.name,
                     item.flags & EXPORT_PRIVATE ? " [private]" : "");
}

bool accepts_args(const GSC_EXPORT_ITEM &item, const uint8_t args) {
  return item.param_count >= args || (item.flags & EXPORT_VARARG);
}

std::vector<std::string>
scripts_declaring_namespace(const std::vector<known_script> &scripts,
                            const ScrVarCanonicalName_t ns) {
  std::vector<std::string> names;
  for (const known_script &script : scripts) {
    for (const GSC_EXPORT_ITEM &item : script.obj->exports()) {
      if (item.name_space == ns) {
        names.push_back(script.name);
        break;
      }
    }
  }
  return names;
}

void analyze_unresolved(const scriptInstance_t inst, const GSC_OBJ *obj,
                        const GSC_IMPORT_ITEM &import,
                        const std::vector<known_script> &scripts,
                        link_failure &failure) {
  const uint8_t type = import.flags & IMPORT_TYPE_MASK;
  const bool local = import.flags & IMPORT_CALL_LOCAL;
  const bool method = is_method_import(type);
  const std::string name = function_name(import.name);
  const std::string ns = namespace_name(import.name_space);
  const std::string qualified = ns.empty() ? name : ns + "::" + name;
  const std::string self_key = script_key(obj->get_name());

  failure.error = "Unresolved external";

  std::vector<std::string> param_mismatch;
  std::vector<std::string> private_hits;
  std::vector<std::string> not_included;
  std::vector<std::string> other_namespace;
  std::vector<std::string> missing_includes;

  for (const GSC_EXPORT_ITEM &item : obj->exports()) {
    if (item.name == import.name && item.name_space == import.name_space &&
        !accepts_args(item, import.param_count)) {
      param_mismatch.push_back(std::format("{}{}{}({}) in this script", ns,
                                           ns.empty() ? "" : "::", name,
                                           plural(item.param_count, "param")));
    }
  }

  std::unordered_set<std::string> included_keys;
  for (const std::string &include : object_includes(obj)) {
    const known_script *script = find_script(scripts, include);
    if (script) {
      included_keys.insert(script->key);
      for (const GSC_EXPORT_ITEM &item : script->obj->exports()) {
        if (item.name == import.name && item.name_space == import.name_space) {
          if (item.flags & EXPORT_PRIVATE) {
            private_hits.push_back(describe_export(*script, item));
          } else if (!accepts_args(item, import.param_count)) {
            param_mismatch.push_back(describe_export(*script, item));
          }
        }
      }
    } else {
      missing_includes.push_back(using_path(include));
    }
  }

  for (const known_script &script : scripts) {
    const bool is_self = script.key == self_key;
    for (const GSC_EXPORT_ITEM &item : script.obj->exports()) {
      if (item.name == import.name) {
        if (item.name_space != import.name_space) {
          other_namespace.push_back(describe_export(script, item));
        } else if (!is_self && !included_keys.contains(script.key)) {
          not_included.push_back(describe_export(script, item) + " -> #using " +
                                 using_path(script.name));
        }
      }
    }
  }

  const bool consults_builtins =
      (type == IMPORT_FUNC_GETFUNCTION || type == IMPORT_FUNC_CALL ||
       type == IMPORT_FUNC_METHOD) &&
      (local || import.name_space == EMPTY_NAMESPACE ||
       import.name_space == SYS_NAMESPACE);
  const builtin_info same_kind = find_builtin(inst, import.name, method);
  const builtin_info other_kind = find_builtin(inst, import.name, !method);

  for (const std::string &include : missing_includes) {
    failure.notes.push_back(std::format(
        "#using {} could not be loaded: no script with that path exists",
        include));
  }

  if (!param_mismatch.empty()) {
    failure.error = "Too many arguments";
    failure.cause = std::format(
        "'{}' is defined, but it does not accept {}: {}", qualified,
        plural(import.param_count, "argument"), param_mismatch.front());
    failure.fix = "pass fewer arguments or add the missing parameters to the "
                  "function definition";
  } else if (!private_hits.empty()) {
    failure.cause = std::format(
        "'{}' is private and can only be called from its own script: {}",
        qualified, private_hits.front());
    failure.fix = "remove the 'private' keyword from the definition or call "
                  "a public function instead";
  } else if (consults_builtins && same_kind.exists &&
             import.param_count < same_kind.min_args) {
    failure.error = "Too few arguments";
    failure.cause =
        std::format("builtin {} '{}' requires at least {}, this call passes {}",
                    method ? "method" : "function", name,
                    plural(same_kind.min_args, "argument"), import.param_count);
    failure.fix = "pass the required arguments";
  } else if (consults_builtins && same_kind.exists &&
             same_kind.type == BUILTIN_DEV_ONLY &&
             !(import.flags & IMPORT_CALL_EXTERNAL_DEV)) {
    failure.error = "Developer-only builtin";
    failure.cause = std::format(
        "builtin '{}' only exists in developer builds of the game", name);
    failure.fix = "wrap the call in a devblock /# ... #/ or remove it";
  } else if (same_kind.exists && !consults_builtins) {
    failure.cause =
        type == IMPORT_FUNC_THREAD || type == IMPORT_FUNC_METHOD_THREAD
            ? std::format("builtin '{}' cannot be started with 'thread'", name)
            : std::format("builtin '{}' cannot be namespace qualified ('{}')",
                          name, qualified);
    failure.fix = std::format("call it directly as {}{}(...)",
                              method ? "<entity> " : "", name);
  } else if (other_kind.exists && (local || consults_builtins)) {
    failure.cause =
        method ? std::format("'{}' is a builtin function, not a method", name)
               : std::format("'{}' is a builtin method, it must be called on "
                             "an entity",
                             name);
    failure.fix = method
                      ? std::format("remove the entity before the call: "
                                    "{}(...)",
                                    name)
                      : std::format("call it on an entity: self {}(...)", name);
  } else if (!not_included.empty()) {
    failure.cause =
        std::format("'{}' is defined in a script this file does not #using: {}",
                    qualified, not_included.front());
    failure.fix =
        std::format("add the #using line shown above to {}", obj->get_name());
    for (size_t i = 1; i < not_included.size(); ++i) {
      failure.notes.push_back("also defined: " + not_included[i]);
    }
  } else if (!other_namespace.empty()) {
    failure.cause = std::format(
        "no function '{}' exists in namespace '{}', but it exists in another "
        "namespace: {}",
        name, ns.empty() ? std::string("<none>") : ns, other_namespace.front());
    failure.fix = "call it with the namespace it is declared in "
                  "(namespace::" +
                  name + ") and #using that script";
    for (size_t i = 1; i < other_namespace.size(); ++i) {
      failure.notes.push_back("also defined: " + other_namespace[i]);
    }
  } else if (!missing_includes.empty()) {
    failure.cause = std::format(
        "'{}' was expected in a #using script that could not be loaded",
        qualified);
    failure.fix = "fix the #using path or add the missing script";
  } else {
    failure.cause =
        local ? std::format(
                    "'{}' is not a builtin {} in this build and no "
                    "loaded script defines it (searched {})",
                    name, method ? "method" : "function",
                    plural(static_cast<int32_t>(scripts.size()), "script"))
              : std::format(
                    "no loaded script defines '{}' (searched {})", qualified,
                    plural(static_cast<int32_t>(scripts.size()), "script"));
    failure.fix = local ? "remove the call, or define the function and "
                          "qualify it with its namespace if it lives in "
                          "another script"
                        : "check the spelling, or define the function in a "
                          "script this file #using's";
  }

  const bool involves_scripts =
      !local || !param_mismatch.empty() || !private_hits.empty() ||
      !not_included.empty() || !other_namespace.empty();
  if (involves_scripts && import.name_space != EMPTY_NAMESPACE &&
      import.name_space != SYS_NAMESPACE) {
    const std::vector<std::string> declaring =
        scripts_declaring_namespace(scripts, import.name_space);
    if (declaring.size() > 1) {
      failure.notes.push_back(
          std::format("namespace '{}' is declared by {}: {}", ns,
                      plural(static_cast<int32_t>(declaring.size()), "script"),
                      utils::string::join(declaring, ", ")));
    }
  }
}

std::string join_lines(const std::vector<int32_t> &lines) {
  std::string result = std::to_string(lines[0]);
  for (size_t i = 1; i < lines.size(); ++i) {
    result += std::format(", {}", lines[i]);
  }
  return result;
}

void append_field(std::string &out, const std::string_view label,
                  const std::string_view color, const std::string_view value) {
  if (!value.empty()) {
    out +=
        std::format("^1  {:<8} {}{}\n", std::string(label) + ":", color, value);
  }
}

void append_failure(std::string &out, const link_failure &failure) {
  append_field(out, "File", "^5", failure.script);
  if (!failure.lines.empty()) {
    append_field(out, failure.lines.size() > 1 ? "Lines" : "Line", "^2",
                 join_lines(failure.lines));
    append_field(
        out, "Source", "^7",
        script::get_source_line(failure.script, failure.lines.front()));
  }
  append_field(out, "Call", "^5", failure.call);
  append_field(out, "Error", "^1", failure.error);
  append_field(out, "Cause", "^7", failure.cause);
  append_field(out, "Fix", "^3", failure.fix);
  for (const std::string &note : failure.notes) {
    append_field(out, "Note", "^7", note);
  }
}

std::string resolve_hex_tokens(const std::string &input) {
  std::string result;
  size_t pos = 0;
  while (pos < input.size()) {
    size_t end = pos;
    while (end < input.size() &&
           std::isxdigit(static_cast<uint8_t>(input[end]))) {
      ++end;
    }
    const bool bounded =
        (pos == 0 || !std::isalnum(static_cast<uint8_t>(input[pos - 1]))) &&
        (end == input.size() ||
         !std::isalnum(static_cast<uint8_t>(input[end])));
    if (end - pos >= 6 && end - pos <= 8 && bounded) {
      const std::string name =
          script::resolve_hash(static_cast<ScrVarCanonicalName_t>(
              std::stoul(input.substr(pos, end - pos), nullptr, 16)));
      result += name.empty() ? input.substr(pos, end - pos) : name;
      pos = end;
    } else {
      result += input[pos++];
    }
  }
  return result;
}

std::vector<std::string>
message_lines(const std::initializer_list<const char *> parts) {
  std::vector<std::string> lines;
  for (const char *part : parts) {
    std::istringstream stream(resolve_hex_tokens(part ? part : ""));
    std::string line;
    while (std::getline(stream, line)) {
      const size_t start = line.find_first_not_of(" \t\r");
      const size_t end = line.find_last_not_of(" \t\r");
      if (start != std::string::npos) {
        lines.push_back(line.substr(start, end - start + 1));
      }
    }
  }
  return lines;
}

std::string header(const std::string_view title) {
  return std::format("^1*********************{}*********************\n", title);
}

constexpr std::string_view SEPARATOR =
    "^1------------------------------------------------------------\n";
constexpr std::string_view FOOTER =
    "^1************************************************************\n";

report build_link_report(const scriptInstance_t inst, const char *failures) {
  report result{};
  result.text = header(inst == SCRIPTINSTANCE_CLIENT ? "CSC LINK ERROR"
                                                     : "GSC LINK ERROR") +
                failures;
  return result;
}

report build_missing_script_report(const scriptInstance_t inst,
                                   const std::string &missing) {
  report result{};
  result.text = header(inst == SCRIPTINSTANCE_CLIENT ? "CSC LOAD ERROR"
                                                     : "GSC LOAD ERROR");
  append_field(result.text, "Script", "^5", missing);
  append_field(result.text, "Error", "^1", "Script file not found");

  const std::vector<known_script> scripts = collect_scripts(inst);
  const std::string key = script_key(missing);
  const std::string file_name = key.substr(key.find_last_of('/') + 1);
  std::vector<std::string> referenced_by;
  std::vector<std::string> similar;
  for (const known_script &script : scripts) {
    for (const std::string &include : object_includes(script.obj)) {
      if (script_key(include) == key) {
        referenced_by.push_back(script.name);
        break;
      }
    }
    if (script.key.substr(script.key.find_last_of('/') + 1) == file_name) {
      similar.push_back(using_path(script.name));
    }
  }

  append_field(result.text, "Cause", "^7",
               "no script with this path exists in the loaded zones or in "
               "the scripts/custom_scripts folders");
  if (!referenced_by.empty()) {
    append_field(result.text, "Used by", "^5",
                 utils::string::join(referenced_by, ", "));
  }
  if (!similar.empty()) {
    append_field(result.text, "Fix", "^3",
                 "a script with the same name exists at: " +
                     utils::string::join(similar, ", "));
  } else {
    append_field(result.text, "Fix", "^3",
                 "correct the #using path or add the missing script");
  }

  return result;
}

std::string_view runtime_cause(const char *source_file) {
  static const std::unordered_map<std::string, std::string_view> causes{
      {"scr_variable",
       "the script VM ran out of variable slots: a script keeps creating "
       "variables, arrays, structs or threads faster than they are freed "
       "(usually an endless loop adding to an array or starting threads)"},
      {"scr_stringlist", "the script VM ran out of string storage or a string "
                         "exceeded the maximum length"},
      {"scr_memorytree", "the script VM ran out of memory for script data"},
      {"scr_animtree",
       "a script uses an animtree or animation that is not loaded"},
  };

  const std::string stem = utils::string::to_lower(
      std::filesystem::path(source_file ? source_file : "").stem().string());
  const auto cause = causes.find(stem);
  return cause == causes.end() ? std::string_view{} : cause->second;
}

void append_callstack(std::string &out, const scriptInstance_t inst,
                      const uint8_t *pos) {
  bool first = true;
  for (const script::script_frame &frame :
       script::get_script_frames(inst, pos)) {
    if (frame.file.empty()) {
      continue;
    }
    if (std::exchange(first, false)) {
      append_field(out, "File", "^5", frame.file);
      append_field(out, "Line", "^2",
                   frame.line < 0 ? "" : std::to_string(frame.line));
      append_field(out, "Source", "^7", frame.source);
    } else {
      append_field(out, "Called", "^5",
                   frame.line < 0
                       ? frame.file
                       : std::format("{}:{}", frame.file, frame.line));
    }
  }
}

report build_runtime_report(const scriptInstance_t inst, const char *message,
                            const char *detail, const char *extra,
                            const char *source_file) {
  report result{};
  result.text = header(inst == SCRIPTINSTANCE_CLIENT ? "CSC RUNTIME ERROR"
                                                     : "GSC RUNTIME ERROR");

  const std::vector<std::string> lines =
      message_lines({message, detail, extra});
  for (const std::string &line : lines) {
    append_field(result.text, "Error", "^1", line);
  }
  append_field(result.text, "Cause", "^7", runtime_cause(source_file));

  append_callstack(result.text, inst, nullptr);
  return result;
}

std::string report_text(const report &report, const std::string &outcome) {
  std::string text = report.text;
  if (!outcome.empty()) {
    text += SEPARATOR;
    append_field(text, "Result", "^7", outcome);
  }
  text += FOOTER;
  return text;
}
} // namespace

void print_report(const report &report, const std::string &outcome) {
  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s",
                        report_text(report, outcome).c_str());
}

void report_runaway_loop(const game::scr::scriptInstance_t inst,
                         const uint8_t *pos, const uint32_t elapsed_ms) {
  report result{};
  result.text = header(inst == SCRIPTINSTANCE_CLIENT ? "CSC INFINITE LOOP"
                                                     : "GSC INFINITE LOOP");
  append_field(result.text, "Error", "^1",
               std::format("a loop ran for {} ms without a wait", elapsed_ms));
  append_callstack(result.text, inst, pos);
  append_field(
      result.text, "Cause", "^7",
      inst == SCRIPTINSTANCE_CLIENT
          ? "the loop never yields, so the client freezes until it ends"
          : "the loop never yields, so the server frame never ends and "
            "the whole server freezes");
  append_field(result.text, "Fix", "^3",
               "add a wait (or waittill) inside the loop");
  print_report(result, {});
}

void show_error_popup(const std::string &text) {
  static std::atomic<bool> popup_pending{false};
  if (popup_pending.exchange(true)) {
    return;
  }
  scheduler::once(
      [text] {
        popup_pending = false;
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "disconnect\n");
        scheduler::once(
            [text] {
              game::ui::UI_OpenErrorPopupWithMessage(
                  game::LOCAL_CLIENT_0, game::errorCode::NONE, text.c_str());
            },
            scheduler::pipeline::main, 500ms);
      },
      scheduler::pipeline::main);
}

namespace {
game::EngineDependentDvar restart_delay;
std::atomic<int64_t> restart_not_before{0};
std::atomic<bool> restart_queued{false};
std::atomic<bool> console_muted{false};
utils::hook::detour LaunchGame_hook;
utils::hook::detour Com_Printf_hook;

void Com_Printf_Muted(const game::consoleChannel_e channel,
                      const game::consoleLabel_e label, const char *fmt, ...) {
  if (console_muted) {
    return;
  }

  va_list args;
  va_start(args, fmt);
  va_list measure;
  va_copy(measure, args);
  std::string text(std::max(vsnprintf(nullptr, 0, fmt, measure), 0), '\0');
  va_end(measure);
  vsnprintf(text.data(), text.size() + 1, fmt, args);
  va_end(args);

  Com_Printf_hook.invoke<void>(channel, label, "%s", text.c_str());
}

int64_t now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

void LaunchGame_Delayed() {
  const int64_t remaining = restart_not_before.load() - now_ms();
  if (remaining <= 0) {
    LaunchGame_hook.invoke<void>();
    return;
  }

  if (restart_queued.exchange(true)) {
    return;
  }

  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT,
                        "^3Script error: restarting the map in %.1f seconds\n",
                        static_cast<double>(remaining) / 1000.0);
  scheduler::once(
      [] {
        restart_queued = false;
        game::cbuf::Cbuf_AddText(game::LOCAL_CLIENT_0, "launchgame\n");
      },
      scheduler::pipeline::main, std::chrono::milliseconds(remaining));
}

void stop_server_map(const char *file, const int32_t line,
                     const report &report) {
  const int32_t delay = restart_delay.get_int();
  restart_not_before = now_ms() + delay * 1000ll;
  const std::string text =
      report_text(report, std::format("The map restarts in {} seconds", delay));
  console_muted = true;
  scheduler::once([] { console_muted = false; }, scheduler::pipeline::main);
  game::com::Com_Error_(file, line, game::errorParm::DROP, "\x15%s",
                        text.c_str());
}

bool show_script_error(const report &report) {
  if (game::is_server()) {
    return false;
  }
  print_report(report, {});
  if (game::com::Com_IsInGame()) {
    show_error_popup(report.text);
  }
  return true;
}

void Com_Error_ScriptNotFound(const char *file, const int32_t line,
                              const game::errorParm code, const char *fmt,
                              const char *script) {
  const report missing = build_missing_script_report(active_instance, script);
  if (!show_script_error(missing)) {
    stop_server_map(file, line, missing);
  }
}

void Com_Error_LinkErrors(const char *file, const int32_t line,
                          const game::errorParm code, const char *fmt,
                          const int32_t count, const char *details) {
  const report link = build_link_report(active_instance, details);
  if (!show_script_error(link)) {
    stop_server_map(file, line, link);
  }
}

std::optional<std::string> missing_include(const std::string &script) {
  const std::vector<known_script> scripts = collect_scripts(active_instance);
  const std::string key = script_key(script);
  for (const known_script &owner : scripts) {
    if (owner.key != key) {
      continue;
    }
    for (const std::string &include : object_includes(owner.obj)) {
      const std::string include_key = script_key(include);
      if (std::ranges::none_of(scripts, [&](const known_script &known) {
            return known.key == include_key;
          })) {
        return include;
      }
    }
  }
  return std::nullopt;
}

void Com_Error_LinkScript(const char *file, const int32_t line,
                          const game::errorParm code, const char *fmt,
                          const char *script, const char *reason,
                          const char *detail) {
  if (!game::is_server()) {
    return;
  }

  report failure{};
  if (const auto missing = missing_include(script)) {
    failure = build_missing_script_report(active_instance, *missing);
  } else {
    failure.text = header("GSC LINK ERROR");
    append_field(failure.text, "Script", "^5", script);
    append_field(failure.text, "Error", "^1", reason);
    append_field(failure.text, "Detail", "^7", detail);
  }
  stop_server_map(file, line, failure);
}

void Com_Error_ScriptRuntime(const char *file, const int32_t line,
                             const game::errorParm code, const char *fmt,
                             const char *vm, const char *error,
                             const char *detail, const char *extra,
                             const char *source, const int32_t source_line) {
  const report runtime =
      build_runtime_report(active_instance, error, detail, extra, source);
  if (!show_script_error(runtime)) {
    stop_server_map(file, line, runtime);
  }
}

utils::hook::detour ReportObjLinkError_hook;
utils::hook::detour ReportObjLinkError2_hook;
utils::hook::detour Scr_LoadScript_hook;
utils::hook::detour GscObjResolve_hook;
utils::hook::detour VM_RuntimeError_hook;

uint32_t Scr_LoadScript_Scoped(const scriptInstance_t inst,
                               const char *filename) {
  const instance_scope scope(inst);
  return Scr_LoadScript_hook.invoke<uint32_t>(inst, filename);
}

int32_t GscObjResolve_Scoped(const scriptInstance_t inst, GSC_OBJ *obj,
                             objFileInfo_t *fileInfo) {
  const instance_scope scope(inst);
  return GscObjResolve_hook.invoke<int32_t>(inst, obj, fileInfo);
}

void VM_RuntimeError_Scoped(const scriptInstance_t inst, uint8_t *pos,
                            const uint32_t errorCode, const char *message,
                            const char *detail) {
  const instance_scope scope(inst);
  VM_RuntimeError_hook.invoke<void>(inst, pos, errorCode, message, detail);
}

void ReportObjLinkError_Report(scriptInstance_t inst, GSC_OBJ *prime_obj,
                               objFileInfo_t *fileInfo, GSC_IMPORT_ITEM *import,
                               char *errorString, int errorStringLength) {
  if (!valid_object(prime_obj) || !import || !errorString ||
      errorStringLength <= 0) {
    ReportObjLinkError_Impl(inst, prime_obj, fileInfo, import, errorString,
                            errorStringLength);
    return;
  }

  static const GSC_OBJ *scripts_owner{};
  static std::vector<known_script> scripts;
  if (scripts_owner != prime_obj) {
    scripts = collect_scripts(inst);
    scripts_owner = prime_obj;
  }

  link_failure failure{};
  failure.script = prime_obj->get_name();
  failure.lines = Scr_GetImportLineNumbers(inst, prime_obj, fileInfo, import);
  failure.call = describe_call(*import);
  analyze_unresolved(inst, prime_obj, *import, scripts, failure);

  std::string text = errorString[0] ? std::string(SEPARATOR) : "";
  append_failure(text, failure);
  strncat_s(errorString, errorStringLength, text.c_str(), _TRUNCATE);
}

void ignore_linker_message() {}
} // namespace

struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "script_error"; }
#endif

  void post_unpack() override {
    if (!game::is_legacy_client()) {
      utils::hook::call(game::select(0x1412C84B2, 0x0, 0x1401566B7),
                        Com_Error_ScriptNotFound);
      utils::hook::call(game::select(0x1412CAC6D, 0x0, 0x140158EB2),
                        Com_Error_LinkErrors);
      utils::hook::call(game::select(0x1412C858C, 0x0, 0x140156796),
                        Com_Error_LinkScript);
      utils::hook::call(game::select(0x1412D5065, 0x0, 0x140161174),
                        Com_Error_ScriptRuntime);
      utils::hook::call(game::select(0x1412CAC09, 0x0, 0x140158E49),
                        ignore_linker_message);
      utils::hook::call(game::select(0x1412CAC18, 0x0, 0x140158E58),
                        ignore_linker_message);

      Scr_LoadScript_hook.create(Scr_LoadScript, Scr_LoadScript_Scoped);
      GscObjResolve_hook.create(GscObjResolve, GscObjResolve_Scoped);
      VM_RuntimeError_hook.create(VM_RuntimeError, VM_RuntimeError_Scoped);
    }

    if (game::is_server()) {
      restart_delay = game::register_dvar_int(
          "sv_errorRestartDelay", 5, 0, 300, game::DVAR_NONE,
          "Seconds to wait before launching the map again after a script "
          "error stopped it");
      Com_Printf_hook.create(game::com::Com_Printf, Com_Printf_Muted);
      LaunchGame_hook.create(game::lobby::LobbyHostLaunch_LaunchGame_f,
                             LaunchGame_Delayed);
    }

    ReportObjLinkError_hook.create(ReportObjLinkError,
                                   ReportObjLinkError_Report);
    if (game::is_client()) {
      ReportObjLinkError2_hook.create(ReportObjLinkError2,
                                      ReportObjLinkError_Report);
    }
  }
};
} // namespace script_error

REGISTER_COMPONENT(script_error::component)
