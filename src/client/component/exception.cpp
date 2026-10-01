#include <std_include.hpp>

#include <cstdint>

#include <loader/component_loader.hpp>

#include "dedicated/map_recovery.hpp"
#include "exception.hpp"
#include "scheduler.hpp"
#include "script_error.hpp"
#include <game/game.hpp>
#include <game/utils.hpp>

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

#include <dbghelp.h>
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
} exception_data{};

constexpr size_t MAX_RECOVERIES_PER_MINUTE = 3;
game::EngineDependentDvarMut crash_recovery;
std::mutex recovery_mutex;
std::deque<std::chrono::steady_clock::time_point> recent_recoveries;

std::mutex pending_drop_mutex;
std::string pending_drop_message;
std::atomic_bool pending_drop{false};
thread_local bool worker_recovering = false;

bool is_game_thread() { return main_thread_id == GetCurrentThreadId(); }

game::TLSData *engine_tls() {
  __try {
    return game::sys::Sys_GetTLS();
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return nullptr;
  }
}

jmp_buf *armed_error_jmp_buf() {
  game::TLSData *tls = engine_tls();
  if (!tls || !tls->errorJmpBuf) {
    return nullptr;
  }
  const auto *buffer = reinterpret_cast<const _JUMP_BUFFER *>(tls->errorJmpBuf);
  return buffer->Rip ? tls->errorJmpBuf : nullptr;
}

bool recovery_budget_left() {
  std::scoped_lock lock(recovery_mutex);
  const auto now = std::chrono::steady_clock::now();
  while (!recent_recoveries.empty() && now - recent_recoveries.front() > 1min) {
    recent_recoveries.pop_front();
  }
  return recent_recoveries.size() < MAX_RECOVERIES_PER_MINUTE;
}

bool can_recover_thread() {
  return (!crash_recovery || crash_recovery.get_bool()) &&
         armed_error_jmp_buf() && recovery_budget_left();
}

bool can_recover_crash() {
  return exception_data.code != EXCEPTION_STACK_OVERFLOW && can_recover_thread();
}

void record_recovery() {
  std::scoped_lock lock(recovery_mutex);
  recent_recoveries.push_back(std::chrono::steady_clock::now());
}

[[noreturn]] void recover_thread(const std::string &message) {
  if (is_game_thread()) {
    static std::string reason;
    reason = message;
    game::com::Com_Error(game::errorParm::DROP, "%s", reason.c_str());
  }

  {
    std::scoped_lock lock(pending_drop_mutex);
    pending_drop_message = message;
  }
  pending_drop = true;
  worker_recovering = true;
  longjmp(*armed_error_jmp_buf(), 1);
}

void show_mouse_cursor() {
  while (ShowCursor(TRUE) < 0)
    ;
}

const char *get_exception_string(uint32_t exception);

[[noreturn]] void restart_server(const std::string &reason) {
  exception_log(true, "Server crash: %s\nRestarting the server process.",
                reason.c_str());
  utils::thread::suspend_other_threads();

  STARTUPINFOW startup_info{};
  startup_info.cb = sizeof(startup_info);
  PROCESS_INFORMATION process_info{};
  if (CreateProcessW(nullptr, GetCommandLineW(), nullptr, nullptr, TRUE, 0,
                     nullptr, nullptr, &startup_info, &process_info)) {
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
  }
  TerminateProcess(GetCurrentProcess(), exception_data.code);
  __assume(false);
}

std::string describe_crash();

void display_error_dialog() {
  if (game::is_server()) {
    restart_server(describe_crash());
  }

  const resolved_frame frame = resolve_address(exception_data.address);
  const char *exception_name = get_exception_string(exception_data.code);
  const std::string location = get_crash_module_info(exception_data.address);
  const std::string minidumps_out =
      (game::get_appdata_path() / "minidumps").string();

  const char *error_str = utils::string::va(
      "%s (0x%08X) at %s\n\n"
      "Address: 0x%p (RVA: 0x%llX)\n"
      "Module: %s\n"
      "%s%s"
      "\nA crash dump has been saved to:\n%s\n"
      "Please report this crash and upload the dump file on our Discord:\n"
      "https://dc.ezz.lol\n",
      exception_name, exception_data.code, location.c_str(),
      exception_data.address, frame.rva, frame.module_name.c_str(),
      frame.function_name.empty() ? "" : "Function: ",
      frame.function_name.empty() ? "" : (frame.function_name + "\n").c_str(),
      minidumps_out.c_str());

  utils::thread::suspend_other_threads();
  show_mouse_cursor();

  game::show_error(error_str, "Ezz ERROR");

  if (game::quiet_crash()) {
    utils::thread::terminate_other_threads(exception_data.code);
  } else {
    ShellExecuteA(nullptr, "open", minidumps_out.c_str(), nullptr, nullptr,
                  SW_SHOWNORMAL);
  }

  TerminateProcess(GetCurrentProcess(), exception_data.code);
}

std::string describe_crash() {
  std::string what;
  switch (exception_data.code) {
  case EXCEPTION_ACCESS_VIOLATION: {
    if (exception_data.caller) {
      return std::format(
          "called an invalid function pointer 0x{:X}{} from {}",
          exception_data.target,
          exception_data.target < 0x10000 ? " (null pointer)" : "",
          get_crash_module_info(exception_data.caller));
    }
    what = std::format("invalid memory access: tried to {} 0x{:X}{}",
                       exception_data.access == 1 ? "write to" : "read",
                       exception_data.target,
                       exception_data.target < 0x10000 ? " (null pointer)"
                                                       : "");
    break;
  }
  case EXCEPTION_INT_DIVIDE_BY_ZERO:
    what = "integer division by zero";
    break;
  case EXCEPTION_ILLEGAL_INSTRUCTION:
  case EXCEPTION_PRIV_INSTRUCTION:
    what = "invalid instruction";
    break;
  case EXCEPTION_STACK_OVERFLOW:
    what = "stack overflow";
    break;
  default:
    what = std::format("{} (0x{:08X})", get_exception_string(exception_data.code),
                       exception_data.code);
    break;
  }

  what += " in " + get_crash_module_info(exception_data.address);

  return what;
}

void reset_state() {
  if (game::is_server()) {
    if (!is_game_thread()) {
      display_error_dialog();
    }

    static std::string reason;
    reason = "Server crash: " + describe_crash();
    script_error::mark_reported(reason);
    map_recovery::notify_clients(reason);
    game::com::Com_Error(game::errorParm::DROP, "%s", reason.c_str());
  }

  if (!can_recover_crash()) {
    display_error_dialog();
  }

  record_recovery();
  recover_thread(std::format(
      "The game crashed and was returned to the main menu.\n\n{}\n\n"
      "A crash dump was saved to:\n{}\n"
      "If this keeps happening, please report it on https://dc.ezz.lol",
      describe_crash(), (game::get_appdata_path() / "minidumps").string()));
}

void call_from_crash_site(CONTEXT &context, void (*target)()) {
  for (int32_t i = 0; i < 4 && (context.Rsp & 0xF) != 0; ++i) {
    DWORD64 image_base{};
    const PRUNTIME_FUNCTION function =
        RtlLookupFunctionEntry(context.Rip, &image_base, nullptr);
    if (function) {
      void *handler_data{};
      DWORD64 establisher_frame{};
      RtlVirtualUnwind(UNW_FLAG_NHANDLER, image_base, context.Rip, function,
                       &context, &handler_data, &establisher_frame, nullptr);
    } else {
      context.Rip = *reinterpret_cast<DWORD64 *>(context.Rsp);
      context.Rsp += 8;
    }
  }

  context.Rsp -= 8;
  *reinterpret_cast<DWORD64 *>(context.Rsp) = context.Rip;
  context.Rip = reinterpret_cast<DWORD64>(target);
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
  line(utils::string::va("Base: 0x%llX", game::get_base()));
  line(utils::string::va("Thread ID: %lu (%s)", GetCurrentThreadId(),
                         is_game_thread() ? "main" : "auxiliary"));

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

void write_minidump(const LPEXCEPTION_POINTERS exceptioninfo,
                    const bool in_background) {
  const std::string crash_name =
      (game::get_appdata_path() / "minidumps" /
       utils::string::va("ezz-crash-%s.zip", get_timestamp().data()))
          .string();

  auto archive = [crash_name, dump = create_minidump(exceptioninfo),
                  info = generate_crash_info(exceptioninfo)]() {
    utils::compression::zip::archive zip_file{};
    zip_file.add("crash.dmp", dump);
    zip_file.add("info.txt", info);
    if (!zip_file.write(crash_name, "Ezz Crash Dump")) {
      utils::io::remove_file(crash_name);
    }
  };

  if (in_background) {
    std::thread(std::move(archive)).detach();
  } else {
    archive();
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
                is_game_thread() ? "main" : "auxiliary");

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
  if (game::is_server() && is_game_thread()) {
    exception_log(true, "  Result:     %s",
                  map_recovery::on_map_stopped().c_str());
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

  write_minidump(exceptioninfo, !game::is_server() && can_recover_crash());
  call_from_crash_site(*exceptioninfo->ContextRecord, reset_state);

  return EXCEPTION_CONTINUE_EXECUTION;
}

utils::hook::detour com_error_abort_hook;
void com_error_abort_stub() {
  if (std::exchange(worker_recovering, false)) {
    return;
  }
  com_error_abort_hook.invoke<void>();
}

utils::hook::detour cl_frame_hook;
void cl_frame_stub(const int64_t local_client_num, const int32_t msec) {
  if (pending_drop.exchange(false)) {
    static std::string reason;
    {
      std::scoped_lock lock(pending_drop_mutex);
      reason = pending_drop_message;
    }
    game::com::Com_Error(game::errorParm::DROP, "%s", reason.c_str());
  }
  cl_frame_hook.invoke<void>(local_client_num, msec);
}

void WINAPI set_unhandled_exception_filter_stub(LPTOP_LEVEL_EXCEPTION_FILTER) {
  // Don't register anything here...
}
} // namespace

bool try_recover_fatal(const std::string &message) {
  if (!can_recover_thread()) {
    return false;
  }
  record_recovery();
  if (!is_game_thread()) {
    recover_thread(message);
  }
  return true;
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
    if (!game::is_server()) {
      crash_recovery = game::register_dvar_bool(
          "com_crashRecovery", true, game::DVAR_ARCHIVE,
          "Return to the main menu instead of closing the game after a crash "
          "or fatal error");
      com_error_abort_hook.create(game::com::Com_ErrorAbort.get(),
                                  com_error_abort_stub);
      cl_frame_hook.create(game::cl::CL_Frame.get(), cl_frame_stub);
    }

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
