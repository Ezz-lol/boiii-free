#include <std_include.hpp>

#include <component/gsc/func/func.hpp>

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
namespace io {
namespace stream {
void gscr_println(scriptInstance_t inst) {
  uint32_t argc = Scr_GetNumParam(inst);
  std::string out = "";
  for (uint32_t idx = 0; idx < argc; ++idx) {
    const char *msg = Scr_GetString(inst, idx);
    if (msg && msg[0]) {
      out += msg;
    }
  }
  fprintf(stdout, "[Scr] %s\n", out.c_str());
  fflush(stdout);
  game::com::Com_Printf(consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s\n", out.c_str());

#ifndef NDEBUG
  trace("[Scr] {}", out);
#endif
}

#ifdef NDEBUG
void gscr_trace(scriptInstance_t inst) {}
#else
void gscr_trace(scriptInstance_t inst) {
  uint32_t argc = Scr_GetNumParam(inst);
  std::string out = "";
  for (uint32_t idx = 0; idx < argc; ++idx) {
    const char *msg = Scr_GetString(inst, idx);
    if (msg && msg[0]) {
      out += msg;
    }
  }

  trace("[Scr] {}", out);
}
#endif

void gscr_print(scriptInstance_t inst) {
  uint32_t argc = Scr_GetNumParam(inst);
  std::string out = "";
  for (uint32_t idx = 0; idx < argc; ++idx) {
    const char *msg = Scr_GetString(inst, idx);
    if (msg && msg[0]) {
      out += msg;
    }
  }
  fprintf(stdout, "[Scr] %s", out.c_str());
  fflush(stdout);
  game::com::Com_Printf(consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s", out.c_str());

#ifndef NDEBUG
  trace("[Scr] {}", out);
#endif
}

void gscr_printf(scriptInstance_t inst) {
  const char *format = Scr_GetString(inst, 0);
  if (!format)
    return;
  std::string buffer;

  int32_t arg_index = 1;
  for (size_t i = 0; format[i] != '\0'; i++) {
    if (format[i] == '%' &&
        arg_index < static_cast<int32_t>(Scr_GetNumParam(inst))) {
      char specifier = format[++i];
      switch (specifier) {
      case 's': {
        const char *arg = Scr_GetString(inst, arg_index);
        if (!arg) {
          Scr_ParamError(inst, arg_index,
                         "Argument to printf is not a string; string expected "
                         "for %s specifier");
          return;
        }
        buffer.insert(buffer.end(), arg, arg + std::strlen(arg));
        break;
      }
      case 'd':
      case 'i': {
        int arg = Scr_GetInt(inst, arg_index);
        buffer += std::to_string(arg);
        break;
      }
      case 'o': {
        int arg = Scr_GetInt(inst, arg_index);
        char conv_buffer[33];
        auto [ptr, ec] = std::to_chars(
            conv_buffer, conv_buffer + sizeof(conv_buffer), arg, 8);
        if (ec == std::errc()) {
          *ptr = '\0';
          buffer += conv_buffer;
        } else {
          Scr_ParamError(inst, arg_index,
                         "Failed to format integer argument for %o specifier");
          return;
        }
        break;
      }
      case 'u': {
        int arg = Scr_GetInt(inst, arg_index);
        buffer += std::to_string(static_cast<uint32_t>(arg));
        break;
      }
      case 'x':
      case 'X': {
        int arg = Scr_GetInt(inst, arg_index);
        char conv_buffer[9];
        auto [ptr, ec] = std::to_chars(
            conv_buffer, conv_buffer + sizeof(conv_buffer), arg, 16);
        if (ec == std::errc()) {
          *ptr = '\0';
          if (specifier == 'X') {
            for (char *p = conv_buffer; *p; p++) {
              *p = static_cast<char>(
                  std::toupper(static_cast<unsigned char>(*p)));
            }
          }
          buffer += conv_buffer;
        } else {
          Scr_ParamError(inst, arg_index,
                         "Failed to format integer argument for %x specifier");
          return;
        }
        break;
      }

      case 'c': {
        int arg = Scr_GetInt(inst, arg_index);
        buffer.push_back(static_cast<char>(arg));
        break;
      }
      case 'f': {
        float arg = Scr_GetFloat(inst, arg_index);
        buffer += std::to_string(arg);
        break;
      }
        /*
         TODO:
         - width and precision specifiers (e.g. %.2f, %5d)
         - %p - is this even possible to support?
         - %n - is this possible?
         - length modifiers for floats (e.g. %Lf)
            - Scr_GetFloat only returns 32-bit float, so we would need to
         add a new function Scr_GetDouble to retrieve 64-bit double
         arguments
         - length modifiers for integers (e.g. %lld, %hhd)
             1. Scr_GetInt only returns 32-bit int, so we would need to add
         new functions Scr_GetInt64 and Scr_GetInt8 to retrieve 64-bit and
                8-bit integer arguments, respectively
              2. We would also need to modify the argument parsing logic to
                 determine which Scr_Get function to call based on the
         length modifier in the format string
          - handle %% for literal % character
        */

        // Either not a specifier or unsupported. Just treat it as a normal
        // % character and continue.
      default:
        buffer.push_back('%');
        buffer.push_back(specifier);
      }
      arg_index++;
    } else {
      buffer.push_back(format[i]);
    }
  }

  game::com::Com_Printf(consoleChannel_e::CHANNEL_DONT_FILTER,
                        game::consoleLabel_e::DEFAULT, "%s", buffer.data());
  fprintf(stdout, "%s", buffer.data());
  fflush(stdout);

#ifndef NDEBUG
  trace("[Scr] {}", buffer);
#endif
}
struct component final : generic_component {
  DEFINE_COMPONENT_NAME("gsc/func/io/stream");

  void post_unpack() override {
    // Log
    register_variadic_builtin("println", gscr_println, 0);
    register_variadic_builtin("trace", gscr_trace, 0);
    register_variadic_builtin("print", gscr_print, 0);
    register_variadic_builtin("printf", gscr_printf, 1);
  }
};
} // namespace stream
} // namespace io
} // namespace func
} // namespace gsc

REGISTER_COMPONENT(gsc::func::io::stream::component)
