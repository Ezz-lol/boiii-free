#include <sstream>
#include "../std_include.hpp"
#include <cstdint>
#include <atomic>
#include "loader/component_loader.hpp"

#include "scheduler.hpp"

#include <game/game.hpp>
#include <game/utils.hpp>
#include <game/impl/snd/snd.hpp>
#include <game/impl/snd/sd/sd.hpp>
#include <game/impl/cl/cl.hpp>
#include <game/impl/cg/cg.hpp>

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

#include <mmeapi.h>

namespace client_patches {
namespace {

utils::hook::detour preload_map_hook;

const game::dvar_t *cl_yaw_speed;
const game::dvar_t *cl_pitch_speed;

void stop_intro_if_needed() {
  if (game::com::Com_SessionMode_GetMode() != game::eModes::ZOMBIES &&
      game::com::Com_SessionMode_GetMode() != game::eModes::CAMPAIGN) {
    return;
  }

  scheduler::once(
      [] {
        scheduler::schedule(
            [] {
              if (!game::sys::Sys_IsDatabaseReady()) {
                return scheduler::cond_continue;
              }

              game::cinematic::Cinematic_StopPlayback(0, true);
              return scheduler::cond_end;
            },
            scheduler::main);
      },
      scheduler::main, 15s);
}

void preload_map_stub(game::LocalClientNum_t local_client_num,
                      const char *mapname, const char *gametype) {
  game::com::Com_GametypeSettings_SetGametype(gametype, true);
  stop_intro_if_needed();
  preload_map_hook.invoke(local_client_num, mapname, gametype);
}

void reduce_process_affinity() {
  const DWORD_PTR affinity =
      (1ULL << (std::min(std::thread::hardware_concurrency(), 4U))) - 1;
  SetProcessAffinityMask(GetCurrentProcess(), affinity);
}

void reset_process_affinity() {
  DWORD_PTR affinity_proc, affinity_sys;
  GetProcessAffinityMask(GetCurrentProcess(), &affinity_proc, &affinity_sys);
  SetProcessAffinityMask(GetCurrentProcess(), affinity_sys);
}

void fix_amd_cpu_stuttering() {
  scheduler::once(
      [] {
        reduce_process_affinity();
        scheduler::once(reset_process_affinity, scheduler::pipeline::main, 1s);
      },
      scheduler::pipeline::main);
}

MMRESULT mixer_open_stub() { return MMSYSERR_NODRIVER; }

bool is_mod_loaded_stub() { return false; }

void patch_is_mod_loaded_checks() {
  const std::vector<uintptr_t> is_mod_loaded_addresses = {
      0x1420F7484_g, 0x1420F74A4_g, 0x1420F73E4_g, 0x1420F73B4_g, 0x1420F6E57_g,
      0x1413E6A54_g, 0x1415E7EBB_g, 0x1415E87BB_g, 0x1415EBAC9_g, 0x1415F1F09_g,
      0x1415F1FB9_g, 0x1415F2080_g, 0x1415F7F40_g, 0x141A8D0ED_g, 0x141AA70F9_g,
      0x141EA06FB_g, 0x141EA8C7E_g, 0x141EB1A39_g, 0x141ECBA9D_g, 0x1420F6E1D_g,
  };

  for (const auto &address : is_mod_loaded_addresses) {
    utils::hook::call(address, is_mod_loaded_stub);
  }
}

float cl_key_state_yaw_speed_stub(void *key) {
  return game::cl::CL_KeyState(key) * cl_yaw_speed->current.value.value;
}

float cl_key_state_pitch_speed_stub(void *key) {
  return game::cl::CL_KeyState(key) * cl_pitch_speed->current.value.value;
}

game::fileHandle_t
fs_f_open_file_write_to_dir_stub(const char *filename,
                                 [[maybe_unused]] const char *dir,
                                 const char *os_base_path) {
  return game::fs::FS_FOpenFileWriteToDir(filename, "boiii_players",
                                          os_base_path);
}

game::fileHandle_t
fs_f_open_file_read_from_dir_stub(const char *filename,
                                  [[maybe_unused]] const char *dir,
                                  const char *os_base_path) {
  return game::fs::FS_FOpenFileReadFromDir(filename, "boiii_players",
                                           os_base_path);
}

int i_stricmp_stub(const char *s0, [[maybe_unused]] const char *s1) {
  return game::I_stricmp(s0, "boiii_players");
}

void fs_add_game_directory_stub(const char *path,
                                [[maybe_unused]] const char *dir) {
  utils::hook::invoke<void>(0x1422A2AF0_g, path, "boiii_players");
}

void patch_players_folder_name() {
  // Override 'players' folder
  utils::hook::call(0x14134764F_g, fs_f_open_file_write_to_dir_stub); // ??
  utils::hook::set<uint8_t>(0x14134762E_g, 0xEB);                     // ^^

  utils::hook::call(0x1413477EE_g, fs_f_open_file_write_to_dir_stub); // ??
  utils::hook::set<uint8_t>(0x1413477CD_g, 0xEB);                     // ^^

  utils::hook::call(0x141C20A1F_g, fs_f_open_file_write_to_dir_stub); // ??
  utils::hook::set<uint8_t>(0x141C209FE_g, 0xEB);                     // ^^

  utils::hook::call(0x1422F391E_g, fs_f_open_file_write_to_dir_stub); // ??

  utils::hook::call(0x141C2090F_g, fs_f_open_file_read_from_dir_stub); // ??
  utils::hook::set<uint8_t>(0x141C208EE_g, 0xEB);                      // ^^

  utils::hook::call(0x1422F3773_g, fs_f_open_file_read_from_dir_stub); // ??

  utils::hook::call(0x1422A2A61_g, i_stricmp_stub); // ??
  utils::hook::call(0x1422A2C82_g, i_stricmp_stub); // FS_AddGameDirectory

  utils::hook::call(0x1422A45A4_g, fs_add_game_directory_stub); // FS_Startup
}

void stub_func() { return; }

utils::hook::detour sd_alloc_block_hook;

utils::hook::detour sd_free_hook;

utils::hook::detour sd_alloc_sanity_hook;

/*
  The sound driver's aligned allocation pool is well-optimized and generally
  effective.

  However, when a high quantity of sounds are loaded with tail padding,
  allocation blocks can become highly fragmented. This can result in an
  attempted allocation to fail where there is sufficient remaining capacity in
  the pool, but insufficient contiguous free space to satisfy the allocation
  request.

  To resolve this, the best approach would be to implement a defragmentation
  routine that can be called when an allocation fails due to fragmentation. This
  routine would consolidate free blocks in the pool to create larger contiguous
  free spaces, allowing subsequent allocation attempts to succeed.

  Unfortunately, this is not feasible with the the SD allocator pool, as
  pointers to allocations are passed directly to the caller. Defragmentation
  would therefore cause all existing pointers to become invalid, leading to
  crashes and other undefined behavior.

  The best way to have implemented the pool would have been to either:
  1. Store the allocation pointers in an acting allocator struct, and instead
     return `sd_byte**` to callers, allowing the allocator to defragment as
  needed
  2. To use an allocation ID-based system where unique identifiers are returned
     to callers for a given allocation and can then be passed to the allocator
  to receive the pointer to the _current_ allocation address
  3. To implement a more traditional heap allocator with headers for each block
  that could be moved around in memory as needed.

  Because an allocator was not used in the engine which would allow
  defragmentation, and manually re-implementing and patching a large portion of
  the entire sound driver architecture to use e.g. `sd_byte**` allocation
  pointers is infeasible outside of a full decompilation project, the
  time-sensitive solution seems to be to replace the pool allocator entirely
  with a standard heap allocator (malloc/free).
*/
void replace_sd_allocator() {
  // Don't do sanity checks on the engine's sound allocation
  // pool, as we are replacing the allocator entirely
  sd_alloc_sanity_hook.create(game::snd::sd::SD_AllocSanity.get(), stub_func);
  sd_alloc_block_hook.create(game::snd::sd::SD_Alloc.get(),
                             game::snd::sd::SD_HeapAlloc);
  sd_free_hook.create(game::snd::sd::SD_Free.get(), game::snd::sd::SD_HeapFree);
}

utils::hook::detour live_delayed_com_error_hook;
void live_delayed_com_error_stub(const char *comErrorString, int32_t code) {
  void *return_address = _ReturnAddress();
  // Log caller and error message
  game::com::Com_Printf(
      0, game::consoleLabel_e::DEFAULT,
      "Live_DelayedComError called from 0x%p with message: %s and code: %d\n",
      return_address, comErrorString, code);
  printf(
      "Live_DelayedComError called from 0x%p with message: %s and code: %d\n",
      return_address, comErrorString, code);

  live_delayed_com_error_hook.invoke(comErrorString, code);
}

utils::hook::detour CL_CheckForResendHook;

template <typename T, std::atomic<T *> &storage> T *malloc_store(size_t size) {
  T *result = reinterpret_cast<T *>(malloc(size));
  storage.store(result, std::memory_order_seq_cst);

  return result;
}

template <typename T, std::atomic<T *> &storage> void free_zero(T *ptr) {
  free(ptr);
  storage.store(nullptr, std::memory_order_seq_cst);
}

template <typename T, std::atomic<T *> &storage>
T *Hunk_UserAlloc_StoreGlobal(game::hunk::HunkUser *user, size_t size,
                              int32_t alignment, const char *name) {
  T *result = reinterpret_cast<T *>(
      game::hunk::Hunk_UserAlloc(user, size, alignment, name));
  storage.store(result, std::memory_order_seq_cst);

  return result;
}

/*
   `storage` must be a `game::atomicarray` with item type `T`.
   Its length cannot be automatically derived in the template, so the array's
   type is specified as `auto` to allow count (`N`) computation in the function,
   with a proceeding assertion to ensure the passed `storage` type is a
   `game::atomicarray<T, N>`.
*/
template <typename T, auto &storage>
T *Hunk_UserAlloc_StoreGlobal_FirstNull(game::hunk::HunkUser *user, size_t size,
                                        int32_t alignment, const char *name) {
  constexpr uint32_t N = ARRAYSIZE(storage);

  static_assert(std::is_same_v<std::remove_reference_t<decltype(storage)>,
                               game::atomicarray<T *, N>>,
                "Type Error: 'storage' MUST be a game::atomicarray<T*, N>");

  T *result = reinterpret_cast<T *>(
      game::hunk::Hunk_UserAlloc(user, size, alignment, name));
  /*
     compare_exchange_strong requires an lvalue - cannot use an inlined
     `nullptr` rvalue
  */
  T *null = nullptr;
  for (uint32_t i = 0; i < N && !storage[i].compare_exchange_strong(
                                    null, result, std::memory_order_seq_cst,
                                    std::memory_order_relaxed);
       ++i) {
  }

  return result;
}

template <typename T, std::atomic<T *> &storage>
void Hunk_UserFree_ResetGlobal(game::hunk::HunkUser *user, T *ptr) {
  game::hunk::Hunk_UserFree(user, reinterpret_cast<void *>(ptr));
  storage.store(nullptr, std::memory_order_seq_cst);
}

utils::hook::detour CG_FreeCGEnts_hook;
utils::hook::detour CG_ClearCGEnts_hook;

// TODO: use when TAC protection removed
// utils::hook::detour AllocatePerLocalClientMemory_hook;

/*
  Store allocation pointers protected by Treyarch's anticheat (TAC) in our own
  globals.

  Allocation pointers protected by TAC are stored on the stack with de/encrypted
  stack offset computed with original, unprotected address where the allocation
  pointer would usually be stored.

  As such, and where de/encryption routines vary at each access and allocation,
  and are dependent on access occurring in the specific location of
  de/encryption, we cannot feasibly access these globals in boiii.

  To circumvent this, we can instead get each global's allocation pointer at the
  site of allocation, and store them in our own globals for use elsewhere.

  We then clear each global's allocation pointer when freed by hooking its
  corresponding free call in the engine.
*/
void store_tac_protected_allocs() {
  /*
     TODO: remove TAC protection and use this re-implementation
    AllocatePerLocalClientMemory_hook.create(
      game::cl::AllocatePerLocalClientMemory.get(),
      game::cl::AllocatePerLocalClientMemory_Impl);
  */

  CG_FreeCGEnts_hook.create(game::cg::CG_FreeCGEnts.get(),
                            game::cg::CG_FreeCGEnts_Impl);
  CG_ClearCGEnts_hook.create(game::cg::CG_ClearCGEnts.get(),
                             game::cg::CG_ClearCGEnts_Impl);

  /*
     TODO: remove this block and the `Hunk_UserAlloc_StoreGlobal` and
    `Hunk_UserAlloc_StoreGlobal_FirstNull` functions after TAC protection
    removed
  */
  {
    utils::hook::call(0x140840929_g,
                      reinterpret_cast<void *>(
                          Hunk_UserAlloc_StoreGlobal<game::level::cl::cgPool,
                                                     game::cg::cgArray_store>));
    utils::hook::call(
        0x1408421C3_g,
        reinterpret_cast<void *>(
            Hunk_UserAlloc_StoreGlobal<game::level::cl::cgsPool,
                                       game::cg::cgsArray_store>));
    utils::hook::call(
        0x140843A4F_g,
        reinterpret_cast<void *>(
            Hunk_UserAlloc_StoreGlobal<game::anim::ViewModelInfo,
                                       game::cg::cg_viewModelArray_store>));
    utils::hook::call(
        0x140843A70_g,
        reinterpret_cast<void *>(
            Hunk_UserAlloc_StoreGlobal<game::cg::ClientPlayerAttachmentInfo,
                                       game::cg::cg_attachmentsArray_store>));
    utils::hook::call(
        0x14085B9F5_g,
        reinterpret_cast<void *>(Hunk_UserAlloc_StoreGlobal_FirstNull<
                                 game::level::cl::centityPool_t,
                                 game::cg::cg_entitiesArray_store>));
  }

  // CG global hunk frees
  {
    utils::hook::call(
        0x140853E13_g,
        reinterpret_cast<void *>(
            Hunk_UserFree_ResetGlobal<game::cg::ClientPlayerAttachmentInfo,
                                      game::cg::cg_attachmentsArray_store>));
    utils::hook::call(
        0x140853E22_g,
        reinterpret_cast<void *>(
            Hunk_UserFree_ResetGlobal<game::anim::ViewModelInfo,
                                      game::cg::cg_viewModelArray_store>));
    utils::hook::call(0x140855728_g,
                      reinterpret_cast<void *>(
                          Hunk_UserFree_ResetGlobal<game::level::cl::cgsPool,
                                                    game::cg::cgsArray_store>));
    utils::hook::call(0x140856EC3_g,
                      reinterpret_cast<void *>(
                          Hunk_UserFree_ResetGlobal<game::level::cl::cgPool,
                                                    game::cg::cgArray_store>));
  }

  /*
     SV game level allocations.
     These never seem to be freed as far as I can tell. They would be
     automatically freed on application exit, so most likely this is how they
     are being freed.
  */
  {
    utils::hook::call(0x1419D7E22_g,
                      reinterpret_cast<void *>(
                          malloc_store<game::level::gentity_pool,
                                       game::level::g_entities_cl_allocation>));
  }
}
} // namespace

class component final : public client_component {
public:
  void post_unpack() override {

    store_tac_protected_allocs();
    replace_sd_allocator();

    fix_amd_cpu_stuttering();

    // Log location of caller to allow straightforward debugging
    live_delayed_com_error_hook.create(game::live::Live_DelayedComError.get(),
                                       live_delayed_com_error_stub);

    // Don't modify process priority
    utils::hook::nop(0x142334C98_g, 6);

    // Kill microphones for now
    utils::hook::set(0x15AAE9254_g, mixer_open_stub);

    preload_map_hook.create(0x14135A1E0_g, preload_map_stub);

    // Keep client ranked when mod loaded
    utils::hook::jump(0x1420D5BA0_g, is_mod_loaded_stub);
    patch_is_mod_loaded_checks();

    // Kill Client/Server Index Mismatch error
    utils::hook::set<uint8_t>(0x1400A7588_g, 0xEB);

    // Always get loadscreen gametype from s_gametype
    utils::hook::set<uint8_t>(0x14228F5DC_g, 0xEB);

    cl_yaw_speed = game::register_dvar_float(
        "cl_yawspeed", 140.0f, std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max(), game::DVAR_NONE,
        "Max yaw speed in degrees for game pad and keyboard");
    cl_pitch_speed = game::register_dvar_float(
        "cl_pitchspeed", 140.0f, std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max(), game::DVAR_NONE,
        "Max pitch speed in degrees for game pad");

    // CL_AdjustAngles
    utils::hook::call(0x1412F3324_g,
                      cl_key_state_yaw_speed_stub); // cl_yawspeed
    utils::hook::call(0x1412F3344_g, cl_key_state_yaw_speed_stub); // ^^

    utils::hook::call(0x1412F3380_g,
                      cl_key_state_pitch_speed_stub); // cl_pitchspeed
    utils::hook::call(0x1412F33A1_g, cl_key_state_pitch_speed_stub); // ^^

    /*
      CL_CheckForResend inexplicably, inconsistently triggers a segmentation
      fault when pushing its localClientNum argument onto the stack.

      The root cause of the segmentation fault is presently unclear. In a dump
      of the executable just after arxan unpack and subsequent integrity check
      stub patches, all calls to the function are correct, as is the instruction
      used by CL_CheckForResend to push the localClientNum argument onto the
      stack. The analyzed binary would not trigger this segmentation fault.

      As such, this instruction is almost certainly being mutated at runtime by
      arxan.

      This segmentation fault was previously resolved by patching the first byte
      of the corresponding instruction (`mov [rsp-8+arg_18], rbx`) to be 0xC3
      (retn). Likely as a result of some other arxan runtime binary mutation,
      this does not trigger an immediate return, but instead causes normal
      function execution, without the aforementioned segmentation fault.

      This also causes the `localClientNum` argument to always be
      set to 0, breaking server connection for non-primary, local clients.

      This crash, as well as a few other patches required to be made to
      CL_CheckForResend's internal logic, presented the need for total
      replacement of the function.

      Total replacement of a function can usually be performed by `hook`ing
      the function and instead executing another function which does not invoke
      the hooked function. In this case, the crash-triggering instruction would
      still be executed, as hook invocation executes the hooked function's
      prologue.

      To resolve this, we instead replace all known calls to the original
      function with its replacement function. We also hook and replace the
      original function in case of unknown, obfuscated callers.
    */
    // CL_MapLoading call to CL_CheckForResend
    // Note: crash inconsistently occurs when CL_CheckForResend is called here.
    utils::hook::call(0x141359DB4_g, game::cl::CL_CheckForResend_Impl);
    // CL_Frame call to CL_CheckForResend
    utils::hook::call(0x1413514BE_g, game::cl::CL_CheckForResend_Impl);

    CL_CheckForResendHook.create(game::cl::CL_CheckForResend.get(),
                                 game::cl::CL_CheckForResend_Impl);

    patch_players_folder_name();
  }
};
} // namespace client_patches

REGISTER_COMPONENT(client_patches::component)