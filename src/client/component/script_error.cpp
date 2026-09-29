#include <std_include.hpp>

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

struct link_failure {
  std::string error;
  std::string script;
  std::vector<int32_t> lines;
  std::string call;
  std::string cause;
  std::string fix;
  std::vector<std::string> notes;
};

struct link_context {
  scriptInstance_t inst{};
  const GSC_OBJ *obj{};
  objFileInfo_t *info{};
  std::vector<link_failure> failures;
  std::vector<std::string> messages;
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

std::mutex link_mutex;
std::mutex reported_mutex;
std::string reported_message;
link_context current_link;
const GSC_OBJ *script_cache_owner{};
std::shared_ptr<const std::vector<known_script>> script_cache;

bool valid_object(const GSC_OBJ *obj) {
  return obj != nullptr && obj->hasMagic(GSC_OBJ::T7_MAGIC);
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
    if (!valid_object(obj) ||
        !utils::string::to_lower(std::string(name)).ends_with(extension)) {
      return;
    }
    std::string key = script_key(name);
    if (seen.insert(key).second) {
      scripts.push_back({std::string(name), std::move(key), obj});
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
  const auto it = std::ranges::find(scripts, key, &known_script::key);
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
    if (!script) {
      missing_includes.push_back(using_path(include));
      continue;
    }
    included_keys.insert(script->key);
    for (const GSC_EXPORT_ITEM &item : script->obj->exports()) {
      if (item.name != import.name || item.name_space != import.name_space) {
        continue;
      }
      if (item.flags & EXPORT_PRIVATE) {
        private_hits.push_back(describe_export(*script, item));
      } else if (!accepts_args(item, import.param_count)) {
        param_mismatch.push_back(describe_export(*script, item));
      }
    }
  }

  for (const known_script &script : scripts) {
    const bool is_self = script.key == self_key;
    for (const GSC_EXPORT_ITEM &item : script.obj->exports()) {
      if (item.name != import.name) {
        continue;
      }
      if (item.name_space != import.name_space) {
        other_namespace.push_back(describe_export(script, item));
      } else if (!is_self && !included_keys.contains(script.key)) {
        not_included.push_back(describe_export(script, item) + " -> #using " +
                               using_path(script.name));
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

void record_link_failure(const scriptInstance_t inst, const GSC_OBJ *obj,
                         objFileInfo_t *info, const GSC_IMPORT_ITEM *import) {
  if (!valid_object(obj) || import == nullptr) {
    return;
  }

  std::shared_ptr<const std::vector<known_script>> scripts;
  {
    std::scoped_lock lock(link_mutex);
    if (script_cache_owner == obj) {
      scripts = script_cache;
    }
  }
  if (!scripts) {
    scripts = std::make_shared<const std::vector<known_script>>(
        collect_scripts(inst));
    std::scoped_lock lock(link_mutex);
    script_cache_owner = obj;
    script_cache = scripts;
  }

  link_failure failure{};
  failure.script = obj->get_name();
  failure.lines = Scr_GetImportLineNumbers(inst, obj, info, import);
  failure.call = describe_call(*import);
  analyze_unresolved(inst, obj, *import, *scripts, failure);

  std::scoped_lock lock(link_mutex);
  current_link.failures.push_back(std::move(failure));
}

std::optional<link_failure> parse_too_many_parameters(const std::string &text) {
  static const std::regex pattern(
      R"re(Too many parameters: "([^"]*)" with (\d+) parameters in "([^"]*)")re");
  std::smatch match;
  if (!std::regex_search(text, match, pattern)) {
    return std::nullopt;
  }

  link_failure failure{};
  failure.error = "Too many arguments";
  failure.script = match[3].str();
  const std::string name = match[1].str();
  const int32_t args = std::stoi(match[2].str());

  if (valid_object(current_link.obj)) {
    for_each_import(current_link.obj, [&](const GSC_IMPORT_ITEM &import) {
      if (failure.call.empty() && import.param_count == args &&
          function_name(import.name) == name) {
        failure.call = describe_call(import);
        failure.lines = Scr_GetImportLineNumbers(
            current_link.inst, current_link.obj, current_link.info, &import);
        const builtin_info builtin =
            find_builtin(current_link.inst, import.name,
                         is_method_import(import.flags & IMPORT_TYPE_MASK));
        if (builtin.exists) {
          failure.cause =
              std::format("builtin '{}' accepts at most {}, this "
                          "call passes {}",
                          name, plural(builtin.max_args, "argument"), args);
        }
      }
    });
  }

  if (failure.call.empty()) {
    failure.call = std::format("{}({})", name, plural(args, "arg"));
  }
  if (failure.cause.empty()) {
    failure.cause =
        std::format("'{}' does not accept {}", name, plural(args, "argument"));
  }
  failure.fix = "remove the extra arguments";
  return failure;
}

std::string source_line(const std::string &script, const int32_t line) {
  std::string source = script::get_source_line(script, line);
  const size_t start = source.find_first_not_of(" \t");
  return start == std::string::npos ? std::string() : source.substr(start);
}

std::string join_lines(const std::vector<int32_t> &lines) {
  std::string result;
  for (const int32_t line : lines) {
    if (!result.empty()) {
      result += ", ";
    }
    result += std::to_string(line);
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
    append_field(out, "Source", "^7",
                 source_line(failure.script, failure.lines.front()));
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
  static const std::regex hex_token(R"re(\b[0-9A-Fa-f]{6,8}\b)re");
  std::string result;
  std::sregex_iterator it(input.begin(), input.end(), hex_token);
  size_t last = 0;
  for (; it != std::sregex_iterator(); ++it) {
    const std::string token = it->str();
    const std::string name = script::resolve_hash(
        static_cast<ScrVarCanonicalName_t>(std::stoul(token, nullptr, 16)));
    result.append(input, last, it->position() - last);
    result += name.empty() ? token : name;
    last = it->position() + token.size();
  }
  result.append(input, last);
  return result;
}

std::vector<std::string> message_lines(const char *message) {
  std::vector<std::string> lines;
  std::istringstream stream(resolve_hex_tokens(message ? message : ""));
  std::string line;
  while (std::getline(stream, line)) {
    while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) {
      line.pop_back();
    }
    const size_t start = line.find_first_not_of(" \t\x15");
    if (start != std::string::npos) {
      lines.push_back(line.substr(start));
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

report build_link_report(const scriptInstance_t inst, const char *message) {
  std::vector<link_failure> failures;
  std::vector<std::string> messages;
  {
    std::scoped_lock lock(link_mutex);
    failures = std::move(current_link.failures);
    messages = std::move(current_link.messages);
    current_link.failures.clear();
    current_link.messages.clear();
  }

  std::vector<std::string> notes;
  for (const std::string &text : messages) {
    if (text.find("Too many parameters") != std::string::npos) {
      if (std::optional<link_failure> failure =
              parse_too_many_parameters(text)) {
        failures.push_back(std::move(*failure));
      }
    } else if (text.starts_with("Could not load")) {
      notes.push_back(text);
    }
  }

  report result{};
  result.text = header(inst == SCRIPTINSTANCE_CLIENT ? "CSC LINK ERROR"
                                                     : "GSC LINK ERROR");
  if (failures.empty()) {
    for (const std::string &line : message_lines(message)) {
      result.text += std::format("^1  {}\n", line);
    }
  }
  for (size_t i = 0; i < failures.size(); ++i) {
    if (i > 0) {
      result.text += SEPARATOR;
    }
    append_failure(result.text, failures[i]);
  }
  for (const std::string &note : notes) {
    append_field(result.text, "Warning", "^3", note);
  }

  if (failures.size() == 1) {
    const link_failure &failure = failures.front();
    result.summary = std::format(
        "Script link error in {}{}: {} {}", failure.script,
        failure.lines.empty() ? "" : ":" + std::to_string(failure.lines[0]),
        utils::string::to_lower(failure.error), failure.call);
  } else if (!failures.empty()) {
    result.summary = std::format(
        "{} in {}",
        plural(static_cast<int32_t>(failures.size()), "script link error"),
        failures.front().script);
  } else {
    result.summary = "Script link error";
  }
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

  result.summary = "Script file not found: " + missing;
  return result;
}

std::string_view runtime_cause(const std::string_view source_file) {
  if (source_file.find("scr_variable") != std::string_view::npos) {
    return "the script VM ran out of variable slots: a script keeps creating "
           "variables, arrays, structs or threads faster than they are freed "
           "(usually an endless loop adding to an array or starting threads)";
  }
  if (source_file.find("scr_stringlist") != std::string_view::npos) {
    return "the script VM ran out of string storage or a string exceeded the "
           "maximum length";
  }
  if (source_file.find("scr_memorytree") != std::string_view::npos) {
    return "the script VM ran out of memory for script data";
  }
  if (source_file.find("scr_animtree") != std::string_view::npos) {
    return "a script uses an animtree or animation that is not loaded";
  }
  return {};
}

std::string append_callstack(std::string &out, const scriptInstance_t inst,
                             const uint8_t *pos) {
  static const std::regex frame_pattern(
      R"re(file '([^']*)'(?:, line (\d+) :: ?(.*))?)re");

  std::string location;
  for (const std::string &frame : script::get_script_callstack(inst, pos)) {
    std::smatch match;
    if (!std::regex_search(frame, match, frame_pattern)) {
      continue;
    }
    const std::string where =
        match[2].matched ? std::format("{}:{}", match[1].str(), match[2].str())
                         : match[1].str();
    if (location.empty()) {
      location = where;
      append_field(out, "File", "^5", match[1].str());
      append_field(out, "Line", "^2", match[2].str());
      append_field(out, "Source", "^7", match[3].str());
    } else {
      append_field(out, "Called", "^5", where);
    }
  }
  return location;
}

report build_runtime_report(const scriptInstance_t inst, const char *message,
                            const char *source_file) {
  report result{};
  result.text = header(inst == SCRIPTINSTANCE_CLIENT ? "CSC RUNTIME ERROR"
                                                     : "GSC RUNTIME ERROR");

  const std::vector<std::string> lines = message_lines(message);
  for (const std::string &line : lines) {
    append_field(result.text, "Error", "^1", line);
  }
  append_field(result.text, "Cause", "^7",
               runtime_cause(source_file ? source_file : ""));

  const std::string location = append_callstack(result.text, inst, nullptr);

  const std::string error = lines.empty() ? "script error" : lines.front();
  result.summary =
      location.empty()
          ? std::format("Script runtime error: {}", error)
          : std::format("Script runtime error in {}: {}", location, error);
  return result;
}
} // namespace

bool is_script_vm_failure(const char *source_file, const game::errorParm code) {
  return source_file != nullptr &&
         std::string_view(source_file).find("clientscript") !=
             std::string_view::npos &&
         (code == game::errorParm::FATAL || code == game::errorParm::DROP ||
          code == game::errorParm::SCRIPT_DROP);
}

report build_report(const game::errorParm code, const char *message,
                    const char *source_file) {
  const std::string_view text = message ? message : "";
  const scriptInstance_t inst = text.find(".csc") != std::string_view::npos
                                    ? SCRIPTINSTANCE_CLIENT
                                    : SCRIPTINSTANCE_SERVER;

  if (code == game::errorParm::FATAL &&
      text.find("script error(s)") != std::string_view::npos) {
    return build_link_report(inst, message);
  }

  static const std::regex missing_pattern(
      R"re(Script file not found: '([^']*)')re");
  std::cmatch match;
  if (message && std::regex_search(message, match, missing_pattern)) {
    return build_missing_script_report(inst, match[1].str());
  }

  return build_runtime_report(inst, message, source_file);
}

void print_report(const report &report, const std::string &outcome) {
  std::string text = report.text;
  if (!outcome.empty()) {
    text += SEPARATOR;
    append_field(text, "Result", "^7", outcome);
  }
  text += FOOTER;

  game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s", text.c_str());
  mark_reported(report.summary);
}

void report_runaway_loop(const game::scr::scriptInstance_t inst,
                         const uint8_t *pos, const uint32_t elapsed_ms) {
  report result{};
  result.text = header(inst == SCRIPTINSTANCE_CLIENT ? "CSC INFINITE LOOP"
                                                     : "GSC INFINITE LOOP");
  append_field(result.text, "Error", "^1",
               std::format("a loop ran for {} ms without a wait", elapsed_ms));
  append_callstack(result.text, inst, pos);
  append_field(result.text, "Cause", "^7",
               inst == SCRIPTINSTANCE_CLIENT
                   ? "the loop never yields, so the client freezes until it ends"
                   : "the loop never yields, so the server frame never ends and "
                     "the whole server freezes");
  append_field(result.text, "Fix", "^3",
               "add a wait (or waittill) inside the loop");
  print_report(result, "a 0.05s wait is now inserted each time this loop "
                       "repeats, so it runs once per frame instead of freezing");
}

void mark_reported(const std::string &message) {
  std::scoped_lock lock(reported_mutex);
  reported_message = message;
}

bool is_reported(const char *message) {
  std::scoped_lock lock(reported_mutex);
  return message != nullptr && !reported_message.empty() &&
         reported_message == message;
}

namespace {
utils::hook::detour report_obj_link_error_hook;
utils::hook::detour report_obj_link_error2_hook;
utils::hook::detour link_obj_hook;

void report_obj_link_error_stub(scriptInstance_t inst, GSC_OBJ *prime_obj,
                                objFileInfo_t *fileInfo,
                                GSC_IMPORT_ITEM *import, char *errorString,
                                int errorStringLength) {
  record_link_failure(inst, prime_obj, fileInfo, import);
  ReportObjLinkError_Impl(inst, prime_obj, fileInfo, import, errorString,
                          errorStringLength);
}

void flush_link_messages() {
  std::vector<std::string> messages;
  {
    std::scoped_lock lock(link_mutex);
    messages = std::move(current_link.messages);
    current_link.messages.clear();
    current_link.failures.clear();
    current_link.obj = nullptr;
    current_link.info = nullptr;
    script_cache_owner = nullptr;
    script_cache.reset();
  }
  for (const std::string &text : messages) {
    game::com::Com_Printf(game::consoleChannel_e::CHANNEL_DONT_FILTER,
                          game::consoleLabel_e::DEFAULT, "^3%s\n",
                          text.c_str());
  }
}

int32_t link_obj_stub(scriptInstance_t inst, GSC_OBJ *obj,
                      objFileInfo_t *info) {
  {
    std::scoped_lock lock(link_mutex);
    current_link.inst = inst;
    current_link.obj = obj;
    current_link.info = info;
    current_link.failures.clear();
    current_link.messages.clear();
  }
  const int32_t result = link_obj_hook.invoke<int32_t>(inst, obj, info);
  flush_link_messages();
  return result;
}

bool skip_error_newline = false;

void error_message_print_stub(const char *text) {
  if (is_reported(text)) {
    skip_error_newline = true;
    return;
  }
  game::sys::Sys_Print(text);
}

void error_newline_print_stub(const char *text) {
  if (std::exchange(skip_error_newline, false)) {
    return;
  }
  game::sys::Sys_Print(text);
}

void error_banner_print_stub(const int32_t channel, const int32_t label,
                             const char *fmt, const char *message,
                             const char *detail) {
  if (is_reported(message)) {
    return;
  }
  game::com::Com_Printf(static_cast<game::consoleChannel_e>(channel),
                        static_cast<game::consoleLabel_e>(label), fmt, message,
                        detail);
}

void linker_print_stub(const char *fmt, ...) {
  char buffer[0x1000]{};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf_s(buffer, _TRUNCATE, fmt, ap);
  va_end(ap);

  std::string text = buffer;
  while (!text.empty() && (text.back() == '\n' || text.back() == ' ')) {
    text.pop_back();
  }
  const size_t start = text.find_first_not_of("* ");
  text = start == std::string::npos ? std::string() : text.substr(start);

  std::scoped_lock lock(link_mutex);
  current_link.messages.push_back(std::move(text));
}
} // namespace

struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "script_error"; }
#endif

  void post_unpack() override {
    report_obj_link_error_hook.create(ReportObjLinkError,
                                      report_obj_link_error_stub);
    if (game::is_client()) {
      report_obj_link_error2_hook.create(ReportObjLinkError2,
                                         report_obj_link_error_stub);
    }

    if (game::is_server()) {
      link_obj_hook.create(game::scr::GscObjResolve.get(), link_obj_stub);
      for (const uintptr_t offset :
           {0x1F3, 0x269, 0x3FA, 0x474, 0x5B1, 0x7B4, 0x948}) {
        utils::hook::call(game::scr::GscObjResolve.offset(offset),
                          linker_print_stub);
      }
      utils::hook::call(game::scr::Scr_ResolveScriptFunction.offset(0x141),
                        linker_print_stub);

      utils::hook::call(game::com::Com_Error_.offset(0x265),
                        error_message_print_stub);
      utils::hook::call(game::com::Com_Error_.offset(0x271),
                        error_newline_print_stub);
      utils::hook::call(game::com::Com_Error_.offset(0x43E),
                        error_banner_print_stub);
      utils::hook::call(game::com::Com_ErrorCleanup.offset(0x1AB),
                        error_banner_print_stub);
    }
  }
};
} // namespace script_error

REGISTER_COMPONENT(script_error::component)
