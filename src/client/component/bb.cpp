#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>

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

  const char *eventName = Scr_GetString(inst, 0);
  const char *formatString = Scr_GetString(inst, 1);

  std::string formatStr = formatString;
  std::ostringstream messageStream;
  size_t formatLen = formatStr.length();

  int32_t paramIndex = 2;
  int32_t vectorComponent = 0;

  for (size_t i = 0; i < formatLen; ++i) {
    // Check for the start of a format specifier
    if (formatStr[i] == '%') {
      // Handle escaped percent signs "%%"
      if (i + 1 < formatLen && formatStr[i + 1] == '%') {
        messageStream << '%';
        ++i;
        continue;
      }

      // Extract the full format specifier token
      std::string specifier = "%";
      size_t j = i + 1;
      while (
          j < formatLen && formatStr[j] != 'd' && formatStr[j] != 'i' &&
          formatStr[j] != 'o' && formatStr[j] != 'u' && formatStr[j] != 'x' &&
          formatStr[j] != 'X' && formatStr[j] != 'f' && formatStr[j] != 'F' &&
          formatStr[j] != 'e' && formatStr[j] != 'E' && formatStr[j] != 'g' &&
          formatStr[j] != 'G' && formatStr[j] != 'c' && formatStr[j] != 's' &&
          formatStr[j] != 'p') {
        specifier += formatStr[j];
        j++;
      }

      // If a valid specifier character was found, process the argument
      if (j < formatLen) {
        char specChar = formatStr[j];
        specifier += specChar;
        i = j; // Advance main loop index past the specifier

        if (paramIndex < numParam) {
          var::ScrVarType type = Scr_GetType(inst, paramIndex);
          str512_t tempBuffer;

          // 1. Handle String Formats
          if (specChar == 's') {
            const char *strVal = "";
            if (type == var::ScrVarType::STRING ||
                type == var::ScrVarType::LOCALIZED_STRING) {
              strVal = Scr_GetString(inst, paramIndex);
            }
            snprintf(tempBuffer, sizeof(tempBuffer), specifier.c_str(), strVal);
            messageStream << tempBuffer;
            paramIndex++;
          }
          // 2. Handle Floating-Point Formats
          else if (specChar == 'f' || specChar == 'F' || specChar == 'e' ||
                   specChar == 'E' || specChar == 'g' || specChar == 'G') {
            double floatVal = 0.0;
            if (type == var::ScrVarType::FLOAT) {
              floatVal = static_cast<double>(Scr_GetFloat(inst, paramIndex));
              paramIndex++;
            } else if (type == var::ScrVarType::INT) {
              floatVal = static_cast<double>(Scr_GetInt(inst, paramIndex));
              paramIndex++;
            } else if (type == var::ScrVarType::VECTOR) {
              vec3_t vectorValue;
              Scr_GetVector(inst, paramIndex, &vectorValue);

              if (vectorComponent == 0)
                floatVal = static_cast<double>(vectorValue.x);
              else if (vectorComponent == 1)
                floatVal = static_cast<double>(vectorValue.y);
              else if (vectorComponent == 2)
                floatVal = static_cast<double>(vectorValue.z);

              vectorComponent++;
              if (vectorComponent == 3) {
                vectorComponent = 0;
                paramIndex++;
              }
            } else {
              paramIndex++;
            }
            snprintf(tempBuffer, sizeof(tempBuffer), specifier.c_str(),
                     floatVal);
            messageStream << tempBuffer;
          }
          // 3. Handle Integer Formats
          else {
            int32_t intVal = 0;
            if (type == var::ScrVarType::INT) {
              intVal = Scr_GetInt(inst, paramIndex);
              paramIndex++;
            } else if (type == var::ScrVarType::FLOAT) {
              intVal = static_cast<int32_t>(Scr_GetFloat(inst, paramIndex));
              paramIndex++;
            } else if (type == var::ScrVarType::VECTOR) {
              vec3_t vectorValue;
              Scr_GetVector(inst, paramIndex, &vectorValue);

              if (vectorComponent == 0)
                intVal = static_cast<int32_t>(vectorValue.x);
              else if (vectorComponent == 1)
                intVal = static_cast<int32_t>(vectorValue.y);
              else if (vectorComponent == 2)
                intVal = static_cast<int32_t>(vectorValue.z);

              vectorComponent++;
              if (vectorComponent == 3) {
                vectorComponent = 0;
                paramIndex++;
              }
            } else {
              paramIndex++;
            }
            snprintf(tempBuffer, sizeof(tempBuffer), specifier.c_str(), intVal);
            messageStream << tempBuffer;
          }
        } else {
          // Fallback if the format string expects more arguments than
          // provided
          messageStream << specifier;
        }
      } else {
        // Malformed specifier near the end of the string, print verbatim
        messageStream << specifier;
        i = j;
      }
    } else {
      // Pass standard characters through verbatim
      messageStream << formatStr[i];
    }
  }

  fprintf(stdout, "[BB][0] %s: %s\n", eventName, messageStream.str().c_str());
  fflush(stdout);
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

  if (name && name[0]) {
    fprintf(stdout, "[BB][%d] %s: %s\n", static_cast<int32_t>(controllerIndex),
            name, buffer.c_str());
    fflush(stdout);
  } else {
    fprintf(stdout, "[BB][%d]: %s\n", static_cast<int32_t>(controllerIndex),
            buffer.c_str());
    fflush(stdout);
  }
#endif
}
} // namespace bb
} // namespace game
namespace bb {

void stub_func() { return; }

utils::hook::detour BB_Print_hook;
utils::hook::detour BB_Send_hook;
utils::hook::detour BB_CheckSend_hook;

utils::hook::detour GScr_BBPrint_hook;

void redirect_bb_logging_to_stdout() {
  BB_Send_hook.create(
      game::bb::BB_Send.get(),
      reinterpret_cast<fastcallPtr_t<void(game::ControllerIndex_t, bool)>>(
          stub_func));
  BB_CheckSend_hook.create(
      game::bb::BB_CheckSend.get(),
      reinterpret_cast<fastcallPtr_t<void(game::ControllerIndex_t)>>(
          stub_func));
  GScr_BBPrint_hook.create(game::scr::gscr::GScr_BBPrint.get(),
                           game::scr::gscr::GScr_BBPrint_StdoutRedirect);
  BB_Print_hook.create(game::bb::BB_Print.get(),
                       game::bb::BB_Print_StdoutRedirect);
}

class component final : public client_component {
public:
  void post_unpack() override { redirect_bb_logging_to_stdout(); }
};
} // namespace bb
REGISTER_COMPONENT(bb::component);