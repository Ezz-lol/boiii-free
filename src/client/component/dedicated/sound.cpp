#include <std_include.hpp>

#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include <game/impl/snd/snd.hpp>
#include <game/impl/snd/sd/sd.hpp>

#include <component/scheduler.hpp>
#include <component/game_event.hpp>

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <string>

namespace dedicated {
namespace sound {
using namespace game;
using namespace game::snd;
using namespace game::snd::cmd;
using namespace game::snd::sd;
using namespace game::snd::sndl;
namespace {

utils::hook::detour DB_LoadXFile_hook;
bool DB_LoadXFile_LoadSoundBanksBlocking(
    const char *path, game::db::DBFile f,
    game::db::xzone::XZoneBuffer *fileBuffer, const char *filename,
    game::db::XBlock *blocks, game::db::DB_Interrupt *interrupt, uint8_t *buf,
    game::PMemStack side, int flags) {
  bool succeeded = DB_LoadXFile_hook.invoke<bool>(
      path, f, fileBuffer, filename, blocks, interrupt, buf, side, flags);

  if (succeeded && (game::db::load::g_load->flags & 0x1000C00) != 0) {
    g_sb->loadGate = qfalse;
    SND_LoadSoundsWait();
  }

  return succeeded;
}

void SNDL_FreeClearBank(SndBankLoad *load, int64_t offset, uint64_t len) {
  if (load->loadedEntries) {
    SD_HeapFree(load->loadedEntries);
  }
  if (load->loadedData) {
    SD_HeapFree(load->loadedData);
  }
  if (load->loadAssetBank.entries) {
    SD_HeapFree(load->loadAssetBank.entries);
  }
  if (load->streamAssetBank.entries) {
    SD_HeapFree(load->streamAssetBank.entries);
  }

  memset(load, offset, len);
}

utils::hook::detour SND_Init_hook;
void SND_Init_SetInitialised() {
  SND_Init_hook.invoke();
  *g_pc_nosnd = qtrue;
  g_snd->verified_0.init = qtrue;
  g_sb->bankMagic = 0x12233445;
}

utils::hook::detour SND_QueueAdd_hook;
void Snd_QueueAdd_ProcessImmediately([[maybe_unused]] SndQueue *queue,
                                     SndCommandType cmd, uint32_t size,
                                     SndCommand data) {
  SND_CommandSND(cmd, static_cast<uint64_t>(size), data);
}

utils::hook::detour SND_Active_hook;
qboolean SND_Active_ForceTrue() {
  g_snd->verified_0.init = qtrue;
  return SND_Active_hook.invoke<qboolean>();
}

utils::hook::detour SNDL_Update_hook;
void SNDL_Update_Safe() {
  if (SND_GetDuckById(g_snd->verified_0.defaultHash) &&
      SND_GetReverb(g_snd->verified_0.defaultHash, "default")) {
    SNDL_Update_hook.invoke();
  }
}

utils::hook::detour G_SndEnabled_hook;
utils::hook::detour SND_ShouldInit_hook;
bool return_true() { return true; }

utils::hook::detour SND_QueueFlush_hook;
utils::hook::detour SND_ProcessSNDQueue_hook;
void stub_func() { return; }

utils::hook::detour SND_EnqueueLoadedAssets_hook;
utils::hook::detour SND_StartTocRead_hook;
utils::hook::detour snd_bank_load_error_hook;

void snd_bank_load_error_stub(SndBankLoad *load) {
  const std::string sound_path =
      (game::get_game_path() / "zone" / "snd").string();
  const std::string_view zone =
      load && load->bank && load->bank->zone && load->bank->zone[0]
          ? load->bank->zone
          : "UNKNOWN";

  fprintf(stderr,
          "^3[Sound] A sound bank for zone %s could not be loaded. If its "
          "files are "
          "missing, copy them to '%s'. Dedicated-server sound files are "
          "optional; restart with '-nosnd' to run without them.\n",
          zone.data(), sound_path.c_str());
  fflush(stderr);
  snd_bank_load_error_hook.invoke(load);
}

/*
  Sound load, processing, and data access functionality was consistently either
  removed or disabled in dedicated server. This was a valid optimization for the
  stated intent - a dedicated, multiplayer server using only Treyarch maps - as
  Treyarch multiplayer maps never require server-side sound handling in any
  form.

  Treyarch zombies and custom maps for any gamemode, however, generally do
  require server-side sound processing, and the lack of it causes a wide variety
  of sound-related bugs. This function re-enables where possible and otherwise
  re-implements sound functionality in the dedicated server engine.

  This fixes most bugs related to server-side sound handling.

  For example:
  - Map music, sound effects, or voicelines not playing, erroneously playing in
  a loop, playing at the wrong time, or all playing at the same time - occurs in
  most zombies maps.
  - Maps implementing manual sound loops with intermittent delay generated
  by the `soundgetplaybacktime` GSC function crashing the server with `G_Spawn:
  no free entities` error; Die Rise, for example.

  Does not fix:
  - Perk machine jingles inconsistently playing when player is in proximity.
*/

inline void enable_sound() {
  snd_bank_load_error_hook.create(SND_BankLoadError.get(),
                                  snd_bank_load_error_stub);

  /*
    In the lines of code where the client versions of SND_EnqueueLoadedAssets
    and SND_StartTocRead require usage of `SD_Alloc`, in dedicated server, a
    `nullptr` immediate value is used instead, causing these steps of bank
    load to immediately fail, and bank load to never occur.

    Hook these functions and replace them with the client-equivalent
    implementation.
  */
  SND_EnqueueLoadedAssets_hook.create(SND_EnqueueLoadedAssets.get(),
                                      SND_EnqueueLoadedAssets_Impl);
  SND_StartTocRead_hook.create(SND_StartTocRead.get(), SND_StartTocRead_Impl);

  /*
    In client, in SNDL_RemoveBank, SD_Free is called to free the
    heap-allocated bank data of a SndBankLoad, before clearing the
    SndBankLoad with memset.

    This obviously is not performed in server, as SD_Alloc is also not
    used in the unmodified engine; it was reimplemented and used in the above
    SND_EnqueueLoadedAssets_Impl.

    This hooks the memset call to instead first free these allocations,
    if they are present, to prevent memory leak. The client frees these
    allocations in the same location.
  */
  utils::hook::call(0x14064AB30_g, SNDL_FreeClearBank);

  /*
    After loading level XPAK, block on loading its soundbanks, just as
    client does.
  */
  DB_LoadXFile_hook.create(game::db::load::DB_LoadXFile.get(),
                           DB_LoadXFile_LoadSoundBanksBlocking);

  /*
    The dedicated server does not have an async sound queue, and the
    initialization in client is heavily arxan obfuscated.
    Suffice to say I have so far been unable (in a time-sensitive manner) to
    verify accurate structure and values for async queue initialization.

    Fortunately, the dedicated server actually doesn't need an async queue at
    all. Sounds only need to be processed at initial load, but not afterwards,
    except as requested by scripts.

    The below hooks circumvent attempted usage of the (non-existent) async
    sound queue, instead forwarding queue additions to the intended handler,
    immediately.
  */
  SND_QueueAdd_hook.create(SND_QueueAdd.get(), Snd_QueueAdd_ProcessImmediately);
  SND_ProcessSNDQueue_hook.create(SND_ProcessSNDQueue.get(), stub_func);
  SND_QueueFlush_hook.create(SND_QueueFlush.get(), stub_func);

  /*
    Enable sound
  */
  SND_Active_hook.create(SND_Active.get(), SND_Active_ForceTrue);
  SND_Init_hook.create(SND_Init.get(), SND_Init_SetInitialised);
  G_SndEnabled_hook.create(G_SndEnabled.get(), return_true);
  SND_ShouldInit_hook.create(SND_ShouldInit.get(), return_true);

  /*
     Gracefully skip sound update instead of crashing if default sound assets
     not loaded.

     Fixes crash on server shutdown or restart.
  */
  SNDL_Update_hook.create(SNDL_Update.get(), SNDL_Update_Safe);
}

} // namespace

struct component final : server_component {
  void post_unpack() override {

    if (!utils::flags::has_flag("nosnd")) {
      enable_sound();
    }
  }
};
} // namespace sound
} // namespace dedicated
REGISTER_COMPONENT(dedicated::sound::component)