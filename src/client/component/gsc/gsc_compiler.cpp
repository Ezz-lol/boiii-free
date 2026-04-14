#include <std_include.hpp>
#include "gsc_compiler.hpp"
#include "gsc_lexer.hpp"
#include "gsc_parser.hpp"
#include "gsc_emitter.hpp"
#include "game/game.hpp"
#include <utils/nt.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>
#include <unordered_set>

namespace gsc_compiler {
namespace {
std::string try_read_file(const std::filesystem::path &path) {
  std::error_code ec;
  if (!std::filesystem::exists(path, ec))
    return {};
  std::ifstream f(path, std::ios::binary);
  if (!f.is_open())
    return {};
  return std::string((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());
}

std::pair<std::string, std::string>
resolve_insert_file(const std::string &insert_path,
                    const std::string &source_file) {
  std::string norm_path = insert_path;
  for (char &c : norm_path)
    if (c == '\\')
      c = '/';

  // 1) Relative to source file + walk up parents
  auto src_dir = std::filesystem::path(source_file).parent_path();
  auto candidate = src_dir / norm_path;
  auto result = try_read_file(candidate);
  if (!result.empty())
    return {result, candidate.generic_string()};

  auto parent = src_dir;
  for (int depth = 0; depth < 10 && parent.has_parent_path(); depth++) {
    parent = parent.parent_path();
    candidate = parent / norm_path;
    result = try_read_file(candidate);
    if (!result.empty())
      return {result, candidate.generic_string()};
  }

  try {
    // 2) AppData: %localappdata%/boiii/data/ and data/scripts/
    const auto appdata_data = game::get_appdata_path() / "data";
    candidate = appdata_data / norm_path;
    result = try_read_file(candidate);
    if (!result.empty())
      return {result, candidate.generic_string()};

    candidate = appdata_data / "scripts" / norm_path;
    result = try_read_file(candidate);
    if (!result.empty())
      return {result, candidate.generic_string()};

    // 3) Game folder: boiii/ and boiii/scripts/
    const auto host_boiii = utils::nt::library{}.get_folder() / "boiii";
    candidate = host_boiii / norm_path;
    result = try_read_file(candidate);
    if (!result.empty())
      return {result, candidate.generic_string()};

    candidate = host_boiii / "scripts" / norm_path;
    result = try_read_file(candidate);
    if (!result.empty())
      return {result, candidate.generic_string()};
  } catch (...) {
  }

  return {{}, {}};
}

using define_map = std::unordered_map<std::string, std::string>;
using func_define_map =
    std::unordered_map<std::string,
                       std::pair<std::vector<std::string>, std::string>>;

// Internal preprocessor - shares macro tables across recursive #insert calls
std::string preprocess_impl(const std::string &source,
                            const std::string &filename,
                            std::unordered_set<std::string> &included_files,
                            std::string &error, define_map &defines,
                            func_define_map &func_defines) {
  auto canon = std::filesystem::path(filename).generic_string();
  if (included_files.count(canon))
    return {};
  included_files.insert(canon);

  // Conditional compilation state
  struct cond_state {
    bool active;
    bool has_matched;
  };
  std::vector<cond_state> cond_stack;

  auto is_skipping = [&]() -> bool {
    for (auto &c : cond_stack)
      if (!c.active)
        return true;
    return false;
  };

  auto is_macro_defined = [&](const std::string &name) -> bool {
    return defines.count(name) > 0 || func_defines.count(name) > 0;
  };

  std::istringstream stream(source);
  std::string output;
  output.reserve(source.size());
  std::string line;
  int line_number = 0;

  while (std::getline(stream, line)) {
    line_number++;
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    // Backslash line continuation - count joined lines to preserve line numbers
    int continuation_count = 0;
    while (!line.empty() && line.back() == '\\') {
      line.pop_back();
      while (!line.empty() && (line.back() == ' ' || line.back() == '\t'))
        line.pop_back();
      line += ' ';
      std::string next_line;
      if (!std::getline(stream, next_line))
        break;
      if (!next_line.empty() && next_line.back() == '\r')
        next_line.pop_back();
      size_t nws = next_line.find_first_not_of(" \t");
      if (nws != std::string::npos)
        line += next_line.substr(nws);
      continuation_count++;
    }

    size_t first_nonws = line.find_first_not_of(" \t");
    if (first_nonws != std::string::npos && line[first_nonws] == '#') {
      std::string trimmed = line.substr(first_nonws);

      // Extract directive name for conditional handling
      auto extract_directive_arg = [](const std::string &dir,
                                      size_t prefix_len) -> std::string {
        std::string rest = dir.substr(prefix_len);
        size_t i = 0;
        while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t'))
          i++;
        size_t start = i;
        while (i < rest.size() &&
               (std::isalnum(static_cast<unsigned char>(rest[i])) ||
                rest[i] == '_'))
          i++;
        return rest.substr(start, i - start);
      };

      // Conditional compilation directives must be processed even when skipping
      if (trimmed.substr(0, 6) == "#ifdef") {
        auto macro = extract_directive_arg(trimmed, 6);
        bool defined = is_macro_defined(macro);
        bool parent_active = !is_skipping();
        cond_stack.push_back(
            {parent_active && defined, parent_active && defined});
        output += "\n";
        for (int ci = 0; ci < continuation_count; ci++)
          output += "\n";
        continue;
      }
      if (trimmed.substr(0, 7) == "#ifndef") {
        auto macro = extract_directive_arg(trimmed, 7);
        bool defined = is_macro_defined(macro);
        bool parent_active = !is_skipping();
        cond_stack.push_back(
            {parent_active && !defined, parent_active && !defined});
        output += "\n";
        for (int ci = 0; ci < continuation_count; ci++)
          output += "\n";
        continue;
      }
      if (trimmed.substr(0, 5) == "#else") {
        if (!cond_stack.empty()) {
          auto &top = cond_stack.back();
          // Check if parent scope is active
          bool parent_active = true;
          for (size_t ci = 0; ci + 1 < cond_stack.size(); ci++)
            if (!cond_stack[ci].active) {
              parent_active = false;
              break;
            }
          top.active = parent_active && !top.has_matched;
        }
        output += "\n";
        for (int ci = 0; ci < continuation_count; ci++)
          output += "\n";
        continue;
      }
      if (trimmed.substr(0, 6) == "#endif") {
        if (!cond_stack.empty())
          cond_stack.pop_back();
        output += "\n";
        for (int ci = 0; ci < continuation_count; ci++)
          output += "\n";
        continue;
      }

      // All other directives are skipped when inside a false conditional block
      if (is_skipping()) {
        output += "\n";
        for (int ci = 0; ci < continuation_count; ci++)
          output += "\n";
        continue;
      }

      if (trimmed.substr(0, 6) == "#undef") {
        auto macro = extract_directive_arg(trimmed, 6);
        defines.erase(macro);
        func_defines.erase(macro);
        output += "\n";
        for (int ci = 0; ci < continuation_count; ci++)
          output += "\n";
        continue;
      }

      if (trimmed.substr(0, 7) == "#define") {
        std::string rest = trimmed.substr(7);
        size_t i = 0;
        while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t'))
          i++;
        size_t name_start = i;
        while (i < rest.size() &&
               (std::isalnum(static_cast<unsigned char>(rest[i])) ||
                rest[i] == '_'))
          i++;
        std::string macro_name = rest.substr(name_start, i - name_start);

        if (macro_name.empty()) {
          output += "\n"; // keep line numbers
          continue;
        }

        if (i < rest.size() && rest[i] == '(') {
          i++;
          std::vector<std::string> params;
          while (i < rest.size() && rest[i] != ')') {
            while (i < rest.size() &&
                   (rest[i] == ' ' || rest[i] == '\t' || rest[i] == ','))
              i++;
            size_t ps = i;
            while (i < rest.size() && rest[i] != ',' && rest[i] != ')' &&
                   rest[i] != ' ')
              i++;
            if (i > ps)
              params.push_back(rest.substr(ps, i - ps));
          }
          if (i < rest.size())
            i++;
          while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t'))
            i++;
          std::string body = rest.substr(i);
          auto comment_pos = body.find("//");
          if (comment_pos != std::string::npos)
            body = body.substr(0, comment_pos);
          while (!body.empty() && (body.back() == ' ' || body.back() == '\t'))
            body.pop_back();
          func_defines[macro_name] = {params, body};
        } else {
          while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t'))
            i++;
          std::string value = rest.substr(i);
          auto comment_pos = value.find("//");
          if (comment_pos != std::string::npos)
            value = value.substr(0, comment_pos);
          while (!value.empty() &&
                 (value.back() == ' ' || value.back() == '\t'))
            value.pop_back();
          defines[macro_name] = value;
        }

        output += "\n";
        for (int ci = 0; ci < continuation_count; ci++)
          output += "\n";
        continue;
      }

      if (trimmed.substr(0, 7) == "#insert") {
        std::string rest = trimmed.substr(7);
        size_t i = 0;
        while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t'))
          i++;
        std::string insert_path = rest.substr(i);
        while (!insert_path.empty() &&
               (insert_path.back() == ';' || insert_path.back() == ' ' ||
                insert_path.back() == '\t'))
          insert_path.pop_back();
        for (char &c : insert_path)
          if (c == '\\')
            c = '/';

        auto [file_content, resolved_path] =
            resolve_insert_file(insert_path, filename);
        if (!file_content.empty()) {
          std::string inserted =
              preprocess_impl(file_content, resolved_path, included_files,
                              error, defines, func_defines);
          output += inserted;
          if (!inserted.empty() && inserted.back() != '\n')
            output += "\n";
        } else {
          try {
            auto appdata_scripts =
                (game::get_appdata_path() / "data" / "scripts").string();
            printf(
                "^3[GSC] Warning: #insert file not found: '%s' — "
                "download/create the .gsh headers and place them in '%s\\'\n",
                insert_path.data(), appdata_scripts.data());
          } catch (...) {
            printf("^3[GSC] Warning: #insert file not found: '%s'\n",
                   insert_path.data());
          }
          output += "\n";
        }
        continue;
      }
    }

    // Skip code lines inside false conditional blocks
    if (is_skipping()) {
      output += "\n";
      for (int ci = 0; ci < continuation_count; ci++)
        output += "\n";
      continue;
    }

    std::string processed = line;

    for (auto &[name, def] : func_defines) {
      auto &[params, body] = def;
      size_t search_pos = 0;
      while ((search_pos = processed.find(name, search_pos)) !=
             std::string::npos) {
        if (search_pos > 0 && (std::isalnum(static_cast<unsigned char>(
                                   processed[search_pos - 1])) ||
                               processed[search_pos - 1] == '_')) {
          search_pos++;
          continue;
        }
        size_t after_name = search_pos + name.size();
        if (after_name < processed.size() &&
            (std::isalnum(static_cast<unsigned char>(processed[after_name])) ||
             processed[after_name] == '_')) {
          search_pos++;
          continue;
        }

        if (after_name < processed.size() && processed[after_name] == '(') {
          size_t paren_start = after_name + 1;
          int depth = 1;
          size_t pi = paren_start;
          while (pi < processed.size() && depth > 0) {
            if (processed[pi] == '(')
              depth++;
            else if (processed[pi] == ')')
              depth--;
            if (depth > 0)
              pi++;
          }
          if (depth == 0) {
            std::string args_str =
                processed.substr(paren_start, pi - paren_start);
            size_t paren_end = pi + 1;

            std::vector<std::string> args;
            int d = 0;
            size_t arg_start = 0;
            for (size_t ai = 0; ai <= args_str.size(); ai++) {
              if (ai == args_str.size() || (args_str[ai] == ',' && d == 0)) {
                std::string arg = args_str.substr(arg_start, ai - arg_start);
                size_t fs = arg.find_first_not_of(" \t");
                size_t ls = arg.find_last_not_of(" \t");
                if (fs != std::string::npos)
                  args.push_back(arg.substr(fs, ls - fs + 1));
                else
                  args.push_back("");
                arg_start = ai + 1;
              } else {
                if (args_str[ai] == '(')
                  d++;
                else if (args_str[ai] == ')')
                  d--;
              }
            }

            std::string expanded = body;
            for (size_t pi2 = 0; pi2 < params.size() && pi2 < args.size();
                 pi2++) {
              size_t sp = 0;
              while ((sp = expanded.find(params[pi2], sp)) !=
                     std::string::npos) {
                if (sp > 0 && (std::isalnum(static_cast<unsigned char>(
                                   expanded[sp - 1])) ||
                               expanded[sp - 1] == '_')) {
                  sp++;
                  continue;
                }
                size_t ep = sp + params[pi2].size();
                if (ep < expanded.size() &&
                    (std::isalnum(static_cast<unsigned char>(expanded[ep])) ||
                     expanded[ep] == '_')) {
                  sp++;
                  continue;
                }
                expanded.replace(sp, params[pi2].size(), args[pi2]);
                sp += args[pi2].size();
              }
            }

            processed.replace(search_pos, paren_end - search_pos, expanded);
          } else {
            search_pos++;
          }
        } else {
          search_pos++;
        }
      }
    }

    for (auto &[name, value] : defines) {
      size_t search_pos = 0;
      while ((search_pos = processed.find(name, search_pos)) !=
             std::string::npos) {
        if (search_pos > 0 && (std::isalnum(static_cast<unsigned char>(
                                   processed[search_pos - 1])) ||
                               processed[search_pos - 1] == '_')) {
          search_pos++;
          continue;
        }
        size_t after = search_pos + name.size();
        if (after < processed.size() &&
            (std::isalnum(static_cast<unsigned char>(processed[after])) ||
             processed[after] == '_')) {
          search_pos++;
          continue;
        }
        processed.replace(search_pos, name.size(), value);
        search_pos += value.size();
      }
    }

    // Built-in macros: __FILE__, __LINE__, __DATE__, __TIME__
    {
      size_t pos = 0;
      std::string file_val = "\"" + filename + "\"";
      while ((pos = processed.find("__FILE__", pos)) != std::string::npos) {
        processed.replace(pos, 8, file_val);
        pos += file_val.size();
      }
      pos = 0;
      std::string line_val = std::to_string(line_number);
      while ((pos = processed.find("__LINE__", pos)) != std::string::npos) {
        processed.replace(pos, 8, line_val);
        pos += line_val.size();
      }
    }

    output += processed;
    output += "\n";
    for (int ci = 0; ci < continuation_count; ci++)
      output += "\n";
  }

  return output;
}

std::string preprocess(const std::string &source, const std::string &filename,
                       std::unordered_set<std::string> &included_files,
                       std::string &error) {
  define_map defines;
  func_define_map func_defines;

  // __DATE__ / __TIME__
  {
    auto now =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm lt{};
    localtime_s(&lt, &now);
    char date_buf[32], time_buf[32];
    std::strftime(date_buf, sizeof(date_buf), "%b %d %Y", &lt);
    std::strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &lt);
    defines["__DATE__"] = std::string("\"") + date_buf + "\"";
    defines["__TIME__"] = std::string("\"") + time_buf + "\"";
  }

  defines["SERVER_FRAME"] = ".05";
  defines["CLIENT_FRAME"] = ".016";
  defines["WAIT_SERVER_FRAME"] = "{wait(SERVER_FRAME);}";
  defines["WAIT_CLIENT_FRAME"] = "{wait(CLIENT_FRAME);}";

  defines["VERSION_SHIP"] = "0";
  defines["VERSION_DLC1"] = "1";
  defines["VERSION_DLC2"] = "2";
  defines["VERSION_DLC3"] = "3";
  defines["VERSION_DLC4"] = "4";
  defines["VERSION_TU"] = "5";

  func_defines["REGISTER_SYSTEM"] = {
      {"__sys", "__func_init_preload", "__reqs"},
      "function autoexec __init__sytem__() { "
      "system::register(__sys,__func_init_preload,undefined,__reqs); }"};
  func_defines["REGISTER_SYSTEM_EX"] = {
      {"__sys", "__func_init_preload", "__func_init_postload", "__reqs"},
      "function autoexec __init__sytem__() { "
      "system::register(__sys,__func_init_preload,__func_init_postload,__reqs);"
      " }"};

  func_defines["IS_TRUE"] = {{"__a"}, "( isdefined( __a ) && __a )"};
  func_defines["DEFAULT"] = {{"__var", "__default"},
                             "if(!isdefined(__var))__var=__default"};
  func_defines["DEFAULT2"] = {{"__var", "__default1", "__default2"},
                              "if(!isdefined(__var))__var=(isdefined(__"
                              "default1)?__default1:__default2)"};
  func_defines["VAL"] = {{"__var", "__default"},
                         "(isdefined(__var)?__var:__default)"};
  func_defines["IS_EQUAL"] = {{"__a", "__b"}, "( __a === __b )"};
  func_defines["SQR"] = {{"__var"}, "( (__var) * (__var) )"};
  func_defines["ARRAY_ADD"] = {
      {"__array", "__item"},
      "if(!isdefined(__array))__array=[]; __array[__array.size]=__item;"};
  func_defines["SPAWNFLAG"] = {
      {"__e", "__f"},
      "(isdefined(__e.spawnflags)&&((__e.spawnflags & __f) == __f))"};
  func_defines["RGB"] = {{"__r", "__g", "__b"}, "(__r/255,__g/255,__b/255)"};
  func_defines["FLAT_ORIGIN"] = {{"__origin"},
                                 "( __origin[0], __origin[1], 0 )"};
  func_defines["FLAT_ANGLES"] = {{"__angles"}, "( 0, __angles[1], 0 )"};

  defines["HORIZONTAL_ALIGN_SUBLEFT"] = "0";
  defines["HORIZONTAL_ALIGN_LEFT"] = "1";
  defines["HORIZONTAL_ALIGN_CENTER"] = "2";
  defines["HORIZONTAL_ALIGN_RIGHT"] = "3";
  defines["HORIZONTAL_ALIGN_FULLSCREEN"] = "4";
  defines["HORIZONTAL_ALIGN_NOSCALE"] = "5";
  defines["HORIZONTAL_ALIGN_TO640"] = "6";
  defines["HORIZONTAL_ALIGN_CENTER_SAFEAREA"] = "7";
  defines["VERTICAL_ALIGN_SUBTOP"] = "0";
  defines["VERTICAL_ALIGN_TOP"] = "1";
  defines["VERTICAL_ALIGN_CENTER"] = "2";
  defines["VERTICAL_ALIGN_BOTTOM"] = "3";
  defines["VERTICAL_ALIGN_FULLSCREEN"] = "4";
  defines["VERTICAL_ALIGN_NOSCALE"] = "5";
  defines["VERTICAL_ALIGN_TO480"] = "6";
  defines["VERTICAL_ALIGN_CENTER_SAFEAREA"] = "7";

  return preprocess_impl(source, filename, included_files, error, defines,
                         func_defines);
}
} // namespace

compile_result compile(const std::string &source, const std::string &filename) {
  compile_result result{};

  // Step 0: Preprocess (#define, #insert)
  std::unordered_set<std::string> included_files;
  std::string preprocess_error;
  std::string preprocessed =
      preprocess(source, filename, included_files, preprocess_error);
  if (!preprocess_error.empty()) {
    result.errors.push_back({preprocess_error, filename, 0, 0});
    return result;
  }

  // Step 1: Tokenize
  auto lex_result = tokenize(preprocessed);
  if (!lex_result.success) {
    result.errors.push_back({lex_result.error, filename, lex_result.error_line,
                             lex_result.error_column});
    return result;
  }

  // Step 2: Parse
  auto parse_res = parse(lex_result.tokens);
  if (!parse_res.success) {
    result.errors.push_back({parse_res.error, filename, parse_res.error_line,
                             parse_res.error_column});
    return result;
  }

  // Step 3: Emit bytecode
  auto emit_result = emit(parse_res.root, filename);
  if (!emit_result.success) {
    result.errors.push_back({emit_result.error, filename,
                             emit_result.error_line, emit_result.error_column});
    return result;
  }

  result.success = true;
  result.bytecode = std::move(emit_result.data);
  for (auto &hn : emit_result.hash_names)
    result.hash_names.push_back(
        {hn.hash, std::move(hn.name), hn.line, hn.params});
  result.replacefuncs = std::move(emit_result.replacefuncs);
  return result;
}
} // namespace gsc_compiler