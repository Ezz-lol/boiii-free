#include <std_include.hpp>

#include <cstdint>

#include <loader/component_loader.hpp>

#include "error_help.hpp"
#include "exception.hpp"
#include "scheduler.hpp"
#include <game/game.hpp>

#include <errhandlingapi.h>
#include <utils/compression.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/thread.hpp>

#include <exception/minidump.hpp>

// In case of clangd compilation
#if __has_include("version.hpp")
#include "version.hpp"
#else
#ifndef VERSION
#define VERSION "0"
#endif
#ifndef SHORTVERSION
#define SHORTVERSION "0"
#endif
#endif

#include <CommCtrl.h>
#include <dbghelp.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dbghelp.lib")

namespace exception {

namespace {
void exception_log(bool err, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int32_t buf_len = vsnprintf(nullptr, 0, fmt, args);

  std::string buffer;
  buffer.resize(buf_len + 1);
  va_start(args, fmt);
  vsnprintf(buffer.data(), buffer.size(), fmt, args);
  va_end(args);

  std::FILE *io = err ? stderr : stdout;
  fprintf(io, "%s\n", buffer.c_str());
  fflush(io);

  game::trace("{}{}", err ? "[Error] " : "", buffer.c_str());
}
static uint32_t main_thread_id{};
static std::once_flag sym_init_flag{};

void ensure_symbols_initialized() {
  std::call_once(sym_init_flag, [] {
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);
  });
}

struct resolved_frame {
  std::string module_name;
  std::string function_name;
  std::string file_name;
  uint64_t address = 0;
  uint64_t module_base = 0;
  uint64_t rva = 0;
  uint32_t line_number = 0;
};

resolved_frame resolve_address(void *addr) {
  resolved_frame frame{};
  frame.address = reinterpret_cast<uint64_t>(addr);

  const utils::nt::library mod = utils::nt::library::get_by_address(addr);
  if (mod) {
    frame.module_name = mod.get_name();
    frame.module_base = reinterpret_cast<uint64_t>(mod.get_ptr());
    frame.rva = frame.address - frame.module_base;

    if (frame.module_name == "BlackOps3.exe")
      frame.rva += 0x140000000;
  } else {
    frame.module_name = "unknown";
    frame.rva = frame.address;
  }

  ensure_symbols_initialized();

  // Try to resolve function name from PDB symbols
  alignas(SYMBOL_INFO) char sym_buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
  SYMBOL_INFO *sym = reinterpret_cast<SYMBOL_INFO *>(sym_buffer);
  sym->SizeOfStruct = sizeof(SYMBOL_INFO);
  sym->MaxNameLen = MAX_SYM_NAME;

  uint64_t displacement = 0;
  if (SymFromAddr(GetCurrentProcess(), frame.address, &displacement, sym)) {
    frame.function_name = sym->Name;
    if (displacement > 0)
      frame.function_name += utils::string::va("+0x%llX", displacement);
  }

  // Try to resolve source file and line
  IMAGEHLP_LINE64 line_info{};
  line_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
  unsigned long line_displacement = 0;
  if (SymGetLineFromAddr64(GetCurrentProcess(), frame.address,
                           &line_displacement, &line_info)) {
    frame.file_name = line_info.FileName;
    frame.line_number = line_info.LineNumber;
  }

  return frame;
}

std::string format_frame(const resolved_frame &f, size_t index) {
  std::string entry = utils::string::va("\t[%zu] %s + 0x%llX", index,
                                        f.module_name.c_str(), f.rva);

  if (!f.function_name.empty())
    entry += utils::string::va("  (%s)", f.function_name.c_str());

  if (!f.file_name.empty() && f.line_number > 0)
    entry +=
        utils::string::va("  [%s:%lu]", f.file_name.c_str(), f.line_number);

  return entry;
}

std::vector<resolved_frame>
capture_stackwalk(const LPEXCEPTION_POINTERS exceptioninfo,
                  int32_t max_frames = 48) {
  std::vector<resolved_frame> frames;

  if (!exceptioninfo || !exceptioninfo->ContextRecord)
    return frames;

  ensure_symbols_initialized();

  CONTEXT ctx = *exceptioninfo->ContextRecord;

  STACKFRAME64 stack_frame{};
  stack_frame.AddrPC.Offset = ctx.Rip;
  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Offset = ctx.Rbp;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Offset = ctx.Rsp;
  stack_frame.AddrStack.Mode = AddrModeFlat;

  const HANDLE process = GetCurrentProcess();
  const HANDLE thread = GetCurrentThread();

  for (int32_t i = 0; i < max_frames; ++i) {
    if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &stack_frame,
                     &ctx, nullptr, SymFunctionTableAccess64,
                     SymGetModuleBase64, nullptr))
      break;

    if (stack_frame.AddrPC.Offset == 0)
      break;

    frames.push_back(
        resolve_address(reinterpret_cast<void *>(stack_frame.AddrPC.Offset)));
  }

  return frames;
}

std::string get_crash_module_info(void *address) {
  const resolved_frame frame = resolve_address(address);
  return std::format("{}+0x{:X}{}", frame.module_name, frame.rva,
                     frame.function_name.empty()
                         ? ""
                         : std::format(" ({})", frame.function_name.c_str()));
}

utils::hook::detour mini_dump_write_dump_hook;

qboolean WINAPI mini_dump_write_dump_stub(
    const HANDLE h_process, const uint32_t process_id, const HANDLE h_file,
    const MINIDUMP_TYPE dump_type,
    const PMINIDUMP_EXCEPTION_INFORMATION exception_param,
    const PMINIDUMP_USER_STREAM_INFORMATION user_stream_param,
    const PMINIDUMP_CALLBACK_INFORMATION callback_param) {
  wchar_t filename[MAX_PATH];
  if (GetFinalPathNameByHandleW(h_file, filename, std::size(filename),
                                VOLUME_NAME_DOS)) {
    std::wstring path = filename;
    if (path.find(L"\\\\?\\") == 0) {
      path = path.substr(4);
    }

    const std::filesystem::path p(path);
    const std::filesystem::path minidumps_path =
        game::get_appdata_path() / "minidumps";
    std::error_code error;
    if (p.extension() == L".dmp" &&
        !std::filesystem::equivalent(p.parent_path(), minidumps_path, error)) {
      std::filesystem::create_directories(minidumps_path);

      const std::filesystem::path new_path = minidumps_path / p.filename();
      const HANDLE new_handle =
          CreateFileW(new_path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

      if (new_handle != INVALID_HANDLE_VALUE) {
        const qboolean result = mini_dump_write_dump_hook.invoke<qboolean>(
            h_process, process_id, new_handle, dump_type, exception_param,
            user_stream_param, callback_param);
        CloseHandle(new_handle);
        return result;
      }
    }
  }

  return mini_dump_write_dump_hook.invoke<qboolean>(
      h_process, process_id, h_file, dump_type, exception_param,
      user_stream_param, callback_param);
}

thread_local struct {
  uint32_t code = 0;
  void *address = nullptr;
  uintptr_t access = 0;
  uintptr_t target = 0;
  void *caller = nullptr;
  std::string culprit;
  std::string scripts;
} exception_data{};

bool is_game_thread();

constexpr std::array<uint32_t, game::scr::SCRIPTINSTANCE_MAX>
    SCRIPT_VARIABLE_COUNT{0x1FBD0, 0xFDE8};

struct script_state {
  const char *running = nullptr;
  bool executing = false;
  bool stack_corrupt = false;
  bool frames_corrupt = false;
  bool variables_corrupt = false;
};

bool read_script_state(const game::scr::scriptInstance_t inst,
                       script_state &state) {
  __try {
    volatile auto &vm = game::scr::vm::gScrVmPub->instance[inst];
    auto *stack = const_cast<game::scr::var::ScrVarValue_t *>(vm.stack);
    auto *frames =
        const_cast<game::scr::vm::function_frame_t *>(vm.function_frame_start);
    state.executing = vm.function_count > 0;
    state.stack_corrupt =
        vm.top < stack || vm.top >= stack + std::size(vm.stack);
    state.frames_corrupt =
        vm.function_frame < frames ||
        vm.function_frame >= frames + std::size(vm.function_frame_start);

    const volatile game::scr::var::ScrVar_t *variables =
        game::scr::vm::gScrVarGlob->instance[inst].scriptVariables;
    for (uint32_t i = 0; variables && i < SCRIPT_VARIABLE_COUNT[inst]; ++i) {
      if (static_cast<uint32_t>(variables[i].value.type) >=
          static_cast<uint32_t>(game::scr::var::ScrVarType::COUNT)) {
        state.variables_corrupt = true;
        break;
      }
    }

    const uint8_t *pos = game::scr::vm::gFs->instance[inst].pos;
    for (uint32_t i = 0;
         state.executing && i < game::scr::gObjFileInfoCount->instance[inst];
         ++i) {
      const game::scr::GSC_OBJ *obj =
          game::scr::gObjFileInfo->instance[inst][i].activeVersion;
      const uint8_t *code =
          reinterpret_cast<const uint8_t *>(obj) + obj->cseg_offset;
      if (pos >= code && pos < code + obj->cseg_size) {
        state.running = obj->get_name();
        break;
      }
    }
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

const char *thread_role() {
  if (is_game_thread()) {
    return "main";
  }
  if (game::sys::Sys_IsServerThread.get() && game::sys::Sys_IsServerThread()) {
    return "server";
  }
  return "auxiliary";
}

std::string describe_scripts() {
  const std::string_view role = thread_role();
  const bool server_thread =
      role == "server" || (game::is_server() && role == "main");

  std::string report;
  for (const game::scr::scriptInstance_t inst :
       {game::scr::SCRIPTINSTANCE_SERVER, game::scr::SCRIPTINSTANCE_CLIENT}) {
    script_state state{};
    if (!read_script_state(inst, state)) {
      continue;
    }

    const bool client = inst == game::scr::SCRIPTINSTANCE_CLIENT;
    const char *name = client ? "CSC" : "GSC";
    const bool crashing_thread =
        client ? role == "main" && !game::is_server() : server_thread;
    if (crashing_thread && state.executing) {
      report += std::format(
          "\nThe {} script{} was running when the game crashed, so a script "
          "(often from a mod or custom map) may have triggered it.",
          name,
          state.running ? std::format(" '{}'", state.running) : std::string{});
    }
    if (state.variables_corrupt || state.stack_corrupt ||
        state.frames_corrupt) {
      report += std::format("\nThe {} script {} corrupted.", name,
                            state.variables_corrupt ? "variables were"
                            : state.stack_corrupt   ? "VM stack was"
                                                    : "VM call frames were");
    }
  }
  return report;
}

bool is_game_thread() { return main_thread_id == GetCurrentThreadId(); }

void show_mouse_cursor() {
  while (ShowCursor(TRUE) < 0)
    ;
}

const char *get_exception_string(uint32_t exception);

std::string describe_crash() {
  if (exception_data.code == EXCEPTION_ACCESS_VIOLATION) {
    if (exception_data.caller) {
      return std::format("called an invalid function pointer 0x{:X}{} from {}",
                         exception_data.target,
                         exception_data.target < 0x10000 ? " (null pointer)"
                                                         : "",
                         get_crash_module_info(exception_data.caller));
    }
    return std::format("invalid memory access: tried to {} 0x{:X}{} in {}",
                       exception_data.access == 1 ? "write to" : "read",
                       exception_data.target,
                       exception_data.target < 0x10000 ? " (null pointer)" : "",
                       get_crash_module_info(exception_data.address));
  }
  return std::format("{} in {}", get_exception_string(exception_data.code),
                     get_crash_module_info(exception_data.address));
}

std::string crash_reason() {
  std::string reason = std::format("The game crashed: {}.{}", describe_crash(),
                                   exception_data.scripts);
  if (!exception_data.culprit.empty()) {
    reason += "\n\n" + exception_data.culprit;
  } else if (exception_data.scripts.empty()) {
    reason += "\n\nThe cause could not be determined from the crash itself. "
              "If it keeps happening: verify the game files in Steam, update "
              "your graphics driver, disable overlays (Steam, Discord, MSI "
              "Afterburner/RTSS) and remove CPU/GPU/RAM overclocks.";
  }
  return error_help::strip_colors(reason);
}

[[noreturn]] void restart_game() {
  STARTUPINFOW startup_info{};
  startup_info.cb = sizeof(startup_info);
  PROCESS_INFORMATION process_info{};
  if (CreateProcessW(nullptr, GetCommandLineW(), nullptr, nullptr, FALSE, 0,
                     nullptr, nullptr, &startup_info, &process_info)) {
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
  }
  TerminateProcess(GetCurrentProcess(), exception_data.code);
  __assume(false);
}

bool ask_restart(const std::string &text,
                 const wchar_t *headline = L"The game crashed.") {
  constexpr int32_t RESTART_BUTTON = 100;
  constexpr int32_t CLOSE_BUTTON = 101;
  const TASKDIALOG_BUTTON buttons[] = {
      {RESTART_BUTTON, L"Restart game\nClose the game and start it again."},
      {CLOSE_BUTTON, L"Close game"},
  };
  constexpr std::string_view repeated_headline = "The game crashed: ";
  std::string body = text;
  if (body.starts_with(repeated_headline)) {
    body.erase(0, repeated_headline.size());
    body[0] = static_cast<char>(std::toupper(body[0]));
  }
  const std::wstring content = utils::string::convert(body);

  TASKDIALOGCONFIG config{};
  config.cbSize = sizeof(config);
  config.dwFlags = TDF_USE_COMMAND_LINKS | TDF_SIZE_TO_CONTENT;
  config.pszWindowTitle = L"Ezz ERROR";
  config.pszMainIcon = TD_ERROR_ICON;
  config.pszMainInstruction = headline;
  config.pszContent = content.c_str();
  config.pButtons = buttons;
  config.cButtons = static_cast<UINT>(std::size(buttons));
  config.nDefaultButton = RESTART_BUTTON;

  int32_t pressed = CLOSE_BUTTON;
  return SUCCEEDED(TaskDialogIndirect(&config, &pressed, nullptr, nullptr)) &&
         pressed == RESTART_BUTTON;
}

void display_error_dialog() {
  const std::string minidumps_out =
      (game::get_appdata_path() / "minidumps").string();
  const std::string error_str = std::format(
      "{}\n\nDetails: {} (0x{:08X}) at {:p} on the {} thread.\n\n"
      "Crash dump saved in:\n{}\nPlease report it on https://dc.ezz.lol",
      crash_reason(), get_exception_string(exception_data.code),
      exception_data.code, game::derelocate(exception_data.address),
      thread_role(), minidumps_out);

  utils::thread::suspend_other_threads();
  show_mouse_cursor();

  if (game::quiet_crash() || game::is_headless()) {
    game::show_error(error_str, "Ezz ERROR");
    utils::thread::terminate_other_threads(exception_data.code);
  } else if (ask_restart(error_str)) {
    restart_game();
  } else {
    ShellExecuteA(nullptr, "open", minidumps_out.c_str(), nullptr, nullptr,
                  SW_SHOWNORMAL);
  }

  TerminateProcess(GetCurrentProcess(), exception_data.code);
}

void reset_state() {
  if (game::is_server()) {
    if (!is_game_thread()) {
      display_error_dialog();
    }

    std::string reason = std::format(
        "Server crash: {} (0x{:08X}) at {}",
        get_exception_string(exception_data.code), exception_data.code,
        get_crash_module_info(exception_data.address));
    game::com::Com_Error(game::errorParm::DROP, "%s", reason.c_str());
  } else {

    display_error_dialog();
  }
}

size_t get_reset_state_stub() {
  static void *stub = utils::hook::assemble([](utils::hook::assembler &a) {
    a.get().sub(rsp, 0x10);
    a.get().or_(rsp, 0x8);
    a.jmp(reset_state);
  });

  return reinterpret_cast<size_t>(stub);
}

std::string get_timestamp() {
  tm ltime{};
  char timestamp[MAX_PATH] = {0};
  const __time64_t time = _time64(nullptr);

  _localtime64_s(&ltime, &time);
  strftime(timestamp, sizeof(timestamp) - 1, "%Y-%m-%d-%H-%M-%S", &ltime);

  return timestamp;
}

const char *get_exception_string(const uint32_t exception) {
#define EXCEPTION_CASE(code)                                                   \
  case EXCEPTION_##code:                                                       \
    return "EXCEPTION_" #code
  switch (exception) {
    EXCEPTION_CASE(ACCESS_VIOLATION);
    EXCEPTION_CASE(DATATYPE_MISALIGNMENT);
    EXCEPTION_CASE(BREAKPOINT);
    EXCEPTION_CASE(SINGLE_STEP);
    EXCEPTION_CASE(ARRAY_BOUNDS_EXCEEDED);
    EXCEPTION_CASE(FLT_DENORMAL_OPERAND);
    EXCEPTION_CASE(FLT_DIVIDE_BY_ZERO);
    EXCEPTION_CASE(FLT_INEXACT_RESULT);
    EXCEPTION_CASE(FLT_INVALID_OPERATION);
    EXCEPTION_CASE(FLT_OVERFLOW);
    EXCEPTION_CASE(FLT_STACK_CHECK);
    EXCEPTION_CASE(FLT_UNDERFLOW);
    EXCEPTION_CASE(INT_DIVIDE_BY_ZERO);
    EXCEPTION_CASE(INT_OVERFLOW);
    EXCEPTION_CASE(PRIV_INSTRUCTION);
    EXCEPTION_CASE(IN_PAGE_ERROR);
    EXCEPTION_CASE(ILLEGAL_INSTRUCTION);
    EXCEPTION_CASE(NONCONTINUABLE_EXCEPTION);
    EXCEPTION_CASE(STACK_OVERFLOW);
    EXCEPTION_CASE(INVALID_DISPOSITION);
    EXCEPTION_CASE(GUARD_PAGE);
    EXCEPTION_CASE(INVALID_HANDLE);
  default:
    return "UNKNOWN";
  }
#undef EXCEPTION_CASE
}

std::string get_memory_registers(const LPEXCEPTION_POINTERS exceptioninfo) {
  if (!exceptioninfo || !exceptioninfo->ContextRecord) {
    return {};
  }

  const PCONTEXT ctx = exceptioninfo->ContextRecord;
  std::string info{"registers:\r\n{\r\n"};

  const std::function<void(const char *key, const uint64_t value)> reg =
      [&info](const char *key, const uint64_t value) {
        info.append(utils::string::va("\t%s = 0x%llX\r\n", key, value));
      };

  reg("rax", ctx->Rax);
  reg("rbx", ctx->Rbx);
  reg("rcx", ctx->Rcx);
  reg("rdx", ctx->Rdx);
  reg("rsp", ctx->Rsp);
  reg("rbp", ctx->Rbp);
  reg("rsi", ctx->Rsi);
  reg("rdi", ctx->Rdi);
  reg("r8", ctx->R8);
  reg("r9", ctx->R9);
  reg("r10", ctx->R10);
  reg("r11", ctx->R11);
  reg("r12", ctx->R12);
  reg("r13", ctx->R13);
  reg("r14", ctx->R14);
  reg("r15", ctx->R15);
  reg("rip", ctx->Rip);

  info.append("}");
  return info;
}

std::string get_callstack_summary(const LPEXCEPTION_POINTERS exceptioninfo,
                                  int32_t trace_depth = 48) {
  std::string info{"callstack:\r\n{\r\n"};

  std::vector<resolved_frame> frames =
      capture_stackwalk(exceptioninfo, trace_depth);

  if (frames.empty()) {
    // Fallback to RtlCaptureStackBackTrace if StackWalk64 fails
    void *backtrace_stack[32]{};
    const uint16_t count =
        RtlCaptureStackBackTrace(0, 32, backtrace_stack, nullptr);
    for (uint16_t i = 0; i < count; ++i) {
      resolved_frame f = resolve_address(backtrace_stack[i]);
      info.append(format_frame(f, i));
      info.append("\r\n");
    }
  } else {
    for (size_t i = 0; i < frames.size(); ++i) {
      info.append(format_frame(frames[i], i));
      info.append("\r\n");
    }
  }

  info.append("}");
  return info;
}

std::string generate_crash_info(const LPEXCEPTION_POINTERS exceptioninfo) {
  std::string info{};
  const std::function<void(const std::string &text)> line =
      [&info](const std::string &text) {
        info.append(text);
        info.append("\r\n");
      };

  const resolved_frame crash_frame =
      resolve_address(exceptioninfo->ExceptionRecord->ExceptionAddress);

  line("Ezz Crash Dump");
  line(std::string{});
  line("Version: "s + VERSION);
  line("Timestamp: "s + get_timestamp());
  line(utils::string::va(
      "Exception: 0x%08X (%s)", exceptioninfo->ExceptionRecord->ExceptionCode,
      get_exception_string(exceptioninfo->ExceptionRecord->ExceptionCode)));
  line(utils::string::va("Address: 0x%llX", crash_frame.address));
  line(utils::string::va("Module: %s + 0x%llX", crash_frame.module_name.c_str(),
                         crash_frame.rva));
  if (!crash_frame.function_name.empty())
    line("Function: " + crash_frame.function_name);
  if (!crash_frame.file_name.empty() && crash_frame.line_number > 0)
    line(utils::string::va("Source: %s:%lu", crash_frame.file_name.c_str(),
                           crash_frame.line_number));
  if (!exception_data.culprit.empty())
    line("Diagnosis: " + error_help::strip_colors(exception_data.culprit));
  if (!exception_data.scripts.empty())
    line("Scripts:" + exception_data.scripts);
  line(utils::string::va("Base: 0x%llX", game::get_base()));
  line(utils::string::va("Thread ID: %lu (%s)", GetCurrentThreadId(),
                         thread_role()));

  if (exceptioninfo->ExceptionRecord->ExceptionCode ==
      EXCEPTION_ACCESS_VIOLATION) {
    const char *op =
        exceptioninfo->ExceptionRecord->ExceptionInformation[0] == 1
            ? "write to"
            : "read from";
    uintptr_t target = exceptioninfo->ExceptionRecord->ExceptionInformation[1];
    line(utils::string::va(
        "Access Violation: Attempted to %s 0x%012llX%s", op, target,
        target < 0x10000 ? " (NULL pointer dereference)" : ""));
  }

  RTL_OSVERSIONINFOW version_info{};
  version_info.dwOSVersionInfoSize = sizeof(version_info);

  // Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored                                               \
    "-Wcast-function-type" // warning: cast between incompatible function types
                           // (for loader)
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored                                                 \
    "-Wpragmas" // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored                                                 \
    "-Wcast-function-type" // warning: cast between incompatible function types
                           // (for loader)
#endif
  typedef fastcallPtr_t<NTSTATUS(PRTL_OSVERSIONINFOW)> RtlGetVersionFunc;
  const RtlGetVersionFunc rtl_get_version = reinterpret_cast<RtlGetVersionFunc>(
      GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion"));
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

  if (rtl_get_version && NT_SUCCESS(rtl_get_version(&version_info))) {
    line(utils::string::va("OS Version: %u.%u.%u", version_info.dwMajorVersion,
                           version_info.dwMinorVersion,
                           version_info.dwBuildNumber));
  } else {
    line("OS Version: unavailable");
  }
  line(std::string{});
  line(get_callstack_summary(exceptioninfo));
  const std::string registers = get_memory_registers(exceptioninfo);
  if (!registers.empty()) {
    line(std::string{});
    line(registers);
  }

  return info;
}

void write_minidump(const LPEXCEPTION_POINTERS exceptioninfo) {
  const std::string crash_name =
      (game::get_appdata_path() / "minidumps" /
       utils::string::va("ezz-crash-%s.zip", get_timestamp().data()))
          .string();

  utils::compression::zip::archive zip_file{};
  zip_file.add("crash.dmp", create_minidump(exceptioninfo));
  zip_file.add("info.txt", generate_crash_info(exceptioninfo));
  if (!zip_file.write(crash_name, "Ezz Crash Dump")) {
    utils::io::remove_file(crash_name);
  }
}

// Empty string used as fallback for null localization pointers
char ui_localize_fallback[4] = "";

long WINAPI crash_fix_exception_handler(PEXCEPTION_POINTERS exception_info) {
  const PEXCEPTION_RECORD record = exception_info->ExceptionRecord;
  PCONTEXT context = exception_info->ContextRecord;

  long result = EXCEPTION_CONTINUE_SEARCH;

  switch (record->ExceptionCode) {
  case EXCEPTION_ACCESS_VIOLATION:
  case STATUS_ILLEGAL_INSTRUCTION: {
    const uintptr_t addr =
        reinterpret_cast<uintptr_t>(record->ExceptionAddress);
    const uintptr_t base = game::get_base();
    const uintptr_t offset = addr - base;
#ifndef NDEBUG
    const char *patch_name = nullptr;
#endif

    if (game::is_legacy_client()) {
      switch (offset) {
      // Killcam animation crash - invalid anim data access
      case 0x234B9BD:
#ifndef NDEBUG
        patch_name = "Killcam animation (invalid anim data)";
#endif
        context->Rax = 0;
        context->Rip = base + 0x234D14B;
        goto continue_execution;

      // CG_ZBarrierAttachWeapon - null weapon pointer in zombie barriers
      case 0x464FEF:
#ifndef NDEBUG
        patch_name = "ZBarrier weapon attach (null weapon)";
#endif
        context->Rax = 0;
        context->Rip = base + 0x4651A2;
        goto continue_execution;

      // asmsetanimationrate - bad entity reference
      case 0x15E4B5A:
#ifndef NDEBUG
        patch_name = "asmsetanimationrate (bad entity ref)";
#endif
        context->Rip = base + 0x15E4B83;
        goto continue_execution;

      // Orphaned thread crash
      case 0x12EE4CC:
#ifndef NDEBUG
        patch_name = "Orphaned thread";
#endif
        context->Rip = base + 0x12EE5C8;
        goto continue_execution;

      // Character index out-of-bounds crash
      case 0x234210C:
#ifndef NDEBUG
        patch_name = "Character index out-of-bounds";
#endif
        context->Rip = base + 0x2342136;
        goto continue_execution;

      // HKS internal crash
      case 0x1CAB4F1:
#ifndef NDEBUG
        patch_name = "HKS/Lua internal error";
#endif
        context->Rip = base + 0x1CAB69E;
        goto continue_execution;

      // Null localization string crashes
      case 0x2279323:
#ifndef NDEBUG
        patch_name = "Null localization string (UI)";
#endif
        context->Rdx = reinterpret_cast<uintptr_t>(ui_localize_fallback);
        goto continue_execution;

      case 0x2278B96:
#ifndef NDEBUG
        patch_name = "Null localization string (UI)";
#endif
        context->Rsi = reinterpret_cast<uintptr_t>(ui_localize_fallback);
        goto continue_execution;

      case 0x228ED56:
#ifndef NDEBUG
        patch_name = "Null localization string (UI)";
#endif
        context->Rcx = reinterpret_cast<uintptr_t>(ui_localize_fallback);
        goto continue_execution;

      // Unknown UI crash
      case 0x1EAAA27:
#ifndef NDEBUG
        patch_name = "UI crash (unknown)";
#endif
        context->Rip = base + 0x1EAABB3;
        goto continue_execution;

      // Non-existent clientfield crashes (CSC side)
      case 0xC15B80:
      case 0xC15C50:
      case 0xC18CF5:
#ifndef NDEBUG
        patch_name = "Non-existent clientfield (CSC)";
#endif
        context->Rcx = 1; // CSC instance
        context->Rdx =
            reinterpret_cast<uintptr_t>("Clientfield does not exist");
        context->R8 = 0;
        context->Rip = base + 0x12EA430; // Scr_Error
        goto continue_execution;

      // Non-existent clientfield crashes (GSC side)
      case 0x1A6BD1B:
      case 0x1A6BE2E:
      case 0x1A6BF2E:
      case 0x1A6BFCD:
      case 0x1A6C246:
      case 0x1A6C356:
      case 0x1A6C40D:
      case 0x1A6C697:
      case 0x1A6C894:
#ifndef NDEBUG
        patch_name = "Non-existent clientfield (GSC)";
#endif
        context->Rcx = 0; // GSC instance
        context->Rdx =
            reinterpret_cast<uintptr_t>("Clientfield does not exist");
        context->R8 = 0;
        context->Rip = base + 0x12EA430; // Scr_Error
        goto continue_execution;

      // Non-existent clientfield (additional crash sites)
      case 0x133EC1:
      case 0x133EEB:
#ifndef NDEBUG
        patch_name = "Non-existent clientfield (additional)";
#endif
        context->Rip = base + 0x133F12;
        goto continue_execution;

      case 0x133F31:
#ifndef NDEBUG
        patch_name = "Non-existent clientfield (additional)";
#endif
        context->Rip = base + 0x133F42;
        goto continue_execution;

      // Random crash on Zetsubou No Shima
      case 0x13591D3:
#ifndef NDEBUG
        patch_name = "Zetsubou No Shima map bug";
#endif
        context->Rip = base + 0x13591DA;
        goto continue_execution;
      default: {
        break;
      }
      continue_execution: {
        result = EXCEPTION_CONTINUE_EXECUTION;
        break;
      }
      }
    }

#ifndef NDEBUG
    if (patch_name) {
      exception_log(true, "^3[Exception] Known crash patched: %s (base+0x%llX)",
                    patch_name, offset);
    }
#endif
    break;
  }
  default: {
    break;
  }
  }
  return result;
}

std::string find_culprit(const std::vector<resolved_frame> &frames) {
  if (exception_data.code == EXCEPTION_IN_PAGE_ERROR) {
    return "Windows could not read part of the game from the disk.\nFix: "
           "Check the drive for errors (or reconnect it if it is external), "
           "then verify the game files in Steam.";
  }

  const utils::nt::library crashed =
      utils::nt::library::get_by_address(exception_data.address);
  if (crashed.get_ptr() == utils::nt::library::get_by_address(
                               reinterpret_cast<const void *>(&find_culprit))
                               .get_ptr()) {
    return "This is a bug in the Ezz client, not a problem with your PC or "
           "game files.\nFix: Please report it on https://dc.ezz.lol with the "
           "crash dump.";
  }

  std::vector<const void *> addresses{exception_data.address};
  for (const resolved_frame &frame : frames) {
    addresses.push_back(reinterpret_cast<const void *>(frame.address));
  }
  for (const void *address : addresses) {
    const utils::nt::library module =
        utils::nt::library::get_by_address(address);
    if (module) {
      if (std::string help = error_help::explain_module(module.get_path());
          !help.empty()) {
        return help;
      }
    }
  }
  return {};
}

bool is_harmless_error(const LPEXCEPTION_POINTERS exceptioninfo) {
  const uint32_t code = exceptioninfo->ExceptionRecord->ExceptionCode;
  return code == STATUS_INTEGER_OVERFLOW || code == STATUS_FLOAT_OVERFLOW ||
         code == STATUS_SINGLE_STEP;
}

long WINAPI exception_filter(const LPEXCEPTION_POINTERS exceptioninfo) {
  if (is_harmless_error(exceptioninfo)) {
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  const resolved_frame crash_frame =
      resolve_address(exceptioninfo->ExceptionRecord->ExceptionAddress);
  const char *exception_name =
      get_exception_string(exceptioninfo->ExceptionRecord->ExceptionCode);

  // Detailed console crash report
  exception_log(true, "========== CRASH DETECTED ==========");
  exception_log(true, "  Exception:  %s (0x%08lX)", exception_name,
                exceptioninfo->ExceptionRecord->ExceptionCode);
  exception_log(true, "  Module:     %s + 0x%llX",
                crash_frame.module_name.c_str(), crash_frame.rva);
  if (!crash_frame.function_name.empty())
    exception_log(true, "  Function:   %s", crash_frame.function_name.c_str());
  if (!crash_frame.file_name.empty() && crash_frame.line_number > 0)
    exception_log(true, "  Source:     %s:%u", crash_frame.file_name.c_str(),
                  crash_frame.line_number);
  exception_log(true, "  Address:    0x%llX", crash_frame.address);
  exception_log(true, "  Thread:     %lu (%s)", GetCurrentThreadId(),
                thread_role());

  if (exceptioninfo->ExceptionRecord->ExceptionCode ==
      EXCEPTION_ACCESS_VIOLATION) {
    const char *op =
        exceptioninfo->ExceptionRecord->ExceptionInformation[0] == 1
            ? "write to"
            : "read from";
    const uintptr_t target =
        exceptioninfo->ExceptionRecord->ExceptionInformation[1];
    exception_log(true, "  Details:    Attempted to %s 0x%012llX%s", op, target,
                  target < 0x10000 ? " (NULL pointer dereference)" : "");
  }

  // Print condensed callstack to console
  std::vector<resolved_frame> frames = capture_stackwalk(exceptioninfo, 16);
  if (!frames.empty()) {
    exception_log(true, "  Callstack:");
    for (size_t i = 0; i < frames.size(); ++i) {
      const resolved_frame *f = &frames[i];

      if (!f->function_name.empty()) {
        exception_log(true, "    [%zu] 0x%llX - %s!%s", i, f->address,
                      f->module_name.c_str(), f->function_name.c_str());
      } else {
        exception_log(true, "    [%zu] 0x%llX - %s + 0x%llX", i, f->address,
                      f->module_name.c_str(), f->rva);
      }
      fflush(stderr);
    }
  }
  exception_log(true, "=====================================");

  exception_data.code = exceptioninfo->ExceptionRecord->ExceptionCode;
  exception_data.address = exceptioninfo->ExceptionRecord->ExceptionAddress;
  exception_data.access =
      exceptioninfo->ExceptionRecord->ExceptionInformation[0];
  exception_data.target =
      exceptioninfo->ExceptionRecord->ExceptionInformation[1];
  exception_data.caller =
      exception_data.code == EXCEPTION_ACCESS_VIOLATION &&
              exception_data.access == 8
          ? *reinterpret_cast<void **>(exceptioninfo->ContextRecord->Rsp)
          : nullptr;
  exception_data.culprit = find_culprit(frames);
  exception_data.scripts = describe_scripts();

  write_minidump(exceptioninfo);
  exceptioninfo->ContextRecord->Rip = get_reset_state_stub();

  return EXCEPTION_CONTINUE_EXECUTION;
}

void WINAPI set_unhandled_exception_filter_stub(LPTOP_LEVEL_EXCEPTION_FILTER) {
  // Don't register anything here...
}
} // namespace

void show_fatal_error(const std::string &message) {
  if (!game::quiet_crash() && !game::is_headless()) {
    utils::thread::suspend_other_threads();
    show_mouse_cursor();
    if (ask_restart(error_help::strip_colors(message) +
                        "\n\nIf this keeps happening, please report it on "
                        "https://dc.ezz.lol",
                    L"The game hit an error and has to close.")) {
      restart_game();
    }
  }
  TerminateProcess(GetCurrentProcess(), 1);
}

struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "exception"; }
#endif

  component() {
    main_thread_id = GetCurrentThreadId();
    SetUnhandledExceptionFilter(exception_filter);

    std::filesystem::create_directories(game::get_appdata_path() / "minidumps");
  }

  void post_load() override {
    const utils::nt::library ntdll("ntdll.dll");
    using SetFilterFunc =
        fastcallPtr_t<void(LPTOP_LEVEL_EXCEPTION_FILTER filter)>;
    SetFilterFunc set_filter =
        ntdll.get_proc<SetFilterFunc>("RtlSetUnhandledExceptionFilter");

    set_filter(exception_filter);
    utils::hook::jump(set_filter, set_unhandled_exception_filter_stub);

    const utils::nt::library dbghelp = utils::nt::library::load("dbghelp.dll");
    if (dbghelp) {
      mini_dump_write_dump_hook.create(
          dbghelp.get_proc<decltype(mini_dump_write_dump_stub) *>(
              "MiniDumpWriteDump"),
          mini_dump_write_dump_stub);
    }

    AddVectoredExceptionHandler(1, crash_fix_exception_handler);
  }

  void post_unpack() override {
    scheduler::once(
        [] {
          game::cbuf::Cbuf_AddText(
              game::LOCAL_CLIENT_0,
              utils::string::va(
                  "dumpdir \"%s\"\n",
                  (game::get_appdata_path() / "minidumps").string().c_str()));
        },
        scheduler::pipeline::main);
  }
};
} // namespace exception

REGISTER_COMPONENT(exception::component)
