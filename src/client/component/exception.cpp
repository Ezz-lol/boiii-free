#include <std_include.hpp>

#include <cstdint>

#include <loader/component_loader.hpp>

#include "dedicated/map_recovery.hpp"
#include "error_help.hpp"
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

extern "C" unsigned long _tls_index;

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
} exception_data{};

constexpr size_t MAX_RECOVERIES_PER_MINUTE = 3;
game::EngineDependentDvarMut crash_recovery;
std::mutex recovery_mutex;
std::deque<std::chrono::steady_clock::time_point> recent_recoveries;

constexpr const wchar_t *CRASH_RESTARTS_ENV = L"BOIII_CRASH_RESTARTS";
constexpr uint32_t MAX_QUICK_RESTARTS = 2;
std::atomic<int32_t> archives_in_flight{0};
const std::chrono::steady_clock::time_point process_start =
    std::chrono::steady_clock::now();
std::atomic_bool recovery_in_progress{false};

std::mutex pending_drop_mutex;
std::string pending_drop_reason;
std::string pending_drop_footer;
std::atomic_bool pending_drop{false};
thread_local bool worker_recovering = false;

constexpr uint64_t RECOVERY_WATCH_MS = 60000;
constexpr uint64_t RECOVERY_HANG_MS = 30000;
std::atomic<uint64_t> last_frame_tick{0};
std::atomic<uint64_t> last_recovery_tick{0};
std::string last_recovery_reason;
bool nested_error_recovered = false;

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
  return exception_data.code != EXCEPTION_STACK_OVERFLOW &&
         !recovery_in_progress && can_recover_thread();
}

void watch_recovery();

void record_recovery(const std::string &reason) {
  {
    std::scoped_lock lock(recovery_mutex);
    recent_recoveries.push_back(std::chrono::steady_clock::now());
    last_recovery_reason = reason;
  }
  recovery_in_progress = true;
  watch_recovery();
}

constexpr std::array<uint32_t, game::scr::SCRIPTINSTANCE_MAX>
    SCRIPT_VARIABLE_COUNT{0x1FBD0, 0xFDE8};
std::array<volatile game::scr::var::ScrVar_t *, game::scr::SCRIPTINSTANCE_MAX>
    known_variable_tables{};
constexpr size_t SCRIPT_NAME_SEARCH_HASH_SIZE = 0x40000;
std::array<std::atomic_bool, game::scr::SCRIPTINSTANCE_MAX>
    wiped_script_instances{};

void remember_variable_tables() {
  for (size_t inst = 0; inst < known_variable_tables.size(); ++inst) {
    if (!known_variable_tables[inst]) {
      known_variable_tables[inst] =
          game::scr::vm::gScrVarGlob->instance[inst].scriptVariables;
    }
  }
}

bool is_variable_table_valid(const volatile game::scr::var::ScrVar_t *vars,
                             const uint32_t count) {
  __try {
    for (uint32_t i = 0; i < count; ++i) {
      if (static_cast<uint32_t>(vars[i].value.type) >=
          static_cast<uint32_t>(game::scr::var::ScrVarType::COUNT)) {
        return false;
      }
    }
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

void wipe_script_instance(const game::scr::scriptInstance_t inst) {
  volatile auto &glob = game::scr::vm::gScrVarGlob->instance[inst];
  if (glob.scriptNameSearchHashList) {
    std::memset(const_cast<game::scr::var::ScrVarIndex_t *>(
                    glob.scriptNameSearchHashList),
                0, SCRIPT_NAME_SEARCH_HASH_SIZE);
  }
  game::scr::var::ScrVar_InitVariables(inst);
  game::scr::var::ScrVar_InitClassMap(inst);
  if (uint8_t *saved = game::scr::saved_world_object_valid.get()) {
    for (const size_t slot :
         {static_cast<size_t>(inst), static_cast<size_t>(inst) + 2}) {
      saved[slot * game::scr::SAVED_WORLD_OBJECT_STRIDE] = 0;
    }
  }
}

const char *script_instance_name(const game::scr::scriptInstance_t inst) {
  return inst == game::scr::SCRIPTINSTANCE_CLIENT ? "CSC" : "GSC";
}

void reset_vm_runtime(const game::scr::scriptInstance_t inst) {
  volatile auto &vm = game::scr::vm::gScrVmPub->instance[inst];
  volatile auto &glob = game::scr::vm::gScrVmGlob->instance[inst];
  vm.top = const_cast<game::scr::var::ScrVarValue_t *>(vm.stack);
  vm.function_frame =
      const_cast<game::scr::vm::function_frame_t *>(vm.function_frame_start);
  vm.function_count = 0;
  vm.localVars = const_cast<uint32_t *>(glob.localVarsStack) - 1;
  vm.callNesting = 0;
  vm.inparamcount = 0;
  vm.outparamcount = 0;
  vm.debugCode = false;
  vm.abort_on_error = false;
  vm.terminal_error = false;
  vm.block_execution = false;
  glob.dialog_error_message = nullptr;
}

const char *running_script_name(const game::scr::scriptInstance_t inst) {
  __try {
    const uint8_t *pos = game::scr::vm::gFs->instance[inst].pos;
    for (uint32_t i = 0; i < game::scr::gObjFileInfoCount->instance[inst];
         ++i) {
      const game::scr::GSC_OBJ *obj =
          game::scr::gObjFileInfo->instance[inst][i].activeVersion;
      const uint8_t *code =
          reinterpret_cast<const uint8_t *>(obj) + obj->cseg_offset;
      if (pos >= code && pos < code + obj->cseg_size) {
        return obj->get_name();
      }
    }
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }
  return nullptr;
}

std::string stop_aborted_script() {
  game::scr::scriptInstance_t inst;
  if (is_game_thread()) {
    inst = game::scr::SCRIPTINSTANCE_CLIENT;
  } else if (game::sys::Sys_IsServerThread.get() &&
             game::sys::Sys_IsServerThread()) {
    inst = game::scr::SCRIPTINSTANCE_SERVER;
  } else {
    return {};
  }

  if (game::scr::vm::gScrVmPub->instance[inst].function_count <= 0) {
    return {};
  }
  const char *script = running_script_name(inst);
  reset_vm_runtime(inst);
  return std::format(
      "\n\nThe {} script^5{}^7 was running when this happened and has been "
      "stopped.",
      script_instance_name(inst),
      script ? std::format(" '{}'", script) : std::string{});
}

std::string reset_corrupt_script_vm(const game::scr::scriptInstance_t inst) {
  const char *name = script_instance_name(inst);

  volatile auto &glob = game::scr::vm::gScrVarGlob->instance[inst];
  if (!glob.scriptVariables && known_variable_tables[inst]) {
    glob.scriptVariables = known_variable_tables[inst];
  }
  const bool variables_valid =
      !glob.scriptVariables ||
      is_variable_table_valid(glob.scriptVariables,
                              SCRIPT_VARIABLE_COUNT[inst]);
  if (!variables_valid) {
    wipe_script_instance(inst);
    wiped_script_instances[inst] = true;
  }

  volatile auto &vm = game::scr::vm::gScrVmPub->instance[inst];
  auto *stack = const_cast<game::scr::var::ScrVarValue_t *>(vm.stack);
  auto *frames =
      const_cast<game::scr::vm::function_frame_t *>(vm.function_frame_start);
  const bool top_valid =
      vm.top >= stack && vm.top < stack + std::size(vm.stack);
  const bool frame_valid =
      vm.function_frame >= frames &&
      vm.function_frame < frames + std::size(vm.function_frame_start);
  if (variables_valid && top_valid && frame_valid) {
    return {};
  }

  reset_vm_runtime(inst);
  return std::format(
      "\n^3The {} script {} corrupted, so its script system was reset.^7", name,
      !variables_valid ? "variables were"
      : !top_valid     ? "VM stack was"
                       : "VM call frames were");
}

std::string reset_corrupt_script_vms() {
  return reset_corrupt_script_vm(game::scr::SCRIPTINSTANCE_SERVER) +
         reset_corrupt_script_vm(game::scr::SCRIPTINSTANCE_CLIENT);
}

[[noreturn]] void drop_to_main_menu(const std::string &reason,
                                    const std::string &footer) {
  static std::string message;
  message = reason + reset_corrupt_script_vms() + footer;
  script_error::mark_reported(message);
  game::com::Com_Error(game::errorParm::DROP, "%s", message.c_str());
  __assume(false);
}

constexpr size_t CRITICAL_SECTION_COUNTS_OFFSET = 0xB0;

void leave_script_critical_sections() {
  auto *const tls_block = *reinterpret_cast<uint8_t **>(
      __readgsqword(0x58) + sizeof(void *) * _tls_index);
  auto *const counts = reinterpret_cast<volatile int32_t *>(
      tls_block + CRITICAL_SECTION_COUNTS_OFFSET);
  for (const game::sys::CriticalSection critsect :
       {game::sys::CriticalSection::SCRIPT_STRING,
        game::sys::CriticalSection::VM}) {
    while (counts[static_cast<int32_t>(critsect)] > 0) {
      game::sys::Sys_LeaveCriticalSection(critsect);
    }
  }
}

[[noreturn]] void recover_thread(const std::string &reason,
                                 const std::string &footer) {
  leave_script_critical_sections();
  if (is_game_thread()) {
    drop_to_main_menu(reason, footer);
  }

  std::string report = reason;
  if (game::sys::Sys_IsServerThread.get() && game::sys::Sys_IsServerThread()) {
    report += reset_corrupt_script_vm(game::scr::SCRIPTINSTANCE_SERVER);
  }
  {
    std::scoped_lock lock(pending_drop_mutex);
    pending_drop_reason = std::move(report);
    pending_drop_footer = footer;
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

uint32_t inherited_restarts() {
  wchar_t count[16]{};
  return GetEnvironmentVariableW(CRASH_RESTARTS_ENV, count, std::size(count))
             ? std::wcstoul(count, nullptr, 10)
             : 0;
}

bool started_recently() {
  return std::chrono::steady_clock::now() - process_start < 1min;
}

bool restart_allowed() {
  return game::is_server() &&
         (!started_recently() || inherited_restarts() < MAX_QUICK_RESTARTS);
}

void wait_for_archives() {
  for (int32_t i = 0; archives_in_flight > 0 && i < 1200; ++i) {
    std::this_thread::sleep_for(50ms);
  }
}

[[noreturn]] void restart_process(const std::string &reason) {
  exception_log(true, "%s\nRestarting the server process.", reason.c_str());
  wait_for_archives();

  SetEnvironmentVariableW(
      CRASH_RESTARTS_ENV,
      std::to_wstring(started_recently() ? inherited_restarts() + 1 : 1)
          .c_str());

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

std::string minidumps_folder() {
  return (game::get_appdata_path() / "minidumps").string();
}

[[noreturn]] void display_error_dialog(const std::string &reason) {
  wait_for_archives();
  const std::string minidumps_out = minidumps_folder();

  utils::thread::suspend_other_threads();
  show_mouse_cursor();

  game::show_error(
      std::format(
          "{}\n\nCrash dumps are saved in:\n{}\nPlease report this "
          "and upload the dump file on our Discord:\nhttps://dc.ezz.lol",
          error_help::strip_colors(reason), minidumps_out),
      "Ezz ERROR");

  if (game::quiet_crash()) {
    utils::thread::terminate_other_threads(exception_data.code);
  } else {
    ShellExecuteA(nullptr, "open", minidumps_out.c_str(), nullptr, nullptr,
                  SW_SHOWNORMAL);
  }

  TerminateProcess(GetCurrentProcess(), exception_data.code);
  __assume(false);
}

std::string describe_crash() {
  std::string what;
  switch (exception_data.code) {
  case EXCEPTION_ACCESS_VIOLATION: {
    if (exception_data.caller) {
      return std::format("called an invalid function pointer 0x{:X}{} from {}",
                         exception_data.target,
                         exception_data.target < 0x10000 ? " (null pointer)"
                                                         : "",
                         get_crash_module_info(exception_data.caller));
    }
    what = std::format(
        "invalid memory access: tried to {} 0x{:X}{}",
        exception_data.access == 1 ? "write to" : "read", exception_data.target,
        exception_data.target < 0x10000 ? " (null pointer)" : "");
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
    what = get_exception_string(exception_data.code);
    break;
  }

  what += std::format(" in {} (0x{:08X})",
                      get_crash_module_info(exception_data.address),
                      exception_data.code);

  return what;
}

std::string crash_reason(const std::string_view headline,
                         const std::string &script) {
  std::string reason =
      std::format("^1{}: {}.^7{}", headline, describe_crash(), script);
  if (!exception_data.culprit.empty()) {
    reason += "\n\n" + exception_data.culprit;
  } else if (script.empty()) {
    reason += "\n\n^3The cause could not be determined from the crash itself. "
              "^7If it keeps happening: verify the game files in Steam, update "
              "your graphics driver, disable overlays (Steam, Discord, MSI "
              "Afterburner/RTSS) and remove CPU/GPU/RAM overclocks.";
  }
  return reason;
}

[[noreturn]] void give_up(const std::string &reason) {
  if (restart_allowed()) {
    restart_process(reason);
  }
  display_error_dialog(reason);
}

[[noreturn]] void close_after_hang() {
  std::string reason;
  {
    std::scoped_lock lock(recovery_mutex);
    reason = last_recovery_reason;
  }
  display_error_dialog(std::format("The game stopped responding after "
                                   "recovering from an error and has to "
                                   "close.\n\n{}",
                                   reason));
}

void watch_recovery() {
  last_recovery_tick = GetTickCount64();
  static std::once_flag watchdog;
  std::call_once(watchdog, [] {
    std::thread([] {
      while (true) {
        std::this_thread::sleep_for(1s);
        const uint64_t now = GetTickCount64();
        const uint64_t recovered = last_recovery_tick;
        if (now - recovered < RECOVERY_WATCH_MS &&
            now - std::max(last_frame_tick.load(), recovered) >
                RECOVERY_HANG_MS) {
          close_after_hang();
        }
      }
    }).detach();
  });
}

void reset_state() {
  if (game::is_server()) {
    if (!is_game_thread()) {
      give_up("Server crash: " + describe_crash());
    }

    static std::string reason;
    reason = "Server crash: " + describe_crash();
    script_error::mark_reported(reason);
    map_recovery::notify_clients(reason);
    game::com::Com_Error(game::errorParm::DROP, "%s", reason.c_str());
  }

  const std::string script = stop_aborted_script();
  if (!can_recover_crash()) {
    give_up(crash_reason(recovery_in_progress
                             ? "The game crashed again while recovering "
                               "from a crash"
                             : "The game crashed",
                         script));
  }

  const std::string reason = crash_reason("The game crashed", script);
  record_recovery(reason);
  recover_thread(reason,
                 std::format("\n\nThe game was returned to the main menu. A "
                             "crash dump was saved to:\n{}\nIf this keeps "
                             "happening, please report it on "
                             "https://dc.ezz.lol",
                             minidumps_folder()));
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
  if (!exception_data.culprit.empty())
    line("Diagnosis: " + exception_data.culprit);
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
    ++archives_in_flight;
    std::thread([archive = std::move(archive)] {
      archive();
      --archives_in_flight;
    }).detach();
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

std::string find_culprit(const std::vector<resolved_frame> &frames) {
  if (exception_data.code == EXCEPTION_IN_PAGE_ERROR) {
    return "Windows could not read part of the game from the disk.\nFix: "
           "Check the drive for errors (or reconnect it if it is external), "
           "then verify the game files in Steam.";
  }

  const utils::nt::library crashed =
      utils::nt::library::get_by_address(exception_data.address);
  if (crashed.get_ptr() == utils::nt::library{}.get_ptr()) {
    const resolved_frame frame = resolve_address(exception_data.address);
    return std::format("The crash happened inside the Ezz client itself{}.\n"
                       "Fix: Please report it on https://dc.ezz.lol with the "
                       "crash dump.",
                       frame.function_name.empty()
                           ? std::string{}
                           : std::format(" ({})", frame.function_name));
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
  exception_data.culprit = find_culprit(frames);

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

bool script_system_initialized(const game::scr::scriptInstance_t inst) {
  return game::scr::vm::gScrVarPub->instance[inst].timeArrayId != 0;
}

utils::hook::detour scr_exec_thread_hook;
game::scr::var::ScrVarIndex_t
scr_exec_thread_stub(const game::scr::scriptInstance_t inst, const uint8_t *pos,
                     const uint32_t num_params,
                     game::scr::var::ScrVarValue_t *return_value,
                     const game::scr::var::ScrVarIndex_t self) {
  if (recovery_in_progress && !script_system_initialized(inst)) {
    reset_vm_runtime(inst);
    return {};
  }
  return scr_exec_thread_hook.invoke<game::scr::var::ScrVarIndex_t>(
      inst, pos, num_params, return_value, self);
}

utils::hook::detour scr_init_system_hook;
void scr_init_system_stub(const game::scr::scriptInstance_t inst) {
  if (wiped_script_instances[inst].exchange(false)) {
    wipe_script_instance(inst);
  }
  scr_init_system_hook.invoke<void>(inst);
}

utils::hook::detour scr_free_thread_hook;
void scr_free_thread_stub(const game::scr::scriptInstance_t inst,
                          const game::scr::var::ScrVarIndex_t thread_id) {
  if (thread_id) {
    scr_free_thread_hook.invoke<void>(inst, thread_id);
  }
}

utils::hook::detour cl_frame_hook;
void cl_frame_stub(const int64_t local_client_num, const int32_t msec) {
  if (pending_drop.exchange(false)) {
    std::string reason;
    std::string footer;
    {
      std::scoped_lock lock(pending_drop_mutex);
      reason = pending_drop_reason;
      footer = pending_drop_footer;
    }
    drop_to_main_menu(reason, footer);
  }
  recovery_in_progress = false;
  nested_error_recovered = false;
  last_frame_tick = GetTickCount64();
  remember_variable_tables();
  cl_frame_hook.invoke<void>(local_client_num, msec);
}

void WINAPI set_unhandled_exception_filter_stub(LPTOP_LEVEL_EXCEPTION_FILTER) {
  // Don't register anything here...
}
} // namespace

void recover_fatal_error(const std::string &reason) {
  if (recovery_in_progress || !can_recover_thread()) {
    return;
  }
  const std::string message = reason + stop_aborted_script();
  record_recovery(message);
  recover_thread(message, "\n\nThe game was returned to the main menu.");
}

void recover_nested_error(const std::string &reason) {
  game::qboolean *error_entered = game::com::com_errorEntered.get();
  if (!error_entered || !*error_entered || !is_game_thread() ||
      nested_error_recovered || !can_recover_thread()) {
    return;
  }
  nested_error_recovered = true;
  const std::string message = reason + stop_aborted_script();
  record_recovery(message);
  *error_entered = {};
  recover_thread(message, "\n\nThe game was returned to the main menu.");
}

void restart_after_fatal_error(const std::string &message) {
  if (restart_allowed()) {
    restart_process("Server fatal error: " + message);
  }
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
      scr_init_system_hook.create(game::scr::Scr_InitSystem.get(),
                                  scr_init_system_stub);
      scr_exec_thread_hook.create(game::scr::Scr_ExecThread.get(),
                                  scr_exec_thread_stub);
      scr_free_thread_hook.create(game::scr::Scr_FreeThread.get(),
                                  scr_free_thread_stub);
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

  void pre_destroy() override { wait_for_archives(); }
};
} // namespace exception

REGISTER_COMPONENT(exception::component)
