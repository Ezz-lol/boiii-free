#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <utils/hook.hpp>

#include <mmeapi.h>
#include <sstream>

namespace game {

namespace scr {
namespace gscr {
void GScr_BBPrint_StdoutRedirect(scriptInstance_t inst) {
#ifndef NDEBUG
  int32_t numParam = Scr_GetNumParam(inst);

  // BBPrint requires at least an event name and a format string
  if (numParam < 2) {
    return;
  }

  const char *event_name = Scr_GetString(inst, 0);
  const char *format_string = Scr_GetString(inst, 1);

  std::string format_str = format_string;
  std::ostringstream message_stream;
  size_t format_len = format_str.length();

  int32_t param_index = 2;
  int32_t vector_component = 0;

  for (size_t i = 0; i < format_len; ++i) {
    // Check for the start of a format specifier
    if (format_str[i] == '%') {
      // Handle escaped percent signs "%%"
      if (i + 1 < format_len && format_str[i + 1] == '%') {
        message_stream << '%';
        ++i;
        continue;
      }

      // Extract the full format specifier token
      std::string specifier = "%";
      size_t j = i + 1;
      while (j < format_len && format_str[j] != 'd' && format_str[j] != 'i' &&
             format_str[j] != 'o' && format_str[j] != 'u' &&
             format_str[j] != 'x' && format_str[j] != 'X' &&
             format_str[j] != 'f' && format_str[j] != 'F' &&
             format_str[j] != 'e' && format_str[j] != 'E' &&
             format_str[j] != 'g' && format_str[j] != 'G' &&
             format_str[j] != 'c' && format_str[j] != 's' &&
             format_str[j] != 'p') {
        specifier += format_str[j];
        j++;
      }

      // If a valid specifier character was found, process the argument
      if (j < format_len) {
        char spec_char = format_str[j];
        specifier += spec_char;
        i = j; // Advance main loop index past the specifier

        if (param_index < numParam) {
          var::ScrVarType type = Scr_GetType(inst, param_index);
          str512_t temp_buffer;

          // 1. Handle String Formats
          if (spec_char == 's') {
            const char *str_val = "";
            if (type == var::ScrVarType::STRING ||
                type == var::ScrVarType::LOCALIZED_STRING) {
              str_val = Scr_GetString(inst, param_index);
            }
            snprintf(temp_buffer, sizeof(temp_buffer), specifier.c_str(),
                     str_val);
            message_stream << temp_buffer;
            param_index++;
          }
          // 2. Handle Floating-Point Formats
          else if (spec_char == 'f' || spec_char == 'F' || spec_char == 'e' ||
                   spec_char == 'E' || spec_char == 'g' || spec_char == 'G') {
            double float_val = 0.0;
            if (type == var::ScrVarType::FLOAT) {
              float_val = static_cast<double>(Scr_GetFloat(inst, param_index));
              param_index++;
            } else if (type == var::ScrVarType::INT) {
              float_val = static_cast<double>(Scr_GetInt(inst, param_index));
              param_index++;
            } else if (type == var::ScrVarType::VECTOR) {
              vec3_t vector_value;
              Scr_GetVector(inst, param_index, &vector_value);

              if (vector_component == 0) {
                float_val = static_cast<double>(vector_value.x);
              } else if (vector_component == 1) {
                float_val = static_cast<double>(vector_value.y);
              } else if (vector_component == 2) {
                float_val = static_cast<double>(vector_value.z);
              }

              vector_component++;
              if (vector_component == 3) {
                vector_component = 0;
                param_index++;
              }
            } else {
              param_index++;
            }
            snprintf(temp_buffer, sizeof(temp_buffer), specifier.c_str(),
                     float_val);
            message_stream << temp_buffer;
          }
          // 3. Handle Integer Formats
          else {
            int32_t int_val = 0;
            if (type == var::ScrVarType::INT) {
              int_val = Scr_GetInt(inst, param_index);
              param_index++;
            } else if (type == var::ScrVarType::FLOAT) {
              int_val = static_cast<int32_t>(Scr_GetFloat(inst, param_index));
              param_index++;
            } else if (type == var::ScrVarType::VECTOR) {
              vec3_t vector_value;
              Scr_GetVector(inst, param_index, &vector_value);

              if (vector_component == 0) {
                int_val = static_cast<int32_t>(vector_value.x);
              } else if (vector_component == 1) {
                int_val = static_cast<int32_t>(vector_value.y);
              } else if (vector_component == 2) {
                int_val = static_cast<int32_t>(vector_value.z);
              }

              vector_component++;
              if (vector_component == vec3_t::size()) {
                vector_component = 0;
                param_index++;
              }
            } else {
              param_index++;
            }
            snprintf(temp_buffer, sizeof(temp_buffer), specifier.c_str(),
                     int_val);
            message_stream << temp_buffer;
          }
        } else {
          // Fallback if the format string expects more arguments than
          // provided
          message_stream << specifier;
        }
      } else {
        // Malformed specifier near the end of the string, print verbatim
        message_stream << specifier;
        i = j;
      }
    } else {
      // Pass standard characters through verbatim
      message_stream << format_str[i];
    }
  }

  const char *out = utils::string::va("[BB][0] %s: %s", event_name,
                                      message_stream.str().c_str());
  fprintf(stdout, "%s\n", out);
  fflush(stdout);
  game::trace("{}", out);
#endif
}
} // namespace gscr
} // namespace scr

namespace bb {

void BB_Print_StdoutRedirect(game::ControllerIndex_t controllerIndex,
                             const char *name, const char *fmt, ...) {
#ifndef NDEBUG
  std::string buffer;

  va_list args;
  va_start(args, fmt);
  int32_t buf_len = vsnprintf(nullptr, 0, fmt, args) + 1;
  buffer.resize(buf_len);

  va_start(args, fmt);
  vsnprintf(buffer.data(), buf_len, fmt, args);
  va_end(args);

  if (name != nullptr && name[0] != NULL) {
    const char *out = utils::string::va("[BB][%d] %s: %s",
                                        static_cast<int32_t>(controllerIndex),
                                        name, buffer.c_str());
    fprintf(stdout, "%s\n", out);
    fflush(stdout);
    game::trace("{}", out);
  } else {
    const char *out = utils::string::va(
        "[BB][%d]: %s", static_cast<int32_t>(controllerIndex), buffer.c_str());
    fprintf(stdout, "%s\n", out);
    fflush(stdout);
    game::trace("{}", out);
  }
#endif
}
} // namespace bb
} // namespace game
namespace bb {

void stub_func() {}

utils::hook::detour BB_Print_hook;
utils::hook::detour BB_Send_hook;
utils::hook::detour BB_CheckSend_hook;

void redirect_bb_logging_to_stdout() {
  if (game::is_client()) {
    BB_Send_hook.create(
        game::bb::BB_Send.get(),
        reinterpret_cast<fastcallPtr_t<void(game::ControllerIndex_t, bool)>>(
            stub_func));
    BB_CheckSend_hook.create(
        game::bb::BB_CheckSend.get(),
        reinterpret_cast<fastcallPtr_t<void(game::ControllerIndex_t)>>(
            stub_func));
    BB_Print_hook.create(game::bb::BB_Print.get(),
                         game::bb::BB_Print_StdoutRedirect);
  }

  game::scr::builtin::table::common_functions->BBPrint.actionFunc =
      game::scr::gscr::GScr_BBPrint_StdoutRedirect;
}

class component final : public generic_component {
  DEFINE_COMPONENT_NAME("bb");

public:
  void post_unpack() override { redirect_bb_logging_to_stdout(); }
};
} // namespace bb
REGISTER_COMPONENT(bb::component);
