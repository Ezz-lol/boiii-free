// GAME BUILD: BlackOps3.exe PE CheckSum 0x06531394 (Ezz BOIII client binary).
// Ported from LocalPlayer123/BO3-4-Player-Local-Splitscreen-on-PC (Unlicense),
// donor PE 0x06517980. Code RVAs >= 0x1DFF150 shifted by -0x6C0; data RVAs
// unchanged. Every patch still verifies original bytes and skips on mismatch.
#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "game/utils.hpp"
#include "scheduler.hpp"
#include "splitscreen_reloc.hpp"
#include "splitscreen_signin.hpp"

#include <utils/hook.hpp>
#include <utils/finally.hpp>

#include <d3d11.h> // sun shadow sidecar views (COM calls only, no import library)

// Local splitscreen beyond the PC's hard limit of two players.
//
// WHY THIS IS A COMPONENT AND NOT A SCRIPT
//
// Everything here was first proven with external Python that attaches to the
// running process (tools/reloc_range.py, tools/stride_fix.py,
// tools/nativize_daemon.py, all measured on 2026-08-05). Two things cannot be
// done that way:
//
//   * player data for a third local client is rejected at STARTUP - creating
//     _2/_3 files crashes at "Press ENTER to Start", RVA 0x01D36066 - which is
//     before any external tool can attach
//   * every session otherwise has to re-derive the module base and re-apply the
//     whole chain by hand
//
// WHAT IS IMPLEMENTED HERE
//
//   1. the arrays proven [4] on PS4 and [2] on the PC, relocated so slot 2
//      exists instead of overwriting whatever the linker put next
//   2. a repair for one per-client base pointer that arrives corrupted
//   3. a keep-alive that stops the launch handshake stalling on the injected
//      local client
//
// DELAYED WORK
//
// clientGameStates[2], guest identities and storage are owned here. s_gamePads
// is also owned here, but its relocation is deliberately delayed until the
// first real two-player lobby: activating it during startup crashed gamepad
// init at RVA 0x022E9550, while the same validated 38-reference move is stable
// after seats 0 and 1 exist. The netchan table is part of the normal relocation
// set; four-player-only pieces remain in tools/chain4.ps1.

namespace splitscreen {
namespace {
// --- the per-client base that arrives mangled -------------------------
//
// Measured with tools/stride_tracer.py, same process, same code path:
//
//     working 2 players   16423 executions, base 0x000001C366843310
//     failing 3 players       1 execution,  base 0xBF217B476B83E047
//
// and the table below holds the correct value in BOTH cases. So the datum
// is right at rest and wrong on arrival - it travels through
// mov rax,[rsp+0x50] / ror rax,0x20 behind an Arxan integrity compare.
// Reading it from its source restores the correct value; it is not a null
// guard, because the value is neither null nor missing.
constexpr size_t base_table_rva = 0x17ADC958;

// imul rcx, rcx, 0x1e940   - displaced into the cave
constexpr size_t stride_site_rva = 0x1F23651;
constexpr uint8_t stride_site_bytes[] = {0x48, 0x69, 0xC9, 0x40,
                                         0xE9, 0x01, 0x00};

// A SECOND consumer at 0x00F7E918 receives the same mangled value, but
// repairing it there kills the process with no minidump at all - measured
// twice, with and without a guard. Do not add it back without
// understanding why.

// --- storage: give controllers 2 and 3 real buffers -------------------
//
// Read out of the PS4 build (RULE ZERO), storage_api.cpp AllocateMemory,
// called once from Storage_Init:
//
//     total = shared + LOCAL_CLIENT_COUNT * (perControllerFiles + scratch)
//     pool  = PMem_Alloc(total)
//     for (controller = 0; controller < LOCAL_CLIENT_COUNT; controller++)
//         for each file type, for each slot of that type:
//             file->properties = props
//             file->buffer     = pool + used;  used += props->maxSize
//         scratch.buff = pool + used;  used += 0x20000
//
// The PC has exactly the same function at RVA 0x02275840..0x02275AAF,
// with LOCAL_CLIENT_COUNT = 2 in two places and nowhere else:
//
//     0x02275922  lea eax, [r9 + r10*2]   the pool multiplier - the SIB
//                                         scale byte is the count
//     0x02275A80  cmp r12d, 2             the controller loop bound
//
// Both are one byte and both keep their instruction length. Nothing is
// hand-written and no state is faked: the game's own allocator carves
// four controllers out of a pool that is four controllers big, which is
// exactly what the console does. Files whose properties->[0x20] == 3 are
// shared and still resolve to controller 0's buffer, because the reuse
// path reads [rax+rsi] with rsi stepping -0x8958 per controller.
//
// This is why slot 2 had no loadout: not a missing registration step,
// but an allocator that only ever ran twice.
struct byte_patch {
  size_t rva;
  uint8_t expect;
  uint8_t value;
  const char *what;
};

// Allocating for four controllers is necessary but NOT sufficient: the
// same translation unit bounds the controller count in eight more
// places. Found mechanically rather than by eye - every `cmp <r32>, 2`
// whose loop body indexes s_storage by 0x8958 - so the list is complete
// for this TU rather than "the ones that were noticed".
//
// The one that matters most is 0x02276E30, a plain gatekeeper:
//
//     movsxd rsi, ecx ; cmp esi, 2 ; jl ok ; xor al, al ; ret
//
// called from the file-lookup paths. Every storage operation for
// controller 2 returns false there today, whether or not its buffers
// exist. Widening the allocator alone would have produced buffers that
// nothing could ever find - which is worth knowing, because it would
// have looked like the allocator fix had failed.
//
// All nine are `cmp r32, imm8` with the immediate byte at a measured
// offset, so every patch is one byte and keeps its instruction length.
// --- THE LOCAL-CLIENT COUNT: the root of the whole controller-2 problem --
//
// Verified 2026-08-16 (LOG.md "BOOT-ALLOCATION MAP"), by reading each site.
//
// `cl_maxLocalClients` lives at RVA 0x053A2720 and `pe_xref` finds **2630**
// rip-relative reads of it, overwhelmingly `cmp <reg>, [cl_maxLocalClients]`
// bounds checks spread across the entire engine. It holds 2, so ~2630
// validity checks reject local client 2. PS4 CG_GetLocalClientGlobals
// (0x1D76850) shows the consequence in source form:
//
//     ASSERT((unsigned)localClientNum < (unsigned)cl_maxLocalClients)
//       "localClientNum doesn't index cl_maxLocalClients"  cg_local_mp.h:0x8AD
//     if (cgArray == NULL)                      return NULL
//     if (localClientNum >= cl_maxLocalClients) return NULL   <- ctrl 2
//     return cgArray + localClientNum * stride
//
// That NULL is the 0xC0000005 measured when 0x01E0B520 was forced true, and
// the same NULL BG_UnlockablesGetLocalCACRoot asserts on for the gumball
// row. So this is not one more per-controller array to widen - it is the
// value the others are all downstream of.
//
// WHY THIS IS SAFE TO RAISE (the containers are ready):
//   * The per-client CG blocks are NOT fixed [2] arrays. PS4
//     CG_AllocateClientMemory (0x21FD70) sizes cgArray/cgsArray/
//     cg_viewModelArray/cg_attachmentsArray/cg_weaponsArray/cg_destructibles
//     as `count * stride` via Hunk_UserAlloc and stores each into an 8-byte
//     POINTER global (`mov [rsi], rax`). They scale with the count by
//     construction. Same on the PC: 0x00840922 `imul rdx, rdx, 0x342720`
//     with edx = the count.
//   * s_storage and the userData map/records ARE fixed arrays - and this
//     component already relocates all three (storage, client_objs 0x10->0x20,
//     client_ui 0x2340->0x45C0). Hence the gate below.
//
// ORDERING RULE: containers first, count second. Raising the count while a
// target array is still [2] makes the per-client loop write slot 2/3 over
// adjacent live memory - the crash family this project already knows.
// The allocation floor's own RVA, so BO3_SS_SKIP=floor can leave exactly
// this one entry out of the table below.
constexpr size_t alloc_floor_rva = 0x135D68C;

constexpr byte_patch local_client_count_patches[] = {
    // max(CL_SplitscreenPlayerCount(), 2) inside
    // CL_AllocatePerLocalClientMemory
    // (0x0135D650):  call 0x0283AC20 ; mov r14d,2 ; cmp eax,r14d ; cmovge
    // r14d,eax
    // It is a FLOOR, not a cap. Raising it forwards on registers into
    // CG_AllocateClientMemory, FX_AllocateClientMemory, CL_AllocateClientMemory
    // and the cl_maxLocalClients store at 0x0135D489 - no further patch needed.
    {0x135D68C, 0x02, 0x04,
     "local-client count floor: max(count,2) -> max(count,4)"},

    // THE FLAG-4 OVERRIDE - found 2026-08-24 after three count-source
    // fixes in a row failed the same way (cl_max 3 -> 2 at the map-load
    // reallocation, with floor byte 03 and dvar 3 verified live). The
    // in-game allocation pass calls CL_AllocatePerLocalClientMemory with
    // flags bit 2 set, and on that path the PC DISCARDS the computed
    // max(CL_SplitscreenPlayerCount(), floor):
    //
    //   0135D67C  test bpl, 4
    //   0135D682  mov  edi, esi          ; maxClients = 0x12 (18)
    //   0135D684  lea  r14d, [rsi-0x10]  ; local = 2, HARD-CODED
    //
    // PS4 (0x416A10) has no such override: 0x12 is the PublicBotGame
    // maxCLIENTS ([rbp-0x18] = 0x12 at 0x416B8C) while the LOCAL count
    // stays max(SplitscreenPlayerCount(), 2) on every mode branch. The
    // PC baked local = 18-16 into the lea when the port fixed MAX_LOCAL
    // at 2. NOPing only the lea keeps maxClients = 18 and lets the
    // console computation through: a 2-player session still allocates
    // exactly 2 (max(2, floor 2)), so stock behaviour is bit-identical
    // until three seats have really seated.
    {0x135D6A4, 0x44, 0x90, "flag-4 alloc: drop the hard local=2 (1/4)"},
    {0x135D6A5, 0x8D, 0x90, "flag-4 alloc: drop the hard local=2 (2/4)"},
    {0x135D6A6, 0x76, 0x90, "flag-4 alloc: drop the hard local=2 (3/4)"},
    {0x135D6A7, 0xF0, 0x90, "flag-4 alloc: drop the hard local=2 (4/4)"},

    // The boot BIND loop 0x0135DDC8..0x0135DE24 - the SOLE caller of
    // Com_LocalClient_SetControllerIndex (0x020EFF00) and the only boot code
    // that sets a slot's controllerIndex and beingUsed flag. Runs i=0..1.
    {0x135DE43, 0x02, 0x04, "boot bind loop: local clients 2 -> 4"},

    // ...and its controllerIndex clamp `edx = min(i, 1)`:
    //     mov edx,1 ; cmp edi,edx ; cmovle edx,edi ; call SetControllerIndex
    // PS4 clamps min(i,3) (0xE49DC5, I_tmin(i,3)). Without this, slots 2 and 3
    // would both bind controllerIndex 1 and collide with slot 1.
    {0x135DE2F, 0x01, 0x03,
     "boot bind clamp: controllerIndex min(i,1) -> min(i,3)"},

    // Lua GetCountUsedAndSignedInLocalClients (0x01FC73E0, found with
    // find_luibinding.py) counts local clients 0..1 where PS4 0xD2EE40
    // counts 0..3:
    //
    //   01FC73F8  call 0x020EF990   Com_LocalClient_IsBeingUsed(lc)
    //   01FC7403  call 0x020EF930   Com_LocalClient_GetControllerIndex(lc)
    //   01FC740A  call 0x01EC7030   LiveUser_IsSignedIn(ci)
    //   01FC7417  cmp ebx, 2 / jl   <- THE BOUND, imm8 at 0x01FC7419
    //
    // Measured 2026-08-16, ZM offline lobby, after
    // SigninLocalClient(2,true) returned 0: the channel reported
    // [2/1/3] - SplitscreenNum 3, but this counter still 2, and the
    // lobby drew two players. This loop simply never looks at local
    // client 2.
    //
    // Safe on its own terms: the body calls three PURE PREDICATES and
    // increments a local. It indexes no per-client array, so widening
    // it cannot write anywhere. If IsBeingUsed(2) is false the count
    // stays 2 and status 72 says why.
    {0x1FBAC99, 0x02, 0x04, "GetCountUsedAndSignedInLocalClients: 2 -> 4"},

    // LobbyHost_AddLocalClients - the one that decides who is IN THE LOBBY.
    //
    // With the count fixed, three local clients are used and signed in
    // (status 72) and the lobby STILL listed two:
    // `Engine.LobbyGetSessionClients` returned exactly two tables. That is
    // a different subsystem, and PS4 0xCA5CA0 is its specification:
    //
    //   session = LobbyHostData_GetSession(lobbyType)
    //   for ci in 0..3:                          <- FOUR (0xCA5CE2)
    //       if !ShouldAddController(session, ci) continue
    //       LobbySession_AddClient(ci, session, LiveUser_GetXuid(ci),
    //                              LiveUser_GetClientName(ci), ...)
    //
    // and `ShouldAddController` (0xCA5F50) is only:
    //
    //   lc = Com_ControllerIndex_GetLocalClientNum(ci)
    //   if (!Com_LocalClient_IsBeingUsed(lc)) return false
    //   mode = LobbyBase_GetNetworkMode()
    //   return mode == 2 ? Live_IsUserSignedInToDemonware(ci) : mode <= 1
    //
    // Controller 2 now passes both: status 72 measures IsBeingUsed(2) true,
    // and offline is not network mode 2. So the ONLY thing keeping it out
    // of the lobby is the PC's loop bound.
    //
    // The PC function is 0x01ED7560, identified by its "adding local
    // clients" string reference at 0x01ED7667 and the same call shape
    // (0x020EF7C0 GetLocalClientNum -> 0x020EF990 IsBeingUsed -> skip).
    // Its loop closes at
    //
    //   01ED76F7  inc ebx
    //   01ED76F9  cmp ebx, 2      83 FB 02, immediate at 0x01ED76FB
    //   01ED76FC  jge out
    //
    // THE THREE THE LOBBY PANEL ACTUALLY ASKS.
    //
    // Traced with the channel's own `T an` during a guest re-join: the
    // panel drives its rows from `Engine.GetUsedControllerCount()` and
    // `Engine.IsControllerBeingUsed(i)`, and only ever reached
    // `GetPlayerStats(0,0,0)` / `GetItemImage(...)` for controller 0.
    // Measured with three players in the lobby:
    //
    //   IsControllerBeingUsed(0/1/2) -> true, (3) -> false
    //   GetUsedControllerCount()     -> 2      <- disagrees with the above
    //   GetNonUsedControllerCount()  -> 0
    //   GetMaxControllerCount()      -> 2.0f
    //
    // All three are their own loops over controllers, bounded at two:
    //
    //   01FE428C  cmp ebx, 2   GetUsedControllerCount    (0x01FE4260)
    //   01FE35F7  cmp ebx, 2   GetNonUsedControllerCount (0x01FE35C0)
    //   01FE3378  mov dword ptr [rax-8], 0x40000000      (0x01FE3370)
    //
    // PS4 settles all three: `Lua_CoD_LuaCall_GetUsedControllerCount`
    // (0xD5B970) and `...GetNonUsedControllerCount` (0xD5BBA0) both loop
    // `cmp [rbp-0x10], 4`, and `...GetMaxControllerCount` (0xD5BCF0)
    // pushes the literal 4.
    //
    // The two loops call only predicates (GetLocalClientNum,
    // IsBeingUsed, and GPad_IsActive in the non-used one) and increment a
    // local, so widening them cannot write anywhere - the same argument
    // as the counter above. The third is a constant, and 2.0f -> 4.0f is
    // one byte because only 0x40000000 -> 0x40800000 differs.
    // TEMPORARILY OFF - bisect for the sign-in crash (2026-08-18 pm).
    //
    // The third sign-in aborts the process from the WINDOW thread with
    // a CRT invalid-parameter fastfail: WndProc 0x02334790 loads the
    // dvar pointer at data RVA 0x08E94DF8 and calls Dvar_GetInt
    // (0x022BE860) with it. That slot is NULL - and it is null in a
    // healthy ONE-player session too, measured. So the branch is
    // normally never reached, and something makes the window proc take
    // it once a third controller exists. These two counting loops are
    // exactly what makes a third controller visible to UI/input code,
    // and the file already records the same shape biting once:
    // raising GetMaxControllerCount invited code to touch controller 3
    // and killed the boot with __report_rangecheckfailure.
    //
    // If the three sign-ins survive with these OFF, they are the cause
    // and the fix is to scope them (or leave them off - they only feed
    // Lua counters). If it still dies, they are cleared and the next
    // suspect is the s_gamePads relocation + its six gamepad bounds.
    // {0x01FE428E, 0x02, 0x04, "GetUsedControllerCount: 2 -> 4"},
    // {0x01FE35F9, 0x02, 0x04, "GetNonUsedControllerCount: 2 -> 4"},

    // GetMaxControllerCount is DELIBERATELY LEFT AT 2.0f.
    //
    //   {0x01FE337D, 0x00, 0x80, "GetMaxControllerCount: 2.0f -> 4.0f"}
    //
    // Applied together with the two loops above and the client died at
    // startup twice. Windows Error Reporting named it exactly:
    //
    //   BlackOps3.exe + 0x02C413A4   0xC0000409 subcode 8
    //   02C41374  mov ecx, 8 / call ... / int 0x29
    //
    // which is `__report_rangecheckfailure` - the /GS bounds check MSVC
    // emits for a fixed-size array index. So something took the raised
    // maximum and indexed a genuinely [2] array with it.
    //
    // The difference from the other two is a real one, not a hunch. The
    // counting loops only ever report what is already TRUE and measured -
    // IsControllerBeingUsed(0/1/2) are all true, so "3 used" is a fact.
    // A raised MAXIMUM is an invitation: Lua then iterates 0..3 and
    // touches controller 3, which has no seat at all (status 72:
    // controllerIndex -1), and this file already records controller 3
    // killing a boot for exactly that reason.
    //
    // Revisit only after the seat table is four slots deep.
    //
    // REVISITED 2026-09-27 - set to THREE, the controllers that have seats.
    // The stock ui/uieditor/datasources.lua (PC and PS4 alike) runs at every
    // UI init
    //     for c = 0, Engine.GetMaxControllerCount() - 1 do
    //         ... Engine.CreateModel(GetModelForController(c), "scriptNotify")
    //         (+ numArgs, hudItems.*, WorldSpaceIndicators, playerConnected
    //         ...)
    // so with 2 controller 2 never had these models. Measured: at the first
    // frontend after boot scriptNotify existed for c0/c1 only (a temporary
    // UI_Model_CreateModelFromPath trace named Engine.CreateModel 0x01FAF920
    // from Lua as the only boot-time creator), and the in-game HUD - built
    // at UI init, before any CG_Init - subscribes with GetModel (hud.lua
    // line 873), so player 3's HUD never received a script notify:
    // force_scoreboard arrived for c2 and forceScoreboard stayed 0.
    // PS4 returns 4 because it seats 4; this mod seats 3 (4 with player 4).
    // 3.0f = 0x40400000: third immediate byte 0x00 -> 0x40.
    {0x1FD6BFD, 0x00, 0x80,
     "GetMaxControllerCount: 2.0f -> 4.0f (controllers with seats; 4 since "
     "player 4)"},

    // GetMaxLocalControllers, same shape (0x01FE3390: `mov dword [rax-8],
    // 0x40000000` at 0x01FE3398), same value, same reason (2026-09-27):
    // the console's own join path. Stock CoDMenu.lua turns an unused
    // controller's button into "unused_gamepad_button" -> Lobby.lua ->
    // LobbyAddLocalClient(c), which checks lobby_maxLocalPlayers - set by
    // Lobby_SetMaxLocalPlayers(4 offline) CAPPED at this value (PS4 4, PC 2);
    // CoDMenu also subscribes the button models of controllers
    // 0..GetMaxLocalControllers()-1 (B = leave for non-primary players).
    // Lua users: actions.lua Lobby_SetMaxLocalPlayers (min) and CoDMenu
    // ButtonBits subscriptions (nil-tolerant - measured: hud.lua's GetModel
    // subscription for a missing model raised no error). 3.0f = seats.
    {0x1FD6C1D, 0x00, 0x80,
     "GetMaxLocalControllers: 2.0f -> 4.0f (controllers with seats; 4 since "
     "player 4)"},

    // Engine.GetPlayerStats - the call the gobblegum row is built from.
    //
    // Measured with three players in the ZM lobby, the row drawn for
    // player 1 and player 2 and EMPTY for player 3:
    //
    //   Engine.GetPlayerStats(0,0,0) -> userdata
    //   Engine.GetPlayerStats(1,0,0) -> userdata
    //   Engine.GetPlayerStats(2,0,0) -> nil
    //
    // Native RVA 0x01FCAD00 (find_luibinding.py), and its FIRST gate is a
    // controller bound:
    //
    //   01FCAE5B  cmp r14d, 1     41 83 FE 01, immediate at 0x01FCAE5E
    //   01FCAE5F  ja  -> nil
    //   01FCAE65  cmp esi, 3      the LOCATION argument, already 0..3
    //   01FCAE73  call 0x01EBE720(ci, loc) / test al,al / je -> nil
    //
    // PS4 `Lua_CoD_LuaCall_GetPlayerStats` (0xD36D20) bounds the same
    // argument at `cmp [rbp-0x1c], 4` (0xD36F13) with an assert - four
    // controllers, not two. So 1 -> 3 is the console's own value.
    //
    // The second gate is 0x01EBE720 -> the ten-record stats walk
    // 0x01EA9A30, which status 63 measures as PASSING for controller 2.
    // LOG.md records this byte as "necessary, not sufficient" from an
    // earlier session - it was insufficient then because that walk still
    // failed. It no longer does.
    {0x1FBE6DE, 0x01, 0x03, "Engine.GetPlayerStats: controller bound 1 -> 3"},

    // LobbyHost_AddLocalClients. RE-APPLIED once its array was relocated.
    //
    // The whole of this table is gated on `ok == size(reloc_tables)`,
    // and reloc_tables now includes `netchan` - so if that relocation
    // fails for any reason this byte is not written and the crash below
    // cannot come back.
    {0x1ECACEB, 0x02, 0x04, "LobbyHost_AddLocalClients: controllers 2 -> 4"},

    // THE ACTIVATION LOOP BOUND - the missing half of the launch.
    //
    // PS4 CL_SetupClientsForIngame -> CL_LocalClients_SetAllUsedActive
    // (0x1517020): `for i in 0..3: SetActive(i, IsBeingUsed(i))`, at
    // EVERY launch, right before CL_AllocatePerLocalClientMemory. The
    // PC twin is the inlined loop at 0x0283AB30: change-detect per
    // client (cmp cl,al / je next), or/and of the active bit in
    // clientUIActives[i], and the Dvar_SetInt tail the count cave sits
    // in. Its bound is TWO, so client 2's used-but-inactive mismatch is
    // never even examined - measured in the first running three-player
    // round as [3/2/3] with the cave at 0 executions.
    //
    // Widened to THREE, not four: clientUIActives slot 2 is real,
    // zeroed memory once voice_comm has moved (this whole table is
    // gated on every relocation succeeding, voice_comm included), and
    // slot 3 is still foreign. Nothing is faked - the loop then runs
    // the engine's own rule (active = IsBeingUsed) over the clients
    // that really exist.
    {0x27C1A45, 0x02, 0x03, "SetAllUsedActive loop: local clients 2 -> 3"},

    // THE CONNECT LOOP - measured, not guessed (2026-08-18).
    //
    // PS4 CL_MapLoading (0x40CB40) is `for (lc = 0; lc < 4; lc++) { if
    // (!IsActive(lc)) continue; CL_Disconnect(lc,0);
    // SetActive(lc,1); ... SetLocalClientConnectionState(lc, 5|6);
    // SetCUIFlag(lc, 4); }` - the step that turns an ACTIVE local
    // client into a CONNECTED one. The PC twin is the loop ending at
    // 0x01359DB9, identified by tracing CL_LocalClient_IsActive and
    // reading the caller: local client 1 flipped to connected (flags
    // 0x03 -> 0x07) in the same instant the last IsActive caller was
    // 0x01359D37, inside this loop. Its body is the same shape:
    //   01359D43  call CL_Disconnect
    //   01359D4C  call CL_LocalClient_SetActive(lc, true)
    //   01359D61/6C  connectionState = 5 or 6
    //   01359D7C  or [flags], ebp        <- the CONNECTED bit
    //
    // It walks clientUIActives BY BYTE OFFSET, so its bound is a size,
    // not a count: `cmp rsi, 0x20F0` = 2 * 0x1078. That is why local
    // client 2 was activated and then never connected - measured three
    // runs in a row as flags 0x01 with connectionState 0, while 0 and 1
    // reached 0x37 / state 0xB.
    //
    // 0x20F0 -> 0x3168 (3 * 0x1078), as two length-preserving byte
    // writes into the imm32. The container is real: voice_comm has
    // moved out of slot 2 and this whole table is gated on that.
    // pe_find says the 7-byte form `48 81 FE F0 20 00 00` occurs
    // EXACTLY ONCE in the image, so this is the only such bound.
    // PLAYER 4 (2026-09-27): 4. Index 3 touches clientUIActives[3] +0/+8
    // (the owned head, zeroed), seat record 3 and [4] arrays only; the body
    // skips a client that is not in use, as it did for index 2 in 2-player
    // games.
    {0x1359DDC, 0xF0, 0xE0, "connect loop end: 2*0x1078 -> 4*0x1078 (low)"},
    {0x1359DDD, 0x20, 0x41, "connect loop end: 2*0x1078 -> 4*0x1078 (high)"},

    // ...and THE LOOP COUNTER ITSELF, which is a SECOND bound in the
    // same loop and the one that actually ends it:
    //   01359DC7  inc ebx
    //   01359DCC  add rsi, 0x1078      (the clientUIActives cursor)
    //   01359DD3  add r15, 0x25780     (the per-client connection cursor)
    //   01359DDA  cmp ebx, 2           <- THE BACK EDGE
    //   01359DDD  jl 0x01359D30
    // Patching only the rsi guard above left this at 2 and the run was
    // unchanged - measured: both patched bytes verified live in memory
    // and local client 2 still ended at flags 0x01. Two bounds, one
    // loop; both have to move or nothing does.
    {0x1359DFC, 0x02, 0x04, "connect loop counter: local clients 2 -> 4"},

    // The per-client reset loop in the same function (clears flag bit 6
    // and resets keyCatchers at 0x01359BC7/0x01359BCC). It must cover
    // the same clients the connect loop now does, or client 2 keeps a
    // stale keyCatcher state into the round.
    {0x1359BF8, 0x02, 0x04, "map-load reset loop: local clients 2 -> 4"},

    // THE NEXT WALL AFTER THE CBUF RELOCATION, and the first crash of
    // the day that was NOT the memcpy: 0xC0000409
    // (__report_rangecheckfailure, rcx = 8) at 0x02C413A4, reached from
    // 0x012F3511. Registers named it outright - rbx = 2 (the index),
    // r14 = 0x20F0 = 2 * 0x1078 (the old clientUIActives extent), and
    // clientUIActives' base 0x053D8BC0 sitting on the stack.
    //
    //   012F34FA  cmp  rbx, 2            <- imm at 0x012F34FD
    //   012F34FE  jae  0x012F3511        -> __report_rangecheckfailure
    //   012F3500  lea  rax, [0x052F29C4]
    //   012F3507  mov  byte [rbx + rax], 0
    //
    // A per-local-client BYTE array of two elements at 0x052F29C4, and
    // the /GS guard kills the process when local client 2 writes to it.
    //
    // Safe to widen without relocating, and that was CHECKED, not
    // assumed: pe_xref on 0x052F29C6 and 0x052F29C7 finds ZERO code
    // references to either byte (one raw hit each, both rejected as
    // data). They are padding, not a foreign global - unlike the Cbuf
    // records, whose slot 2 was a live pointer and therefore had to be
    // relocated instead.
    // 3 -> 4 for player 4 (2026-09-27): the 4-player lobby died with the same
    // fail-fast (0xC0000409/8, WER dump boiii.exe.25516.dmp) in CL_ClearKeys(3)
    // called from Lua. find_lea 0x052F29C4..C8: one lea to the base, nothing
    // else - slot 3 (0x052F29C7) is padding too.
    {0x12F351D, 0x02, 0x04,
     "per-client byte array 0x052F29C4 range check: 2 -> 4"},

    // THE FIRST REAL-GAMEPLAY TEST (2026-09-27 21:31, tools/play_round.py):
    // player 3 aimed and fired (LT+RT) and the game fail-fasted (0xC0000409,
    // __report_rangecheckfailure, rbx = rbp = 2; WER boiii.exe.25192.dmp)
    // from 0x0131B386 <- button dispatch 0x012F70CB <- gamepad input
    // 0x022F3EF0. Every earlier round had idle guests - the fire RELEASE
    // was never reached for lc >= 2. A scan of the input code for
    // `cmp r, 2 ; jae -> call 0x02C41374` found exactly four such checks:
    // CL_ClearKeys (above), CL_Init (scoped elsewhere) and these two.
    //
    // 0x0131B260 = PC IN_Attack_Up: PS4 IN_Attack_Up (0x3E0D00) does
    //   gAttackEdgeDetected[lc] = 0; IN_KeyUp(&playersKb[lc] + 0x198);
    //   IN_KeyUp(&playersKb[lc] + 0x450)
    // and the PC is the same shape (byte at 0x052F29C4[lc], two kbuttons
    // 0x2B8 apart in the 0x498 playersKb record). playersKb is already
    // [4] (players_kb_refs; the dump's r12 = new block + 0x198); the byte
    // array's slots 2/3 are the padding checked above.
    {0x131B28A, 0x02, 0x04, "IN_Attack_Up range check: local clients 2 -> 4"},
    // 0x0131C050: per-frame analog-trigger edge on playersKb[lc] +0x2E4;
    // on release it clears byte 0x052F3360[lc] behind `cmp rbx,2`.
    // find_lea 0x052F3360..68 (2026-09-27): no reference to 0x052F3362/63
    // (the next global is a 16-byte vector at 0x052F3364), image bytes 0 -
    // padding. Same caller as IN_Attack_Up.
    {0x131C0CA, 0x02, 0x04,
     "trigger-edge byte array 0x052F3360 range check: 2 -> 4"},

    // Same family, found 2026-09-14 the hard way: a fail-fast
    // (0xC0000409, subcode 8) 15 s after cl_max=3 with NO dialog and no
    // BOIII dump, because __report_rangecheckfailure bypasses SEH. The
    // WER dump's raw stack (rsp+0x58) named the caller: the CG frame
    // function 0x00A129A9, `imul rbx, r15, 0x1E940` (cgs stride, r15 =
    // lc), guarding a per-client BYTE array at 0x04D1DC94:
    //
    //   00A15B57  cmp  byte [r15 + r13 + 0x04D1DC94], r12b
    //   00A15B65  cmp  r15, 2               <- imm at 0x00A15B68
    //   00A15B69  jae  0x00A16476           -> __report_rangecheckfailure
    //   00A15B72  mov  byte [r15 + r13 + 0x04D1DC94], r12b
    //
    // Rule 4 checked: the array has exactly three references in the
    // image (that pair and an unguarded setter at 0x00A07A03) and bytes
    // 0x04D1DC96/97 - slots 2/3 - have none at all: padding, so it widens
    // in place like the two above, no relocation.
    // 4 for player 4: find_lea 0x04D1DC94..98 and range_xref on 0x04D1DC96..98
    // (2026-09-27) - slot 3 unreferenced, as the note above says.
    {0xA15B68, 0x02, 0x04,
     "CG frame per-client byte array 0x04D1DC94 range check: 2 -> 4"},

    // THE NETCHAN POLL - why local client 2 parks at CA_CONFIRMLOADING.
    //
    // PS4 Com_ClientPacketEvent (0xE491A0) is the client packet
    // router, and it does NOT match packets by address - it walks the
    // local clients and polls each one's OWN netchan:
    //
    //   00E491DB  cmp [rbp-0x2637C], 4        <- ALL FOUR
    //   00E491F1  Com_LocalClient_IsBeingUsed(i)
    //   00E4921B  Com_LocalClient_GetControllerIndex(i)
    //   00E49244  add rcx, 0x24B70            clc + netchan offset
    //   00E4924D  Netchan_GetMessage(controllerIndex, ...)
    //
    // The PC twin is 0x020F7AC7.., identified by the same call pair
    // (IsBeingUsed 0x020EF990 then GetControllerIndex 0x020EF930 within
    // 80 bytes - only 9 sites in the image) and confirmed line for
    // line, down to the identical 0x26390 stack frame:
    //
    //   020F7AD2  call IsBeingUsed(ebx)
    //   020F7ADF  mov  rsi, [0x053D8BB8]      client connections
    //   020F7AE8  call GetControllerIndex(ebx)
    //   020F7AED  lea  r9, [rsi + 0x24B68]    (PS4 +0x24B70)
    //   020F7B03  add  r9, rdi                + i * 0x25780
    //   020F7B12  call 0x02174970             Netchan_GetMessage
    //   020F7B99  inc ebx / add rdi, 0x25780
    //   020F7BA2  cmp ebx, 2                  <- THE BOUND
    //
    // Bounded at TWO, so local client 2's netchan is NEVER READ. His
    // challenge/connect replies sit in his queue forever, which is
    // exactly the measured symptom: 0 and 1 reach CA_ACTIVE (0xB) while
    // 2 stays at CA_CONFIRMLOADING (6) with no crash and no error.
    //
    // This also retires the "packets are routed by ADDRESS and all
    // local clients share one" theory from earlier today - the router
    // never looks at the address at all. It is a plain count bound.
    //
    // Safe at three: the block it indexes is the same clientConnection
    // array (stride 0x25780) the connect loop already walks three deep
    // after its own two bounds were widened, and the allocator carves
    // it from cl_maxLocalClients = 3.
    // NOT APPLIED STATICALLY - see apply_netchan_poll_widen(). Doing it
    // from this table crashed the LOBBY right after the third sign-in
    // (18:53 build): in the frontend cl_maxLocalClients is 2, so the
    // clientConnection array is only carved for two, and polling index
    // 2 reads past its end. Exactly the "widen a bound whose body
    // indexes a [2] array" trap CLAUDE.md names. The bound has to
    // follow cl_maxLocalClients, so it is written at MAP LOAD, once the
    // allocator has really carved three.
    // {0x020F7BA4, 0x02, 0x03, "netchan poll"},   applied dynamically

    // WHY IT NEEDED THE RELOCATION FIRST - applied once WITHOUT it and
    // it crashed:
    //
    // Measured 2026-08-16, ZM offline lobby: SigninLocalClient(2,true)
    // still returned 0 and [3/1/3], and then
    //
    //   Fatal error 0xC0000005 at 0x00007FF753813AD0 (base 0x7FF7516A0000)
    //   -> RVA 0x02173AD0
    //
    //   02173AB2  lea  rcx, [0x16E69F40]
    //   02173AB9  imul rsi, rsi, 0x128        ; rsi = arg0, sign-extended
    //   02173AC0  add  rsi, rcx
    //   02173AC3  mov  rbx, [rsi]             ; reads element arg0
    //   02173AD0  cmp  rax, qword ptr [rbx]   ; <- FAULT, rbx is garbage
    //
    // So the lobby-add path walks a per-index array at **0x16E69E20,
    // stride 0x128** (0x16E69F40 is that base + 0x120, one field inside
    // element 0; 0x16E69E20 and 0x16E69EB0 are the other two fields, all
    // three indexed by the same imul). Index 2 lands past its end.
    //
    // That is exactly the family CLAUDE.md warns about: widening a bound
    // whose body indexes a [2] array just moves the crash. So the array
    // was relocated first, the same way s_storage / client_ui /
    // s_targets were: `netchan` in splitscreen_reloc.hpp, base
    // 0x16E69E20, 0x250 -> 0x4A0, fifteen references from find_lea.py
    // over the whole span (thirteen lea plus two GENUINE base-relative
    // ABS32 forms, both verified by disassembling from the enclosing
    // function start).

    // THE PER-CLIENT FRAME PUMP - why the 14:10 run parked (2026-08-18).
    //
    // With the connect loop widened, local client 2 reached flags 0x05 /
    // connectionState 6 (CA_CONFIRMLOADING, DWARF enum connstate_t) and
    // then sat there for 10+ minutes while 0 and 1 climbed to CA_ACTIVE
    // (0xB). What advances a connection is CL_CheckForResend, and on the
    // PS4 it has exactly TWO callers (elf_calls): CL_MapLoading (once,
    // at load) and CL_Frame - and Com_Frame runs `for (lc = 0; lc < 4;
    // lc++) CL_Frame(lc, msec)` (0xE4D38D: cmp 4) unconditionally,
    // every frame; CL_Frame gates itself per client. The PC twin
    // (CL_Frame = 0x013513E0, identified by its clientUIActives
    // indexing, IsBeingUsed/IsActive calls and CA_ACTIVE check):
    //
    //   020F95D0  mov edx, r14d ; mov ecx, ebx
    //   020F95D5  call 0x013513E0        CL_Frame(lc, msec)
    //   020F95DA  inc ebx ; cmp ebx, 2 ; jl
    //
    // bounded at TWO - client 2's handshake is never pumped.
    //
    // THIS BYTE IS DELIBERATELY *NOT* IN THIS TABLE. It is written by
    // run_cl_init_for_local_client2() instead, and only AFTER CL_Init
    // has actually run for client 2. The ordering is the whole point:
    // on the PS4 CL_Init(i) runs for all four clients in Com_Init,
    // long before Com_Frame ever calls CL_Frame. Applying the widen
    // statically inverts that - CL_Frame(2) starts being called from
    // the very next frame, in the lobby, for a client that has never
    // been initialised, and its first 60 bytes run BEFORE the bit-1
    // gate protects anything:
    //
    //   0135141A  mov ebp, [lc*0x1078 + 0x53D8BC8]   connectionState
    //   0135141F  call 0x020F00C0                    (predicate)
    //   0135142A  call 0x020EF990                    IsBeingUsed(lc)
    //   01351435  xchg [0x0569B234], eax             pending-error slot
    //   01351441  call 0x0228DE40(lc)                error popup for lc
    //   01351446  ...bit 1 gate finally here
    //
    // IsBeingUsed(2) is TRUE from the moment player 3 signs in, so
    // that pre-gate stretch really does execute for an uninitialised
    // client. A three-player lobby died silently at exactly that point
    // on the 16:19 build (no dump, no event-log entry, status 96 = 0
    // proving CL_Init itself never ran, so the widen is the only new
    // code that had executed). Unproven as the cause - but the correct
    // order makes the question moot, so it is not worth a run to
    // settle. See run_cl_init_for_local_client2().

    // CL_Init's OWN range check. The PC keeps a standalone CL_Init at
    // 0x013593EF (zero direct callers - the boot path inlines its
    // copies for clients 0/1; found via its Cbuf_Execute call). Its
    // body indexes the [2] byte array cl_waitingOnServerToLoadMap
    // (0x053D4988) and MSVC guards the index: `cmp rbx, 2 / jae
    // __report_rangecheckfailure` at 0x01359468. Calling it for local
    // client 2 without this widen dies as 0xC0000409 subcode 8 - the
    // same family as GetMaxControllerCount above. Element 2 is the
    // padding byte 0x053D498A: zero code references, and the widened
    // connect loop has ALREADY been zeroing it every map load since
    // the 13:30 build, measured harmless in a 10-minute round.
    // NOT HERE ANY MORE - see run_cl_init_for_local_client2(), which
    // widens this and the Cbuf_Execute check only for the duration of
    // its own single CL_Init(2) call and puts them back immediately.
    //
    // Why: the 13:30 build signed three players in and played a round.
    // The lobby only began dying at the SECOND guest sign-in after
    // these two bytes were added, and they were the only new bytes.
    // Leaving a /GS range check widened permanently means the STOCK
    // engine runs with a bound that no longer matches the array behind
    // it - every engine call through here, all session, for a widening
    // we need for exactly one call of our own. Scoped is both safer and
    // sufficient.
    // {0x0135946B, 0x02, 0x03, "CL_Init range check"},   scoped instead

    // Cbuf_Execute's range check, the same shape (0x020EC1AD). CL_Init
    // calls Cbuf_Execute(lc, ci), so this must move with it. For local
    // client 2 the call is verified inert end to end: the pre-check
    // busy byte [2] is padding (0x1689DF2A, zero refs), the inner
    // per-client gate (0x020EC320) reads a BSS dword nothing ever
    // writes for lc2 (0x1689DF64, zero refs) and takes its clean
    // cookie-checked exit, and the tail (0x020EC8E0) processes a
    // global queue with no per-client indexing at entry.
    // {0x020EC1B0, 0x02, 0x03, "Cbuf_Execute range check"},  scoped too

    // THE LOBBY'S OWN VIEW OF HOW MANY LOCAL CLIENTS IT HAS.
    //
    // Measured 2026-08-19 in a healthy three-player ZM lobby, which is
    // what makes this one different from a guess:
    //
    //   GetLobbyClientCount(0/1)      -> 3   he IS in the roster
    //   LobbyGetSessionClients(0/1)   -> 3 entries
    //   GetLobbyLocalClientCount(0/1) -> 2   <- but only TWO local
    //
    // So local client 2 is listed in the lobby (that is why the roster
    // shows "player 3") while the lobby never owned him as a LOCAL
    // player - which is exactly the user-visible symptom: his row has
    // no loadout data and DEACTIVATE SPLITSCREEN will not release him,
    // while player 2 behaves normally.
    //
    // The Lua binding GetLobbyLocalClientCount (0x01F14C40) is a
    // wrapper; the count itself is 0x01EFF910, reached through
    // 0x01ECB030(lobbyIndex, 1) and the jump table at 0x01F000F0:
    //
    //   01EFF926  mov ecx, ebx
    //   01EFF928  call 0x01EC7310(i)     -> client object or null
    //   01EFF974  inc esi                 count++
    //   01EFF976  inc ebx
    //   01EFF978  cmp ebx, 2              <- imm at 0x01EFF97A
    //
    // CLASSIFIED BEFORE WIDENING, and it passes: the loop body's ONLY
    // use of the index is 0x01EC7310(i), which reaches 0x01EC7030 and
    // 0x01EC6E80 - both `lea rcx,[0x0340F180] / [rcx + i*8]`. That is
    // the client_objs table, already relocated [2] -> [4] by this
    // component, and BOTH instructions (0x01EC7033, 0x01EC6E83) are in
    // its 19-reference set - checked in splitscreen_reloc.hpp, not
    // assumed. Slot 2 is therefore owned memory, and the
    // `cmp dword [obj+0x30], 0 / setg` guard makes an unpopulated slot
    // return false instead of counting garbage.
    //
    // Verified live: 0x01EFF97A 02 -> 04 made the count return 3 with
    // the game healthy. Without the client_objs relocation this same
    // byte would read past a two-slot array.
    {0x1EF31FA, 0x02, 0x04, "GetLobbyLocalClientCount loop: 2 -> 4"},

    // DEACTIVATE SPLITSCREEN, bounded the same way.
    //
    // LobbyRemoveAllLocalSplitscreenClient (0x01F16D60) is the button,
    // and it never even considers local client 2:
    //
    //   01F16E00  inc ebx
    //   01F16E02  cmp ebx, 2              <- imm at 0x01F16E04
    //   01F16E05  jl  0x01F16D70
    //
    // Same classification and the same answer: the body reaches the
    // index through 0x01EC7310 / 0x01EC6E80 (client_objs, relocated and
    // verified above) and 0x020EF7C0, whose scan was read LIVE as
    // 0x1A8A7508..0x1A8A7574 stepping 0x24 - exactly three records, so
    // it already reaches index 2.
    //
    // EFFECT NOT YET CONFIRMED. Widening it live left a NON-CONTIGUOUS
    // seat table (0 in use, 1 removed, 2 still in use) and the lobby
    // then refused to activate splitscreen at all. It is not
    // established that the button was pressed after the poke, so this
    // is not "the widen failed" - but the contiguity requirement is
    // real and matters: the engine never produces a 0+2-with-a-gap
    // layout itself. If a removal pass must leave a gap, prefer not
    // running it at all over producing that state.
    {0x1F0A684, 0x02, 0x04,
     "LobbyRemoveAllLocalSplitscreenClient loop: 2 -> 4"},

    // LiveUser_IsUserGuest (0x01EC70C0) rejects every controller index >= 2
    // BEFORE it reads the isGuest byte:
    //
    //   01EC70D0  cmp edi, 1
    //   01EC70D3  ja  -> xor al,al ; ret        <- ci 2 and 3 always "not a
    //   guest"
    //   01EC70EC  movzx eax, byte [userData[ci] + 0x29]   never reached
    //
    // That single bound is what keeps player 3 without gobblegums.
    // Storage_Pump's
    // steady-state per-target loop has a GUEST branch that lets a guest inherit
    // its target-1 loadout files from the primary:
    //
    //   02277376  call 0x01EC70C0(ci, &sponsor)     is this a guest?
    //   02277387  call 0x020EF7C0 -> 0x020EF990     is its seat active?
    //   02277393  if !guest  -> skip
    //   02277397  if !seat   -> skip
    //   0227739B  cmp esi, 1                        target 1 only
    //   022773A0  index s_storage by the SPONSOR, call 0x02276570
    //
    // and 0x02276570 marks those files ready inline (0x02276708). With the
    // bound at 1, controller 2 fails the first test every frame forever.
    //
    // Rule 4: the only per-controller array this function indexes is
    // s_UserDataForControllerMap (0x0340F180), which this component already
    // relocates to four entries - so the widen has somewhere real to point.
    //
    // Verified live 2026-08-19: with this widened, userData[2].isActive set and
    // clientGameStates[2] flags bit 0 set, controller 2 went from 18 ready file
    // slots to 52, done[] completed all four targets, and Engine.GetCACRoot(2)
    // returned userdata instead of nil.
    {0x1EBA642, 0x01, 0x03, "LiveUser_IsUserGuest bound: ci<=1 -> ci<=3"},

    // THE FRONTEND CLIENT COMMANDS (2026-09-26 18:00). After the first
    // three-player GAME OVER that did not crash, the game hung on the
    // black frontend loading screen: lc 1 had been deactivated (flags
    // 0x06, state 0) but lc 2 was still ACTIVE (flags 0x07) and stuck
    // connecting to the frontend map at state 5. The console commands
    // that drop the guests on the way back are loops over local clients:
    //
    //   PS4 CL_Command_DisableAllButPrimaryClients (0x40B550):
    //       for (lc = 0; lc < 4; lc++) if (!IsPrimary(lc)) {
    //           SetActive(lc, false); if (!provisional) SetBeingUsed(lc,
    //           false); }
    //   (registered by CL_RegisterCommands as "disableallbutprimaryclients",
    //    next to "disableallclients")
    //
    // PC CL_RegisterCommands 0x0135A519.. registers the same names; the
    // handlers stop at two:
    //   0x0134C300 disableallbutprimaryclients  cmp ebx,2 at 0x0134C330
    //   0x0134C340 disableallclients            cmp ebx,2 at 0x0134C374
    //   0x0134C390 (same shape, provisional flag 0x053D4982)
    //                                            cmp ebx,2 at 0x0134C3CD
    // Their bodies are SetActive (0x0283AAB0) and SetBeingUsed
    // (0x020EFDE0) - clientUIActives slot 2 and the relocated seat table,
    // the same two the widened SetAllUsedActive loop already writes for
    // lc 2. Three, like every other clientUIActives walker here.
    // PLAYER 4 (2026-09-27): 4. Index 3 touches clientUIActives[3] +0/+8
    // (the owned head, zeroed), seat record 3 and [4] arrays only; the body
    // skips a client that is not in use, as it did for index 2 in 2-player
    // games.
    {0x134C352, 0x02, 0x04,
     "disableallbutprimaryclients loop: local clients 2 -> 4"},
    {0x134C396, 0x02, 0x04, "disableallclients loop: local clients 2 -> 4"},
    {0x134C3EF, 0x02, 0x04,
     "provisional disable-all loop: local clients 2 -> 4"},

    // PLAYER 3 COULD NOT MOVE (2026-09-26 20:40): PC IN_GamepadsMove
    // (0x022F3EF0) polls the pads in `for (ci = 0; ci < 2; ci++)`
    // (`cmp edi,2` at 0x022F40F2) - four sticks and two triggers through
    // CL_GamepadEvent (0x0133FAB0), then the button list through
    // CL_GamepadButtonEventForPort (0x0133F9B0). PS4 IN_GamepadsMove
    // (0xDBA1F0) loops `ci < 4`. Controller 2 was never polled in game, so
    // player 3's sticks and buttons never reached a usercmd (the menus
    // use a different input path, which is why pad 2 worked there).
    // Rule 4: s_gamePads (0x17E6E310 x 0x70) is walked for four pads by
    // GPad_UpdateAll and already served pad 2 in the menus; the per-client
    // targets are gaGlobs and playerKeys (both relocated to [4]),
    // clientUIActives slot 2 (real) and the relocated seat table; a pad
    // with no local client maps to -1, which the stock loop already
    // meets for an unjoined pad 1. Three, like the other client loops.
    // FOUR for player 4 (2026-09-27). Correction to the note above: on PS4 this
    // IS the menu path too - IN_GamepadsMove -> CL_GamepadButtonEventForPort
    // (0x4017F0) -> CL_GamepadButtonEvent -> UI_CoD_KeyEvent (elf_calls);
    // controller 3's A never reached the Lua while this stopped at 3. Index 3:
    // gaGlobs, playerKeys, s_gamePads [4]; seat record 3; clientUIActives[3]
    // keyCatchers (+4, the slot's owned head).
    {0x2287024, 0x02, 0x04,
     "IN_GamepadsMove: poll controllers 2 -> 4 (players 3/4 sticks/buttons, "
     "menus too)"},

    // PLAYER 3 STUCK LOADING FROM THE SECOND ROUND ON (2026-09-27): the
    // netchan thread pumps only controllers 0 and 1. PS4 Netchan_Thread
    // (0xE7E650) runs `for c < 4` (0xE7E91D): IsSignedIn(c) ->
    // Netchan_Transmit, SendKeepAlives, FreeStaleMessages,
    // FreeStaleUserBandwidth, SendAcks. PC 0x02176E80.. has the same body
    // (0x02177020, 0x02175D50, 0x02174710, 0x02175A00) with `cmp ebx,2`
    // at 0x02176EE2. So controller 2 never had queued messages
    // transmitted/retransmitted, periodic acks or stale-message cleanup.
    // Measured: after a round its type-0 node survived with
    // +0x70 = 0x10001 (fragments-present bit, last fragment index 1),
    // where controller 1's lists were freed - and PS4
    // Netchan_ProcessFragment drops as duplicate every fragment whose
    // index is <= that. The next round's message 1 from the host was
    // swallowed, the client never learned serverXuid (clc+0x24B68 is set
    // only by receiving; the packet sender 0x013407E0 skips while it is
    // 0), never sent, and the server never sent a gamestate: server
    // client 2 CONNECTED, stats 1, ack 0 until the timeout (3 of 4
    // second rounds).
    // Rule 4, every global the four callees index, read from the LIVE
    // code: s_netchan rows (relocated to four, 0x1FBE0000), per-TYPE
    // tables (keepalive bits 0x16E49B2C, stale timeouts 0x1718CDB0),
    // global pools/free lists (0x16E69C08, 0x1718CDF8), critical
    // sections, a loopback table indexed by address (0x16E69C10), and
    // clientGameStates via 0x020EF950 (relocated, three slots). Three,
    // not four: clientGameStates has three.
    // PLAYER 4: 4 - s_netchan rows [4], clientGameStates 4 records (a6f3fd8).
    {0x211E444, 0x02, 0x04,
     "Netchan_Thread pump: controllers 2 -> 4 (transmit/acks/stale cleanup)"},

    // ...and the one that actually runs on the way back (measured
    // 2026-09-26 18:20 with firstsnap_watch: lc 1 went 0x07 -> 0x06
    // while lc 2 stayed 0x07 and then sat at state 5 on the frontend).
    // 0x0135DCD0 is the client setup for the two sides of a level load:
    //   frontend (0x02148350 true): for lc < 2 { SetBeingUsed(lc, lc==0 ||
    //       signed in); SetActive(lc, lc == 0); lc 0 stays primary }
    //                                   bound cmp ebx,2 at 0x0135DD35
    //   in game: for lc < 2 { SetBeingUsed / SetActive(lc, lc < count) /
    //       0x020EFF00 }                bound cmp edi,2 at 0x0135DE21 -
    //       already widened to 4 above as the "boot bind loop" row.
    // PS4 does both over four clients (Com_LocalClients_AssignUIContexts-
    // ForFrontEnd 0xE35BD0 and CL_SetupClientsForIngame 0x40B870 ->
    // CL_LocalClients_SetAllUsedActive, all `cmp 4`). Same two writes
    // (clientUIActives slot 2, relocated seat table) as the rows above;
    // three like them, since the frontend branch walks clientUIActives.
    // PLAYER 4: 4 - body GetControllerIndex / IsSignedIn (userData [4]) /
    // SetBeingUsed (seat record 3) / SetActive (flags +0, owned), disassembled.
    {0x135DD57, 0x02, 0x04, "frontend client setup loop: local clients 2 -> 4"},
};

constexpr byte_patch storage_patches[] = {
    {0x2218DF5, 0x51, 0x91, "storage pool: SIB scale x2 -> x4"},
    {0x2218F53, 0x02, 0x04, "AllocateMemory: controllers 2 -> 4"},
    {0x2219A0E, 0x02, 0x04, "clear-all loop: controllers 2 -> 4"},
    {0x2219AF3, 0x02, 0x04, "file lookup A: controllers 2 -> 4"},
    {0x2219B95, 0x02, 0x04, "file lookup B: controllers 2 -> 4"},
    {0x2219D1A, 0x02, 0x04, "file lookup C: controllers 2 -> 4"},
    {0x2219FA4, 0x02, 0x04, "file lookup D: controllers 2 -> 4"},
    {0x221A1C3, 0x02, 0x04, "file lookup E: controllers 2 -> 4"},
    {0x221A30B, 0x02, 0x04, "controller gate 0x02276E30: 2 -> 4"},
    {0x221AB16, 0x02, 0x04, "controller gate 0x02277640: 2 -> 4"},

    // Found after the nine above, and they must be applied HERE rather
    // than from a script at lobby time. Measured 2026-08-07: controller 1
    // has storage and local-file stats readiness at the FIRST MENU,
    // before it signs in, because the boot pass covers it. Applying these
    // later means the guests miss that pass entirely and can never
    // acquire the readiness flags that gate the loadout.
    //
    // 0x0135C8AD is the per-controller update loop that drives
    // Storage_Pump: `call 0x01E26570 / inc ebx / cmp ebx, 2 / jl`. The
    // mechanical scan missed it because it is not in the storage TU.
    // 4 -> 3, measured 2026-08-18 20:29.
    //
    // This loop drives Storage_Pump per controller, and at 4 it makes
    // the engine do per-controller work for controller 3, which HAS NO
    // SEAT (the seat table is three deep; controllerIndex 3 = -1). The
    // intermittent third-sign-in crash is a command-buffer COMPACTION
    // with a junk length - dest 0x0573B190, src 0x0573B5A9,
    // src - dest = rbx = 0x419, rsi in the command-buffer globals -
    // i.e. a per-client command buffer whose "bytes used" field was
    // never initialised. Our own CL_Init was proven NOT to be running
    // at the time (status 87 code 22), so the engine is reaching that
    // buffer on its own, and this widen is what invites it in.
    //
    // Three is the honest number: three seats exist, so three
    // controllers get pumped. This file already records the identical
    // lesson once - GetMaxControllerCount was deliberately left at 2.0f
    // because raising the MAXIMUM invited code to touch controller 3
    // and killed the boot with __report_rangecheckfailure. Same shape,
    // same rule: never widen a controller bound past the number of
    // seats that actually exist.
    //
    // 2026-09-27, PLAYER 4: now FOUR, because both causes are gone -
    // controller 3 has a seat (record 3, signin_new_slots 4) and the Cbuf
    // records are [4] (reloc_tables "cbuf", slot 3 zero-filled, whose
    // `cmp [rsi+0xC],0 / je` takes the clean exit). Measured before:
    // s_storage[3] had buffers but no XUID and flags 00 00 00 00 - it
    // was never pumped; controller 2's record (pumped) is complete.
    {0x135C8CD, 0x02, 0x04,
     "per-controller update loop (Storage_Pump): 2 -> 4"},

    // Storage_Read refuses every controller >= 2, so no per-controller
    // file is ever read for a guest: `movsxd rdi, ecx / cmp edi, 2 / jge
    // return false`. Also missed by the scan.
    {0x221AA7F, 0x02, 0x04, "Storage_Read controller bound: 2 -> 4"},

    // Two more of the identical shape, found with tools/bound_scan.py.
    {0x221AC63, 0x02, 0x04, "storage fn 0x02277780 controller bound: 2 -> 4"},
    {0x221AE3F, 0x02, 0x04, "storage fn 0x02277960 controller bound: 2 -> 4"},

    // THE TASK REAPER LOOP - why controller 2 never got a loadout.
    //
    //   020F91D0  mov  ecx, ebx
    //   020F91D2  call 0x022B0770      TaskManager2_ProcessTasks(ebx)
    //   020F91D7  inc  ebx
    //   020F91D9  cmp  ebx, 2          <- the bound
    //   020F91DC  jl   0x020F91D0
    //
    // The game reaps finished tasks per controller, and only ever for 0
    // and 1. Measured from inside the component (status 37..40): StartOp
    // runs exactly ONCE each for controllers 0, 1 and 2, so controller 2
    // starts its gamer-profile read like everyone else - and then
    // s_localFileOpData[2].opStatus stays at DONE instead of returning to
    // IDLE, because nothing ever reaps its task. Controllers 0 and 1 end
    // at IDLE.
    //
    // That single unreaped task then blocks EVERYTHING, because the 'hdd'
    // busy query (0x02274D30) is TaskIsInProgress on ONE GLOBAL task and
    // ignores the controller: Storage_Pump refuses to assign an xuid to
    // any controller waiting in shutdown, so s_storage[3] stays empty,
    // and controller 2's storage reads never complete, so ready[2] on the
    // per-controller stats records is never set and the lobby draws no
    // gobblegums for it.
    //
    // Forcing the reap with a detour was tried three ways and always took
    // the renderer down - it runs completion handlers from frames the game
    // did not schedule them in. Widening this bound instead lets the GAME
    // reap them, in its own frames, which is the whole difference.
    // NOT APPLIED. Widening it works exactly as intended and still kills
    // the renderer:
    //
    //   all four s_storage slots assigned, 01 00 01 01 each
    //   StartOp 1/1/1/1 - controller 3's read starts for the first time
    //   ready odd rows 01 01 01 00 - controller 2 ready
    //   ...and the client renders pure black
    //
    // That is the FOURTH route to completing these tasks, after forcing
    // ProcessTasks from the Storage_Pump detour, from the per-controller
    // update, and deferring it past boot. All four set the flags. All
    // four take the renderer down. So it is not about WHO reaps or WHEN -
    // completing the guests' gamer-profile read is itself what breaks
    // rendering, and the game doing it natively is no different.
    //
    // The next thing to learn is what inside the completion handler does
    // it. The handler is FileProperties.readCallback at properties->[0x28],
    // invoked from 0x0227712C; for the stats records it reaches
    // 0x01EA9EC0. Hook that and find which of its callees the frontend
    // cannot survive, rather than trying a fifth way to trigger it.
    //
    // RE-ENABLED, together with the stats-only read filter.
    //
    // On its own this blacked out the client, because reaping completed
    // the guests' SETTINGS read and that runs Com_RunAutoExec /
    // Settings_RunCallbacks with localClient = -1. With the filter in
    // place the guests never start a settings read at all (measured: 5
    // stats reads allowed, 14 others blocked), so the only completions
    // left to reap are the stats ones - which do nothing but set
    // ready[ci].
    {0x20ECA5B, 0x02, 0x04,
     "TaskManager2_ProcessTasks per-controller loop: 2 -> 4"},
};

// s_storageMem.pool. Zero until AllocateMemory has run, so reading it
// tells us whether we got in first. Every previous attempt relocated
// s_storage from an external script LONG after Storage_Init, which is
// why records 2 and 3 were empty - there was nothing left to allocate.
constexpr size_t storage_pool_rva = 0x1789FD78;

// --- launch handshake -------------------------------------------------
constexpr size_t lobby_pool_rva = 0x155FD410;
constexpr size_t lobby_pool_stride = 0x66828;
constexpr size_t game_lobby_index = 1;
constexpr size_t session_clients_offset = 0xF8;
constexpr size_t session_client_stride = 0x30;
constexpr size_t acks_offset = 0x2780;
constexpr size_t launch_sequence_rva = 0x156CA510;
constexpr size_t gate_arrays[] = {0x23D0, 0x65FA0};
constexpr size_t copy_fields[] = {0x08, 0x0C, 0x10, 0x20};
constexpr size_t reference_slot = 1;
constexpr size_t injected_slots[] = {2, 3};

size_t base() { return game::get_engine_base(); }

// A reference table is data from outside the process image, and one of
// its entries can point at a page that is not committed yet - the Arxan
// region is full of them at post_unpack. Reading such an address throws
// and takes post_unpack down with it: measured, the bit array was
// relocated, the storage dry run then faulted, and every status field
// after that point stayed zero while `attempts` froze at 1. The
// component looked like it had never run.
//
// So every table read goes through this. A table can now be wrong
// without being fatal.
bool readable(const void *p, const size_t size) {
  MEMORY_BASIC_INFORMATION mbi{};
  if (!VirtualQuery(p, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) {
    return false;
  }
  if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) {
    return false;
  }
  const auto start = reinterpret_cast<size_t>(mbi.BaseAddress);
  const auto end = start + mbi.RegionSize;
  return reinterpret_cast<size_t>(p) + size <= end;
}

bool write_bytes(void *target, const void *data, const size_t size) {
  DWORD old{};
  if (!VirtualProtect(target, size, PAGE_EXECUTE_READWRITE, &old)) {
    return false;
  }

  const auto restore = utils::finally([&] {
    DWORD tmp{};
    VirtualProtect(target, size, old, &tmp);
  });

  std::memcpy(target, data, size);
  return true;
}

// Memory must land close to the module: rip-relative displacements and the
// ABS32 form (the RVA used as a displacement off a base register) are both
// 32-bit. A plain VirtualAlloc lands terabytes away and no 32-bit offset
// reaches it - that mistake cost a whole relocation once.
// diagnostics, read from outside via the status block
uint32_t alloc_regions_seen = 0;
uint32_t alloc_free_seen = 0;
uint32_t alloc_last_error = 0;

// Probe fixed candidate addresses just past the module, exactly as
// tools/reloc_range.py does - that approach is proven and lands around
// base + 0x1FB00000 in practice.
//
// An earlier version walked the region list with VirtualQuery instead and
// found ZERO free regions (measured through the status block: 6 regions
// probed, 0 free) because one large region straight after the module pushed
// the probe past the search window in a handful of steps. Stepping 64 KB
// candidates and letting VirtualAlloc simply say no is simpler and correct.
// PADDING EITHER SIDE OF EVERY RELOCATED ARRAY.
//
// The game indexes per-controller arrays with -1. `ControllerIndex_t`
// -1 means "no controller yet", and the bounds checks in this codebase
// only test the UPPER bound - the gate at 0x02276E30 is
// `movsxd rsi, ecx / cmp esi, 2 / jl ok`, which -1 passes. It then reads
// s_storage[-1].
//
// In the stock image that lands in whatever .data precedes the array:
// garbage, but MAPPED, so it never faults. A relocated array sits at the
// very front of its own VirtualAlloc region, so index -1 is unmapped and
// the process dies instantly.
//
// Measured 2026-08-12: signing in the second local client crashed at RVA
// 0x02276E71 reading 0x1FAC76B8, which is exactly
// (relocated s_storage base) - 0x8958 + 0x10, with rsi = -1.
//
// So reproduce the stock topology: over-allocate and hand back a pointer
// with slack on both sides. The padding is zero-filled, so an invalid
// index reads a null and the caller's own `test rax,rax / je` turns it
// into a clean "nothing there" - which is better defined than the stock
// behaviour of reading a live neighbour.
//
// This is not a null guard bolted onto one crash site. It removes the
// entire class for every array the component relocates, which matters
// because CLAUDE.md already records that per-site guards just move the
// crash to the next array.
constexpr size_t reloc_padding = 0x10000; // > any observed element size

void *allocate_near_module(const size_t size) {
  const auto module_base = base();

  const auto *dos = reinterpret_cast<const IMAGE_DOS_HEADER *>(module_base);
  const auto *nt =
      reinterpret_cast<const IMAGE_NT_HEADERS *>(module_base + dos->e_lfanew);
  const size_t image_size = nt->OptionalHeader.SizeOfImage;

  auto candidate =
      (module_base + image_size + 0xFFFF) & ~static_cast<size_t>(0xFFFF);

  for (size_t i = 0; i < 0x4000; ++i, candidate += 0x10000) {
    ++alloc_regions_seen;
    if (candidate - module_base > 0x60000000) {
      break; // beyond the reach of a 32-bit displacement
    }

    auto *p = VirtualAlloc(reinterpret_cast<void *>(candidate),
                           size + 2 * reloc_padding, MEM_COMMIT | MEM_RESERVE,
                           PAGE_EXECUTE_READWRITE);
    if (p) {
      ++alloc_free_seen;
      // Hand back the middle. VirtualAlloc zero-fills, so the slack
      // on both sides reads as null for out-of-range indices.
      return reinterpret_cast<void *>(reinterpret_cast<size_t>(p) +
                                      reloc_padding);
    }
    alloc_last_error = GetLastError();
  }

  return nullptr;
}

// NEVER call printf from here. Measured 2026-08-06 with breadcrumbs: the
// status write immediately BEFORE a printf lands, the one immediately
// AFTER it never does. printf takes post_unpack down, which is why the
// component relocated the bit array and then appeared to have done
// nothing at all - no stride fix, no second table, no daemon, and every
// status field after that point stuck at zero.
//
// There is no console in a normal launch anyway (CLAUDE.md), so these
// messages never went anywhere. They are kept as code because the
// wording documents the failure cases, but they must not reach the CRT.
// 2026-09-28 (port to game build 0x06517980): the messages now go to the
// component's own trace file - formatted into a buffer only, never
// through stdout - because a patch that stands down on a new build must
// say so. In the player build the trace file is refused, so they vanish.
void trace_text(const char *text); // defined after trace_write

template <typename... Args> void note(const char *fmt, Args... args) {
  char buf[512]{};
  std::snprintf(buf, sizeof(buf), fmt, args...);
  for (auto *p = buf; *p; ++p) {
    if (*p == '\n' || *p == '\r') {
      *p = ' ';
    }
  }
  trace_text(buf);
}

// Defined further down, next to the status block itself. Declared here
// because relocate() reports its progress through it and has to come
// first in the file.
void set_status(size_t index, uint32_t value);

// Where each table actually landed, filled by relocate(). Needed because
// the client-object table holds ABSOLUTE pointers into the client-UI
// array, so once both have moved, every pointer has to be recomputed
// from the two new bases - including entries 0 and 1, which would
// otherwise still point into the old array.
size_t new_base_rva[8] = {};

// Breadcrumb, one status slot per table: the LAST stage reached.
//
//   1 entered   2 allocated   3 copied   4 verified   5 written
//
// Needed because try_apply was observed to stop between set_status(9)
// and set_status(1) with the bit array demonstrably relocated - i.e.
// somewhere inside this function for the second table - and there was
// no way to see how far it got. Guessing which line dies is exactly
// what this project keeps paying for.
void mark(const size_t slot, const uint32_t stage) {
  set_status(11 + slot, stage);
}

constexpr size_t gamepads_reserved_size = 0x1C0;
size_t gamepads_reserved_rva = 0;

bool reserve_gamepads_region() {
  if (gamepads_reserved_rva) {
    return true;
  }
  auto *reserved = allocate_near_module(gamepads_reserved_size);
  if (!reserved) {
    return false;
  }
  gamepads_reserved_rva = reinterpret_cast<size_t>(reserved) - base();
  return true;
}

bool relocate(const reloc_table &t, const size_t slot) {
  mark(slot, 1);

  auto *fresh = allocate_near_module(t.new_size);

  // Publish the allocator counters HERE, not at the end of try_apply.
  // At the end they are lost whenever anything above them fails, and
  // they are the diagnosis for exactly that case.
  set_status(5, alloc_regions_seen);
  set_status(6, alloc_free_seen);
  set_status(7, alloc_last_error);

  if (!fresh) {
    note("[splitscreen] %s: no memory within reach of a 32-bit offset\n",
         t.name);
    return false;
  }
  mark(slot, 2);

  const auto new_rva = reinterpret_cast<size_t>(fresh) - base();

  std::memcpy(fresh, reinterpret_cast<void *>(base() + t.base_rva), t.old_size);
  std::memset(static_cast<uint8_t *>(fresh) + t.old_size, 0,
              t.new_size - t.old_size);
  mark(slot, 3);

  // Verify EVERY reference against what the instruction currently holds
  // before writing any of them. The dry run has twice caught a table that
  // no longer matched the image, which would otherwise have corrupted code.
  size_t deviating = 0;
  for (size_t i = 0; i < t.count; ++i) {
    const auto &r = t.refs[i];
    const auto *field =
        reinterpret_cast<const int32_t *>(base() + r.insn_rva + r.disp_offset);
    const auto expected =
        r.rip_relative
            ? static_cast<int32_t>(r.target_rva - (r.insn_rva + r.length))
            : static_cast<int32_t>(r.target_rva);
    if (!readable(field, sizeof(int32_t)) || *field != expected) {
      ++deviating;
    }
  }

  // The end-bound refs are verified in the same pass and against the
  // same rule: nothing is written unless every reference in the table,
  // both kinds, still matches the image.
  for (size_t i = 0; i < t.end_count; ++i) {
    const auto &r = t.end_refs[i];
    const auto *field =
        reinterpret_cast<const int32_t *>(base() + r.insn_rva + r.disp_offset);
    const auto expected =
        r.rip_relative
            ? static_cast<int32_t>(r.target_rva - (r.insn_rva + r.length))
            : static_cast<int32_t>(r.target_rva);
    if (!readable(field, sizeof(int32_t)) || *field != expected) {
      ++deviating;
    }
  }

  if (deviating) {
    note("[splitscreen] %s: %zu of %zu references do not match the image - "
         "NOTHING written\n",
         t.name, deviating, t.count);
    return false;
  }
  mark(slot, 4);

  size_t done = 0;
  for (size_t i = 0; i < t.count; ++i) {
    const auto &r = t.refs[i];
    const auto moved = new_rva + (r.target_rva - t.base_rva);
    const auto value =
        r.rip_relative ? static_cast<int32_t>(moved - (r.insn_rva + r.length))
                       : static_cast<int32_t>(moved);

    if (write_bytes(
            reinterpret_cast<void *>(base() + r.insn_rva + r.disp_offset),
            &value, sizeof(value))) {
      ++done;
    }
  }

  // End-bound references, if this array has any.
  //
  // Some loops carry no count and walk the array as
  //     for (p = &a[0]; p < &a[N]; p++)
  // with BOTH addresses baked in as immediates - measured here as
  // `lea rax,[0x0340F180]` / `lea r9,[0x0340F190]` / `cmp rax,r9 / jl`.
  // Shifting those like ordinary references would move the array and
  // leave every loop still stopping after the OLD number of elements,
  // which reads as "the relocation did nothing" while actually being
  // half-applied. They point at the END, so they get base + NEW size.
  size_t end_done = 0;
  for (size_t i = 0; i < t.end_count; ++i) {
    const auto &r = t.end_refs[i];
    auto *field = reinterpret_cast<void *>(base() + r.insn_rva + r.disp_offset);
    if (!readable(field, sizeof(int32_t))) {
      continue;
    }
    const auto moved = new_rva + t.new_size;
    const auto value =
        r.rip_relative ? static_cast<int32_t>(moved - (r.insn_rva + r.length))
                       : static_cast<int32_t>(moved);
    if (write_bytes(field, &value, sizeof(value))) {
      ++end_done;
    }
  }

  mark(slot, 5);
  note("[splitscreen] %s: %zu/%zu references moved to RVA 0x%zX (0x%X -> 0x%X "
       "bytes)\n",
       t.name, done, t.count, new_rva, t.old_size, t.new_size);
  // If stage 5 is the last breadcrumb ever seen, the printf above is
  // what kills post_unpack - it is the only non-trivial statement
  // between finishing one table and entering the next.
  mark(slot, 6);
  if (done == t.count && end_done == t.end_count) {
    if (slot < std::size(new_base_rva)) {
      new_base_rva[slot] = new_rva;
    }
    return true;
  }
  return false;
}

// Element size of clientUIActive_t on the PC, read out of the static
// array ctor: `lea rcx,[0x14FB3420] / mov edx,0x1170 / mov r8d,2`.
// PS4 calls the type clientUIActive_t with element 0x1078; strides
// differ between platforms, so only the name and the shape transfer.
constexpr size_t client_ui_stride = 0x1170;

// Element size of Storage on the PC, the same value the relocation table
// uses (0x112B0 for two records). PS4 calls the struct Storage with
// element 0x8658 - xuid, readOnLoginProcessed[4], StorageFile files[64],
// StorageScratch, inShutdown - and the PC keeps that layout but not the
// size: files are 0x220 here against 0x218 there, and inShutdown lands on
// +0x8850, which is what `cmp byte [r12 + 0x8850], 0` inside the PC
// Storage_Pump confirms.
constexpr size_t storage_stride = 0x8958;

// Point the client-object table at the relocated client-UI array.
//
// The table has NO writer - in the stock image it already contains
// pointers, so it is statically initialised and fixed up by base
// relocations at load time. That is why relocating it alone changed the
// crash from "dereferences 18.75f" to "dereferences NULL" and nothing
// more. Entries have to be produced, not moved.
//
// Elements 2 and 3 get a private COPY of element ONE, plus their own
// identity. Copying a live record into private memory is the one pattern
// this project has repeatedly found safe; hand-writing a whole record is
// the one it has repeatedly found fatal.
//
// ELEMENT 1, NOT ELEMENT 0 - measured on a fresh boot, first menu,
// nothing done:
//
//   userData[0] +0x29 = 00  +0x30 = 2    the primary local profile
//   userData[1] +0x29 = 01  +0x30 = 2    a SIGNED-IN SECONDARY profile
//   s_storage[1] already has an XUID and flags 01 00 01 01
//   ready rec 1 (local files) = 01 01 00 00  -> controllers 0 AND 1
//
// Controller 1 gets its storage and its local-file stats readiness purely
// by EXISTING at boot as a signed-in secondary profile. Its splitscreen
// sign-in adds nothing - the flags are set before it. So a guest needs to
// be that same shape, present before Storage_Init, which is why this runs
// from post_unpack and not from chain4 at lobby time.
//
// Element 0 would be wrong twice over: it is the primary (+0x29 = 00) and
// copying it gave an empty name and a duplicate identity.
//
// The XUID scheme reproduces what chain4 produces at lobby time, which was
// verified byte-identical to a real player's record apart from the XUID
// and the name digit: element 1's XUID + i, and '0' + i at the name digit.
constexpr size_t userdata_xuid = 0x00;
constexpr size_t userdata_name_digit = 0x11;
constexpr size_t userdata_signedin = 0x30;

// The copy CANNOT happen at post_unpack. Measured 2026-08-07: doing it
// there produced
//
//     ctrl 2  xuid 02 00 00 00 ...  name " 2"  +0x30 = 00
//
// because the game has not populated element 1 yet - it is still zeroed,
// so `xuid + i` gave a literal 2 and the name was just the digit. The
// relocation itself is fine (elements 0 and 1 fill in afterwards, in the
// NEW array), only the timing of the copy was wrong.
//
// So the table pointers are written at post_unpack - they must exist
// before anything indexes the map - and the guest records are filled the
// instant element 1 becomes a signed-in profile, which is the state whose
// mere existence earns controller 1 its storage and its local-file stats
// readiness during the boot pass.
size_t guest_array_rva = 0;
bool guests_filled = false;

// Defined further down with the count patches; the session-mode fix in
// fill_guests_when_ready() is gated on it.
extern bool raise_local_client_count;

void fill_guests_when_ready() {
  if (guests_filled || !guest_array_rva) {
    return;
  }

  const auto array = base() + guest_array_rva;
  const auto donor = array + client_ui_stride; // element 1

  uint32_t donor_signedin = 0;
  std::memcpy(&donor_signedin,
              reinterpret_cast<const void *>(donor + userdata_signedin),
              sizeof(donor_signedin));

  // PS4, userData_t +0x70 signInState / XUSER_SIGNIN_STATE, read out of
  // the debug build's DWARF (LiveUser_GetSignInState, 0xC94DD0):
  //
  //     0 = eXUserSigninState_NotSignedIn
  //     1 = eXUserSigninState_SignedInLocally
  //     2 = eXUserSigninState_SignedInToLive
  //
  // FILL AS SOON AS THE DONOR IS SIGNED IN AT ALL, not once it reaches
  // 2. The test that matters is LiveUser_IsSignedIn, and on both
  // platforms that is `signInState > 0` - PS4 0xC94D10 ends in
  // `cmp dword [rax+0x70], 0 / setg`, and the PC 0x01EC7030 is the same
  // instruction against +0x30. So 1 already qualifies.
  //
  // Waiting for 2 cost real ground: the game's per-controller update
  // loop runs only a HANDFUL of times during boot and then stops for
  // good (measured 3/3/3/2 calls with tools/call_counter.py), so every
  // tick spent waiting is a pump the guests can never get back.
  // mirror_signin_state below keeps them level with the donor
  // afterwards, so nothing is lost by starting early.
  if (donor_signedin == 0) {
    return; // not populated yet - try again next tick
  }

  // FILLING PAST BOOT WAS TRIED AND IS WORSE. Measured 2026-08-12:
  // gating this on `update_calls >= 2000` did fill the guests (status
  // 15 = 2 at 8622 sweeps) but left s_storage[2] AND [3] completely
  // empty, and the stuck 'hdd' task was still there. So the task is NOT
  // created by the guests - it exists with or without them - and
  // delaying the fill only costs controller 2 the storage it otherwise
  // gets. Hypothesis refuted; fill at boot.

  // COPY THE IDENTITY HEAD ONLY, NEVER THE WHOLE RECORD.
  //
  // Storage_Pump needs exactly two fields, and the PS4 build says so in
  // four instructions:
  //
  //   LiveUser_IsSignedIn(c)  =  userData[c].signInState > 0   (0xC94D10)
  //   LiveUser_GetXuid(c)     =  IsSignedIn ? userData[c].xuid : 0
  //
  // Nothing else in the record is read on that path. The first 0x40 bytes
  // hold all of it - xuid, the gamertag text, the secondary-profile marker
  // at +0x29, signInState at +0x30 - and, checked against a live dump, not
  // one qword in that range looks like a pointer.
  //
  // The rest of the 0x1170 record is a different matter. Copying it whole
  // duplicates every heap pointer a finished profile owns into two more
  // records; the first of the three to be reset frees memory the other two
  // still point at. Measured 2026-08-07: the blob copy ran, produced
  // correct-looking guests, and the process then died with an access
  // violation inside the relocated region. The same lesson is already in
  // CLAUDE.md for s_storage - only bounded, pointer-free copies are safe.
  constexpr size_t identity_head = 0x40;
  static_assert(identity_head > userdata_signedin,
                "signInState must be copied");

  for (size_t i = 2; i < 4; ++i) {
    const auto dst = array + i * client_ui_stride;
    std::memcpy(reinterpret_cast<void *>(dst),
                reinterpret_cast<const void *>(donor), identity_head);

    // Guest xuid = donor + i, and it STAYS that way.
    //
    // 2026-08-28 this was briefly changed to the console formula
    // (dwGetGuestUserID, PS4 0x005701A0: host & 0x00FFFFFFFFFFFF00 |
    // ci<<56 | ci) on the theory that guest 2 was being taken for a
    // network player. It was REVERTED the same day: THE PC DOES NOT
    // IMPLEMENT THAT SCHEME. Searched the whole main image - the mask
    // 0x00FFFFFFFFFFFF00 appears ZERO times, and neither
    // dwGetGuestUserID nor dwIsGuestUserID exists (dw = Demonware,
    // console-only). Nothing on PC reads that shape back, so stamping
    // it only moved our guests OUT of the id space the platform's own
    // working guest uses.
    //
    // Measured live: element 1 - the guest the GAME itself creates for
    // player 2, which works - carries 0xE469D710D8772BD9, an id space
    // unrelated to the host's 0x5A543F111030FB46. donor+i keeps our
    // guests in that same space; the console formula did not.
    uint64_t xuid = 0;
    std::memcpy(&xuid, reinterpret_cast<const void *>(donor + userdata_xuid),
                sizeof(xuid));
    xuid += i;
    std::memcpy(reinterpret_cast<void *>(dst + userdata_xuid), &xuid,
                sizeof(xuid));

    *reinterpret_cast<char *>(dst + userdata_name_digit) =
        static_cast<char>('0' + static_cast<int>(i));
  }

  // AUDIT 2026-09-25 - THIS DOES NOT DO WHAT THE TEXT BELOW CLAIMS. It sits
  // in fill_guests_when_ready(), which runs ONCE, at boot, the moment the
  // host is signed in at all (see the donor gate above) - long before the
  // player picks a lobby. At that point the network mode is already 0, so
  // the call is skipped, and a later menu choice (the main-menu path that
  // produced "18 Max") is never corrected. The PRIVATE GAME path works
  // because that menu sets offline itself. A real fix must run when the
  // lobby is created or splitscreen is activated, and must not flip the
  // mode under a live online lobby - not attempted blind.
  //
  // PUT THE SESSION IN LOCAL-SPLITSCREEN MODE, the way the console does.
  //
  // On PS4 activating splitscreen MAKES the session local: the menu calls
  // Lua SessionModeSetOffline, and Com_SessionMode_IsLocalSplitscreen
  // (0xE43000) is exactly IsMode(1) && IsNetworkMode(0). This component
  // never did that, so the session kept whatever mode the player entered
  // with. Activating from the MAIN MENU therefore left a bot-game session
  // and the lobby panel showed "2 Players (18 Max)" - 18 = 0x12, the
  // PublicBotGame maxClients (PS4 0x00416B8C) - with the third seat
  // enrolled through the network-client path instead of the local one.
  // Entering via PRIVATE GAME was a workaround for this;
  // this is the fix.
  //
  // Not guessed - traced. The Lua binding table entry at 0x03417DA8 pairs
  // the name string "SessionModeSetOffline" (0x030366C8) with C function
  // 0x01FD0170, and that function's whole body is:
  //     xor ecx, ecx ; call 0x020F75B0      -> SetNetworkMode(0)
  // and 0x020F75B0 is a bitfield write into the packed session-state
  // dword at 0x168ED7F4, networkMode occupying bits 6..9 (mask 0x3C0,
  // shift 6):
  //     eax = [0x168ED7F4]; ecx = mode<<6; ecx ^= eax; ecx &= 0x3C0;
  //     eax ^= ecx; [0x168ED7F4] = eax
  //
  // The ENGINE'S OWN setter is called rather than poking the bitfield, so
  // the layout stays the engine's business. It runs once, here, at the
  // moment guests are actually seated - a cold path, not the count stub
  // (calling engine code from that hot detour is what broke three
  // launches on 2026-08-28).
  //
  // Only ever forces OFFLINE, and only when guests exist. This mod is
  // offline-only by design, so there is no online session to disturb;
  // the read below also skips the call when it is already 0.
  if (raise_local_client_count) {
    constexpr uint32_t session_state_rva = 0x1686E874;
    constexpr uint32_t set_network_mode_rva = 0x20EAE30;
    constexpr uint32_t network_mode_mask = 0x3C0;
    constexpr uint32_t network_mode_shift = 6;

    const auto *state =
        reinterpret_cast<const uint32_t *>(base() + session_state_rva);
    if (readable(state, sizeof(uint32_t))) {
      const auto net = (*state & network_mode_mask) >> network_mode_shift;
      if (net != 0) {
        const auto set_net =
            reinterpret_cast<void (*)(int)>(base() + set_network_mode_rva);
        set_net(0);
        note("[splitscreen] session network mode %u -> 0 (local)\n", net);
      }
    }
  }

  guests_filled = true;
  set_status(15, donor_signedin);
}

// PC Storage_Pump(ControllerIndex_t), located 2026-08-07. The PS4 build
// names it and gives the logic (0xF7F120); the PC address came from its
// own image, never assumed:
//
//   022771B0  push rdi / sub rsp,0x40
//   022771BB  call 0x01EC7030          LiveUser_IsSignedIn(controller)
//   022771CB  jmp  0x02275AB0          tail call ClearStorage if not
//   022771E2  lea  rbp, [0x1790D9C0]   s_storage
//   022771EE  imul r12, r12, 0x8958    &s_storage[controller]
//   022771F8  call 0x01EC7310          LiveUser_GetXuid(controller)
//   02277245  mov  qword [r12], rsi    AssignStorage - the xuid we want
//
// Why calling it is necessary. Measured with tools/call_counter.py at
// boot: the pump runs a HANDFUL of times (controller 1: 3, controller 2:
// 3, controller 3: 2) and then never again - its caller loops are gated
// off once the menu is up. So whether a guest gets storage is a race
// between the guest existing and the last pump of the boot sequence, and
// controller 3 lost that race every single run.
//
// This is not faking state. Storage_Pump is the game's own function, run
// on the game's own thread, against a controller that really is signed in
// - it is the same call the boot sequence makes, just once more, after
// the guests exist.
constexpr uint32_t storage_pump_rva = 0x221A680;
constexpr uint8_t storage_pump_prologue[] = {0x40, 0x57, 0x48,
                                             0x83, 0xEC, 0x40};

size_t storage_base_rva = 0;
bool guests_pumped = false;

// Which pipelines actually tick. Measured 2026-08-07: registering the
// pump on scheduler::pipeline::main gave exactly ONE tick in sixty
// seconds, while the async loops ran throughout - so "I put it on the
// game thread" was not the same as "it runs". scheduler::loop returns
// cond_continue and never erases the task, so the pipeline itself is
// what is not being executed.
//
// Rather than guess again, count all three and let the measurement say.
uint32_t ticks_main = 0;
uint32_t ticks_renderer = 0;
uint32_t ticks_async = 0;

// WHY THIS HAS TO RETRY, AND WHY ONE PUMP IS NEVER ENOUGH.
//
// PS4 Storage_Pump (0xF7F120) assigns the xuid in two stages:
//
//   if (s->xuid != xuid && !s->inShutdown) {
//       ClearStorage(c); s->inShutdown = 1;          // stage one
//   }
//   if (s->inShutdown) {
//       for (t = 0; t < 4; t++)
//           if (StorageTarget_IsBusy(c, t)) return;  // <- no assignment
//       s->inShutdown = 0;
//       AssignStorage(c, xuid);                      // stage two
//   }
//   ...
//   StorageTarget_ProcessQueue(c, t, op)             // drains the queue
//
// Stage two only happens on a LATER call, once no target is busy - and
// the thing that makes a target stop being busy is ProcessQueue at the
// end of this same function. So a controller that is pumped once and
// then abandoned is stuck in shutdown for good: its queue never drains,
// so it stays busy, so it never gets assigned.
//
// Measured 2026-08-07, exactly that: s_storage[3].inShutdown == 1 while
// slots 0, 1 and 2 all read 0. Controller 3 got two pumps during boot
// and none afterwards, because the game's per-controller loop stops once
// the menu is up (0 calls in 30 s).
//
// Retrying is therefore the whole fix. Nothing here invents state - it
// calls the game's own function until the game's own logic completes.
// --- s_targets: the storage pending-vector table ----------------------
//
// THE PREREQUISITE FOR PUMPING CONTROLLER 2 OR 3 AT ALL.
//
// PS4 GetPendingVector (0xF81A40) states the layout outright:
//
//     addr = &s_targets + type*ROW + 0x30 + controller*0x410
//                                         + operation*0x208
//
//     PC  ROW = 0x850  = 0x30 + 2*0x410     two controllers
//     PS4 ROW = 0x1070 = 0x30 + 4*0x410     four controllers
//
// so the row stride is the console's own number, not a guess, and DWARF
// gives STORAGE_TARGET_TYPE_COUNT = 4, making the table s_targets[4]:
// 0x2140 -> 0x41C0.
//
// Until this is done, ANY storage call for controller 2 or 3 indexes a
// two-controller row out of bounds and lands in the NEXT target's fields.
// Measured 2026-08-07 from the minidump: pumping controller 2 corrupted
// the following target's activeQuery function pointer, and the tail jump
// in StorageTarget_IsActive then executed inside the relocated s_storage
// at RVA 0x1FAE2C43. tools/targets_widen.py had already recorded the same
// class of crash at 0x02277E2D; the patches below are that tool's, which
// is why they are exactly these sixteen sites.
//
// The row stride CHANGES, so a flat copy would shear every row after the
// first - each row is copied individually and the remainder left zeroed,
// which is what controllers 2 and 3 start as anyway.
//
// DO NOT touch the SIB scale. `lea rbx,[rbx + r13*2]` looks like "two
// controllers" and is not: the index is `operation + controller*2` where
// the 2 is OPERATIONS per controller, and 0x410 == 2 * 0x208. Scaling it
// to x4 doubled the controller stride and filled rows 1 and 2 with
// foreign pointers.
constexpr size_t targets_rva = 0x33BCDF0;
constexpr size_t targets_types = 4;
constexpr size_t targets_old_row = 0x850;
constexpr size_t targets_new_row = 0x1070;

struct targets_lea {
  uint32_t insn_rva; // 7-byte rip-relative lea, disp32 at +3
  uint32_t offset;   // offset into the table it points at
};

constexpr targets_lea targets_leas[] = {
    {0x0221B806, 0x00}, {0x0221B94F, 0x00}, {0x0221B9A2, 0x00},
    {0x0221BBCA, 0x00}, {0x0221B864, 0x08}, {0x0221B8E3, 0x20},
    {0x0221B8C3, 0x28}, {0x0221B97E, 0x30}, {0x0221BC91, 0x30},
};

// Every imm32 that carries the OLD row stride. Found exhaustively rather
// than by eye: tools/pe_find.py for the bytes `50 08 00 00` restricted to
// the storage TU returns exactly six, and tools/targets_widen.py only
// ever knew five.
//
// The sixth, 0x02277CF4, is not an `imul` at all - it is
//
//   02277CD4  lea    rdi, [s_targets + 8]      <- rebased with the others
//   02277CE0  mov    rdx, [rdi]                <- target->name
//   02277CE6  call   0x022E93B0                <- I_strcmp
//   02277CEF  inc    ebx
//   02277CF1  add    rdi, 0x850                <- THE ROW STRIDE
//   02277CFB  cmp    rax, 4
//   02277CFF  jb     0x02277CE0
//
// PS4 settles what that loop is - StorageTarget_GetType(const char*)
// (0xF81890) is the same function with the multiply not yet strength
// reduced:
//
//   for (i = 0; i < 4; i++)
//       if (I_strcmp(name, s_targets[i*0x1070].name@+8) == 0) return i;
//   return 4;                       // STORAGE_TARGET_TYPE_INVALID
//
// so `add rdi, 0x850` sits exactly where the console has
// `imul rcx, rcx, 0x1070`. Rebasing the lea while leaving this behind
// walks the NEW table with the OLD geometry and hands garbage pointers
// to strcmp - which is why enabling the widening used to kill the client
// about thirty seconds into boot.
//
// 0x2140, the old TOTAL size, appears nowhere in the TU.
constexpr uint32_t targets_strides[] = {
    0x0221B812, 0x0221B884, 0x0221B8CD, 0x0221B8ED, 0x0221B94B, 0x0221BCA2,
};

// OFF BY DEFAULT - correct in principle, not yet correct in practice.
//
// Measured 2026-08-07 with this enabled:
//   + s_storage[2] reached flags 01 00 01 01 for the first time ever,
//     the same fully-processed pattern the two real profiles have
//   + the 0x1FAE2C43 crash from pumping controller 2 disappeared
//   + the game's per-controller loop started running continuously
//     (112 calls a minute instead of 3) - storage finally makes progress
//   - but the client dies reliably ~30 s into boot, sometimes with no
//     minidump at all
//
// The reference set is NOT the problem: tools/find_lea.py over the whole
// 0x2140 extent finds exactly these nine leas and nothing else, so
// nothing is left pointing at the old table.
//
// The likely difference is WHEN. targets_widen.py was built to run
// against a live, already-initialised table with every thread suspended;
// this runs at post_unpack against a table that is still all zeros, and
// whatever initialises s_targets afterwards has not been checked for its
// own copy of the row stride. The five imul sites are the only stride
// rewrites here - if the initialiser carries 0x850 in another form (an
// `add`, a `lea`, or a total-size 0x2140 bound) it would lay the table
// out with the old geometry while every reader uses the new one, which
// matches a fault that arrives long after the patch.
//
// That scan has since been done and found the sixth stride site,
// 0x02277CF4 (`add rdi, 0x850`), which is now in targets_strides. It was
// a real fix and it moved the failure a long way out - the client used to
// die ~30 s into boot and now survives ~95 s - but it does not finish the
// job. 0x2140, the old total size, appears nowhere in the TU, so the
// stride constants are complete.
//
// What is left is NOT about s_targets any more. With it widened,
// controller 3 gets far enough to reach the next [2]-sized per-controller
// structure and dies there instead, deterministically, at RVA 0x02275034:
//
//   02275014  mov    eax, [rbx + 0x50]     a count
//   02275030  lea    rdx, [rbx + rdx*4]
//   02275034  movsxd r8, [rdx]             <- reads 0x7FF783A94044
//
// with rbx = RVA 0x17AC64F0 and [rbx+0x50] holding 0x291F76C0 instead of
// a small count, reached from 0x022752F7 as
// `f(LiveUser_GetXuid(c), rdi - 0xD8)`. That is a garbage struct, i.e.
// another per-controller array indexed past its second element - the same
// pattern as every other one, in a subsystem that has not been mapped
// yet. CLAUDE.md already warns that fixing one array just moves the crash
// to the next.
//
// LEFT ON, because "off" is not actually the safe option any more.
//
// Measured with it off: the client still dies, intermittently, 45 s to
// 2 min in, with exception 0xC0000096 (privileged instruction) and the
// register signature rax = 0x10A0, rcx = 2, rdx = rip - 4 - i.e. still
// executing garbage inside the relocated s_storage, still for controller
// 2. That is the SAME fault as the 0xC0000005 at 0x1FAE2C43; only the
// allocation base and how far the garbage ran differ.
//
// The reason is that the guests are no longer inert. Once s_storage[2]
// holds an xuid the game does real storage work for controller 2 on its
// own, and every StorageTarget call for it reads past the end of a
// two-controller row. Widening is what FIXES that, not what causes it.
//
// So: on. It is correct per the console layout, it removes the
// activeQuery corruption, and it survives ~95 s against 45 s. The
// remaining fault at 0x02275034 is a different array in a different
// subsystem and is the next thing to map.
constexpr bool enable_targets_widen = true;

bool widen_storage_targets() {
  if (!enable_targets_widen) {
    return false;
  }

  const auto module_base = base();
  const auto old_table = module_base + targets_rva;

  // Verify EVERY site before writing anything - all or nothing.
  for (const auto &l : targets_leas) {
    const auto insn = module_base + l.insn_rva;
    const auto *b = reinterpret_cast<const uint8_t *>(insn);
    if (b[1] != 0x8D ||
        (b[0] != 0x48 && b[0] != 0x4A && b[0] != 0x4C && b[0] != 0x4E)) {
      return false;
    }
    int32_t disp = 0;
    std::memcpy(&disp, reinterpret_cast<const void *>(insn + 3), sizeof(disp));
    if (insn + 7 + disp != old_table + l.offset) {
      return false;
    }
  }
  for (const auto rva : targets_strides) {
    uint32_t imm = 0;
    std::memcpy(&imm, reinterpret_cast<const void *>(module_base + rva),
                sizeof(imm));
    if (imm != targets_old_row) {
      return false;
    }
  }

  auto *fresh = allocate_near_module(targets_types * targets_new_row);
  if (!fresh) {
    return false;
  }
  const auto new_table = reinterpret_cast<size_t>(fresh);

  std::memset(fresh, 0, targets_types * targets_new_row);
  for (size_t t = 0; t < targets_types; ++t) {
    std::memcpy(reinterpret_cast<void *>(new_table + t * targets_new_row),
                reinterpret_cast<const void *>(old_table + t * targets_old_row),
                targets_old_row);
  }

  for (const auto &l : targets_leas) {
    const auto insn = module_base + l.insn_rva;
    const auto disp =
        static_cast<int32_t>(static_cast<int64_t>(new_table + l.offset) -
                             static_cast<int64_t>(insn + 7));
    write_bytes(reinterpret_cast<void *>(insn + 3), &disp, sizeof(disp));
  }
  const uint32_t row = static_cast<uint32_t>(targets_new_row);
  for (const auto rva : targets_strides) {
    write_bytes(reinterpret_cast<void *>(module_base + rva), &row, sizeof(row));
  }

  set_status(27, static_cast<uint32_t>(new_table - module_base));
  return true;
}

// Now driven entirely from the game thread via the detour, so an attempt
// costs one extra pump inside a call the game was making anyway.
// --- s_localFileOpData: the array that was landing on "experiments" ---
//
// PS4 `StartOp` (0xF7C740) does `opData = &s_localFileOpData[ci]` with
// stride 0x1820, and DWARF lists LocalFileOpData among the FOUR-element
// globals. On the PC it is [2] at RVA 0x17908CF0, so element 2 begins at
//
//     0x17908CF0 + 2 * 0x1820 = 0x1790BD30
//
// and that is not spare space - it is the A/B EXPERIMENTS table. The
// format strings settle it: the code reading 0x1790BD44 / 0x1790BD48 /
// 0x1790BE28 passes "experiments", "name", "variant" and
// "dumpExperiments" to its log calls, and 0x02274F90 buckets a user into
// a variant by `xuid % 10000`.
//
// So the moment controller 2 does any local-file work it writes its
// operation state straight over the experiments count and table, and the
// next variant lookup reads a garbage variant count at +0x50 and indexes
// off the end. Measured: a deterministic access violation at RVA
// 0x02275034 reading 0x7FF783A94044, with `[rbx + 0x50]` = 0x291F76C0.
//
// Easy compared to s_targets: the element stride does not change, only
// the count, so it is a flat copy - and tools/find_lea.py over the whole
// 0x3040 extent finds exactly ONE address-taking reference, 0x02274BFE,
// with `imul rdi, rdi, 0x1820` on the very next line. (It also reports
// 0x1AC3BC92, which decodes as `or byte ptr [rip - ...], bh` - data
// misread as code, not a reference.)
constexpr size_t localfileop_rva = 0x17889DF0;
constexpr size_t localfileop_elem = 0x1820;
constexpr uint32_t localfileop_lea = 0x22180CE; // 7-byte lea, disp32 at +3

// Where it ended up. Needed to work out which controller a task belongs
// to: a gamer-profile TaskRecord carries its LocalFileOpData pointer at
// +0x48, so (opData - base) / 0x1820 is the controller index.
size_t localfileop_new_rva = 0;

bool widen_local_file_ops() {
  const auto module_base = base();
  const auto old_array = module_base + localfileop_rva;
  const auto insn = module_base + localfileop_lea;

  const auto *b = reinterpret_cast<const uint8_t *>(insn);
  if (b[0] != 0x48 || b[1] != 0x8D) {
    return false;
  }
  int32_t disp = 0;
  std::memcpy(&disp, reinterpret_cast<const void *>(insn + 3), sizeof(disp));
  if (insn + 7 + disp != old_array) {
    return false;
  }

  auto *fresh = allocate_near_module(4 * localfileop_elem);
  if (!fresh) {
    return false;
  }
  const auto new_array = reinterpret_cast<size_t>(fresh);
  std::memset(fresh, 0, 4 * localfileop_elem);
  std::memcpy(fresh, reinterpret_cast<const void *>(old_array),
              2 * localfileop_elem);

  const auto new_disp = static_cast<int32_t>(static_cast<int64_t>(new_array) -
                                             static_cast<int64_t>(insn + 7));
  if (!write_bytes(reinterpret_cast<void *>(insn + 3), &new_disp,
                   sizeof(new_disp))) {
    return false;
  }

  localfileop_new_rva = new_array - module_base;
  set_status(29, static_cast<uint32_t>(localfileop_new_rva));
  return true;
}

constexpr uint32_t max_pump_attempts = 120;
uint32_t pump_attempts = 0;

// Declared here because pump_guest_storage has to call THROUGH it. Once
// the detour is installed the first bytes of Storage_Pump are a jmp into
// the stub, so calling the address directly would re-enter our own code;
// invoke() runs the original.
utils::hook::detour storage_pump_hook;
bool storage_pump_hooked = false;
bool inside_guest_pump = false;

void pump_guest_storage() {
  if (guests_pumped || !guests_filled || !storage_base_rva) {
    return;
  }

  // Bounded: if forty spaced attempts have not drained the queues,
  // something else is wrong and hammering the storage API forever would
  // only make it harder to see.
  if (pump_attempts >= max_pump_attempts) {
    return;
  }
  set_status(26, ++pump_attempts);

  // The detour is the only sanctioned way in - it was installed after
  // verifying the expected prologue, so if it is not there something is
  // wrong and we do not call blind.
  if (!storage_pump_hooked) {
    guests_pumped = true;
    set_status(20, 2);
    return;
  }

  inside_guest_pump = true;
  storage_pump_hook.invoke<void>(2);
  storage_pump_hook.invoke<void>(3);
  inside_guest_pump = false;

  // Stop as soon as both guests actually hold an xuid, rather than
  // after a fixed number of tries. A single zero reading proves
  // nothing, but a NON-zero one is conclusive.
  const auto storage = base() + storage_base_rva;
  uint64_t xuid2 = 0;
  uint64_t xuid3 = 0;
  std::memcpy(&xuid2,
              reinterpret_cast<const void *>(storage + 2 * storage_stride),
              sizeof(xuid2));
  std::memcpy(&xuid3,
              reinterpret_cast<const void *>(storage + 3 * storage_stride),
              sizeof(xuid3));
  set_status(23, static_cast<uint32_t>(xuid3));
  if (xuid2 && xuid3) {
    guests_pumped = true;
    set_status(20, 1);
  }
}

void pump_on_main() { set_status(21, ++ticks_main); }

void pump_on_renderer() { set_status(24, ++ticks_renderer); }

void count_async_ticks() { set_status(25, ++ticks_async); }

// QUIESCENCE GATE.
//
// Pumping the guests from the async pipeline is only safe while the game
// is not touching storage itself, and that is not a guess - the detour
// counts every Storage_Pump the game makes. Measured: ~31600 during boot,
// then EXACTLY ZERO at the menu (confirmed independently with
// tools/call_counter.py on the caller 0x01E26570, 0 calls in 20 s).
//
// The one time this was ignored - async pumping while the game was still
// pumping 112 times a minute - s_storage[0] and [1] lost their xuids.
// So: only act after the game's own pump count has been unchanged across
// several consecutive checks.
uint32_t last_seen_pumps = 0;
uint32_t quiet_ticks = 0;
constexpr uint32_t quiet_required = 6; // x500ms = 3 s of no game storage work

void reap_guest_tasks(); // defined with the detour, below

void pump_guests_when_quiet() {
  if (guests_pumped || !guests_filled || !storage_base_rva) {
    return;
  }

  // "Never pumped yet" is NOT "gone quiet". Both counters start at zero,
  // so without this the gate is satisfied ~3 s into boot and calls into
  // storage before the game has initialised it - measured, an access
  // violation reading null at RVA 0x022E9B94 (a string helper) two
  // launches in a row. Wait for the game to have done its own storage
  // work at least once, THEN wait for it to stop.
  if (ticks_main == 0) {
    return;
  }

  if (ticks_main != last_seen_pumps) {
    last_seen_pumps = ticks_main;
    quiet_ticks = 0;
    return;
  }
  if (++quiet_ticks < quiet_required) {
    return;
  }
  quiet_ticks = 0;

  // Reap first: Storage_Pump refuses to assign while any target is
  // busy, and the busy queries are global task lookups.
  reap_guest_tasks();
  pump_guest_storage();
}

// Keep the guests level with the donor once they exist. Filling starts at
// signInState 1 to catch the boot pumps; the donor usually settles on 2 a
// moment later, and a guest left behind at 1 would be a state difference
// nobody asked for. Writing the donor's own current value is a copy, not
// an invention.
// Com_ControllerIndex_GetLocalClientNum(ControllerIndex_t) - PC 0x020EF7C0.
//
// WHY THIS IS MEASURED AND NOT INFERRED. On PS4,
// BG_UnlockablesGetLocalCACRoot (0xE4130) is the leaf under the lobby's
// gobblegum row - GetBonusCardSet (0x140E530) loops slots 0x51..0x53 into
// GetLocalEquippedItemInSlot (0x140D420), which calls it - and it does:
//
//     localClient = Com_ControllerIndex_GetLocalClientNum(ci)
//     cgameGlob   = CG_GetLocalClientGlobals(localClient)
//     ASSERT(cgameGlob)          bg_unlockable_items.cpp:0x30F
//
// PS4 asserts the result is non-null. Retail PC has asserts compiled out,
// so a -1 there yields no CACRoot and the row silently draws nothing -
// which is exactly the symptom: storage assigned, ready flags set, the
// loadout file read, and still no gumballs for player 3.
//
// The -1 itself has only ever been INFERRED, from the two-slot end marker
// at 0x020EF7C9. Inferring is not knowing, and the whole priority order of
// the project turns on the answer, so read it out of the live game. One
// byte per controller, so -1 appears as 0xFF and cannot be misread.
constexpr uint32_t local_client_num_rva = 0x20E3040;

// cl_maxLocalClients - RVA 0x053A2720, verified 2026-08-16 as the store
// target of 0x0135D489 and read by ~2630 `cmp <reg>, [cl_maxLocalClients]`
// bounds checks. PS4 CG_GetLocalClientGlobals returns NULL for any
// localClientNum >= this, which is the ctrl-2 null deref. Publish it so the
// count raise can be confirmed from outside instead of inferred.
constexpr uint32_t cl_max_local_clients_rva = 0x05323720;
constexpr uint32_t seed_max_local_clients = 4;
uint32_t max_local_seeds = 0;
// Gated by the caller on the container relocations having succeeded.
// The three count patches and the cl_maxLocalClients hold.
//
// BISECTED 2026-08-17 and CLEARED: with this false - all three patches and
// the hold disabled, verified live (status 60 = 0, status 61 = 2) - a solo
// round start STILL crashed at RVA 0x029154E2. So this group is not the
// cause of the round-start regression. Left ON.
bool raise_local_client_count = true;

// OFF from 2026-08-18 - see probe_local_client_nums. Forcing
// cl_maxLocalClients to 4 fights the allocator, which writes that global
// itself from the count it really used.
bool hold_max_local_clients = false;

// --- WHY SigninLocalClient(2) REFUSES: ask the predicate itself ---------
//
// Native SigninLocalClient (0x01F17AD0) pushes the Lua return 2.0f at
// 0x01F17BC5 on exactly one condition: `0x01E0B520(ci)` returned FALSE.
// Static reading says every sub-check of that predicate PASSES for ci=2
// (both ready tables measure 1, storage_files shows every file type
// present), yet the process keeps returning 2. That contradiction has now
// survived two rounds of disassembly, so stop reading and MEASURE.
//
// These are PURE PREDICATES - they only read state - so they can simply be
// CALLED with ci=2 and their answers published. No detour, no code cave, no
// displaced prologue. That matters: `call_counter` on 0x01E0B520 crashed
// the process on its own cave (touched address 0x15A8A7980), and CLAUDE.md
// lists hand-rolled caves in the reserved .data window as a hazard. Calling
// a read-only function has none of that risk.
//
// The sub-checks, in the order 0x01E0B520 evaluates them:
//   0x01EA9A30(ci)        ten-record stats-source walk   (0x0340D660)
//   0x01EAF490(ci, 1)     six-record loadout-reset walk  (0x0340D880)
//   0x015E2C90(ci, 1)     -> 0x02276E30(ci,3,0) && (ci,5,0)
//   0x02276E30(ci, t, 0)  for t = 0, 7, 9, 0xD, 0xF, 0x12, 0x14, 0x1B, 0x1C
//
// Result is packed into one word, bit set = that check returned TRUE, so a
// ZERO bit names the culprit outright.
constexpr uint32_t signin_predicate_rva = 0x1DFEA90;
constexpr uint32_t ten_record_walk_rva = 0x1E9CFA0;
constexpr uint32_t six_record_walk_rva = 0x1EA2A00;
constexpr uint32_t storage_pair_rva = 0x15E2CB0;
constexpr uint32_t storage_has_file_rva = 0x221A300;

// The file types 0x01E0B520 requires, read off its disassembly.
//
// CORRECTED 2026-08-16: this list had NINE entries and the predicate makes
// ELEVEN calls. The two missing ones are `lea edx,[r8+1]` at 0x01E0B573
// (shoutcaster_settings) and `lea edx,[r8+0xB]` at 0x01E0B5A9
// (loadouts_cp_offline). With only nine probed, status 63 read 0x7FFE -
// every sub-check passing while the predicate itself returned false, which
// looked like a contradiction and was just an incomplete enumeration
// again. Anything reported here must come off the disassembly of
// 0x01E0B520, not off a remembered list.
constexpr int signin_required_files[] = {0,   1,    7,    9,    0xB, 0xD,
                                         0xF, 0x12, 0x14, 0x1B, 0x1C};

void probe_signin_predicate(const int ci) {
  const auto pred =
      reinterpret_cast<bool (*)(int)>(base() + signin_predicate_rva);
  const auto ten =
      reinterpret_cast<bool (*)(int, int)>(base() + ten_record_walk_rva);
  const auto six =
      reinterpret_cast<bool (*)(int, int)>(base() + six_record_walk_rva);
  const auto pair =
      reinterpret_cast<bool (*)(int, int)>(base() + storage_pair_rva);
  const auto has =
      reinterpret_cast<bool (*)(int, int, int)>(base() + storage_has_file_rva);

  uint32_t bits = 0;
  if (pred(ci)) {
    bits |= 1u << 0;
  } // the whole predicate
  if (ten(ci, 1)) {
    bits |= 1u << 1;
  }
  if (six(ci, 1)) {
    bits |= 1u << 2;
  }
  if (pair(ci, 1)) {
    bits |= 1u << 3;
  }
  // storage_pair's own two, broken out so a failure is attributable
  if (has(ci, 3, 0)) {
    bits |= 1u << 4;
  }
  if (has(ci, 5, 0)) {
    bits |= 1u << 5;
  }

  uint32_t bit = 6;
  for (const auto t : signin_required_files) {
    if (has(ci, t, 0)) {
      bits |= 1u << bit;
    }
    ++bit;
  }
  set_status(63, bits);
}

// --- WHY THE LOBBY STILL DRAWS TWO PLAYERS -----------------------------
//
// `SigninLocalClient(2,true)` returns 0 now and SplitscreenNum reads 3,
// but GetCountUsedAndSignedInLocalClients reads 2. PS4 0xD2EE40 defines
// that count exactly:
//
//   for lc in 0..3:
//       if !Com_LocalClient_IsBeingUsed(lc)                continue
//       if !LiveUser_IsSignedIn(GetControllerIndex(lc))    continue
//       count++
//
// Two conditions per slot, and the PC also bounds the loop at 2. Rather
// than guess which of the three is missing for slot 2, ask all three -
// they are pure predicates, so calling them costs nothing and risks
// nothing, the same argument as probe_signin_predicate.
//
// One byte per local client in status 72:
//   bit 0  IsBeingUsed(lc)
//   bit 1  LiveUser_IsSignedIn(GetControllerIndex(lc))
//   bits 4-7  the controllerIndex itself (0xF = -1)
// splitscreen_playerCount - THE number that sizes per-local-client memory.
//
// This slot holds the DVAR POINTER; the dvar's current int is at +0x28.
// CL_SplitscreenPlayerCount (0x0283AC20) is nothing but
//     mov rcx,[0x053D4A00] ; test rcx,rcx ; jne -> Dvar_GetInt ; else 1
// which matches PS4 0x1516BE0 exactly.
//
// CL_AllocatePerLocalClientMemory then does max(that, 2) and hunk-allocates
// the whole per-local-client family - clients, clientConnections, snapshots,
// parseEntities and the 0x1E940 block - for that many clients.
//
// WHY THIS AND NOT THE FLOOR BYTE. Patching the 2 in max(count,2) to a 4
// tells the ALLOCATOR four while the dvar, and therefore every other caller
// of CL_SplitscreenPlayerCount, still says one. Measured 2026-08-17: that one
// byte is the entire round-start crash - BO3_SS_SKIP=floor plays a solo round
// with everything else on. Setting the dvar keeps every reader consistent,
// which is the state the console is in.
constexpr uint32_t splitscreen_player_count_dvar_rva = 0x05355A00;
constexpr uint32_t dvar_current_offset = 0x28;

uint8_t *active_count_slots = nullptr;

// CL_LocalClients_SetAllUsedActive() - the function the cave above lives
// inside. Takes no arguments.
//
// It has to be RUN IN THE LOBBY, not left to map load, and PS4 says why:
// inside CL_ConnectFromLobby the allocator is called at 0x4155A0 and
// CL_LocalClient_SetActive only at 0x41566C - **allocation happens BEFORE
// activation**. So whatever splitscreen_playerCount says when the map
// starts loading is what the memory is carved for, and a count that only
// becomes true during activation is already too late. Measured 2026-08-18:
// the cave produced 3 (status 87) and the round still ran two local clients
// with cl_maxLocalClients = 2.
//
// On console this is a non-issue because a joining player is made active in
// the lobby (CL_Command_SetClientBeingUsedAndActive and friends), so the
// dvar is already right long before START GAME.
constexpr uint32_t set_all_used_active_rva = 0x27C19C0;

// Calling that function is NOT enough, measured 2026-08-18: it fired
// (status 88 = 1) and the count block never ran (86 = 0), because the
// engine's own early-out `cmp cl, al / je` skips everything unless a
// client's active state actually CHANGES - and local client 2 is outside
// its loop bound of 2, so his sign-in changes nothing it can see.
//
// So set the dvar directly, with the engine's own setter, from the game
// thread while sitting in the lobby. That is what the console ends up
// with anyway: on PS4 a joining player is made active in the lobby, so
// splitscreen_playerCount is already correct long before START GAME.
//
// NOT from the allocator detour - doing this during early startup
// black-screened the client three times. Here the game is fully up.
constexpr uint32_t dvar_set_int_rva = 0x226B3A0;

bool set_splitscreen_player_count(const uint32_t value) {
  uint64_t dvar = 0;
  std::memcpy(&dvar,
              reinterpret_cast<const void *>(base() +
                                             splitscreen_player_count_dvar_rva),
              sizeof(dvar));
  if (dvar == 0) {
    return false;
  }
  const auto set =
      reinterpret_cast<void (*)(void *, int)>(base() + dvar_set_int_rva);
  set(reinterpret_cast<void *>(dvar), static_cast<int>(value));
  return true;
}

uint32_t last_active_refresh = 0;
uint32_t active_refreshes = 0;

constexpr uint32_t is_being_used_rva = 0x20E3210;
constexpr uint32_t lc_controller_index_rva = 0x20E31B0;
constexpr uint32_t live_user_is_signed_in_rva = 0x1EBA5A0;

void probe_local_client_slots() {
  const auto used = reinterpret_cast<bool (*)(int)>(base() + is_being_used_rva);
  const auto ctrl =
      reinterpret_cast<int (*)(int)>(base() + lc_controller_index_rva);
  const auto signed_in =
      reinterpret_cast<bool (*)(int)>(base() + live_user_is_signed_in_rva);

  uint32_t packed = 0;
  for (int lc = 0; lc < 4; ++lc) {
    uint32_t b = 0;
    const bool u = used(lc);
    if (u) {
      b |= 1u;
    }
    const int ci = ctrl(lc);
    if (u && signed_in(ci)) {
      b |= 2u;
    }
    b |= static_cast<uint32_t>(ci & 0xF) << 4;
    packed |= b << (lc * 8);
  }
  set_status(72, packed);
}

// HOLD splitscreen_playerCount AT THE TRUE NUMBER OF LOCAL CLIENTS.
//
// PS4 CL_LocalClient_SetActive (0x15167D0) ends with exactly this:
//
//     int count = CL_LocalClient_GetActiveCount();   // 0x1516A20, i < 4
//     Dvar_SetInt(splitscreen_playerCount, count > 0 ? count : 1);
//
// The PC has the same code with that count UNROLLED TO TWO ELEMENTS, at
// 0x0283AB7D and 0x0283AB8C - two `test byte [clientUIActives[i]], 1` over
// 0x053D8BC0 and 0x053D9C38. So the engine's own writer can never produce
// more than 2, and widening it means relocating clientUIActives, which
// find_lea says has 128 address-takers and 8407 references INSIDE the array
// - nothing like the seven clean [2] arrays already moved. Not worth it to
// obtain a number we can compute correctly ourselves.
//
// So this writes the dvar's current value directly, the same shape as the
// cl_maxLocalClients hold above and for the same reason: the engine writes
// its own value back on every activation toggle, so a one-shot set does not
// hold. Nothing is faked - the number written is the count of local clients
// that really are in use and signed in.
//
// Only ever RAISED. Lowering it would fight the engine on the frame a
// player leaves, and the allocator only reads it at map load anyway.
uint32_t player_count_writes = 0;

uint32_t true_local_client_count() {
  const auto used = reinterpret_cast<bool (*)(int)>(base() + is_being_used_rva);
  const auto ctrl =
      reinterpret_cast<int (*)(int)>(base() + lc_controller_index_rva);
  const auto signed_in =
      reinterpret_cast<bool (*)(int)>(base() + live_user_is_signed_in_rva);

  uint32_t n = 0;
  for (int lc = 0; lc < 4; ++lc) {
    if (!used(lc)) {
      continue;
    }
    const int ci = ctrl(lc);
    if (ci >= 0 && signed_in(ci)) {
      ++n;
    }
  }
  return n;
}

// Defined next to signin_relocated, which it reads.
uint32_t seat_count();

void hold_splitscreen_player_count() {
  if (!raise_local_client_count) {
    return;
  }

  uint64_t dvar = 0;
  std::memcpy(&dvar,
              reinterpret_cast<const void *>(base() +
                                             splitscreen_player_count_dvar_rva),
              sizeof(dvar));
  if (dvar == 0) {
    set_status(82,
               0); // not registered yet - CL_SplitscreenPlayerCount returns 1
    return;
  }

  auto *current = reinterpret_cast<uint32_t *>(dvar + dvar_current_offset);
  if (!readable(current, sizeof(uint32_t))) {
    set_status(82, 0xFFFFFFFF);
    return;
  }

  // WIDEN THE DVAR'S DOMAIN, not just its value.
  //
  // BOIII's console said it outright on 2026-08-28:
  //   Error: '3' is not a valid value for dvar 'splitscreen_playerCount'
  //     Domain is any integer from 1 to 2
  //
  // So every ENGINE-side Dvar_SetInt(3) is REJECTED. This component
  // writes the raw value at +0x28 and bypasses that check, which is why
  // the value reads 3 while the engine's own writes silently fail - and
  // why the count kept collapsing back to 2 on paths we do not drive.
  //
  // The domain lives at +0x88 as {min, max}: measured (1, 2) in the live
  // dvar, and the printer at 0x022BC5AE reads exactly `[rbx]` = min and
  // `[rbx+4]` = max before formatting that message (string 0x0305C4C0).
  // LOG.md line 1069 recorded the REGISTRATION as min=1 max=4, so
  // something narrows it to 2 afterwards; widening it back to 4 restores
  // the registered intent rather than inventing a new limit.
  //
  // Only the max is touched, and only upward, so a stock 2-player
  // session is unaffected.
  constexpr size_t dvar_domain_offset = 0x88;
  auto *domain_max = reinterpret_cast<uint32_t *>(dvar + dvar_domain_offset +
                                                  sizeof(uint32_t));
  // No status slot: every one of 0..95 is already spoken for, and
  // borrowing an occupied one is how slot 78 got stamped on earlier
  // today. This is directly observable instead - read the live dvar at
  // [0x053D4A00] + 0x8C and it is 4 once this has run.
  if (readable(domain_max, sizeof(uint32_t)) && *domain_max == 2) {
    const uint32_t four = 4;
    write_bytes(domain_max, &four, sizeof(four));
  }

  // max of the live predicate count and the SEAT count. The predicate
  // count alone read 1 during a round three players were demonstrably
  // playing (live_user_is_signed_in is false for guests in-game), so
  // the hold never raised the dvar and the map-load reallocation
  // shrank cl_maxLocalClients back to 2. Seats are the engine's own
  // in-use rule: 1 at the menu, 3 in a full lobby.
  const auto want = std::max(true_local_client_count(), seat_count());
  set_status(82, *current);
  set_status(83, want);

  if (want > 1 && *current < want) {
    if (write_bytes(current, &want, sizeof(want))) {
      set_status(84, ++player_count_writes);
    }
  }
}

void probe_local_client_nums() {
  const auto fn = reinterpret_cast<int (*)(int)>(base() + local_client_num_rva);
  uint32_t packed = 0;
  for (int ci = 0; ci < 4; ++ci) {
    packed |= static_cast<uint32_t>(fn(ci) & 0xFF) << (ci * 8);
  }
  set_status(51, packed);

  uint32_t max_local = 0;
  std::memcpy(&max_local,
              reinterpret_cast<const void *>(base() + cl_max_local_clients_rva),
              sizeof(max_local));
  set_status(61, max_local);

  // HOLD it at 4, do not seed once.
  //
  // A one-shot seed is not enough: measured 2026-08-16, status 62 read 1
  // (the seed succeeded) while status 61 still read 2 - CL init writes 2
  // back AFTER the probe first sees the value. And seeding at post_unpack
  // refuses outright, because the value is not yet 2 at that point. So the
  // only thing that actually holds is re-applying whenever it reads 2.
  //
  // This does not fight CL_AllocatePerLocalClientMemory: with the floor
  // patch that function computes 4 itself, so once a map load runs it
  // agrees and this stops firing. Status 62 counts the re-applications,
  // which also makes the overwrite visible rather than silent.
  // THE HOLD IS OFF from 2026-08-18, and removing a write is the whole
  // change. Forcing cl_maxLocalClients to 4 guarantees a mismatch now
  // that the count is made true at the source: the allocator writes
  // this global itself from the count it really used (PS4 0x41744B,
  // PC 0x0135D489), so overwriting it with 4 means loops run to four
  // over memory carved for fewer - which is precisely the crash at
  // RVA 0x001C4CE7, where rdx read 4 and rbx was 2.
  //
  // The reason it existed - "2 = ctrl2 rejected" - is stale: measured
  // 2026-08-18, three local clients signed in with cl_maxLocalClients
  // sitting at 2 ([3/1/3] through the Lua channel).
  if (hold_max_local_clients && raise_local_client_count && max_local == 2) {
    auto *v = reinterpret_cast<uint32_t *>(base() + cl_max_local_clients_rva);
    if (write_bytes(v, &seed_max_local_clients,
                    sizeof(seed_max_local_clients))) {
      set_status(62, ++max_local_seeds);
    }
  }

  // Only worth asking once controller 2 actually has a seat - before that
  // the answer is trivially "no local client" and tells us nothing.
  if (fn(2) >= 0) {
    probe_signin_predicate(2);
  }

  probe_local_client_slots();

  // SET splitscreen_playerCount HERE, and here is chosen for two measured
  // reasons.
  //
  // 1. It has to happen in the LOBBY. PS4 CL_ConnectFromLobby allocates at
  //    0x4155A0 and only calls CL_LocalClient_SetActive at 0x41566C -
  //    ALLOCATION HAPPENS BEFORE ACTIVATION - so a count that only becomes
  //    true during activation is structurally too late.
  // 2. It cannot live in per_controller_update_stub, where it was first
  //    put: that detour STOPS RUNNING after a splitscreen sign-in.
  //    Measured 2026-08-18 - status 34 frozen at 10785 across 8 seconds
  //    while the live count read 2 - which is exactly why the refresh never
  //    fired even though every precondition checked out. The async pipeline
  //    is the only one that keeps ticking (STATE.md: async reliable,
  //    renderer ~20 then stops, main ONCE).
  //
  // The guard matters: calling into the dvar system during EARLY STARTUP
  // black-screened the client three times, and our own guest fill makes two
  // clients look used-and-signed-in before the menu. cl_maxLocalClients is
  // only non-zero once CL has initialised and the frontend allocation has
  // run, so it is a sound "the game is up" test, and it costs one read.
  if (active_count_slots != nullptr && max_local >= 2) {
    const auto live = true_local_client_count();
    set_status(91, live);
    set_status(92, last_active_refresh);
    if (live > 1 && live != last_active_refresh &&
        set_splitscreen_player_count(live)) {
      last_active_refresh = live;
      set_status(88, ++active_refreshes);
      set_status(89, live);
    }
  }
  hold_splitscreen_player_count();
}

// PS4 DWARF, `struct userData_t` (dwMessaging.cpp, decl_line 24):
//
//   +0x58 controller  +0x5C isActive  +0x5D isGuest  +0x70 signInState
//
// The PC record is packed differently - signInState is at +0x30 here, not
// +0x70 - but the FIELD ORDER carries, and the live records confirm the
// mapping outright:
//
//   ctrl 0  +0x28=1 +0x29=0   player 1     real profile, active
//   ctrl 1  +0x28=1 +0x29=1   player 2   real splitscreen guest
//   ctrl 2  +0x28=0 +0x29=1   player 3   guest, NOT active
//
// so PC +0x28 is `isActive` and +0x29 is `isGuest`.
//
// WHY IT MATTERS. `SigninLocalClient(2,true)` logged `-> laeuft ...` and
// never returned, which is the signature STATE.md records for a guest
// whose profile data is incomplete. The record is otherwise complete -
// xuid, name, isGuest, signInState all correct - and `isActive` was the
// only field differing from the donor.
//
// It is 0 because the boot fill copied the donor while the donor itself
// was still coming up (status 15 reports the donor's signInState at copy
// time, and it was 1). Mirroring it is the same copy the fill would have
// made a moment later - the donor's own current value, not an invention.
constexpr size_t userdata_is_active = 0x28;
uint32_t active_mirrored = 0;

void mirror_signin_state() {
  // Unconditional, and before the early-outs: the answer is wanted during
  // boot too, not only once guests exist.
  probe_local_client_nums();

  if (!guests_filled || !guest_array_rva) {
    return;
  }

  const auto array = base() + guest_array_rva;
  const auto donor = array + client_ui_stride;
  uint32_t state = 0;
  std::memcpy(&state, reinterpret_cast<const void *>(donor + userdata_signedin),
              sizeof(state));
  if (!state) {
    return;
  }

  uint8_t active = 0;
  std::memcpy(&active,
              reinterpret_cast<const void *>(donor + userdata_is_active),
              sizeof(active));

  for (size_t i = 2; i < 4; ++i) {
    const auto slot = array + i * client_ui_stride;

    uint32_t have = 0;
    std::memcpy(&have, reinterpret_cast<const void *>(slot + userdata_signedin),
                sizeof(have));
    if (have != state) {
      std::memcpy(reinterpret_cast<void *>(slot + userdata_signedin), &state,
                  sizeof(state));
    }

    uint8_t have_active = 0;
    std::memcpy(&have_active,
                reinterpret_cast<const void *>(slot + userdata_is_active),
                sizeof(have_active));
    if (have_active != active) {
      std::memcpy(reinterpret_cast<void *>(slot + userdata_is_active), &active,
                  sizeof(active));
      set_status(55, ++active_mirrored);
    }
  }
}

// Storage_Pump has to be entered on the thread the game enters it on, and
// the scheduler cannot provide that: measured over one minute, pipeline
// `main` ticked ONCE, `renderer` 21 times and then stopped, and only
// `async` kept running - and async is the one thread that must not touch
// storage while the game might be.
//
// So take every chance on the game's own thread FIRST: whenever the game
// pumps controller 1 during boot, pump the guests in the same call chain,
// on the same thread, at a moment the game has already decided is safe
// for storage work. The async retry above is the fallback for after the
// game's loop goes quiet, and it stops the moment this has succeeded.
// TaskManager2_ProcessTasks(ControllerIndex_t) - PC 0x022B0770, found from
// the PS4 name (0x0101CEC0) and confirmed by its body: `cmp ecx, -1`
// against an invalid controller, then a walk of the task list whose head
// is the same global 0x17A91930 that TaskIsInProgress reads.
//
// WHY THE GUESTS NEED IT. Measured: the 'hdd' gamer-profile task sits at
// state 2 for the whole session, holding s_localFileOpData[2], whose
// opStatus is DONE and never returns to IDLE. Elements 0 and 1 complete
// normally. The task is per-controller and ProcessTasks is NOT called in
// a 0..N loop anywhere - the game calls it ad hoc for whichever
// controller it is working on - so controller 2's finished task is never
// reaped.
//
// That one unreaped task is enough to stop everything: the 'hdd' busy
// query (0x02274D30) ignores the controller entirely, so
// StorageTarget_IsBusy is globally true and Storage_Pump refuses to
// assign an xuid to any controller still waiting in shutdown - which is
// exactly the state s_storage[3] is stuck in (inShutdown = 1).
//
// Calling it is not faking anything: it is the game's own reaper, on the
// game's own thread, for a task the game itself created.
constexpr uint32_t process_tasks_rva = 0x2253C50;
constexpr uint8_t process_tasks_prologue[] = {0x83, 0xF9, 0xFF};
bool process_tasks_ok = false;

// The per-controller update, 0x01E26570 - the ONLY caller of Storage_Pump
// (at 0x01E26616), driven by the loop at 0x0135C8A0 whose bound the
// component widens to 4. Its own caller does nothing but
// `inc ebx / cmp ebx,4 / jl`, so once this function returns there is no
// storage state left alive on the stack.
//
// That is the whole reason the reap moved here. Measured 2026-08-12:
// reaping from inside the Storage_Pump detour halved startup survival
// (2/4 launches) even when narrowed to the one controller that owns a
// wedged task, because ProcessTasks runs completion handlers that
// re-enter storage while 0x01E26570 still has work to do for that
// controller. Reaping after 0x01E26570 returns has no such window.
// Toggle for the reap, so its effect can be bisected without touching the
// detour itself. "The process is still alive" turned out to be a
// WORTHLESS success criterion here: with the reap on, three launches in a
// row stayed alive, Lua kept ticking `bereit [1/1/1]`, and the window
// rendered pure black. Always check that the game DRAWS, not that it runs.
bool reaper_enabled = false;

// HOW LONG TO WAIT BEFORE REAPING.
//
// Reaping during boot sets the readiness flags correctly and leaves the
// client rendering pure black (measured 2026-08-12, bisected with this
// same flag). The handlers ProcessTasks runs are the storage read
// callbacks, which normally fire during the boot read - long before LUI
// exists - so running them out of order appears to take the renderer
// down with them.
//
// PS4 does this at SIGN-IN time, not boot: Storage_UserSignedIn calls
// ClearStorageTarget for targets 0 and 2 precisely so a late user
// re-reads. There is no separate ClearStorageTarget on the PC to call -
// only one `mov edx, 2` exists in the whole storage TU and it is not a
// call setup, so it is inlined - but the TIMING is the transferable part.
//
// So wait until the per-controller update has run well past boot, which
// means the frontend is up, and only then reap.
constexpr uint32_t reap_after_updates = 2000;
uint32_t update_calls = 0;

// --- ClearStorage: the console's own way to make a controller re-read --
//
// PS4 ClearStorage (0xF7E7E0), in full:
//
//     s = GetStorage(ci)
//     if (s->xuid == 0) return
//     for (t = 0; t < 4; t++) {
//         ClearStorageTarget(ci, t)          // inlined on the PC
//         s->readOnLoginProcessed[t] = 0     // so ReadOnLogin runs AGAIN
//     }
//     s->xuid = 0                            // so Storage_Pump re-assigns
//
// PC address 0x02275AB0, confirmed by its body - `movsxd rbp, ecx`,
// `lea rcx,[s_storage]`, `imul rsi,rsi,0x8958`, `cmp qword [rsi],0 / je` -
// and by Storage_Pump tail-calling it at 0x022771CB when a controller is
// not signed in.
//
// WHY THIS AND NOT ProcessTasks. The reaper forced completion handlers to
// run from a frame the game had not scheduled them in, and that stopped
// the renderer at every timing tried - black during boot, frozen frame
// afterwards. ClearStorage does none of that: it only marks state DIRTY.
// The game then redoes the whole login read itself, on its own frames,
// through the Storage_Pump flow it already runs - assign, ReadOnLogin,
// Storage_Read, completion - which is exactly the sequence that sets
// ready[ci] for controllers 0 and 1 at boot.
//
// The one thing to watch: it zeroes the xuid, so if the re-assign never
// happens the controller LOSES its storage. Status 35 records the xuid
// afterwards so that is visible rather than guessed.
// --- the guests read STATS ONLY --------------------------------------
//
// The renderer dies because completing the guests' SETTINGS read runs
// Com_RunAutoExec / Com_RunUserConfig / Settings_RunCallbacks /
// Com_RunCustomExec with localClient = -1 (PS4 SettingsReadResult,
// 0x6F5C60). The -1 comes from Com_ControllerIndex_GetLocalClientNum
// (PC 0x020EF7C0), whose end marker covers only two clientGameStates
// slots - and that array cannot be moved from here at any surviving
// moment (both timings measured and recorded in LOG.md).
//
// But nothing about the LOADOUT needs the settings file. `ready[ci]` -
// the flag the lobby checks before drawing gobblegums - is set by the
// STATS records' callback (0x01EA9EC0). The settings callback only does
// the config work that kills the frontend. Different files, different
// callbacks.
//
// So let the guests read the stats files and nothing else. The table at
// RVA 0x0340D840 lists the ten storage file types the stats records map
// to, read out of the running game:
//
//     0x0C 0x0D 0x15 0x16 0x06 0x07 0x08 0x09 0x11 0x12
//     (12, 13, 21, 22, 6, 7, 8, 9, 17, 18)
//
// The five ODD records - 13, 22, 7, 9, 18 - are the per-controller ones
// whose ready bits are still 00 for controllers 2 and 3.
//
// Storage_Read is (ecx = controller, edx = fileType, r8d = index) and
// returns a bool. Refusing a read for a guest is a state the game already
// handles - it is what the stock controller bound at 0x022775AF did for
// every file before the component widened it.
// The ten stats types from the table at RVA 0x0340D840, PLUS the zombie
// loadout file.
//
// Type 20 is STORAGE_ZM_LOADOUTS_OFFLINE - the file the gobblegum icons
// are drawn from. Leaving it out was the reason the first version set
// every readiness flag correctly (ready odd rows 01 01 01 01) and the
// lobby still drew an empty row for player 3: the flags said "stats
// are ready", and the loadout itself had never been read.
//
// Measured: a guest requests types {0,1,3,5,7,9,11,13,15,18,20,22,27,28,30}
// (status 49/50, mask 0x5854AAAB). Of the stats types only the five ODD
// per-controller ones appear - 7, 9, 13, 18, 22 - because the even ones
// are shared and read under controller 0.
constexpr uint32_t guest_allowed_file_types[] = {
    12, 13, 21, 22, 6, 7, 8, 9, 17, 18, // stats records
    20,                                 // STORAGE_ZM_LOADOUTS_OFFLINE
};

// These four complete into callbacks that RUN CONFIGS - the six-record
// table's descriptors are literally "exec gamedata/configs/...", and the
// settings family ends in Com_RunAutoExec/Settings_RunCallbacks with
// localClient = Com_ControllerIndex_GetLocalClientNum(ci). They are safe
// ONLY for a controller that has a seat in clientGameStates; for one that
// does not, that localClient is -1 and the process dies. Measured twice
// tonight: with all four allowed unconditionally, controller 3 (no seat,
// three-slot table) killed one boot and one lobby session silently.
constexpr uint32_t guest_seated_file_types[] = {
    3,    5,    // required by the sign-in gate
    15,   11,   // mp/cp offline loadout resets
    0x1B, 0x1C, // two of the last three
    0,          // user_settings - see below
    1,          // shoutcaster_settings - ditto
};

// FILE 0 IS BACK, AND ONLY BECAUSE ITS COMPLETION IS NOW NEUTERED.
//
// Measured 2026-08-16, first attempt: adding 0, 0x1B and 0x1C together
// INVERTED the predicate bitmask - 0 and 0x1B began passing while files
// 3, 5, 7, 9, 0xD, 0xF, 0x12, 0x14, both ready walks and the storage pair
// ALL started failing, stably, and a 90s settle killed the process.
//
// Read the way that behaves is the whole point: file 0 DID become ready.
// The read is not the problem, the CALLBACK is. PC Storage_OpResultCB
// (the read half of 0x02276FB5) does, in this order:
//
//   02277104  rcx = file->properties
//   02277107  mov dword ptr [rdi+0x40], 1        <- READY, before any callback
//   0227710E  r10 = properties->[0x28]           <- SettingsReadResult
//   0227712C  call r10(ci, fileType, slot, result, &file->ctx)
//   0227712F  test al,al / jne                   <- callback says "keep it"
//   02277133  mov dword ptr [rdi+0x40], esi      <- esi = 0: NOT ready
//
// matching PS4 0xF7DF1A exactly, and reached for a FAILED read as well
// (0227704E jne skips only the DDL_CreateContext). So readiness is a
// RESULT of the completion running, and the completion is free to run
// without its settings side effects - which is what
// settings_read_result_stub below does for a guest. Nothing is faked:
// the game sets the flag itself, in its own code, at its own moment.

// The settings callback, and the two halves of it that must not run for a
// guest. PC SettingsReadResult is RVA 0x0164DC30, taken from the storage
// file table (0x0343ADF0, entry 0 `user_settings`, readCB at +0x28) - not
// guessed - and it is PS4 0x6F5C60 instruction for instruction:
//
//   result == 0:  Com_RunAutoExec(lc, ci)      0x02149640
//                 Com_RunUserConfig(lc, ci)    0x02149660
//                 Settings_RunCallbacks(ci)    0x0164F900   (twice)
//   result != 0:  Storage_Reset(ci, 0, 0)      0x02277640
//                 Settings_ResetCommonVarsToDefault(ci)  0x0164F290
//                 SaveChanges(ci)              0x0164DBB0
//                 Settings_RunCallbacks(ci)    0x0164F900
//   both:         return true
//
// Of those, exactly ONE is storage state rather than settings state:
// `Storage_Reset(ci, 0, 0)`, which is DDL_ResetContext over the file's own
// buffer (PS4 0xF7DAC0, PC 0x02277640 verified: props->[0x48] DDLDef,
// props->[0x18] size, ContextModified at 0x02275DD0). That is the game's
// own recovery for an unreadable profile and it leaves a VALID default
// context behind - which is what the +0x30/+0x38 null pointers on
// controller 2 need. Everything else writes per-controller SETTINGS state
// through arrays this build sizes [2], execs configs, and issues a profile
// WRITE for a guest, and that family is what wiped controller 2's storage
// and blacked out the renderer every previous time.
constexpr uint32_t settings_read_result_rva = 0x164DC50;
constexpr uint8_t settings_read_result_prologue[] = {0x40, 0x57, 0x48,
                                                     0x83, 0xEC, 0x20};
constexpr uint32_t storage_reset_rva = 0x221AB10;

utils::hook::detour settings_read_result_hook;
uint32_t guest_settings_completions = 0;
uint32_t guest_settings_resets = 0;

// File 0 is allowed ONLY while this is true. If the prologue check below
// ever fails, the hook is not installed and the read filter must go back
// to refusing file 0 - otherwise the build silently reproduces the run
// that killed the process. Fail safe, never fail dirty.
bool settings_result_neutered = false;

char settings_read_result_stub(const int controller, const int file_type,
                               const int slot, const int result,
                               void *ddl_context) {
  if (controller >= 2) {
    // Pack what actually happened: 0x8000 marks "seen at all", then the
    // controller and the StorageResult the read came back with. This is
    // the one number the design turns on - a success means the guest's
    // own .cgp loaded and its DDL context is already built, a failure
    // means the Storage_Reset below is what makes the context valid.
    set_status(64, 0x8000u | (static_cast<uint32_t>(controller) << 8) |
                       (static_cast<uint32_t>(result) & 0xFFu));
    set_status(65, ++guest_settings_completions);

    if (result != 0) {
      const auto reset =
          reinterpret_cast<void (*)(int, int, int)>(base() + storage_reset_rva);
      reset(controller, 0, 0);
      set_status(66, ++guest_settings_resets);
    }

    // TRUE, exactly like both of the original's paths. Returning false
    // makes 0x02277133 write the ready state straight back to zero.
    return 1;
  }
  return settings_read_result_hook.invoke<char>(controller, file_type, slot,
                                                result, ddl_context);
}

// --- file 1, shoutcaster_settings: the SAME shape, so the same fix -----
//
// The sign-in predicate needs ELEVEN file types, not the nine this file
// used to list. Read off 0x01E0B520 instruction by instruction, the calls
// to 0x02276E30 pass edx = 0, 1, 7, 9, 0xB, 0xD, 0xF, 0x12, 0x14, 0x1B,
// 0x1C. Types 1 and 0xB were simply missing from signin_required_files,
// so status 63 reported "every sub-check passes" while the predicate
// itself still returned false - measured exactly that way, bits 0x7FFE.
// 0xB (loadouts_cp_offline) was already allowed; 1 was not.
//
// PC ShoutcasterSettingsReadResult is RVA 0x01650620 (storage table entry
// 1, readCB at +0x28), and it is PS4 0x6F7180 plus its inlined
// ShoutcasterResetSettings (0x6F71F0):
//
//   result == 0:  nothing at all
//   result != 0:  Storage_Reset(ci, 1, 0)            0x02277640
//                 Cmd_ExecuteSingleCommand(lc, ci,
//                   "exec .../default_shoutcaster_settings.cfg")  0x020ED380
//                 ShoutcasterSaveChanges(ci)         0x01650590
//   both:         return true
//
// So the success path is already harmless and only the failure path needs
// splitting: keep the DDL reset, drop the config exec and the profile
// write, for the same reason as file 0.
constexpr uint32_t shoutcaster_read_result_rva = 0x1650640;
// 0x40 is the redundant REX prefix on `push rbx`; pe_at.py does not print
// it, so the bytes were taken from the image rather than from the listing.
constexpr uint8_t shoutcaster_read_result_prologue[] = {0x40, 0x53, 0x48,
                                                        0x83, 0xEC, 0x20};

utils::hook::detour shoutcaster_read_result_hook;
uint32_t guest_shoutcaster_completions = 0;
uint32_t guest_shoutcaster_resets = 0;
bool shoutcaster_result_neutered = false;

char shoutcaster_read_result_stub(const int controller, const int file_type,
                                  const int slot, const int result,
                                  void *ddl_context) {
  if (controller >= 2) {
    set_status(68, 0x8000u | (static_cast<uint32_t>(controller) << 8) |
                       (static_cast<uint32_t>(result) & 0xFFu));
    set_status(69, ++guest_shoutcaster_completions);

    if (result != 0) {
      const auto reset =
          reinterpret_cast<void (*)(int, int, int)>(base() + storage_reset_rva);
      reset(controller, 1, 0);
      set_status(70, ++guest_shoutcaster_resets);
    }

    return 1;
  }
  return shoutcaster_read_result_hook.invoke<char>(controller, file_type, slot,
                                                   result, ddl_context);
}

// Files 0, 0x1B and 0x1C were found by MEASURING the predicate rather than
// reading it (status 63, 2026-08-16). `0x01E0B520` requires
// `0x02276E30(ci, t, 0)` for t = 0, 7, 9, 0xD, 0xF, 0x12, 0x14, 0x1B, 0x1C
// on top of the pair 3/5, and controller 2 failed exactly on 0, 0x1B, 0x1C
// while every other check passed. All three sit in the guest-blocked set
// {0,1,11,15,27,28,30} that the status-49 request mask measured - i.e. this
// filter was refusing them, the same way it refused 11 and 15 and thereby
// held the loadout-reset table down.

// WHY 15 AND 11, exactly.
//
// The sign-in gate `0x01EAF490(ci, 1)` walks the SIX-record loadout-reset
// table at RVA 0x0340D880 (stride 0x30; the registrar at 0x01EAF32C proves
// the base by loading the file index from `[rbx-8]` with rbx = base+8) and
// refuses if any record whose targetType matches has `ready[ci] == 0`:
//
//   rec 0  file 14  target 2   mp_reset_loadouts_online.cfg
//   rec 1  file 15  target 1   mp_reset_loadouts_offline.cfg
//   rec 2  file 10  target 2   cp_reset_loadouts_online.cfg
//   rec 3  file 11  target 1   cp_reset_loadouts_offline.cfg
//   rec 4  file 19  target 2   zm_reset_loadouts_online.cfg
//   rec 5  file 20  target 1   zm_reset_loadouts_offline.cfg
//
// Offline play evaluates the target-1 records, so records 1, 3 and 5 are
// the ones that can refuse. Measured live:
//
//   rec 1  01 01 00 00      rec 3  01 01 00 00      rec 5  01 01 01 01
//
// Record 5 is file 20, which was already allowed - and it is the ONLY one
// ready for all four controllers. Records 1 and 3 are files 15 and 11,
// both of which the guests actively request (status-49 mask 0x5854AAAB has
// both bits) and both of which this very filter was refusing. So the
// refusal was self-inflicted twice over, and file 20 is the control that
// proves letting them through is what sets the flag.
//
// Only these two, for the same reason as 3 and 5: turning the filter off
// wholesale kills the process (PS4 SettingsReadResult writes
// s_settingsGlob[ci], another per-controller array that is [2] here).
// The online counterparts 14, 10 and 19 stay blocked - they are target 2
// and cannot gate offline play.

// THE FILTER IS OFF, and this is why.
//
// It only ever existed to stop the renderer blacking out when a guest's
// SETTINGS read completed: PS4 SettingsReadResult (0x6F5C60) runs
// Com_RunAutoExec / Com_RunUserConfig / Settings_RunCallbacks with
// Com_ControllerIndex_GetLocalClientNum(ci), and for a guest that was -1.
// **That -1 is fixed** - clientGameStates now has a seat for controller 2
// and status 51 measures GetLocalClientNum(2) == 2 - so the condition the
// filter was compensating for no longer exists.
//
// Meanwhile the filter had become the thing blocking sign-in. Native
// Engine.SigninLocalClient (RVA 0x01F17AD0) refuses with Lua return 2
// when 0x01E0B520(ci) is false, and that predicate requires
//
//     0x02276E30(ci, 3, 0)  &&  0x02276E30(ci, 5, 0)      via 0x015E2C90
//
// - storage file types 3 and 5, neither of which is in the allow-list
// above. So the guests could never satisfy it and every
// SigninLocalClient(2) came back 2.
//
// Keep the counters and the request mask: they are how the black screen
// would be caught coming back, and status 47 going to 0 is the proof the
// filter is genuinely inert rather than accidentally still on.
// ...but it cannot simply be switched off for EVERY guest, and turning it
// off wholesale killed the process on the next launch.
//
// The seat table is three slots, so controller 2 has a local client and
// **controller 3 still does not** - status 51 measures exactly that:
// ci0=0 ci1=1 ci2=2 ci3=-1. Allowing controller 3's settings read to
// complete puts SettingsReadResult back on the -1 path that the filter
// existed to avoid, and the renderer dies just as it always did.
//
// So the gate is not a list of file types at all. It is the one condition
// the PS4 read actually names: does this controller HAVE a local client?
// Ask the game, per call. A controller with a seat reads everything; a
// controller without one keeps the restrictive list. When the table grows
// to four seats this needs no edit - controller 3 starts passing on its
// own.
bool guest_has_local_client(const int controller) {
  const auto fn = reinterpret_cast<int (*)(int)>(base() + local_client_num_rva);
  return fn(controller) >= 0;
}

utils::hook::detour storage_read_hook;
uint32_t guest_reads_allowed = 0;
uint32_t guest_reads_blocked = 0;
uint64_t guest_read_mask = 0;

bool storage_read_stub(const int controller, const int file_type,
                       const int index) {
  if (controller >= 2) {
    bool allowed = false;
    for (const auto t : guest_allowed_file_types) {
      if (static_cast<uint32_t>(file_type) == t) {
        allowed = true;
        break;
      }
    }
    if (!allowed) {
      for (const auto t : guest_seated_file_types) {
        if (static_cast<uint32_t>(file_type) == t) {
          allowed = guest_has_local_client(controller);
          // The two settings files are only survivable while
          // their completions are neutered for guests.
          if (file_type == 0 && !settings_result_neutered) {
            allowed = false;
          }
          if (file_type == 1 && !shoutcaster_result_neutered) {
            allowed = false;
          }
          break;
        }
      }
    }
    // Record WHICH types a guest asks for, as a 64-bit mask over the
    // 64 storage file types. Blocking everything but the stats files
    // was too blunt - it also blocked the LOADOUT file, which is
    // where the gobblegum icons come from, so the flags came out
    // right and the lobby still drew nothing. The mask says exactly
    // what to let through.
    if (file_type >= 0 && file_type < 64) {
      const auto bit = 1ull << file_type;
      guest_read_mask |= bit;
      set_status(49, static_cast<uint32_t>(guest_read_mask));
      set_status(50, static_cast<uint32_t>(guest_read_mask >> 32));
    }
    // The allow-list is back, WIDENED rather than removed. Removing it
    // wholesale killed the process on two consecutive launches, and
    // gating on "has a local client" instead did not save it - so the
    // -1 was never the only hazard. PS4 SettingsReadResult (0x6F5C60)
    // shows the rest: it finishes with
    //
    //     s_settingsGlob[ci] = 1;  Settings_RunCallbacks(ci);
    //
    // indexed BY CONTROLLER INDEX - one more per-controller array that
    // is very likely [2] on the PC, so a guest completing ANY settings
    // read writes past it. Letting every file type through exposed that
    // for a whole family of files at once.
    //
    // Types 3 and 5 are the two the sign-in gate actually needs
    // (0x015E2C90 calls 0x02276E30 with each). Both are targetType 0,
    // the local gamer-profile family. Widening by exactly those two is
    // the smallest step that can satisfy the gate, and it keeps every
    // other file a guest asks for blocked.
    if (!allowed) {
      set_status(47, ++guest_reads_blocked);
      return false;
    }
    set_status(46, ++guest_reads_allowed);
  }
  return storage_read_hook.invoke<bool>(controller, file_type, index);
}

constexpr uint32_t storage_read_rva = 0x221AA70;
constexpr uint8_t storage_read_prologue[] = {0x48, 0x89, 0x5C, 0x24, 0x08};

// --- clientGameStates: the fix for the -1 ----------------------------
//
// THE WHOLE CHAIN, one instruction deep:
//
//   020EF7C2  lea rax,[0x168CE9B8]   &clientGameStates[0].controllerIndex
//   020EF7C9  lea r8, [0x168CEA00]   END = base+8 + 2*0x24  -> TWO slots
//   020EF7DF  or  eax, -1            not found
//
// so Com_ControllerIndex_GetLocalClientNum(2) is -1, and PS4
// SettingsReadResult (0x6F5C60) then runs Com_RunAutoExec,
// Com_RunUserConfig, Settings_RunCallbacks and Com_RunCustomExec with
// that -1 when the guests' settings read completes. That is the black
// screen - measured four separate ways before one PS4 read named it.
//
// The array cannot be extended in place: 0x168CE9F8 onwards holds live
// float data (0.5f, 0.707f) with six leas and four SSE stores into it.
// So it moves, to the fixed address CLAUDE.md's occupancy map reserves
// for exactly this, using the 76-reference table chain4 has always used.
//
// Three slots, not four - the table's end markers are precomputed for
// three, which is what controller 2 needs and what chain4 defaults to.
bool signin_relocated = false;

// Field offsets from the PS4 DWARF `struct ClientGameState`
// (com_clients.cpp, decl_line 20) and CONFIRMED on the PC by reading the
// live array with tools/cgs_dump.py - the PC head matches field for field
// even though the stride is 0x24 rather than PS4's 0x1C:
//
//   slot 0  flags=1 localClientNum=0 controllerIndex=0 uiContextIndex=0
//   networkID=0 slot 1  flags=0 localClientNum=1 controllerIndex=1
//   uiContextIndex=1 networkID=1
//
// Only the first five fields are written. The PC diverges from PS4 after
// +0x14 (it reads dwords at +0x18 and +0x1C where PS4 has two u16 model
// handles), so nothing beyond +0x10 is assumed.
constexpr size_t cgs_flags = 0x0;
constexpr size_t cgs_local_client_num = 0x4;
constexpr size_t cgs_controller_index = 0x8;
constexpr size_t cgs_ui_context_index = 0xC;
constexpr size_t cgs_network_id = 0x10;

// How many local-client SEATS are in use. Bit 0 of each relocated
// clientGameStates record - the same rule Com_LocalClient_IsBeingUsed
// applies and the same one install_active_count_fix already counts.
// Correct at the menu (1 with one player) and in a full lobby (3).
uint32_t seat_count() {
  if (!signin_relocated) {
    return 0;
  }
  uint32_t n = 0;
  for (uint32_t lc = 0; lc < signin_new_slots; ++lc) {
    uint8_t flags = 0;
    std::memcpy(&flags,
                reinterpret_cast<const void *>(base() + signin_new_base +
                                               lc * signin_stride),
                sizeof(flags));
    if (flags & 1) {
      ++n;
    }
  }
  return n;
}

// BRIDGE ONLY THE SLOTS THE ENGINE ITSELF CALLS INVALID.
//
// Two measured failures shaped this:
//
//   16:53 round: the map-load reallocation asked for the count while
//   the seat bits were wiped-and-rebuilding; the count dipped, the
//   engine re-sized for 2, and the third pane (layout already
//   committed) stayed black.
//
//   17:22 round: a cl_maxLocalClients-latched bridge did NOT close
//   it, because the wipe happens while the engine is still sized -
//   the latch memorized the dip ([94] = 2 at the reallocation with
//   all three seats reading 1 either side of it).
//
// The discriminator is the engine's own record-validity rule, not
// timing: PS4 Com_LocalClient_GetUIContextIndex (0xE35C30) asserts
// clientGameStates[lc].localClientNum == lc and answers -1 when it
// does not hold. A record mid-wipe fails that test (the memset
// zeroes localClientNum, so slots 1/2 read 0); a GENUINE sign-out
// keeps localClientNum == lc and only clears the flags bit. So a
// constituted record is trusted in both directions and remembered,
// and an invalid one - where the engine itself would refuse to
// answer - uses the remembered bit instead of reading zeroed bytes.
uint8_t remembered_seat_bits = 0;

// Highest GENUINELY-constituted (flags==1) seat count seen this
// session. Drives the one-way allocation-floor commit in the stub.
uint32_t committed_seats = 0;

uint32_t bridged_seat_count() {
  if (!signin_relocated) {
    return 0;
  }
  uint32_t n = 0;
  for (uint32_t i = 0; i < signin_new_slots; ++i) {
    const auto slot = base() + signin_new_base + i * signin_stride;
    uint32_t flags = 0;
    uint32_t lcn = 0;
    std::memcpy(&flags, reinterpret_cast<const void *>(slot + cgs_flags),
                sizeof(flags));
    std::memcpy(&lcn,
                reinterpret_cast<const void *>(slot + cgs_local_client_num),
                sizeof(lcn));
    const uint8_t mask = static_cast<uint8_t>(1u << i);
    if (flags & 1) {
      // an in-use record is constituted by definition
      remembered_seat_bits |= mask;
    } else if (i != 0 && lcn == i) {
      // flags 0 with localClientNum still == i is a GENUINE
      // sign-out (the wipe memsets lcn to 0) - honor it at once.
      // Slot 0 is excluded: its lcn is 0 in the wiped state TOO,
      // so an all-zero read cannot be told from mid-wipe; within
      // one process the host never unseats, so its bit only sets.
      remembered_seat_bits &= static_cast<uint8_t>(~mask);
    }
    if (remembered_seat_bits & mask) {
      ++n;
    }
  }
  return n;
}

// Seat i, written to look exactly like the seats the game writes itself.
//
// This is a COPY of an observed state, not an invented one: slot 1 above
// carries `i` in all four fields at the main menu with splitscreen not yet
// active, so `i` in all four is what a fresh seat looks like on this build.
// `flags` stays 0 - bit 0 reads as "being used" (slot 0, the host, has it;
// slot 1 does not), and setting that is the sign-in's job, not ours.
//
// `clear` zeroes the whole seat first. That is correct on the fresh buffer
// during relocation, when nothing is reading it yet, and WRONG as a repair
// on a live array - a memset would expose a window where the game could
// read a half-cleared seat. Repairs therefore write only the five fields,
// each a naturally aligned 4-byte store.
void write_signin_seat(const size_t new_array, const uint32_t i,
                       const bool clear) {
  const auto slot = new_array + i * signin_stride;
  if (clear) {
    std::memset(reinterpret_cast<void *>(slot), 0, signin_stride);
  }
  const uint32_t zero = 0;
  std::memcpy(reinterpret_cast<void *>(slot + cgs_flags), &zero, sizeof(zero));
  std::memcpy(reinterpret_cast<void *>(slot + cgs_local_client_num), &i,
              sizeof(i));
  std::memcpy(reinterpret_cast<void *>(slot + cgs_controller_index), &i,
              sizeof(i));
  std::memcpy(reinterpret_cast<void *>(slot + cgs_ui_context_index), &i,
              sizeof(i));
  std::memcpy(reinterpret_cast<void *>(slot + cgs_network_id), &i, sizeof(i));
}

bool relocate_signin_field() {
  const auto module_base = base();
  const auto old_array = module_base + signin_old_base;
  const auto new_array = module_base + signin_new_base;

  // Verify EVERY reference before writing anything - all or nothing,
  // the same rule as the byte patches.
  uint32_t already = 0;
  for (const auto &r : signin_refs) {
    const auto at = reinterpret_cast<const uint8_t *>(module_base + r.disp_rva);
    if (std::memcmp(at, r.new_bytes, 4) == 0) {
      ++already;
      continue;
    }
    if (std::memcmp(at, r.old_bytes, 4) != 0) {
      return false;
    }
  }

  // The destination must be empty. A cave on an occupied address in
  // this window has cost this project a whole run before.
  const auto *dst = reinterpret_cast<const uint8_t *>(new_array);
  for (size_t i = 0; i < signin_stride * signin_new_slots; ++i) {
    if (dst[i] != 0) {
      return false;
    }
  }

  std::memcpy(reinterpret_cast<void *>(new_array),
              reinterpret_cast<const void *>(old_array),
              signin_stride * signin_old_slots);

  uint32_t done = 0;
  for (const auto &r : signin_refs) {
    if (write_bytes(reinterpret_cast<void *>(module_base + r.disp_rva),
                    r.new_bytes, 4)) {
      ++done;
    }
  }

  const uint8_t slots = static_cast<uint8_t>(signin_new_slots);
  for (const auto rva : signin_bounds) {
    write_bytes(reinterpret_cast<void *>(module_base + rva), &slots, 1);
  }

  for (uint32_t i = signin_old_slots; i < signin_new_slots; ++i) {
    write_signin_seat(new_array, i, true);
  }

  // REPLICATE THE INITIALIZER for the slots we copied.
  //
  // Measured, not assumed: after the relocation the OLD array at
  // 0x168CE9B0 still receives `localClientNum = i` (+0x04) and
  // `networkID = i` (+0x10) on slot 1, while the NEW array receives
  // everything else (flags, controllerIndex, +0x18, +0x1C). Both arrays
  // are live, each holding part of the state, and the result was
  // GetLocalClientNum(1) == 0 - controller 1 colliding with controller 0.
  //
  // It is not a missed reference: tools/find_lea.py over the region and a
  // wider window finds no rip-relative site outside the table, and
  // tools/signin_verify.py read all 76 back out of the live process as
  // still applied (0 reverted), which also rules out Arxan restoring
  // them. So the initializer reaches the old array by some path this
  // table does not cover, and it runs AFTER post_unpack.
  //
  // Those two fields are exactly what PS4 Com_InitClientGameStates
  // (0xE353C0) writes, and the abandoned array confirms the values. So
  // write them here rather than chase the writer: this reproduces an
  // initializer whose output went to an array nothing reads any more.
  //
  // Only these two fields, and only once. controllerIndex and
  // uiContextIndex are NOT forced for the pre-existing slots - the game
  // manages those through SetControllerIndex / RemapControllerIndex and
  // AssignUIContexts*, and PS4 has SwapClients and CompressClients which
  // reorder localClientNum, so continuously forcing any of it would
  // fight the game.
  for (uint32_t i = 0; i < signin_old_slots; ++i) {
    const auto slot = new_array + i * signin_stride;
    std::memcpy(reinterpret_cast<void *>(slot + cgs_local_client_num), &i,
                sizeof(i));
    std::memcpy(reinterpret_cast<void *>(slot + cgs_network_id), &i, sizeof(i));
  }

  set_status(44, done);
  set_status(45, already);
  signin_relocated = true;
  return true;
}

// WHY THIS EXISTS, and why it counts rather than just repairing.
//
// The seats were ALREADY being written when both relocation timings failed
// (same commit, 2d5837d) - so "the seats were empty" is not the
// explanation, and was retracted. The live candidate is timing: PS4
// Com_InitClientGameStates (0xE353C0) memsets the array and then writes
// ONLY localClientNum (+0x04) and networkID (+0x10). It never writes
// controllerIndex. If the game's initializer runs after post_unpack it
// would revert our seat to controllerIndex 0 and put
// Com_ControllerIndex_GetLocalClientNum(2) straight back to -1, with all
// 76 references correctly applied - exactly the "refs applied but nobody
// signs in" signature.
//
// So do not guess. Re-assert the seat when it is wrong and COUNT it:
// a counter that stays at 0 means nothing clobbers the seat and the
// timing theory is dead; one that climbs names the clobber outright.
uint32_t seat_reasserts = 0;

void maintain_signin_seats() {
  if (!signin_relocated) {
    return;
  }

  const auto new_array = base() + signin_new_base;
  for (uint32_t i = signin_old_slots; i < signin_new_slots; ++i) {
    const auto slot = new_array + i * signin_stride;
    uint32_t have = 0;
    std::memcpy(&have,
                reinterpret_cast<const void *>(slot + cgs_controller_index),
                sizeof(have));
    if (have != i) {
      write_signin_seat(new_array, i, false);
      set_status(52, ++seat_reasserts);
    }
  }

  // Slot 2's raw controllerIndex, so a run is readable from outside
  // without attaching anything.
  uint32_t raw = 0;
  std::memcpy(&raw,
              reinterpret_cast<const void *>(new_array + 2 * signin_stride +
                                             cgs_controller_index),
              sizeof(raw));
  set_status(53, raw);

  // Slot 1's localClientNum. Deliberately reported and NOT repaired: it
  // is the field the abandoned array was still receiving, so if it drifts
  // back to 0 that is the initializer running again and the one-shot
  // replication above is not enough.
  uint32_t lcn1 = 0;
  std::memcpy(&lcn1,
              reinterpret_cast<const void *>(new_array + 1 * signin_stride +
                                             cgs_local_client_num),
              sizeof(lcn1));
  set_status(54, lcn1);
}

// --- StartOp, counted from INSIDE the component ---------------------
//
// PS4 StartOp (0xF7C740) is what begins a gamer-profile file operation:
// it checks TaskManager2_TaskIsInProgress(&task_GamerProfileRW), takes
// `opData = &s_localFileOpData[ci]`, and creates the task. PC 0x02274BC0,
// and the lea at 0x02274BFE that the component repoints is inside it.
//
// WHY THIS HAS TO BE MEASURED FROM IN HERE. tools/call_counter.py can
// only attach once BlackOps3.exe is mapped and the status block is
// written - 5.6 s in at best - and StartOp gets ZERO calls after that
// point. The whole boot storage read, including whatever wedges the
// 'hdd' task, is over before any external tool can see it. The component
// runs at post_unpack, before Storage_Init, so it is the only vantage
// point that sees the beginning.
//
// Counted into status 37..40 by controller, and 41 records any first
// argument outside 0..3 - the task whose opData lands in the allocation
// padding implies an index around 8, and this is what would prove it.
constexpr uint32_t start_op_rva = 0x2218090;
constexpr uint8_t start_op_prologue[] = {0x48, 0x89, 0x5C, 0x24, 0x10};
// CL_SplitscreenPlayerCount - MAKE THE NUMBER TRUE AT ITS SOURCE.
//
//   0283AC20  mov rcx, [rip -> splitscreen_playerCount dvar]
//   0283AC27  test rcx, rcx
//   0283AC2A  jne 0x0283AC30            -> jmp Dvar_GetInt
//   0283AC2C  lea eax, [rcx+1]  ; ret   -> 1 when the dvar is unregistered
//
// matching PS4 0x1516BE0 exactly. Every consumer of "how many splitscreen
// players are there" asks THIS - CL_AllocatePerLocalClientMemory computes
// max(this, 2) and allocates the whole per-local-client family for that
// many, and cl_maxLocalClients is then written from the same number.
//
// WHY HERE, after four attempts elsewhere, each measured:
//
//   dvar hold on +0x28 from async     no-op, then a black screen
//   Dvar_SetInt from the allocator    black screen at startup, 3 variants
//   SetAllUsedActive by hand          engine early-out, never counts
//   writing the dvar to 3 by hand     allocation STILL carved 2 and the
//                                     load HUNG - dvar 3 vs memory 2 is
//                                     the same mismatch that crashes
//
// Returning the count here fixes all of that at once: every caller agrees
// by construction, and the ordering problem disappears because the answer
// is right whenever it is asked rather than only after something else has
// run.
//
// And it is a PLAIN MEMORY READ of the seat table - bit 0 of each 0x24
// record, exactly what Com_LocalClient_IsBeingUsed itself does
// (0x020EF9A6: movzx eax, byte [base + lc*0x24] / and al,1). No engine
// call, no dvar system, no thread hazard - which is what broke every
// earlier attempt. Nothing is faked: those flags say the clients really
// are in use, read live as 1,1,1 with three players signed in.
//
// Falls back to the original whenever the seat table has not been
// relocated or reads zero, so it can never invent a count.
constexpr uint32_t splitscreen_player_count_rva = 0x27C1AB0;
constexpr uint8_t splitscreen_player_count_prologue[] = {
    0x48, 0x8B, 0x0D, 0x89, 0x38, 0xB9, 0x02, // mov rcx, [rip+0x2B99DD9]
    0x48, 0x85, 0xC9,                         // test rcx, rcx
};

utils::hook::detour splitscreen_player_count_hook;
uint32_t player_count_queries = 0;
uint32_t player_count_last = 0;

// ---- CL_Init for local client 2 ------------------------------------
//
// PS4 Com_Init runs `for (i = 0; i < 4; i++) CL_Init(i)` (0xE49E98:
// cmp 4) - EVERY local client is initialised at boot, signed in or
// not. The PC boot inlines its copies for clients 0 and 1 only, which
// is why clientUIActives[2] has never carried flag bit 1 ("CL_Init
// ran"; PS4 setter 0x4147E3, PC 0x01359495 - an ABS32-indexed `or`,
// which is why the static bit-1 hunts never found it). CL_Frame gates
// its ENTIRE body on that bit (0x01351446: mov eax,[flags] / shr
// eax,1 / test al,1 / je skip), so an uninitialised client can never
// pump its connection handshake even with the frame loop widened -
// measured 2026-08-18 14:10: local client 2 parked at connectionState
// 6 (CA_CONFIRMLOADING) for 10+ minutes.
//
// Calling the engine's own standalone CL_Init (0x013593EF) for local
// client 2 produces the state instead of faking the bit. Verified
// against the PS4 body line by line - every write for lc2 lands in
// owned or padding memory, CL_ClearState is null-guarded (globals for
// lc2 do not exist in the lobby; the memset skips, exactly like a PS4
// boot for clients 2/3), and the two MSVC range checks on the way
// (CL_Init's own and Cbuf_Execute's) are widened in the count table
// and read back here before any call. CL_MapSwitch_Init /
// CL_Video_Init at its tail zero an idle struct and one flag byte
// (PS4 0x3E8210 / 0x3E8AB0) - harmless re-run.
//
// CALLED FROM THE COUNT DETOUR BELOW, not from the per-controller
// stub: that stub STOPS RUNNING after a splitscreen sign-in (status
// 34 frozen, measured - see the active-count refresh note), and the
// async pipeline answers engine predicates wrongly. The count detour
// runs in ENGINE context on the game thread whenever anything asks
// the splitscreen count - the Lua channel asks per command, and the
// map-load allocator asks right before allocation, so the one-shot
// fires in the lobby or, at the latest, at the perfect pre-launch
// moment. The seat condition (three used seats) comes from the same
// plain memory read the detour already does - no engine predicates.
constexpr uint32_t cl_init_rva = 0x1359410;
constexpr uint32_t cl_init_range_imm_rva = 0x135948B;
constexpr uint32_t cbuf_execute_range_imm_rva = 0x20DFA30;
// The resting value of that range check: 0x02 stock, 0x04 once
// install_cbuf_for_players34() has given local clients 2/3 their own
// command buffers and widened Com_Frame's Cbuf loop (2026-09-28, MP).
// The scoped CL_Init widens below then leave it alone.
uint8_t cbuf_range_resting = 0x02;
// Defined with the IsActive cave, further down. The cgame frame-loop
// widen below may only run when the cave is installed - see there.
extern bool isactive_caved;
constexpr uint32_t cl_frame_pump_imm_rva = 0x20ECE5E;
constexpr uint32_t netchan_poll_imm_rva = 0x20EB424;

bool cl_init2_done = false;

// TWO latches, not one. cl_init2_done means "CL_Init(2) has run" and is
// genuinely a one-shot. lc2_widens_done means "the frame pump and the
// netchan poll have been opened to three", which CANNOT happen at the
// same moment: the init runs as soon as the third SEAT exists (in the
// lobby), while the widens must wait for the ALLOCATION
// (cl_maxLocalClients >= 3), which only happens at map load.
//
// One flag for both meant the first success latched it in the lobby and
// every re-entry path bailed before ever applying the widens. Measured
// in a live three-player round, 2026-08-18 21:45 - see the long note at
// the `cl_init2_done = true` line.
//
// "Fully done" is therefore (cl_init2_done && lc2_widens_done), and
// that is what the three trigger sites now test.
bool lc2_widens_done = false;

bool lc2_fully_done() { return cl_init2_done && lc2_widens_done; }

// RE-ENTRANCY GUARD, made explicit rather than incidental.
//
// The count detour is itself one of the trigger sites, so anything
// called from the worker that asks for the splitscreen count re-enters
// it. The old comment claimed the single latch made that safe because
// it "is set first inside the callee" - it is not: cl_init2_done is set
// AFTER the CL_Init call, so a re-entry while bit 1 was still clear
// would have called CL_Init(2) a second time. Splitting the latches
// widens that window, so guard it outright instead of relying on the
// engine not to ask.
bool lc2_work_in_progress = false;

struct in_progress_guard {
  bool &flag;
  explicit in_progress_guard(bool &f) : flag(f) { flag = true; }
  ~in_progress_guard() { flag = false; }
  in_progress_guard(const in_progress_guard &) = delete;
  in_progress_guard &operator=(const in_progress_guard &) = delete;
};

// How many times our SetActive hook has actually been entered. Packed
// into the HIGH bits of status 87 so one read gives both numbers:
//
//     87 = (calls << 8) | code
//
// Three trigger sites have now reported "never fired" or "saw
// cl_maxLocalClients = 0", and each time the next step was a GUESS at a
// fourth site. This counter ends that: a low count means our hook is
// bypassed (the engine's call is inlined or an Arxan duplicate), a high
// count with code 30 means we ARE called but only ever before the
// allocator stores the count, and a high count with no code at all
// means we never regain control after the original returns.
uint32_t set_active_calls = 0;

void report87(const uint32_t code) {
  set_status(87, (set_active_calls << 8) | (code & 0xFF));
}

// The five SCR_UpdateFrame immediates of the BO3_CG_FRAME group (see the
// long comment in run_cl_init_for_local_client2), shared with the
// patch probe below so the two can never drift apart.
constexpr uint32_t cg_frame_imms[] = {
    0x013E10D4, // cmp r13d,2 - the r_num_viewports counting loop
    0x013E11D2, // cmp ebx,2  - cgame frame loop, copy A exit 1
    0x013E11DF, // cmp ebx,2  - cgame frame loop, copy A exit 2
    0x013E1264, // cmp ebx,2  - cgame frame loop, copy B tail
    0x013E12A6, // cmp ebx,2  - the loading-screen scan
};

// DIAGNOSTIC, opt-in: BO3_PATCH_PROBE=1 writes the five immediates in a
// game WITHOUT a third player (launch_detached.ps1 -OneGuest -PatchProbe).
//
// Why (2026-09-26): the 3-player deaths ~15 s into a round are Arxan
// working from corrupted state. These bytes - with the frame pump
// 0x020F95DE and the netchan poll 0x020F7BA4 - are the only code the
// component writes solely when the engine is sized for three, at map
// load. The morning's two-player A/B round therefore never had them, so
// it did not clear them. If a two-player round with the bytes written
// dies the same way, the write itself trips Arxan (a checksum guard of a
// shape BOIII's integrity list does not cover); if it survives, client
// 2's own code is the culprit. Safe with two players: the widened frame
// loop is gated by the caved IsActive, which refuses lc >=
// cl_maxLocalClients, and the viewport count only counts CA_ACTIVE slots.
void patch_probe_once() {
  static int state = -1; // -1 unread, 1 armed, 0 off or done
  if (state == 0) {
    return;
  }
  if (state < 0) {
    char buf[8] = {};
    GetEnvironmentVariableA("BO3_PATCH_PROBE", buf, sizeof(buf));
    state = std::strcmp(buf, "1") == 0 ? 1 : 0;
    if (state == 0) {
      return;
    }
  }
  if (!isactive_caved) {
    return; // the gate the widened loop relies on is not in yet
  }
  state = 0;
  for (const auto rva : cg_frame_imms) {
    const auto *at = reinterpret_cast<const uint8_t *>(base() + rva);
    if (!readable(at, 1) || *at != 0x02) {
      note("[splitscreen] patch probe: 0x%X not stock, nothing written\n", rva);
      return;
    }
  }
  const uint8_t bound3 = 0x03;
  for (const auto rva : cg_frame_imms) {
    write_bytes(reinterpret_cast<uint8_t *>(base() + rva), &bound3,
                sizeof(bound3));
  }
  note(
      "[splitscreen] patch probe: five CG_FRAME immediates written (2 -> 3)\n");
}

void run_cl_init_for_local_client2() {
  // THE ALLOCATION MUST EXIST FIRST, and this guard is why.
  //
  // The trigger is the count detour, and that fires the moment three
  // seats appear - which is DURING SIGN-IN, in the lobby, not at map
  // load as first assumed. In the lobby cl_maxLocalClients is still 2:
  // the per-local-client family is carved for two, so CL_Init(2), the
  // frame pump and the netchan poll would all touch a third element
  // that does not exist yet. Measured 2026-08-18: the run where this
  // did NOT fire in the lobby was luck, not design.
  //
  // So: refuse until the allocator has really carved three, and do
  // NOT latch cl_init2_done when refusing - the detour is called
  // again at map load (CL_AllocatePerLocalClientMemory asks it at
  // 0x0135D665), which is exactly the moment this becomes valid.
  // NO cl_maxLocalClients GATE ON THE INIT ITSELF - measured 20:18.
  //
  // The gate used to sit here and refused every single time with code
  // 30 (= 30 + 0). The reason is the console's own ordering:
  // SetAllUsedActive runs BEFORE CL_AllocatePerLocalClientMemory, so
  // at the only moment we get control the count is still 0. Gating
  // the init on the allocation is therefore self-defeating.
  //
  // And it is unnecessary. PS4 calls CL_Init(i) for i in 0..3 in
  // Com_Init, at BOOT, long before anything is allocated - it is
  // written to be safe on a client that owns nothing: CL_ClearState
  // null-checks CL_GetLocalClientGlobals and simply skips the memset.
  // That is exactly why the console can initialise all four up front.
  //
  // So: the INIT runs as soon as the seat is real. Only the two BYTE
  // WIDENS (frame pump, netchan poll) still need the allocation,
  // because their loop bodies index per-client arrays - they are
  // applied further down, each behind its own check, and the one-shot
  // flag is not latched until the init has actually taken.
  if (lc2_work_in_progress) {
    return;
  }
  const in_progress_guard guard(lc2_work_in_progress);

  const auto max_local = *reinterpret_cast<const volatile uint32_t *>(
      base() + cl_max_local_clients_rva);

  // Reports through slot 87, NOT 96/97. set_status protects 0x180
  // bytes = slots 0..95, and the occupancy map assigns index 96
  // (0x1A8A7E80) to trace_null_caller - "must stay clear". The first
  // version of this code wrote 96/97 and so reported nothing while
  // sitting on another cave's memory; that is the exact hazard
  // CLAUDE.md records for this .data window. 87 belongs to the
  // active-count cave on the twin 0x0283AB7D, which is measured to
  // never run on the offline ZM path (status 86 = 0 through a whole
  // three-player round), so it is dead weight and safe to borrow.
  //
  // 87: 1 = init ran AND the frame pump was opened
  //     2 = init ran but bit 1 did not appear, pump stays shut
  //     3 = range checks not widened, CL_Init never called
  //     5 = frame-pump site did not hold its expected byte
  //     10/11 = the watch ran but signin_relocated / the count gate
  //             was false;  20 + n = the watch ran and saw n seats
  // Everything here is also readable WITHOUT the status block:
  // clientUIActives[2] bit 1 is the init receipt and 0x020F95DE == 3
  // is the pump - both proved reliable when status slots did not.
  // SCOPED widen. Both sites are MSVC /GS range checks (`cmp rbx,2 /
  // jae __report_rangecheckfailure`) guarding an index into a [2]
  // array, and CL_Init(2) would trip them. They are opened here,
  // held open across exactly one call, and closed again - so the
  // stock engine never executes a single instruction with a bound
  // that disagrees with the array behind it. Verified byte-for-byte
  // before and after; if either site does not read what we expect,
  // nothing is written and nothing is called.
  auto *range_a = reinterpret_cast<uint8_t *>(base() + cl_init_range_imm_rva);
  auto *range_b =
      reinterpret_cast<uint8_t *>(base() + cbuf_execute_range_imm_rva);
  if (*range_a != 0x02 || *range_b != cbuf_range_resting) {
    report87(3);
    return;
  }

  // clientUIActives[2].dword0 - slot 2 is the voice_comm-vacated
  // block, gated (via raise_local_client_count) on every relocation.
  auto *flags =
      reinterpret_cast<volatile uint32_t *>(base() + 0x05359BC0 + 2 * 0x1078);
  if ((*flags & 0x2) == 0) {
    const uint8_t open = 0x03, shut = 0x02;
    // range_b is only opened when it rests at the stock 2; once the
    // Cbuf widen owns it (resting 4) it is already open and stays so.
    const bool touch_b = cbuf_range_resting == 0x02;
    const bool a_ok = write_bytes(range_a, &open, 1);
    const bool b_ok = !touch_b || write_bytes(range_b, &open, 1);
    if (a_ok && b_ok) {
      reinterpret_cast<void (*)(int)>(base() + cl_init_rva)(2);
    }
    // Always restore, even if one write failed or CL_Init threw.
    if (a_ok) {
      write_bytes(range_a, &shut, 1);
    }
    if (b_ok && touch_b) {
      write_bytes(range_b, &shut, 1);
    }
    if (!a_ok || !b_ok) {
      report87(4);
      return;
    }
  }
  // ONLY NOW open the frame pump. Bit 1 is the engine's own receipt
  // that CL_Init really ran (its setter is the last thing CL_Init
  // does, 0x01359495), and it is the same bit CL_Frame tests before
  // touching anything. So the widen is applied strictly after the
  // state it depends on exists - PS4 order, boot-then-frame - and
  // if the init did not take, the pump simply stays at two and the
  // build degrades to the (working, measured) 13:30 behaviour.
  if ((*flags & 0x2) == 0) {
    report87(2);
    return;
  }

  // The init HAS taken (bit 1 is the engine's own receipt). Latch the
  // one-shot only now, so a call arriving before the seat is ready
  // retries later instead of burning the attempt.
  //
  // THIS LATCH COVERS THE INIT ONLY - lc2_widens_done is separate, and
  // conflating the two was a real bug, measured in a live round on
  // 2026-08-18 21:45. The init succeeds in the LOBBY, where
  // cl_maxLocalClients is still 2, so the widens below are deferred
  // with code 6 - but every re-entry path (cl_init_watch,
  // set_active_stub, the count detour) tested cl_init2_done FIRST and
  // returned, so nothing ever came back to apply them. The round then
  // loaded with both bounds still at 2:
  //
  //   lc0/lc1  flags 0x37  state 0x0B (CA_ACTIVE)    playing
  //   lc2      flags 0x07  state 0x06 (CA_CONFIRMLOADING)  parked
  //   0x020F95DE = 02, 0x020F7BA4 = 02   <- never widened
  //   cl_maxLocalClients = 3             <- read DIRECTLY; status
  //                                         slot 61 said 2 and was
  //                                         stale, as LOG.md warns
  //
  // which is the documented "fullscreen parked camera, no HUD, no
  // control" round: the pre-game hold waits for a client that can
  // never finish connecting because his netchan is never polled.
  cl_init2_done = true;

  // The two byte widens need the ALLOCATION, which has not happened
  // yet on this path: SetAllUsedActive runs BEFORE
  // CL_AllocatePerLocalClientMemory (measured - the gate reported
  // code 30, i.e. count 0, every single time). Leave them for a later
  // call, when the count is real; the count detour still fires at map
  // load and re-enters here - which it can only do because the gates
  // now test lc2_widens_done as well.
  if (max_local < 3) {
    report87(6);
    return;
  }

  auto *site = reinterpret_cast<uint8_t *>(base() + cl_frame_pump_imm_rva);
  if (*site != 0x02) {
    report87(5);
    return;
  }
  const uint8_t three = 0x03;
  write_bytes(site, &three, sizeof(three));

  // THE NETCHAN POLL, widened here and only here.
  //
  // Com_ClientPacketEvent (PC 0x020F7AC7.., PS4 Com_ClientPacketEvent
  // 0xE491A0) walks the local clients and polls each one's OWN
  // netchan - PS4 runs it 0..3, the PC 0..1:
  //
  //   020F7AD2  IsBeingUsed(ebx) / 020F7AE8 GetControllerIndex(ebx)
  //   020F7AED  lea r9,[rsi+0x24B68] / 020F7B03 add r9,rdi (i*0x25780)
  //   020F7B12  call 0x02174970   Netchan_GetMessage
  //   020F7BA2  cmp ebx, 2        <- the bound, imm at 0x020F7BA4
  //
  // So local client 2's netchan is never read and his
  // challenge/connect replies are never collected, which is why he
  // parks at CA_CONFIRMLOADING(6) while 0 and 1 reach CA_ACTIVE(0xB).
  //
  // It must NOT be widened in the frontend: there cl_maxLocalClients
  // is 2 and the clientConnection array is carved for two, so index 2
  // reads past the end (measured - it killed the lobby right after
  // the third sign-in). We are called from the count detour at MAP
  // LOAD, so re-read cl_maxLocalClients and only widen once the
  // allocator has really carved three or more.
  auto *poll = reinterpret_cast<uint8_t *>(base() + netchan_poll_imm_rva);
  if (*poll == 0x02) {
    // Slot 86, NOT 98: set_status protects 0x180 bytes = slots
    // 0..95, and 96 is trace_null_caller's cave. 86 belongs to the
    // count cave on the dead twin 0x0283AB7D and is measured to
    // stay 0 on this path, so it is free to borrow (same reasoning
    // as 87 above).
    write_bytes(poll, &three, sizeof(three));
    set_status(86, max_local);
  }

  // DEFAULT OFF. THE BLOCKER IS NO LONGER ARRAYS - it is SNAPSHOT
  // DELIVERY, and that is a genuinely different layer.
  //
  // 2026-08-25 round 4, with cgEntCollWorld/cgEntCollNodes relocated and
  // verified live: the game did NOT crash. It HUNG during map load,
  // ~2 cores pegged, responding, never reaching cl_max=3.
  //
  // PS4 CG_ProcessSnapshots (0x2A86C0) explains it exactly: it is a WAIT
  // LOOP - `while (!cg->snap) { CG_ReadNextSnapshot(); }`. It is the only
  // writer of cg->snap / cg->nextSnap, and it blocks until a snapshot
  // arrives. Local client 2 has no snapshot feed, so the first frame tick
  // for it spins forever.
  //
  // So the remaining work is NOT more [2]->[4] relocations. Client 2 is
  // CA_ACTIVE (clientUIActives[2] state 0x0B) and its netchan poll IS
  // widened (0x020F7BA4 = 03), but nothing delivers snapshots into its
  // clientConnection. The chain to make work, PS4-named:
  //   CL_PacketEvent(lc,...) -> CL_ParseServerMessage(lc)
  //     -> CL_ParseSnapshot(lc,msg)  [PS4 0x422C60]
  //     -> CL_GetSnapshot(lc,n,out)  [PS4 0x3E94A0]
  // On a listen server the local server must also be sizing for and
  // transmitting to a third client.
  //
  // THE CGAME FRAME LOOP - the bound that has hidden the third pane all
  // along, and the reason cg[2].nextSnap is NULL.
  //
  // PC SCR_UpdateFrame walks the local clients and, per client, calls
  // CG_DrawActiveFrame (0x010BF8C0) / CG_ProcessButDontDrawActiveFrame
  // (0x010E50D0). An exhaustive call scan finds EXACTLY ONE call site
  // for each - 0x013E121B and 0x013E123B - both inside this loop. PS4
  // SCR_UpdateFrame runs the same body 0..3 (`cmp [rbp-0x9c],4` at
  // 0x00427CF6, IsActive -> CL_GetLocalClientGlobals ->
  // CG_ProcessButDontDrawActiveFrame -> CL_CGameRendering); the PC
  // compiled the count to a literal 2.
  //
  // Consequences, both measured this session:
  //   * cg->nextSnap (+0x30) and cg->snap (+0x28) are written ONLY by
  //     CG_SetNextSnap / CG_SetInitialSnapshot, called only from
  //     CG_ProcessSnapshots, called only from those two functions. So
  //     lc 2 never gets a snapshot pointer and 0x01F46FAF derefs NULL.
  //   * the third pane could never draw, because the only
  //     CG_DrawActiveFrame call in the image is behind this bound.
  //
  // THREE immediates, one group. MSVC loop-unswitched the source loop
  // into two copies that SHARE the induction variable: copy A (head
  // 0x013E1150) exits at 0x013E11B0/0x013E11BD and falls through into
  // copy B at 0x013E11C6 WITHOUT resetting ebx. Widening only the tail
  // bound would cut the transition edge before lc 2 and copy B would
  // never see it. All three or none.
  //
  // Safe at 4 rather than 3 because the loop's own gate is our caved
  // IsActive (call 0x0283AA50 at 0x013E11D3), which clamps lc >=
  // cl_maxLocalClients - so a client the engine has not sized for is
  // refused before any per-client dereference. Stock IsActive has no
  // such clamp, which is why this may only run with the cave installed.
  // WHY THE FIRST ATTEMPT (build 6D16F5CA) GAVE ZERO PANES: it widened
  // only the frame loop, to 4, while r_num_viewports was still being
  // computed as 2 by a SECOND per-client loop in the same function -
  // 0x013DF820, which counts clients whose
  // clientUIActives[i].connectionState == 0xB (CA_ACTIVE) and hands the
  // total to Dvar_SetIntIfChanged(r_num_viewports) at 0x013E10CC:
  //
  //   013DF830  rax = [rbp-0x50] * 0x1078   ; clientUIActives index
  //   013DF83D  cmp [rax+rdi], 0xB          ; rdi = base+8 = state
  //   013DF847  add ebx, ecx                ; ebx = active count
  //   013E10B1  cmp r13d, 2                 ; THE VIEWPORT CAP
  //
  // PS4 runs that same body to 4 and passes the same count to
  // r_num_viewports (0x00427CBD). Widening the frame loop while the
  // renderer is still told "two viewports" is a mismatch, not a fix.
  //
  // THREE, NEVER FOUR. Both loops index clientUIActives at stride
  // 0x1078 and slot 3 (0x053DBD28..0x053DCD9F) swallows 0x053DC188 -
  // the clientActive base pointer CL_GetLocalClientGlobals loads. Slot 2
  // is real: every crash capture this session read it live at
  // 0x053DACB0 as flags 0x37, connectionState 0x0B (CA_ACTIVE).
  //
  // DEFAULT OFF - opt in with BO3_CG_FRAME=on. THE EVIDENCE IS IN:
  // three instrumented rounds, three DIFFERENT per-client resources, all
  // reached the moment the cgame frame ticks for local client 2:
  //
  //   1. 0x01F46FAF  cg[2].nextSnap NULL          -> snapguard cave
  //   2. 0x02C3DE7F  A[2]/C[2] NULL via 0x01C93850 -> buffer guard
  //   3. 0x02C3DE7F  again, DIFFERENT caller (0x0058B2FD, reached from
  //      CG_SetInitialSnapshot 0x00FC72BA). Guard verified installed and
  //      count still 2, so this path never went through 0x01C93850:
  //        0058B2DE  lea rdi,[0x047E3BA0 + lc*0x401C]  ; direct [2] array
  //        0058B2F8  memset(that, 0, 0x401C)           ; slot 2 = foreign
  //        0058B2FD  mov rcx,[0x032DF8B0 + lc*8]       ; ptr table, [2]
  //        0058B30D  memset(rcx, 0, 0xC400)            ; rdi held FLOATS
  //                  (0x4196000041960000) -> crash
  //
  // So this is a FAMILY, not a list of three: every step into cgame(2)
  // surfaces another per-client resource the PC only ever sized for two,
  // and guarding them one at a time only moves the fault. Finishing the
  // third pane means porting the whole per-client resource allocation
  // from 2 to N - relocating each array AND allocating each buffer - not
  // widening bounds. STATE.md carries the reference tables already built.
  //
  // Until that lands, OFF: the default build is the good one - three
  // players, full frame rate, stable rounds, two panes.
  //
  // Tested 2026-08-25 (build ABA93EC9) with the viewport count and
  // the frame loop widened TOGETHER - the missing half. It got
  // further than ever (cl_max held 3, seats 111, gate(2)=1) and then
  // died in map load at 0x02C3DE7F, a `rep stosb` with rdi = NULL:
  //
  //   00FC5905  call 0x01C93850(ecx = localClientNum)
  //   01C938FF    mov rcx,[rbx + rdi + 0x10615A60]  ; per-client ptr
  //   01C9390D    call memset(rcx, 0, 0x3800)       ; rcx = NULL
  //
  // registers: r14 = rsi = 2 (the local client), rax = rdi = 0, and
  // cgArray (0x04D17C80) also read 0 - this runs before the cgame
  // publish. 0x10615A60 is a per-client table of buffer POINTERS
  // and client 2's entry was never allocated.
  //
  // That is the honest shape of what remains: ticking the cgame
  // frame for client 2 walks into the whole per-client RESOURCE
  // family - buffers reached through pointer tables - which the PC
  // only ever allocates for two. Those are allocations to ADD, not
  // bounds to widen, and each needs its own measurement. Until then
  // the group stays off so the default build is the good one:
  // three players, full frame rate, stable rounds, two panes.
  char cg_frame_env[16] = {};
  GetEnvironmentVariableA("BO3_CG_FRAME", cg_frame_env, sizeof(cg_frame_env));
  if (isactive_caved && std::strcmp(cg_frame_env, "on") == 0) {
    // cg_frame_imms: namespace scope, shared with patch_probe_once().
    bool all_stock = true;
    for (const auto rva : cg_frame_imms) {
      const auto *at = reinterpret_cast<const uint8_t *>(base() + rva);
      if (!readable(at, 1) || *at != 0x02) {
        all_stock = false;
        break;
      }
    }
    if (all_stock) {
      const uint8_t bound3 = 0x03;
      uint32_t wrote = 0;
      for (const auto rva : cg_frame_imms) {
        auto *at = reinterpret_cast<uint8_t *>(base() + rva);
        if (write_bytes(at, &bound3, sizeof(bound3))) {
          ++wrote;
        }
      }
      if (wrote != std::size(cg_frame_imms)) {
        // all-or-nothing: put back whatever landed
        const uint8_t two = 0x02;
        for (const auto rva : cg_frame_imms) {
          write_bytes(reinterpret_cast<uint8_t *>(base() + rva), &two,
                      sizeof(two));
        }
      }
    }
  }

  // BOTH bounds are open and the allocation behind them is real, so
  // the deferred half of the work is finally complete. Only now does
  // the second latch close - until it does, every trigger site keeps
  // re-entering here, which is the whole point of splitting it out.
  lc2_widens_done = true;

  report87(1);
}

// ---- PLAYER 4: CL_Init(3) (2026-09-27, static analysis, untested) --------
//
// PS4 Com_Init runs CL_Init(lc) for lc 0..3 at boot (0xE49E98). The PC boots
// 0..1; run_cl_init_for_local_client2 supplies lc 2. This is the same call
// for lc 3, made the moment seat record 3 is in use (guest_signin_stub).
// PC CL_Init (0x013593EF), disassembled - what it writes for lc 3:
//   CL_GetLocalClientGlobals(3) memset   heap, NULL (skipped) until allocated
//   0x0214C790(3)                        clientObjMap row 3 - entword [4]
//   clientUIActives[3] +8 = 0            owned head of slot 3
//   SetBeingUsed(3, true)                seat record 3
//   byte [0x053D4988 + 3] = 0            cl_waitingOnServerToLoadMap[3] -
//                                        padding (find_lea: 4 base leas;
//                                        range_xref 0x053D498A..C: none)
//     behind /GS check 0x0135946B (cmp rbx, 2)
//   Cbuf_Execute(3, controller 3)        busy byte 0x1689DF2B (padding, the
//                                        next global starts at 0x1689DF30),
//                                        Cbuf records [4]
//     behind /GS check 0x020EC1B0 (cmp rbx, 2)
//   clientUIActives[3] +0 |= 2           the receipt, owned head
// Both checks are opened to 4 for exactly this one call and closed again
// (same scoping as lc 2); they are read back as stock 02 first.
bool cl_init3_done = false;

void run_cl_init_for_local_client3() {
  if (cl_init3_done || lc2_work_in_progress) {
    return;
  }
  const in_progress_guard guard(lc2_work_in_progress);
  auto *range_a = reinterpret_cast<uint8_t *>(base() + cl_init_range_imm_rva);
  auto *range_b =
      reinterpret_cast<uint8_t *>(base() + cbuf_execute_range_imm_rva);
  if (*range_a != 0x02 || *range_b != cbuf_range_resting) {
    return;
  }
  auto *flags =
      reinterpret_cast<volatile uint32_t *>(base() + 0x05359BC0 + 3 * 0x1078);
  if ((*flags & 0x2) == 0) {
    const uint8_t open = 0x04, shut = 0x02;
    const bool touch_b =
        cbuf_range_resting == 0x02; // see run_cl_init_for_local_client2
    const bool a_ok = write_bytes(range_a, &open, 1);
    const bool b_ok = !touch_b || write_bytes(range_b, &open, 1);
    if (a_ok && b_ok) {
      reinterpret_cast<void (*)(int)>(base() + cl_init_rva)(3);
    }
    if (a_ok) {
      write_bytes(range_a, &shut, 1);
    }
    if (b_ok && touch_b) {
      write_bytes(range_b, &shut, 1);
    }
  }
  cl_init3_done = (*flags & 0x2) != 0;
}

// Renderer pipeline. The count detour is asked only a handful of times
// in a lobby (measured on the 16:19 build: status 93 = 5 queries for a
// whole session, and the LAST of them answered 2 - i.e. it happened
// before the third sign-in, so a one-shot hung there never fires), and
// per_controller_update_stub stops running after a splitscreen sign-in
// (status 34 frozen, measured). The renderer loop is the one thing
// proven to keep ticking through a launch (STATE.md fix 2), which is
// exactly when this must already have happened.
//
// The trigger is the seat table read directly - bit 0 of each 0x24
// record, the same plain read the count detour uses, never the engine
// predicates (which answer wrongly off the async pipeline).
// --- SEATING PLAYER 3 THE WAY THE GAME DOES IT ----------------------
//
// `cl_init_watch` below waits for THREE seats, and until now nothing ever
// created the third. Forcing the seat bit by hand DOES create it - measured
// 2026-08-19: controller 2's storage went from 18 ready file slots to 52,
// done[] completed all four targets and Engine.GetCACRoot(2) stopped
// returning nil, so the gobblegum row has everything it needs. But the
// LOBBY is then unaware of the client and the next host attempt dies with
// "Failed to host lobby", the party showing player 1 + player 3. The bit
// is a CONSEQUENCE of a sign-in, never a switch - which is exactly what
// write_signin_seat's comment has always said.
//
// The console does the whole job in one function and the PC counterpart is
// 0x01E0C960 (PS4 Live_HandleClientSplitscreenSignin 0xC16080), read
// instruction for instruction:
//
//   01E0C97E  Com_ControllerIndex_GetLocalClientNum(ci)     -> edi = lc
//   01E0C9BD  Live_StartSigninAny(ci, false)
//   01E0C9C4  CL_ControllerIndex_GetSignInState(ci) < 1     -> undo and bail
//   01E0CA1C  LiveUser_IsUserGuest(ci, &sponsor)
//   01E0CA25  0x01E7EC60(ci): ci is a guest AND ctrl 0 is not
//   01E0CA3B  the SPONSOR's seat must be active
//   01E0CA44  Com_LocalClient_SetBeingUsed(lc, add)         <- the seat bit
//   01E0CA56  userData[ci].isActive = add
//   01E0CA69  offline/local ONLY: 0x01E24D50
//             CORRECTED 2026-09-26: this is NOT "the lobby half" - both
//             0x023345D0 and 0x01E24D50 are a bare `ret 0` on PC (the PS4
//             beta calls Voice_DisableLocalMics there, on removal only).
//             Enrolment into a lobby is ensure_guest2_game_lobby()'s job.
//
// It validates, seats AND registers together, which is the combination a
// single forced byte cannot produce.
//
// THE BOUND WIDEN IS LOAD-BEARING. With LiveUser_IsUserGuest still at
// `cmp edi,1`, controller 2 answers "not a guest" and 0x01E0CA23 jumps
// STRAIGHT to the seat write, skipping every validation above - i.e. it
// reproduces by itself the state that broke hosting. So this refuses to
// call unless the patch is verified in memory first.
constexpr uint32_t guest_signin_rva = 0x1DFFED0;
constexpr uint32_t is_user_guest_imm_rva = 0x1EBA642;
constexpr size_t userdata_is_guest = 0x29;
constexpr uint32_t guest_join_max_attempts = 8;
uint32_t guest_join_attempts = 0;
bool guest_join_done = false;

// Defined with the lobby API further down, where LobbyBase_GetNetworkMode
// and the game-lobby session accessor are verified.
bool offline_lobby_ready_for_player3();

uint8_t seat_flags(const uint32_t lc) {
  uint8_t f = 0;
  std::memcpy(&f,
              reinterpret_cast<const void *>(base() + signin_new_base +
                                             lc * signin_stride),
              sizeof(f));
  return f;
}

// Is CONTROLLER `controller` seated? Not the same as seat_flags(controller):
// the engine re-packs local clients at map load. Measured 2026-09-27
// 11:53: a round with host + player 3 (no player 2) left the records as
// {0: ctrl 0, 1: ctrl 2, 2: ctrl 1} - player 3 in record 1. The record's
// controller is at +8, exactly what Com_ControllerIndex_GetLocalClientNum
// 0x020EF7C0 compares.
bool controller_seated(const int32_t controller) {
  for (uint32_t i = 0; i < signin_new_slots; ++i) {
    const auto *record = reinterpret_cast<const uint8_t *>(
        base() + signin_new_base + i * signin_stride);
    int32_t owner = -1;
    std::memcpy(&owner, record + 8, sizeof(owner));
    if (owner == controller) {
      return (record[0] & 1) != 0;
    }
  }
  return false;
}

// s_gamePads is the one relocation that must NOT run at post_unpack.
// Moving it before the stock gamepad constructor finished crashed at
// 0x022E9550. tools/prepare_gamepad_join.py proved the safe point and the
// complete transaction in a live two-player lobby; this is the same
// transaction moved into the component so a fresh process needs no manual
// memory patching.
constexpr uint32_t gamepad_bound_rvas[] = {
    0x0228519B, 0x0228528B, 0x02286041, 0x02286449, 0x01FD7539, 0x01FD81CE,
};
constexpr size_t expected_gamepad_refs = 38;
bool gamepads_activated = false;
bool gamepads_activation_in_progress = false;

// WHICH DEVICE FEEDS WHICH SLOT (measured 2026-09-27, pid 34892).
//
// A gamepad record (0x70, gamepads_reloc_table: 0xE0 = two stock records,
// 0x1C0 = four) holds at +4 the index of the device it reads; +0 is
// GamePad.enabled (PS4 orbis_gamepad.cpp), refreshed every poll from that
// device (0x022F2A10). The device table at 0x17E6E2D0 (8 x {input index,
// owning slot}) is filled by the enumerate/assign pair the game runs on a
// device change - rescan 0x022F30E0 = enumerate 0x022F20D0 + assign
// 0x022F1AC0 (+ seat-model refresh 0x020EF8F0, 2 slots only), reached
// only through the pointer at 0x17E6E418. "No device" is 8: the slot
// reset writes it (0x022F2B90 `mov dword [rbx+4], 8`) and the assign
// loop tests for it (0x022F1AF8 `cmp eax, 8`).
//
// The relocation used to zero-fill slots 2/3, i.e. device index 0: live
// read slots {0,1,0,0}, device table {(0,0), (1,1), (2,-1)} - both new
// slots read the HOST's controller and the third pad fed nobody. So the
// new slots start as "no device" and the game's own rescan runs once, so
// every connected controller is assigned the way a device change would
// (third pad -> slot 2); a controller plugged in later goes through the
// same rescan by itself.
constexpr size_t gamepad_stride = 0x70;
constexpr size_t gamepad_device_index = 0x4;
constexpr int32_t gamepad_no_device = 8;
static_assert(gamepads_reloc_table.old_size == 2 * gamepad_stride);
static_assert(gamepads_reloc_table.new_size == 4 * gamepad_stride);
constexpr uint32_t gamepad_rescan_rva = 0x2286010;
constexpr uint8_t gamepad_rescan_bytes[] = {
    0x48, 0x83, 0xEC, 0x28,       // sub rsp, 28h
    0xE8, 0xE7, 0xEF, 0xFF, 0xFF, // call 0x022F20D0 enumerate
    0xE8, 0xD2, 0xE9, 0xFF, 0xFF, // call 0x022F1AC0 assign
    0x48, 0x83, 0xC4, 0x28,       // add rsp, 28h
    0xE9, 0x49, 0xD1, 0xE5, 0xFF, // jmp 0x020EF8F0
};

// Does controller slot `slot` have a connected device right now? Only
// meaningful once the table is relocated (the stock array has 2 slots).
bool gamepad_connected(const size_t slot) {
  if (!gamepads_activated || slot >= 4) {
    return false;
  }
  return *reinterpret_cast<const volatile uint8_t *>(
             base() + gamepads_reserved_rva + slot * gamepad_stride) != 0;
}

int32_t gamepad_device_of(const size_t slot) {
  return *reinterpret_cast<const volatile int32_t *>(
      base() + gamepads_reserved_rva + slot * gamepad_stride +
      gamepad_device_index);
}

// Defined after the trace helpers: one line with every slot's device.
void log_gamepad_slots(const char *what);

// DEVICE-TYPE SELECTOR, decoupled from the widened loop bound
// (2026-09-27). Both gamepad loops (poll 0x022F2A10, per-frame update
// 0x022F2E20) hold their bound in a register that the compiler ALSO uses
// as the constant 2 of the device-type selector:
//     lea ebp/esi, [rdi+2]              bound AND "type 2"
//     xor ecx,ecx; lea eax,[rdx-4]; cmp eax,3; cmovbe ecx,ebp/esi
// so widening the bound to 4 (gamepad_bound_rvas) made devices 4..7 -
// the non-XInput API (0x02C36430) - type 4, which no branch handles: those
// pads were never read. The same 11 bytes, rewritten without the
// register (both ends are jump targets - `ja` into the start, `jmp` to
// the `dec ecx` after it - so the length must not change):
//     lea eax,[rdx-4]; cmp eax,4; sbb ecx,ecx; and ecx,2
// = 2 for devices 4..7, else 0, exactly the stock result, whatever the
// bound. Valid with the stock bound too, so it is applied at startup.
// The per-slot refresh 0x022F22DE already uses a literal `mov r8d,2`.
struct type_selector_site {
  uint32_t rva;
  uint8_t expected[11];
};
constexpr type_selector_site gamepad_type_selector_sites[] = {
    {0x02286066,
     {0x33, 0xC9, 0x8D, 0x42, 0xFC, 0x83, 0xF8, 0x03, 0x0F, 0x46, 0xCD}},
    {0x0228646A,
     {0x33, 0xC9, 0x8D, 0x42, 0xFC, 0x83, 0xF8, 0x03, 0x0F, 0x46, 0xCE}},
};
constexpr uint8_t gamepad_type_selector_fixed[11] = {
    0x8D, 0x42, 0xFC, // lea eax, [rdx-4]
    0x83, 0xF8, 0x04, // cmp eax, 4
    0x1B, 0xC9,       // sbb ecx, ecx
    0x83, 0xE1, 0x02, // and ecx, 2
};

void fix_gamepad_type_selectors() {
  for (const auto &site : gamepad_type_selector_sites) {
    auto *p = reinterpret_cast<uint8_t *>(base() + site.rva);
    if (readable(p, sizeof(site.expected)) &&
        std::memcmp(p, site.expected, sizeof(site.expected)) == 0) {
      write_bytes(p, gamepad_type_selector_fixed,
                  sizeof(gamepad_type_selector_fixed));
    }
  }
}

int32_t gamepad_ref_value(const reloc_ref &r, const size_t destination_rva) {
  const auto moved =
      destination_rva + (r.target_rva - gamepads_reloc_table.base_rva);
  return r.rip_relative ? static_cast<int32_t>(moved - (r.insn_rva + r.length))
                        : static_cast<int32_t>(moved);
}

int32_t gamepad_original_ref_value(const reloc_ref &r) {
  return r.rip_relative
             ? static_cast<int32_t>(r.target_rva - (r.insn_rva + r.length))
             : static_cast<int32_t>(r.target_rva);
}

bool gamepad_refs_match(const size_t destination_rva) {
  for (size_t i = 0; i < gamepads_reloc_table.count; ++i) {
    const auto &r = gamepads_reloc_table.refs[i];
    const auto *field =
        reinterpret_cast<const int32_t *>(base() + r.insn_rva + r.disp_offset);
    if (!readable(field, sizeof(*field)) ||
        *field != gamepad_ref_value(r, destination_rva)) {
      return false;
    }
  }
  return true;
}

bool gamepad_bounds_match(const uint8_t expected) {
  for (const auto rva : gamepad_bound_rvas) {
    if (*reinterpret_cast<const uint8_t *>(base() + rva) != expected) {
      return false;
    }
  }
  return true;
}

// WHEN the table may move (2026-09-27, "player 3 can't join before player 2").
//
// Console: s_gamePads is [4] from boot, every pad is polled, any controller
// joins first. Here slots 2/3 only exist after this relocation, and it used
// to wait for TWO seats - the state tools/prepare_gamepad_join.py was proven
// in (LOG 2026-08-19), not a requirement of the transaction:
//   * the one hard precondition is "controller 2 not signed in yet" - his
//     sign-in reads his slot (0x022F27D0 presence), so the table must move
//     first; seats 0 and 1 were never read by it;
//   * the "crashes if done early" family (0x022E9550) was controller 2's
//     dangling key-binding strings - playerKeys is relocated since
//     (LOG "playerKeys ... 70/70") - and it was at post_unpack, not in a lobby;
//   * the six widened loops: poll / per-frame update / assign (pads only),
//     0x01FE35C0 (count connected unused controllers) and 0x01FE4260
//     (GetUsedControllerCount) - none needs any seat.
// Player 3 alone with the host is then seats {0, 2}: the state already
// verified when player 2 leaves by B. At START GAME PS4
// LobbyLaunch_PreloadMap runs CL_SetupClientsForIngame
// (Com_LocalClients_CompressClients, 0xCC3FFE) BEFORE
// CL_AllocatePerLocalClientMemory (0xCC400C): controller 2 is packed into
// lc 1 before the per-client memory is sized, so no lc 2 exists in that
// round (measured before as records {c0, c2, c1}).
// Offline lobby only, as for player 3 himself - never at the LIVE main menu.
bool gamepads_may_activate() {
  if (!(seat_flags(0) & 1)) {
    return false;
  }
  if (seat_flags(1) & 1) {
    return true; // the original, proven point
  }
  return game::com::Com_IsRunningUILevel() && offline_lobby_ready_for_player3();
}

void complete_gamepads(
    size_t destination_abs); // defined with the other completions

bool activate_gamepads_in_lobby() {
  if (gamepads_activated) {
    return true;
  }
  if (gamepads_activation_in_progress || !gamepads_reserved_rva ||
      gamepads_reloc_table.count != expected_gamepad_refs) {
    return false;
  }

  if (!gamepads_may_activate()) {
    return false;
  }

  const in_progress_guard guard(gamepads_activation_in_progress);
  const auto destination_rva = gamepads_reserved_rva;

  // A second entry after a successful transaction is harmless and does no
  // writes. This also recognises a process prepared by the external proof
  // tool, which is useful while old and new test procedures overlap.
  if (gamepad_refs_match(destination_rva) && gamepad_bounds_match(4)) {
    gamepads_activated = true;
    set_status(58, static_cast<uint32_t>(expected_gamepad_refs));
    set_status(59, static_cast<uint32_t>(std::size(gamepad_bound_rvas)));
    return true;
  }

  // Dry-run every reference before the first write. A partial previous
  // relocation or an unexpected build fails closed.
  for (size_t i = 0; i < gamepads_reloc_table.count; ++i) {
    const auto &r = gamepads_reloc_table.refs[i];
    const auto *field =
        reinterpret_cast<const int32_t *>(base() + r.insn_rva + r.disp_offset);
    if (!readable(field, sizeof(*field)) ||
        *field != gamepad_original_ref_value(r)) {
      return false;
    }
  }

  std::array<uint8_t, std::size(gamepad_bound_rvas)> old_bounds{};
  for (size_t i = 0; i < old_bounds.size(); ++i) {
    old_bounds[i] =
        *reinterpret_cast<const uint8_t *>(base() + gamepad_bound_rvas[i]);
    if (old_bounds[i] != 2 && old_bounds[i] != 4) {
      return false;
    }
  }

  auto *destination = reinterpret_cast<uint8_t *>(base() + destination_rva);
  std::memcpy(
      destination,
      reinterpret_cast<const void *>(base() + gamepads_reloc_table.base_rva),
      gamepads_reloc_table.old_size);
  std::memset(destination + gamepads_reloc_table.old_size, 0,
              gamepads_reloc_table.new_size - gamepads_reloc_table.old_size);
  // New slots own no device yet (see gamepad_no_device) - a zero here
  // would make them read device 0, the host's controller.
  for (size_t slot = gamepads_reloc_table.old_size / gamepad_stride;
       slot < gamepads_reloc_table.new_size / gamepad_stride; ++slot) {
    std::memcpy(destination + slot * gamepad_stride + gamepad_device_index,
                &gamepad_no_device, sizeof(gamepad_no_device));
  }

  size_t changed_refs = 0;
  size_t changed_bounds = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < changed_bounds; ++i) {
      write_bytes(reinterpret_cast<void *>(base() + gamepad_bound_rvas[i]),
                  &old_bounds[i], sizeof(old_bounds[i]));
    }
    for (size_t i = 0; i < changed_refs; ++i) {
      const auto &r = gamepads_reloc_table.refs[i];
      const auto original = gamepad_original_ref_value(r);
      write_bytes(reinterpret_cast<void *>(base() + r.insn_rva + r.disp_offset),
                  &original, sizeof(original));
    }
    std::memset(destination, 0, gamepads_reloc_table.new_size);
  };

  for (size_t i = 0; i < gamepads_reloc_table.count; ++i) {
    const auto &r = gamepads_reloc_table.refs[i];
    const auto moved = gamepad_ref_value(r, destination_rva);
    if (!write_bytes(
            reinterpret_cast<void *>(base() + r.insn_rva + r.disp_offset),
            &moved, sizeof(moved))) {
      rollback();
      return false;
    }
    ++changed_refs;
  }

  for (size_t i = 0; i < old_bounds.size(); ++i) {
    const uint8_t four = 4;
    if (old_bounds[i] != four &&
        !write_bytes(reinterpret_cast<void *>(base() + gamepad_bound_rvas[i]),
                     &four, sizeof(four))) {
      rollback();
      return false;
    }
    ++changed_bounds;
  }

  if (!gamepad_refs_match(destination_rva) || !gamepad_bounds_match(4)) {
    rollback();
    return false;
  }

  gamepads_activated = true;
  set_status(58, static_cast<uint32_t>(expected_gamepad_refs));
  set_status(59, static_cast<uint32_t>(std::size(gamepad_bound_rvas)));
  complete_gamepads(base() + destination_rva);

  // Hand the connected controllers to the new slots exactly as a device
  // change would. Unverified bytes: skip - slots 2/3 then wait for the
  // next real device change instead.
  const auto *rescan =
      reinterpret_cast<const void *>(base() + gamepad_rescan_rva);
  if (readable(rescan, sizeof(gamepad_rescan_bytes)) &&
      std::memcmp(rescan, gamepad_rescan_bytes, sizeof(gamepad_rescan_bytes)) ==
          0) {
    reinterpret_cast<void (*)()>(base() + gamepad_rescan_rva)();
    log_gamepad_slots("gamepads relocated rescan=1");
  } else {
    log_gamepad_slots("gamepads relocated rescan=bytes-mismatch");
  }
  return true;
}

// Breadcrumbs go through report87 (codes 40..46) rather than new status
// slots: 11+i, 16+i and 37+i are computed indices and the block only
// protects 0..95, so a fresh literal slot is not safe to assume free.
void try_join_guest2() {
  if (guest_join_done || guest_join_attempts >= guest_join_max_attempts) {
    return;
  }
  // DIAGNOSTIC, opt-in: BO3_GUESTS=1 leaves seat 2 empty, for a true
  // two-player round on the same build (2026-09-26 A/B test of the ~15 s
  // Arxan deaths: patches vs. player 3's code). With fewer than three
  // virtual pads the component still seated a third player on its own.
  {
    static int guests_env = -1;
    if (guests_env < 0) {
      char buf[8] = {};
      GetEnvironmentVariableA("BO3_GUESTS", buf, sizeof(buf));
      guests_env = std::strcmp(buf, "1") == 0 ? 1 : 2;
    }
    if (guests_env == 1) {
      return;
    }
  }
  if (!signin_relocated || !guests_filled || !guest_array_rva) {
    report87(40);
    return;
  }

  // THREE OR MORE LOCAL PLAYERS ARE OFFLINE-ONLY - wait for the local lobby.
  //
  // Test report 2026-09-26: ACTIVATE SPLITSCREEN at the MAIN MENU, then
  // Zombies offline, and the lobby loops (round_monitor_20260926_075157:
  // the gamesettings block ran at 07:59:43, :54 and 08:00:03, the loop
  // was reported at 08:00:33, the game quit at 08:00:43).
  // Measured at that main menu: LobbyBase_GetNetworkMode = 2 (LIVE), game
  // lobby session +0x40 = 0. This function used to fire the moment the
  // native guest sat down, i.e. right there, in the LIVE party. For a
  // guest 0x01E0C960 seats the same way in LIVE as in LOCAL (0x01E0C9EC
  // skips the Live sign-in for guests), but ensure_guest2_game_lobby()
  // can only enrol into the GAME lobby, which does not exist yet - so the
  // engine counted a local client that no lobby knew. That is the state
  // that made hosting fail before ("Failed to host lobby", CLOSED DEAD
  // END: forcing clientGameStates); the two-player early-activation test
  // is what confirms it is this loop.
  //
  // Treyarch caps LIVE at two local players on purpose - PC
  // SplitscreenShouldBeOnline 0x0283AC60 is unconditionally true only for
  // one or two. So player 3 waits, without spending an attempt, until the
  // lobby is not LIVE and the game lobby is up: the state the working
  // PRIVATE GAME -> ACTIVATE SPLITSCREEN path always had when this ran
  // (measured there 2026-09-26: lobby mode 1 = LAN, session "systemlink",
  // game lobby +0x40 = 2).
  if (!offline_lobby_ready_for_player3()) {
    report87(54);
    return;
  }

  // Never call blind - verify the widen landed, the same way every other
  // patch here verifies its bytes before trusting them.
  uint8_t bound = 0;
  std::memcpy(&bound,
              reinterpret_cast<const void *>(base() + is_user_guest_imm_rva),
              sizeof(bound));
  if (bound != 0x03) {
    report87(41);
    return;
  }

  // The host must be seated and seat 2 still free. Player 2 need not be
  // there (console: any local player joins and leaves on his own - he
  // may have left before player 3 joins); when he IS seated,
  // offline_lobby_ready_for_player3 already made sure he is a lobby
  // member first. 0x01E0CA3B checks the sponsor's seat itself.
  if (!controller_seated(0) || controller_seated(2)) {
    report87(42);
    return;
  }

  // Controller 2 must already be marked a guest. Measured as 1 in the
  // filled record; this only READS it, it never invents it - if it is 0
  // the call would take the not-a-guest branch and seat him unvalidated.
  uint8_t is_guest = 0;
  std::memcpy(&is_guest,
              reinterpret_cast<const void *>(base() + guest_array_rva +
                                             2 * client_ui_stride +
                                             userdata_is_guest),
              sizeof(is_guest));
  if (!is_guest) {
    report87(43);
    return;
  }

  // No controller on slot 2, no player 3 - and no attempt spent. The
  // sign-in itself would refuse (CL_ControllerIndex_GetSignInState reads
  // this same enabled byte, LOG "0x022F27D0 IS A GAMEPAD PRESENCE READ"),
  // and this runs every frame (Live_Frame 0x0135C8A4 -> 0x01E26570), so
  // eight refusals would burn the attempts before a controller plugged
  // in later could ever be used. Plugging one in reruns the game's
  // rescan, slot 2 gets it, and the next frame seats him.
  if (!gamepad_connected(2)) {
    report87(56);
    return;
  }

  ++guest_join_attempts;
  report87(44);

  using signin_fn = void (*)(int, bool, bool);
  reinterpret_cast<signin_fn>(base() + guest_signin_rva)(2, true, false);

  // The seat bit is the function's own output, so it is also the honest
  // test of whether it worked. Latch only on success; otherwise let the
  // bounded retries run, because the sign-in state may simply not be up
  // yet on this pass. By controller, not record: after a round without
  // player 2 the engine keeps controller 2 in record 1.
  if (controller_seated(2)) {
    guest_join_done = true;
    report87(45);
  } else {
    report87(46);
  }
}

// ============ THE PANE FIX (PLAN_3-4_SCREENS.md Phases 1-3) ==========
//
// Three players rendered TWO panes. RULE ZERO showed the PC caps the
// pane count in FOUR independent places, which is why every count-only
// attempt failed:
//
//   PS4 CL_SetupScreenPlacements (0x3EC820)
//         total = CL_LocalClient_GetActiveCount()      loop i < 4
//         for lc in 0..3: if IsActive(lc):
//             CG_SetView(lc, paneIdx++, total, NULL)
//       CG_SetView -> CG_GetLocalClientViewParams(lc, pane, total)
//         params = clientViewParamsArray
//                  + wide*0x100 + (total-1)*0x40 + pane*0x10
//
//   PC dispatcher 0x0132E298 runs the SAME algorithm but:
//     C1 GetActiveCount 0x0283AA30 is inline-unrolled to two elements
//     C2 the pane loop is `cmp ebx,2` at 0x0132E2C4 (+3 siblings)
//     C3 clientUIActives is [2] and slot 2 (0x053DACB0) is the BASE of
//        the next foreign per-client array - no storage for pane 2
//     C4 the geometry table 0x032E56F0 is [2][2][2]: THE 3- AND 4-PANE
//        ROWS WERE CUT FROM THE PC DATA. total=3 indexes into FOV
//        constants, so even a correct count would draw garbage.
//
// C4 is the one that makes this a data problem, not a bounds problem.

// ---- Phase 1: clientUIActives [2] -> [4] (the storage) -------------
//
// DWARF names it clientUIActive_t[4] x 0x1078 on PS4 and the PC stride
// is IDENTICAL (imul reg,reg,0x1078 at PC 0x0283AA5A / 0x0090E3E7 and
// PS4 0x15169F2), so this is a flat relocation with no layout change -
// the same lucky case as cg_localEntities' 0x7C00.
//
// 250 references from uiactives_reloc.validated.txt: 142 rip-relative
// (boundary-correct by construction) and 108 ABS32, each proved by
// exhaustive start-candidate search with a clean predecessor chain.
// Every remaining raw disp32 position in real code (16) was decoded
// individually and shown to be a coincidence inside an unrelated
// instruction - four of them `imul rax,rax,0x53D9619`, where the value
// is an arithmetic immediate rather than a displacement.
//
// The 12 loop-END sentinels that point AT 0x053DACB0 are deliberately
// NOT in the table; widen_client_ui_walker_bounds() owns them and is
// updated below to aim at new_base + 4*0x1078.
constexpr uint32_t uia_base_rva = 0x5359BC0;
constexpr uint32_t uia_stride = 0x1078;
constexpr uint32_t uia_old_size = 2 * uia_stride; // 0x20F0
constexpr uint32_t uia_new_size = 4 * uia_stride; // 0x41E0

struct uia_ref {
  uint32_t rva;
  uint8_t len;
  uint8_t disp_off;
  uint32_t delta; // byte offset within the OLD array
  bool rip;       // true = rip-relative, false = ABS32 off the image base
  uint8_t expected[12];
};

constexpr uia_ref uia_refs[] = {
    {0xAB105, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0xB4, 0xEA, 0x2A, 0x05}},
    {0x425961, 7, 3, 0x4, true, {0x48, 0x8D, 0x15, 0x5C, 0x42, 0xF3, 0x04}},
    {0x4F5B16,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x30, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x56C1DD, 7, 3, 0x0, false, {0x8B, 0x84, 0x08, 0xC0, 0x9B, 0x35, 0x05}},
    {0x58F1BA,
     8,
     4,
     0x10,
     false,
     {0x46, 0x39, 0x84, 0x19, 0xD0, 0x9B, 0x35, 0x05}},
    {0x5DB390,
     8,
     3,
     0x10,
     false,
     {0x83, 0xBC, 0x10, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x5DCC30, 7, 3, 0x10, true, {0x48, 0x8D, 0x0D, 0x99, 0xCF, 0xD7, 0x04}},
    {0x5DFF8D, 7, 3, 0x10, true, {0x48, 0x8D, 0x0D, 0x3C, 0x9C, 0xD7, 0x04}},
    {0x6BD277,
     8,
     4,
     0x10,
     false,
     {0x44, 0x39, 0xBC, 0x10, 0xD0, 0x9B, 0x35, 0x05}},
    {0x6BEC8A, 7, 3, 0x0, false, {0x8B, 0x84, 0x10, 0xC0, 0x9B, 0x35, 0x05}},
    {0x6C267A, 7, 3, 0x0, false, {0x8B, 0x84, 0x10, 0xC0, 0x9B, 0x35, 0x05}},
    {0x6C405A, 7, 3, 0x0, false, {0x8B, 0x84, 0x10, 0xC0, 0x9B, 0x35, 0x05}},
    {0x6CCDB0,
     8,
     3,
     0x10,
     false,
     {0x83, 0xBC, 0x19, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x904898, 7, 3, 0x18, true, {0x48, 0x8D, 0x05, 0x39, 0x53, 0xA5, 0x04}},
    {0x90E3E0, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0xD9, 0xB7, 0xA4, 0x04}},
    {0x91F048, 7, 3, 0x18, true, {0x48, 0x8D, 0x05, 0x89, 0xAB, 0xA3, 0x04}},
    {0x9541FF,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x10, 0xC8, 0x9B, 0x35, 0x05, 0x0A}},
    {0x989244,
     8,
     4,
     0x10,
     false,
     {0x44, 0x39, 0xB4, 0x08, 0xD0, 0x9B, 0x35, 0x05}},
    {0xA8446B, 7, 3, 0x10, true, {0x48, 0x8D, 0x0D, 0x5E, 0x57, 0x8D, 0x04}},
    {0xFC722C,
     9,
     4,
     0x10,
     false,
     {0x42, 0x83, 0xBC, 0x2B, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0xFC7249,
     9,
     4,
     0x10,
     false,
     {0x42, 0x83, 0xBC, 0x2B, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0xFC7296,
     8,
     4,
     0x10,
     false,
     {0x42, 0x8B, 0x8C, 0x2B, 0xD0, 0x9B, 0x35, 0x05}},
    {0xFC72A5,
     12,
     4,
     0x10,
     false,
     {0x42, 0xC7, 0x84, 0x2B, 0xD0, 0x9B, 0x35, 0x05, 0x00, 0x00, 0x00, 0x00}},
    {0xFEF2F1, 7, 3, 0x10, true, {0x48, 0x8D, 0x0D, 0xD8, 0xA8, 0x36, 0x04}},
    {0x101F218, 7, 3, 0x0, false, {0x8B, 0x84, 0x38, 0xC0, 0x9B, 0x35, 0x05}},
    {0x102EDF4, 7, 3, 0x0, false, {0x8B, 0x84, 0x18, 0xC0, 0x9B, 0x35, 0x05}},
    {0x104C2C0,
     9,
     4,
     0x4,
     false,
     {0x42, 0xF6, 0x84, 0x38, 0xC4, 0x9B, 0x35, 0x05, 0x10}},
    {0x10D3827,
     8,
     4,
     0x10,
     false,
     {0x42, 0x39, 0xB4, 0x30, 0xD0, 0x9B, 0x35, 0x05}},
    {0x1134436,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x30, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x113F960,
     8,
     4,
     0x0,
     false,
     {0x42, 0x8B, 0x84, 0x38, 0xC0, 0x9B, 0x35, 0x05}},
    {0x11951D4,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x10, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x11D8E8D,
     9,
     4,
     0x8,
     false,
     {0x42, 0x83, 0xBC, 0x30, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x11F20F7,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x10, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x122D0E0,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x10, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x12F3A92,
     9,
     4,
     0x8,
     false,
     {0x41, 0x83, 0xBC, 0x2E, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x12F6F43, 7, 3, 0x8, true, {0x48, 0x8D, 0x15, 0x7E, 0x2C, 0x06, 0x04}},
    {0x12F6F7F, 7, 3, 0x0, true, {0x4C, 0x8D, 0x3D, 0x3A, 0x2C, 0x06, 0x04}},
    {0x12F70C6,
     8,
     4,
     0x0,
     false,
     {0x42, 0x8B, 0x84, 0x30, 0xC0, 0x9B, 0x35, 0x05}},
    {0x12F766E,
     9,
     4,
     0x8,
     false,
     {0x42, 0x83, 0xBC, 0x30, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x12F76A6,
     9,
     4,
     0x8,
     false,
     {0x42, 0x83, 0xBC, 0x30, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x12FF3F9, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0xC8, 0xA7, 0x05, 0x04}},
    {0x12FF8FF, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0xBA, 0xA2, 0x05, 0x04}},
    {0x12FFFDA, 7, 3, 0x0, true, {0x48, 0x8D, 0x15, 0xDF, 0x9B, 0x05, 0x04}},
    {0x1300204,
     8,
     4,
     0x0,
     false,
     {0x41, 0x8B, 0x84, 0x3A, 0xC0, 0x9B, 0x35, 0x05}},
    {0x1301A17,
     9,
     4,
     0x4,
     false,
     {0x41, 0xF6, 0x84, 0x3A, 0xC4, 0x9B, 0x35, 0x05, 0x18}},
    {0x1301F39, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x88, 0x7C, 0x05, 0x04}},
    {0x1308B42,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x11, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x131B3B7, 7, 3, 0x8, true, {0x48, 0x8D, 0x15, 0x0A, 0xE8, 0x03, 0x04}},
    {0x131B729,
     9,
     4,
     0x8,
     false,
     {0x42, 0x83, 0xBC, 0x00, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x131B8A9,
     9,
     4,
     0x8,
     false,
     {0x42, 0x83, 0xBC, 0x00, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x131B9D2,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x11, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x131BC07,
     9,
     4,
     0x8,
     false,
     {0x42, 0x83, 0xBC, 0x38, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x131BE2F,
     9,
     4,
     0x8,
     false,
     {0x42, 0x83, 0xBC, 0x38, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x131C096,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x30, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x131C246, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x7B, 0xD9, 0x03, 0x04}},
    {0x131C362,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x11, 0xC8, 0x9B, 0x35, 0x05, 0x07}},
    {0x131DCA9, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x18, 0xBF, 0x03, 0x04}},
    {0x1320EEC, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0xD5, 0x8C, 0x03, 0x04}},
    {0x1326EA3,
     12,
     4,
     0x8,
     false,
     {0x41, 0xC7, 0x84, 0x3F, 0xC8, 0x9B, 0x35, 0x05, 0x0A, 0x00, 0x00, 0x00}},
    {0x1327001,
     8,
     4,
     0x8,
     false,
     {0x41, 0x8B, 0x9C, 0x0F, 0xC8, 0x9B, 0x35, 0x05}},
    {0x13270B9,
     9,
     4,
     0x8,
     false,
     {0x41, 0x83, 0xBC, 0x1F, 0xC8, 0x9B, 0x35, 0x05, 0x0A}},
    {0x13270F3,
     8,
     4,
     0x10,
     false,
     {0x41, 0x39, 0xBC, 0x1F, 0xD0, 0x9B, 0x35, 0x05}},
    {0x132A63F,
     9,
     4,
     0x8,
     false,
     {0x41, 0x83, 0xBC, 0x07, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x132E31F, 7, 3, 0x1078, true, {0x48, 0x8D, 0x1D, 0x12, 0xC9, 0x02, 0x04}},
    {0x1333123, 7, 3, 0x8, true, {0x48, 0x8D, 0x15, 0x9E, 0x6A, 0x02, 0x04}},
    {0x13392AE, 7, 3, 0x0, false, {0x8B, 0x84, 0x38, 0xC0, 0x9B, 0x35, 0x05}},
    {0x133DF11, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0xA8, 0xBC, 0x01, 0x04}},
    {0x133F0E7, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0xD2, 0xAA, 0x01, 0x04}},
    {0x133F28C,
     8,
     4,
     0x8,
     false,
     {0x43, 0x8B, 0x84, 0x06, 0xC8, 0x9B, 0x35, 0x05}},
    {0x133F317,
     8,
     4,
     0x4,
     false,
     {0x41, 0x8B, 0x94, 0x06, 0xC4, 0x9B, 0x35, 0x05}},
    {0x133F33E,
     9,
     4,
     0x4,
     false,
     {0x41, 0xF6, 0x84, 0x06, 0xC4, 0x9B, 0x35, 0x05, 0x08}},
    {0x133F3BE,
     8,
     4,
     0x4,
     false,
     {0x45, 0x8B, 0x8C, 0x06, 0xC4, 0x9B, 0x35, 0x05}},
    {0x133F683,
     8,
     4,
     0x8,
     false,
     {0x43, 0x8B, 0x84, 0x06, 0xC8, 0x9B, 0x35, 0x05}},
    {0x133F712,
     8,
     4,
     0x4,
     false,
     {0x41, 0x8B, 0x94, 0x06, 0xC4, 0x9B, 0x35, 0x05}},
    {0x133F738,
     9,
     4,
     0x4,
     false,
     {0x41, 0xF6, 0x84, 0x06, 0xC4, 0x9B, 0x35, 0x05, 0x08}},
    {0x133F7B9,
     8,
     4,
     0x4,
     false,
     {0x45, 0x8B, 0x8C, 0x06, 0xC4, 0x9B, 0x35, 0x05}},
    {0x133FA73, 7, 3, 0x0, true, {0x48, 0x8D, 0x15, 0x46, 0xA1, 0x01, 0x04}},
    {0x133FB41,
     8,
     4,
     0x8,
     false,
     {0x42, 0x8B, 0x84, 0x2F, 0xC8, 0x9B, 0x35, 0x05}},
    {0x133FEE3,
     8,
     4,
     0x8,
     false,
     {0x4C, 0x63, 0xB4, 0x03, 0xC8, 0x9B, 0x35, 0x05}},
    {0x133FF4E, 7, 3, 0x4, false, {0x8B, 0x94, 0x3B, 0xC4, 0x9B, 0x35, 0x05}},
    {0x133FF68,
     8,
     3,
     0x4,
     false,
     {0xF6, 0x84, 0x3B, 0xC4, 0x9B, 0x35, 0x05, 0x08}},
    {0x133FFD3, 7, 3, 0x4, false, {0x8B, 0x8C, 0x03, 0xC4, 0x9B, 0x35, 0x05}},
    {0x1340889, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x38, 0x93, 0x01, 0x04}},
    {0x1340DF3, 7, 3, 0x0, true, {0x48, 0x8D, 0x2D, 0xC6, 0x8D, 0x01, 0x04}},
    {0x1342308,
     8,
     3,
     0x4,
     false,
     {0xF6, 0x84, 0x10, 0xC4, 0x9B, 0x35, 0x05, 0x01}},
    {0x1342380, 7, 3, 0x4, false, {0x8B, 0x84, 0x18, 0xC4, 0x9B, 0x35, 0x05}},
    {0x1343E2D,
     8,
     4,
     0x4,
     false,
     {0x42, 0x8B, 0x8C, 0x0B, 0xC4, 0x9B, 0x35, 0x05}},
    {0x1343ECA,
     8,
     4,
     0x8,
     false,
     {0x42, 0x8B, 0x84, 0x0B, 0xC8, 0x9B, 0x35, 0x05}},
    {0x1343F81,
     8,
     4,
     0x4,
     false,
     {0x44, 0x8B, 0x8C, 0x03, 0xC4, 0x9B, 0x35, 0x05}},
    {0x1344043,
     8,
     4,
     0x4,
     false,
     {0x44, 0x8B, 0x8C, 0x01, 0xC4, 0x9B, 0x35, 0x05}},
    {0x13441CA,
     8,
     3,
     0x4,
     false,
     {0xF6, 0x84, 0x08, 0xC4, 0x9B, 0x35, 0x05, 0x08}},
    {0x13442AD,
     8,
     3,
     0x4,
     false,
     {0xF6, 0x84, 0x19, 0xC4, 0x9B, 0x35, 0x05, 0x01}},
    {0x134527E, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x3B, 0x49, 0x01, 0x04}},
    {0x1345831, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x88, 0x43, 0x01, 0x04}},
    {0x1345C88, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x31, 0x3F, 0x01, 0x04}},
    {0x134710B, 7, 3, 0x4, true, {0x48, 0x8D, 0x0D, 0xB2, 0x2A, 0x01, 0x04}},
    {0x1347598, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x21, 0x26, 0x01, 0x04}},
    {0x13479B8, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x01, 0x22, 0x01, 0x04}},
    {0x13481C7, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0xF2, 0x19, 0x01, 0x04}},
    {0x134B86A, 7, 3, 0x8, true, {0x44, 0x89, 0x25, 0x57, 0xE3, 0x00, 0x04}},
    {0x134B871, 7, 3, 0x1080, true, {0x44, 0x89, 0x25, 0xC8, 0xF3, 0x00, 0x04}},
    {0x134B900, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0xB9, 0xE2, 0x00, 0x04}},
    {0x134B9EB,
     8,
     4,
     0x8,
     false,
     {0x41, 0x8B, 0x9C, 0x05, 0xC8, 0x9B, 0x35, 0x05}},
    {0x134BE8E,
     8,
     4,
     0x0,
     false,
     {0x43, 0x8B, 0x84, 0x35, 0xC0, 0x9B, 0x35, 0x05}},
    {0x134BEF8,
     9,
     4,
     0x10,
     false,
     {0x43, 0x83, 0xBC, 0x35, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x134C062, 7, 3, 0x0, true, {0x4C, 0x8D, 0x3D, 0x57, 0xDB, 0x00, 0x04}},
    {0x134C529, 7, 3, 0x8, true, {0x48, 0x8D, 0x15, 0x98, 0xD6, 0x00, 0x04}},
    {0x134C6BB, 7, 3, 0x0, true, {0x48, 0x8D, 0x1D, 0xFE, 0xD4, 0x00, 0x04}},
    {0x134C707, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0xB2, 0xD4, 0x00, 0x04}},
    {0x134CB56, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x6B, 0xD0, 0x00, 0x04}},
    {0x134CBC7, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0xFA, 0xCF, 0x00, 0x04}},
    {0x134CCEF, 7, 3, 0x8, true, {0x48, 0x8D, 0x35, 0xD2, 0xCE, 0x00, 0x04}},
    {0x134CE6E, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x4B, 0xCD, 0x00, 0x04}},
    {0x134D05C, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x5D, 0xCB, 0x00, 0x04}},
    {0x134D2C8, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0xF1, 0xC8, 0x00, 0x04}},
    {0x134D320, 7, 3, 0x0, true, {0x4C, 0x8D, 0x25, 0x99, 0xC8, 0x00, 0x04}},
    {0x134D38A, 7, 3, 0x0, true, {0x4C, 0x8D, 0x25, 0x2F, 0xC8, 0x00, 0x04}},
    {0x134D43C, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x7D, 0xC7, 0x00, 0x04}},
    {0x134D4F6, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0xC3, 0xC6, 0x00, 0x04}},
    {0x134D564, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x55, 0xC6, 0x00, 0x04}},
    {0x134D94C, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x75, 0xC2, 0x00, 0x04}},
    {0x13512D4, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0xED, 0x88, 0x00, 0x04}},
    {0x1351399, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x28, 0x88, 0x00, 0x04}},
    {0x135142C, 7, 3, 0x0, true, {0x4C, 0x8D, 0x35, 0x8D, 0x87, 0x00, 0x04}},
    {0x135945D,
     12,
     4,
     0x8,
     false,
     {0x42, 0xC7, 0x84, 0x37, 0xC8, 0x9B, 0x35, 0x05, 0x00, 0x00, 0x00, 0x00}},
    {0x13594B5,
     9,
     4,
     0x0,
     false,
     {0x42, 0x83, 0x8C, 0x37, 0xC0, 0x9B, 0x35, 0x05, 0x02}},
    {0x13594FF, 7, 3, 0x0, true, {0x48, 0x8D, 0x2D, 0xBA, 0x06, 0x00, 0x04}},
    {0x13598AD, 7, 3, 0x8, true, {0x48, 0x8D, 0x3D, 0x14, 0x03, 0x00, 0x04}},
    {0x1359905, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0xBC, 0x02, 0x00, 0x04}},
    {0x1359947, 7, 3, 0x4, true, {0x48, 0x8D, 0x15, 0x76, 0x02, 0x00, 0x04}},
    {0x1359B88, 7, 3, 0x0, true, {0x4C, 0x8D, 0x2D, 0x31, 0x00, 0x00, 0x04}},
    {0x1359C21, 7, 3, 0x8, true, {0x48, 0x8D, 0x35, 0xA0, 0xFF, 0xFF, 0x03}},
    {0x1359EB8, 7, 3, 0x8, true, {0x4C, 0x8D, 0x05, 0x09, 0xFD, 0xFF, 0x03}},
    {0x1359FFF, 7, 3, 0x8, true, {0x48, 0x8D, 0x35, 0xC2, 0xFB, 0xFF, 0x03}},
    {0x135A274, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x45, 0xF9, 0xFF, 0x03}},
    {0x135CA5D, 7, 3, 0x10, true, {0x48, 0x8D, 0x0D, 0x6C, 0xD1, 0xFF, 0x03}},
    {0x135CE1C,
     8,
     3,
     0x0,
     false,
     {0x83, 0x8C, 0x03, 0xC0, 0x9B, 0x35, 0x05, 0x08}},
    {0x135CE26,
     8,
     3,
     0x0,
     false,
     {0x83, 0xA4, 0x03, 0xC0, 0x9B, 0x35, 0x05, 0xF7}},
    {0x135D1B4, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x05, 0xCA, 0xFF, 0x03}},
    {0x135D8A1, 7, 3, 0x0, true, {0x4C, 0x8D, 0x2D, 0x18, 0xC3, 0xFF, 0x03}},
    {0x135E264,
     8,
     4,
     0x0,
     false,
     {0x42, 0x8B, 0x84, 0x30, 0xC0, 0x9B, 0x35, 0x05}},
    {0x135FB90, 7, 3, 0x0, false, {0x48, 0x8D, 0xBA, 0xC0, 0x9B, 0x35, 0x05}},
    {0x135FBA1, 7, 3, 0x8, false, {0x8B, 0xB4, 0x10, 0xC8, 0x9B, 0x35, 0x05}},
    {0x1361701, 7, 3, 0x0, true, {0x48, 0x8D, 0x35, 0xB8, 0x84, 0xFF, 0x03}},
    {0x1361822, 7, 3, 0x10, true, {0x48, 0x8D, 0x05, 0xA7, 0x83, 0xFF, 0x03}},
    {0x13619ED, 7, 3, 0x10, true, {0x48, 0x8D, 0x0D, 0xDC, 0x81, 0xFF, 0x03}},
    {0x1361A92, 7, 3, 0x0, true, {0x48, 0x8D, 0x35, 0x27, 0x81, 0xFF, 0x03}},
    {0x1361C07, 7, 3, 0x0, true, {0x48, 0x8D, 0x15, 0xB2, 0x7F, 0xFF, 0x03}},
    {0x1361F9D, 7, 3, 0x18, true, {0x48, 0x8D, 0x05, 0x34, 0x7C, 0xFF, 0x03}},
    {0x136217F, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x42, 0x7A, 0xFF, 0x03}},
    {0x1364FB1, 7, 3, 0x0, true, {0x48, 0x8D, 0x1D, 0x08, 0x4C, 0xFF, 0x03}},
    {0x1365077, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x42, 0x4B, 0xFF, 0x03}},
    {0x1366938, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x89, 0x32, 0xFF, 0x03}},
    {0x13CAFFF,
     9,
     4,
     0x8,
     false,
     {0x41, 0x83, 0xBC, 0x1F, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x13CC98F,
     9,
     4,
     0x4,
     false,
     {0x41, 0xF6, 0x84, 0x07, 0xC4, 0x9B, 0x35, 0x05, 0x88}},
    {0x13CFC42, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x7F, 0x9F, 0xF8, 0x03}},
    {0x13D205A,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x10, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x13D206C, 7, 3, 0x0, false, {0x8B, 0x84, 0x10, 0xC0, 0x9B, 0x35, 0x05}},
    {0x13D3A6D,
     8,
     3,
     0x10,
     false,
     {0x83, 0xBC, 0x10, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x13D8C0D, 6, 2, 0x0, true, {0x8B, 0x05, 0xAD, 0x0F, 0xF8, 0x03}},
    {0x13DA84B, 6, 2, 0x8, true, {0x39, 0x1D, 0x77, 0xF3, 0xF7, 0x03}},
    {0x13DA869, 7, 3, 0x8, false, {0x8B, 0xB4, 0x38, 0xC8, 0x9B, 0x35, 0x05}},
    {0x13DA87F, 7, 3, 0x8, false, {0x39, 0xB4, 0x38, 0xC8, 0x9B, 0x35, 0x05}},
    {0x13DA8A0, 7, 3, 0x8, false, {0x8B, 0xB4, 0x38, 0xC8, 0x9B, 0x35, 0x05}},
    {0x13DC5CE, 6, 2, 0x0, true, {0x8B, 0x0D, 0xEC, 0xD5, 0xF7, 0x03}},
    {0x13DDE95, 6, 2, 0x0, true, {0x8B, 0x0D, 0x25, 0xBD, 0xF7, 0x03}},
    {0x13DF81D, 7, 3, 0x8, true, {0x48, 0x8D, 0x3D, 0xA4, 0xA3, 0xF7, 0x03}},
    {0x13E151D, 7, 3, 0x8, true, {0x48, 0x8D, 0x3D, 0xA4, 0x86, 0xF7, 0x03}},
    {0x13E1599, 6, 2, 0x8, true, {0x8B, 0x05, 0x29, 0x86, 0xF7, 0x03}},
    {0x13E2F22, 6, 2, 0x8, true, {0x8B, 0x05, 0xA0, 0x6C, 0xF7, 0x03}},
    {0x13E2F39, 6, 2, 0x0, true, {0x8B, 0x05, 0x81, 0x6C, 0xF7, 0x03}},
    {0x13E3261, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x60, 0x69, 0xF7, 0x03}},
    {0x13E32F5, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0xCC, 0x68, 0xF7, 0x03}},
    {0x13E35A9, 7, 3, 0x8, true, {0x4C, 0x8D, 0x35, 0x18, 0x66, 0xF7, 0x03}},
    {0x13E378A, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x37, 0x64, 0xF7, 0x03}},
    {0x164E034, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x85, 0xBB, 0xD0, 0x03}},
    {0x1E00C11, 7, 3, 0x0, true, {0x48, 0x8D, 0x35, 0xE8, 0x88, 0x55, 0x03}},
    {0x1E00CDF, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x1A, 0x88, 0x55, 0x03}},
    {0x1E19439, 7, 3, 0x8, true, {0x48, 0x8D, 0x15, 0xC8, 0x00, 0x54, 0x03}},
    {0x1E194FE,
     8,
     4,
     0x8,
     false,
     {0x46, 0x8B, 0xB4, 0x38, 0xC8, 0x9B, 0x35, 0x05}},
    {0x1E1B785,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x39, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x1E4B195,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x11, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x1E88FF7, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x0A, 0x05, 0x4D, 0x03}},
    {0x1EB794D, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0xB4, 0x1B, 0x4A, 0x03}},
    {0x1EBE2BE, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x43, 0xB2, 0x49, 0x03}},
    {0x1EC1529, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0xD0, 0x7F, 0x49, 0x03}},
    {0x1EC1DBD, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0x44, 0x77, 0x49, 0x03}},
    {0x1EC2404, 7, 3, 0x10, true, {0x48, 0x8D, 0x15, 0x05, 0x71, 0x49, 0x03}},
    {0x1EC2468, 7, 3, 0x10, true, {0x48, 0x8D, 0x05, 0xA1, 0x70, 0x49, 0x03}},
    {0x1EC4B58, 7, 3, 0x10, true, {0x48, 0x8D, 0x3D, 0xB1, 0x49, 0x49, 0x03}},
    {0x1EE8D8C,
     9,
     4,
     0x8,
     false,
     {0x42, 0x83, 0xBC, 0x38, 0xC8, 0x9B, 0x35, 0x05, 0x0A}},
    {0x1EEE9B9, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x48, 0xAB, 0x46, 0x03}},
    {0x1F097A6, 7, 3, 0x8, true, {0x48, 0x8D, 0x3D, 0x5B, 0xFD, 0x44, 0x03}},
    {0x1F1C591, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x68, 0xCF, 0x43, 0x03}},
    {0x1F1D0B2,
     8,
     3,
     0x4,
     false,
     {0xF6, 0x84, 0x08, 0xC4, 0x9B, 0x35, 0x05, 0x10}},
    {0x1F1D84E, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0xB3, 0xBC, 0x43, 0x03}},
    {0x1F23FD8,
     8,
     3,
     0x10,
     false,
     {0x83, 0xBC, 0x18, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x1F2F9C7, 7, 3, 0x0, false, {0x8B, 0x84, 0x10, 0xC0, 0x9B, 0x35, 0x05}},
    {0x1F3A8D4, 7, 3, 0x0, false, {0x8B, 0x84, 0x18, 0xC0, 0x9B, 0x35, 0x05}},
    {0x1F726A2, 7, 3, 0x10, true, {0x48, 0x8D, 0x05, 0x67, 0x6E, 0x3E, 0x03}},
    {0x1F93EA7, 7, 3, 0x0, false, {0x8B, 0x84, 0x10, 0xC0, 0x9B, 0x35, 0x05}},
    {0x1FA41A0,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x11, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x1FBAEE0,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x10, 0xC8, 0x9B, 0x35, 0x05, 0x0B}},
    {0x1FBFB22, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0xDF, 0x99, 0x39, 0x03}},
    {0x2066941,
     8,
     3,
     0x10,
     false,
     {0x83, 0xBC, 0x10, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x2074EC9,
     9,
     4,
     0x10,
     false,
     {0x42, 0x83, 0xBC, 0x00, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x2084482,
     8,
     3,
     0x10,
     false,
     {0x83, 0xBC, 0x10, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x20876B9,
     9,
     4,
     0x10,
     false,
     {0x42, 0x83, 0xBC, 0x12, 0xD0, 0x9B, 0x35, 0x05, 0x00}},
    {0x20E3A3C, 7, 3, 0x0, false, {0x49, 0x8D, 0xBE, 0xC0, 0x9B, 0x35, 0x05}},
    {0x20E3B21, 7, 3, 0x0, false, {0x49, 0x8D, 0x9E, 0xC0, 0x9B, 0x35, 0x05}},
    {0x20ECA08, 7, 3, 0x8, true, {0x48, 0x8D, 0x0D, 0xF9, 0xCA, 0x26, 0x03}},
    {0x20F077B, 7, 3, 0x0, true, {0x48, 0x8D, 0x1D, 0x7E, 0x8D, 0x26, 0x03}},
    {0x20F2091, 7, 3, 0x0, true, {0x48, 0x8D, 0x15, 0x68, 0x74, 0x26, 0x03}},
    {0x20F20AB, 7, 3, 0x0, true, {0x48, 0x8D, 0x15, 0x4E, 0x74, 0x26, 0x03}},
    {0x2216570, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x89, 0x2F, 0x14, 0x03}},
    {0x2216D0A, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0xEF, 0x27, 0x14, 0x03}},
    {0x2387A80, 7, 3, 0x8, true, {0x48, 0x8D, 0x1D, 0x81, 0x1A, 0xFD, 0x02}},
    {0x252BECC, 7, 3, 0x8, true, {0x48, 0x8D, 0x1D, 0x35, 0xD6, 0xE2, 0x02}},
    {0x2582C5B, 7, 3, 0x0, false, {0x8B, 0x84, 0x10, 0xC0, 0x9B, 0x35, 0x05}},
    {0x2584707,
     8,
     4,
     0x0,
     false,
     {0x42, 0x8B, 0x84, 0x02, 0xC0, 0x9B, 0x35, 0x05}},
    {0x2586192, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x6F, 0x33, 0xDD, 0x02}},
    {0x2586992, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x6F, 0x2B, 0xDD, 0x02}},
    {0x25870DE, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x23, 0x24, 0xDD, 0x02}},
    {0x258F23A, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0xBF, 0xA2, 0xDC, 0x02}},
    {0x2591C18,
     8,
     4,
     0x0,
     false,
     {0x42, 0x8B, 0x8C, 0x22, 0xC0, 0x9B, 0x35, 0x05}},
    {0x259A746, 7, 3, 0x0, false, {0x8B, 0x84, 0x10, 0xC0, 0x9B, 0x35, 0x05}},
    {0x25A0A8C, 7, 3, 0x0, true, {0x4C, 0x8D, 0x35, 0x6D, 0x8A, 0xDB, 0x02}},
    {0x25A4188, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x71, 0x53, 0xDB, 0x02}},
    {0x2789616, 7, 3, 0x0, false, {0x8B, 0x84, 0x30, 0xC0, 0x9B, 0x35, 0x05}},
    {0x2797C8E,
     8,
     3,
     0x8,
     false,
     {0x83, 0xBC, 0x08, 0xC8, 0x9B, 0x35, 0x05, 0x00}},
    {0x27C1610, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0xF1, 0x7E, 0xB9, 0x02}},
    {0x27C1649, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0xB8, 0x7E, 0xB9, 0x02}},
    {0x27C1689, 7, 3, 0x8, true, {0x48, 0x8D, 0x05, 0x78, 0x7E, 0xB9, 0x02}},
    {0x27C16BE, 7, 3, 0x0, true, {0x48, 0x8D, 0x3D, 0x3B, 0x7E, 0xB9, 0x02}},
    {0x27C1715, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0xE4, 0x7D, 0xB9, 0x02}},
    {0x27C1782, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x77, 0x7D, 0xB9, 0x02}},
    {0x27C17B9, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x40, 0x7D, 0xB9, 0x02}},
    {0x27C17ED, 7, 3, 0x1078, true, {0x48, 0x8D, 0x0D, 0x84, 0x8D, 0xB9, 0x02}},
    {0x27C1824, 6, 2, 0x1078, true, {0x84, 0x0D, 0x4E, 0x8D, 0xB9, 0x02}},
    {0x27C1863, 6, 2, 0x1078, true, {0x84, 0x15, 0x0F, 0x8D, 0xB9, 0x02}},
    {0x27C1886, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x73, 0x7C, 0xB9, 0x02}},
    {0x27C18D1, 6, 2, 0x1078, true, {0x84, 0x0D, 0xA1, 0x8C, 0xB9, 0x02}},
    {0x27C18E3, 7, 3, 0x0, true, {0x48, 0x8D, 0x0D, 0x16, 0x7C, 0xB9, 0x02}},
    {0x27C1923, 6, 2, 0x1078, true, {0x84, 0x15, 0x4F, 0x8C, 0xB9, 0x02}},
    {0x27C194D, 7, 3, 0x0, true, {0x48, 0x8D, 0x3D, 0xAC, 0x7B, 0xB9, 0x02}},
    {0x27C1992, 6, 2, 0x1078, true, {0x84, 0x15, 0xE0, 0x8B, 0xB9, 0x02}},
    {0x27C19D1, 7, 3, 0x0, true, {0x48, 0x8D, 0x1D, 0x28, 0x7B, 0xB9, 0x02}},
    {0x27C1A0D, 7, 3, 0x0, true, {0x40, 0x84, 0x35, 0xEC, 0x7A, 0xB9, 0x02}},
    {0x27C1A1C, 7, 3, 0x1078, true, {0x40, 0x84, 0x35, 0x55, 0x8B, 0xB9, 0x02}},
};

bool client_ui_actives_relocated = false;
size_t uia_new_base_rva = 0;

bool relocate_client_ui_actives() {
  if (client_ui_actives_relocated) {
    return true;
  }
  const auto b = base();

  // ROW BISECT. The table black-screened the frontend and one proven
  // false positive was not the whole story, so the remaining bad rows
  // are found by measurement: BO3_PANES_MAX=N applies only the first
  // N rows (table is sorted by RVA). A binary search over launches
  // pins each corrupting row exactly. Unset/0 = all rows.
  size_t uia_apply_count = std::size(uia_refs);
  {
    char max_buf[16] = {};
    GetEnvironmentVariableA("BO3_PANES_MAX", max_buf, sizeof(max_buf));
    const auto v = static_cast<size_t>(std::strtoul(max_buf, nullptr, 10));
    if (v > 0 && v < uia_apply_count) {
      uia_apply_count = v;
    }
  }
  // BO3_PANES_SKIP=i,j,k - row indices excluded from this run, so a
  // found corruptor can be masked without rebuilding.
  bool uia_skip[std::size(uia_refs)] = {};
  {
    char skip_buf[256] = {};
    GetEnvironmentVariableA("BO3_PANES_SKIP", skip_buf, sizeof(skip_buf));
    const char *p = skip_buf;
    while (*p) {
      char *end = nullptr;
      const auto idx = static_cast<size_t>(std::strtoul(p, &end, 10));
      if (end == p) {
        break;
      }
      if (idx < std::size(uia_refs)) {
        uia_skip[idx] = true;
      }
      p = (*end == ',') ? end + 1 : end;
    }
  }

  for (const auto &r : uia_refs) {
    const auto *p = reinterpret_cast<const void *>(b + r.rva);
    if (!readable(p, r.len) || std::memcmp(p, r.expected, r.len) != 0) {
      return false;
    }
  }

  auto *destination = allocate_near_module(uia_new_size);
  if (!destination) {
    return false;
  }
  const auto new_base = reinterpret_cast<size_t>(destination) - b;

  // Not statically zero - copy the two live elements. At post_unpack
  // nothing has run, so no pointer into the old buffer can exist.
  if (!readable(reinterpret_cast<const void *>(b + uia_base_rva),
                uia_old_size)) {
    return false;
  }
  std::memcpy(destination, reinterpret_cast<const void *>(b + uia_base_rva),
              uia_old_size);

  int32_t old_values[std::size(uia_refs)] = {};
  size_t done = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < done; ++i) {
      if (uia_skip[i]) {
        continue; // never written, old_values[i] never captured
      }
      write_bytes(
          reinterpret_cast<void *>(b + uia_refs[i].rva + uia_refs[i].disp_off),
          &old_values[i], sizeof(old_values[i]));
    }
  };
  const auto field_value = [&](const uia_ref &r) {
    const size_t tgt = new_base + r.delta;
    return r.rip ? static_cast<int32_t>(tgt - (r.rva + r.len))
                 : static_cast<int32_t>(tgt);
  };

  for (size_t i = 0; i < uia_apply_count; ++i) {
    if (uia_skip[i]) {
      ++done; // keep rollback indexing aligned; nothing written
      continue;
    }
    const auto &r = uia_refs[i];
    const int32_t value = field_value(r);
    auto *field = reinterpret_cast<void *>(b + r.rva + r.disp_off);
    std::memcpy(&old_values[i], field, sizeof(int32_t));
    if (!write_bytes(field, &value, sizeof(value))) {
      rollback();
      return false;
    }
    ++done;
  }
  for (size_t i = 0; i < uia_apply_count; ++i) {
    if (uia_skip[i]) {
      continue;
    }
    int32_t seen = 0;
    std::memcpy(&seen,
                reinterpret_cast<const void *>(b + uia_refs[i].rva +
                                               uia_refs[i].disp_off),
                sizeof(seen));
    if (seen != field_value(uia_refs[i])) {
      rollback();
      return false;
    }
  }

  uia_new_base_rva = new_base;
  client_ui_actives_relocated = true;
  return true;
}

// ---- Phase 2: the pane geometry table ------------------------------
//
// PC 0x032E56F0 is [2 wide][2 total][2 pane] x 0x10 = 0x80 bytes. The
// index is built at 0x010CB5D0/5D6 as
//     (pane + ((total-1) + wide*2) * 2) * 0x10
// i.e. pane*0x10 + (total-1)*0x20 + wide*0x40.
//
// The console table is [2][4][4] x 0x10 = 0x200 with
//     pane*0x10 + (total-1)*0x40 + wide*0x100
// so both scale-by-2 SIB bytes become scale-by-4, which is
// length-preserving (SIB scale bits only):
//     0x010CB5D0  48 8D 14 51  ->  ...91     lea rdx,[rcx+rdx*4]
//     0x010CB5D6  48 8D 0C 56  ->  ...96     lea rcx,[rsi+rdx*4]
//
// The 0x200 payload is the CONSOLE table, extracted byte-for-byte from
// the PS4 ELF .data at VA 0x03060D50 (data/ps4_clientviewparams.bin).
// Nothing here is authored: total=3 is two quarter panes on top and one
// wide pane below; total=4 is quads.
// THE WHOLE FUNCTION, swept - the first attempt patched three sites and
// blacked out the frontend, because 0x010CB5D0 is SHARED by two index
// paths and only one of them was converted:
//
//   010CB54F  shl rax, 6            total==0 path: wide * 0x40
//   010CB5D0  lea rdx,[rcx+rdx*2]   SHARED: (total-1) + wide*2
//   010CB5D4  je 0x010CB632         branch on splitscreen_horizontal
//   010CB5D6  lea rcx,[rsi+rdx*2]   horizontal:     pane + that*2
//   010CB5DA  lea rdx,[0x032E56F0]  horizontal base
//   010CB632  lea rax,[rsi+rdx*2]   NON-horizontal: pane + that*2
//   010CB644  lea rdx,[0x032E56F0]  NON-horizontal base
//
// With scale-4 in the shared lea and scale-2 left in the non-horizontal
// one - reading the OLD 0x80 table - the default path indexed past the
// table into FOV constants. All six move together or none do.
//
// The console's own scales confirm each target:
//   PS4 002CC551  shl rcx, 8        wide * 0x100
//   PS4 002CC7CD  shl r8, 8 / 7D4 shl rdi,6 / 7DB shl rdx,4
constexpr uint32_t viewparams_rva = 0x32666F0;

struct vp_patch {
  uint32_t rva;
  uint8_t len;
  uint8_t off;  // byte to change (or disp offset for a base lea)
  bool is_base; // true = rip-disp to the table, false = single byte
  uint8_t to;   // new byte value when !is_base
  uint8_t expect[7];
};

constexpr vp_patch viewparams_patches[] = {
    // wide scale on the total==0 path: 0x40 -> 0x100
    {0x10CB56F, 4, 3, false, 0x08, {0x48, 0xC1, 0xE0, 0x06}},
    // shared index lea: scale 2 -> 4
    {0x10CB5F0, 4, 3, false, 0x91, {0x48, 0x8D, 0x14, 0x51}},
    // horizontal path index lea: scale 2 -> 4
    {0x10CB5F6, 4, 3, false, 0x96, {0x48, 0x8D, 0x0C, 0x56}},
    // NON-horizontal path index lea: scale 2 -> 4
    {0x10CB652, 4, 3, false, 0x96, {0x48, 0x8D, 0x04, 0x56}},
    // the two table base leas
    {0x10CB5FA, 7, 3, true, 0x00, {0x48, 0x8D, 0x15, 0xEF, 0xB0, 0x19, 0x02}},
    {0x10CB664, 7, 3, true, 0x00, {0x48, 0x8D, 0x15, 0x85, 0xB0, 0x19, 0x02}},
};

constexpr uint8_t ps4_view_params[0x200] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,
    0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x80, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x80, 0x3E,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
    0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x3F,
};

bool view_params_relocated = false;
size_t viewparams_new_rva = 0;

bool relocate_view_params() {
  if (view_params_relocated) {
    return true;
  }
  const auto b = base();

  for (const auto &p : viewparams_patches) {
    const auto *site = reinterpret_cast<const void *>(b + p.rva);
    if (!readable(site, p.len) || std::memcmp(site, p.expect, p.len) != 0) {
      return false;
    }
  }

  auto *destination = allocate_near_module(sizeof(ps4_view_params));
  if (!destination) {
    return false;
  }
  std::memcpy(destination, ps4_view_params, sizeof(ps4_view_params));
  const auto new_rva = reinterpret_cast<size_t>(destination) - b;

  // One transaction over all six sites, each remembered for rollback.
  uint8_t old_bytes[std::size(viewparams_patches)][4] = {};
  size_t done = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < done; ++i) {
      const auto &p = viewparams_patches[i];
      auto *f = reinterpret_cast<void *>(b + p.rva + p.off);
      write_bytes(f, old_bytes[i], p.is_base ? 4u : 1u);
    }
  };

  for (size_t i = 0; i < std::size(viewparams_patches); ++i) {
    const auto &p = viewparams_patches[i];
    auto *field = reinterpret_cast<void *>(b + p.rva + p.off);
    const size_t n = p.is_base ? 4u : 1u;
    std::memcpy(old_bytes[i], field, n);
    bool wrote;
    if (p.is_base) {
      const int32_t disp = static_cast<int32_t>(new_rva - (p.rva + p.len));
      wrote = write_bytes(field, &disp, sizeof(disp));
    } else {
      wrote = write_bytes(field, &p.to, sizeof(p.to));
    }
    if (!wrote) {
      rollback();
      return false;
    }
    ++done;
  }

  // Read every site back before declaring success.
  for (size_t i = 0; i < std::size(viewparams_patches); ++i) {
    const auto &p = viewparams_patches[i];
    const auto *field = reinterpret_cast<const uint8_t *>(b + p.rva + p.off);
    if (p.is_base) {
      int32_t seen = 0;
      std::memcpy(&seen, field, sizeof(seen));
      if (seen != static_cast<int32_t>(new_rva - (p.rva + p.len))) {
        rollback();
        return false;
      }
    } else if (*field != p.to) {
      rollback();
      return false;
    }
  }

  viewparams_new_rva = new_rva;
  view_params_relocated = true;
  return true;
}

// ---- two [2] arrays the third pane's own code path indexes at 2 ----
//
// Found via the 2026-08-24 19:10 crash - the first run whose pane
// dispatcher actually reached CG_SetView(2):
//
// 1. scrPlaceView - base 0x057FA800, stride 0x7C. On PS4 this is
//    ScreenPlacement scrPlaceView[4] (same element size), reached ONLY
//    through ScrPlace_GetView* (0x42F1E0..), which assert the index
//    against MAX_LOCAL_CLIENTS. The PC compiles the assert out, so
//    &scrPlaceView[uiContextIndex] for context 2 is one-past-the-end
//    of the PC [2] array. The first viewport setup for the third pane
//    then writes a whole ScreenPlacement over the globals that follow:
//    virtualViewableMax {853.33f, 480.0f} landed exactly on the
//    pointer global 0x057FA918 = base + 2*0x7C + 0x20, which
//    0x013E6A34 loads and 0x022BCED0 dereferences -> the crash at
//    0x022BCEE6 with rcx = 0x43F0000044555556 (the float pair read as
//    a pointer; return address 0x013E6A40 on the crash stack).
//    References: exactly THREE leas, all to +0x0, all in the accessor
//    cluster (find_lea over both elements, all classes, 506 MB):
//    0x013E55A9 GetView(lc), 0x013E55C3 GetViewUIContext(ctx),
//    0x013E55E9 GetViewWritable(lc). Everything else receives the
//    pointer - the same architecture the PS4 shows, where elf_xref on
//    scrPlaceView finds only the four accessor functions.
//
// 2. the unnamed per-client 0x54 state at 0x04D322C0 (fields
//    +0x4C/+0x50 compared against a timer, reset fills 1.0f scales -
//    a screen-effect state, naming open). perclient_sweep classifies
//    slot 2 FOREIGN: a live pointer global sits at 0x04D32368 =
//    base + 2*0x54. Both its functions index by the raw local client
//    number, so the first lc=2 tick after cl_maxLocalClients reaches
//    3 sprays defaults over that pointer. Exactly two references,
//    both lea to +0x0 (0x010E5036 reset, 0x0111713D expiry check).
//
// Slot 2 of both arrays is FOREIGN, so per CLAUDE.md rule 4 they are
// RELOCATED, never widened in place: copy the two live elements into
// a 4-slot block and repoint the handful of leas. Slots 2/3 start
// zeroed; the engine's own setup fills them the first time context 2
// activates - which is the very write that used to corrupt the
// neighbours.

struct flat24_site {
  uint32_t rva; // lea instruction start (7 bytes, disp at +3)
  uint8_t expect[7];
};

constexpr flat24_site scrplace_sites[] = {
    {0x013E55C9, {0x48, 0x8D, 0x0D, 0x30, 0x62, 0x39, 0x04}}, // GetView
    {0x013E55E3,
     {0x48, 0x8D, 0x0D, 0x16, 0x62, 0x39, 0x04}}, // GetViewUIContext
    {0x013E5609, {0x48, 0x8D, 0x0D, 0xF0, 0x61, 0x39, 0x04}}, // GetViewWritable
};
constexpr flat24_site perclient54_sites[] = {
    {0x010E5056, {0x48, 0x8D, 0x05, 0x63, 0xE2, 0xBC, 0x03}}, // reset
    {0x0111715D, {0x48, 0x8D, 0x05, 0x5C, 0xC1, 0xB9, 0x03}}, // expiry check
};

// aaGlobArray - AimAssist per-client globals, base 0x036751D0 stride
// 0x4E30. PS4 AimAssist_GetClientGlobals (0x48010) is
// `lea rax,[&aaGlobArray]; imul rcx,lc,0x4e30; add rax,rcx` behind an
// assert lc < MAX_LOCAL_CLIENTS(4) that retail strips - so there is NO
// runtime clamp. CG_SetView calls AimAssist_UpdateScreenDim ->
// GetClientGlobals/Setup, which READ AND WRITE that struct, so the
// moment pane 2 renders it scribbles over slot 2 (0x0367EE30) - dense
// foreign neighbour globals per find_lea, so RELOCATE, never widen.
// All 23 sites verified byte-exact as `lea reg,[rip]->0x036751D0`.
// 0x00039BBC is deliberately EXCLUDED: it decodes as
// `lea rsi,[rcx+0x36751d0]`, a register-relative displacement and not a
// base load - the false-positive class the reference rules warn about.
// SUPERSEDED 2026-09-28 by aaglob_v2_sites (complete table) - kept as the
// record.
constexpr flat24_site aaglob_sites[] = {
    {0x0002D7B6, {0x48, 0x8D, 0x05, 0x13, 0x8A, 0x5C, 0x03}},
    {0x0002DAD3, {0x48, 0x8D, 0x05, 0xF6, 0x86, 0x5C, 0x03}},
    {0x0002F70F, {0x48, 0x8D, 0x05, 0xBA, 0x6A, 0x5C, 0x03}},
    {0x0002FC45, {0x48, 0x8D, 0x05, 0x84, 0x65, 0x5C, 0x03}},
    {0x0002FFD1, {0x48, 0x8D, 0x05, 0xF8, 0x61, 0x5C, 0x03}},
    {0x00034D16, {0x48, 0x8D, 0x05, 0xB3, 0x14, 0x5C, 0x03}},
    {0x000369C8, {0x48, 0x8D, 0x05, 0x01, 0xF8, 0x5B, 0x03}},
    {0x00043793, {0x48, 0x8D, 0x0D, 0x36, 0x2A, 0x5B, 0x03}},
    {0x00043E7D, {0x48, 0x8D, 0x05, 0x4C, 0x23, 0x5B, 0x03}},
    {0x0004EDEE, {0x48, 0x8D, 0x05, 0xDB, 0x73, 0x5A, 0x03}},
    {0x0005219A, {0x48, 0x8D, 0x05, 0x2F, 0x40, 0x5A, 0x03}},
    {0x00056D33, {0x48, 0x8D, 0x0D, 0x96, 0xF4, 0x59, 0x03}},
    {0x0005B969, {0x48, 0x8D, 0x05, 0x60, 0xA8, 0x59, 0x03}},
    {0x0005BA60, {0x48, 0x8D, 0x05, 0x69, 0xA7, 0x59, 0x03}},
    {0x0005D383, {0x48, 0x8D, 0x05, 0x46, 0x8E, 0x59, 0x03}},
    {0x000604DE, {0x48, 0x8D, 0x05, 0xEB, 0x5C, 0x59, 0x03}},
    {0x000639B3, {0x48, 0x8D, 0x05, 0x16, 0x28, 0x59, 0x03}},
    {0x00065353, {0x48, 0x8D, 0x05, 0x76, 0x0E, 0x59, 0x03}},
    {0x00066CC7, {0x48, 0x8D, 0x05, 0x02, 0xF5, 0x58, 0x03}},
    {0x00066D50, {0x48, 0x8D, 0x05, 0x79, 0xF4, 0x58, 0x03}},
    {0x00071B48, {0x48, 0x8D, 0x05, 0x81, 0x46, 0x58, 0x03}},
    {0x00073644, {0x48, 0x8D, 0x05, 0x85, 0x2B, 0x58, 0x03}},
    {0x000753E0, {0x48, 0x8D, 0x05, 0xE9, 0x0D, 0x58, 0x03}},
};

// The UI element-handle registrar's per-LOCAL-CLIENT word array, base
// 0x179DBDD8 stride 2. Loop 3 (head 0x01F33160, bound 0x01F331D2) does
// `lea rsi,[r14+rbx*2]` with rbx = GetLocalClientNum(ctx), then
// `mov word [rsi], ax` - it stores the LUI element handle that
// 0x0270D553 later fails to resolve for context 2 (the rdx=NULL crash).
// Slot 2/3 are FOREIGN, proven: find_lea over the 8-byte window shows
// 0x01F328FB taking the address of 0x179DBDDE (= slot 3) as its OWN
// global. Exactly ONE reference to rewrite.
constexpr flat24_site uielem_sites[] = {
    {0x01F27096, {0x4C, 0x8D, 0x35, 0x3B, 0x5E, 0xA3, 0x15}},
};

// THE MISSING READER (2026-09-26 18:05) - why panes 1 and 2 had no HUD.
// The table above moved only the WRITER. These u16 are the per-client
// "UIVisibilityBit" parent models; the one READER is
// UI_CoD_HUD_UpdateVisibilityBits' model lookup, an ABS32 form that the
// lea-only scan never lists:
//   02771B42  movzx ecx, word ptr [r15 + r12*2 + 0x179DBDD8]   (r15 = image)
// (pe_xref's ABS32 candidates show it.) So every change of a visibility
// bit was published under whatever sat at the OLD address - measured
// live 0x8001 / 0x0003, i.e. player 3's visibility bits (batch 15) -
// while the HUD subscribed under the real handles (0x2A25 / 0x25C2 in
// the moved array). The HUD's "UIVisibilityBit.<n>" models never
// changed, and T7Hud_ZM shows nothing without BIT_HUD_VISIBLE.
bool uielem_reader_retargeted = false;
size_t uielem_new_rva = 0;
bool uielem_relocated = false;

void retarget_uielem_reader() {
  if (uielem_reader_retargeted || !uielem_relocated || !uielem_new_rva) {
    return;
  }
  auto *insn = reinterpret_cast<uint8_t *>(base() + 0x026F9092);
  constexpr uint8_t expect[] = {0x43, 0x0F, 0xB7, 0x8C, 0x67,
                                0xD8, 0xCE, 0x95, 0x17};
  if (!readable(insn, sizeof(expect)) ||
      std::memcmp(insn, expect, sizeof(expect)) != 0) {
    note("[splitscreen] uielem reader: bytes differ - skipped\n");
    return;
  }
  const auto disp = static_cast<int32_t>(uielem_new_rva);
  if (write_bytes(insn + 5, &disp, sizeof(disp))) {
    uielem_reader_retargeted = true;
  }
}

bool scrplace_relocated = false;
bool perclient54_relocated = false;
bool aaglob_relocated = false;
size_t scrplace_new_rva = 0;
size_t perclient54_new_rva = 0;
size_t aaglob_new_rva = 0;

bool relocate_flat24(const char *name, const uint32_t old_base,
                     const uint32_t stride, const flat24_site *sites,
                     const size_t count, bool &flag, size_t &new_rva_out) {
  if (flag) {
    return true;
  }
  const auto b = base();

  for (size_t i = 0; i < count; ++i) {
    const auto *site = reinterpret_cast<const void *>(b + sites[i].rva);
    if (!readable(site, sizeof(sites[i].expect)) ||
        std::memcmp(site, sites[i].expect, sizeof(sites[i].expect)) != 0) {
      note("[splitscreen] %s: unexpected bytes at 0x%X - skipped\n", name,
           sites[i].rva);
      return false;
    }
  }

  auto *destination = static_cast<uint8_t *>(allocate_near_module(4u * stride));
  if (!destination) {
    return false;
  }
  std::memset(destination, 0, 4u * stride);
  std::memcpy(destination, reinterpret_cast<const void *>(b + old_base),
              2u * stride);
  const auto new_rva = reinterpret_cast<size_t>(destination) - b;

  // 32, not 8: aaGlobArray has 23 reference sites. A rollback that
  // overruns this buffer would smash the stack on the failure path.
  int32_t old_disp[32] = {};
  if (count > std::size(old_disp)) {
    note("[splitscreen] %s: too many sites (%zu)\n", name, count);
    return false;
  }
  size_t done = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < done; ++i) {
      auto *f = reinterpret_cast<void *>(b + sites[i].rva + 3);
      write_bytes(f, &old_disp[i], sizeof(old_disp[i]));
    }
  };

  for (size_t i = 0; i < count; ++i) {
    auto *field = reinterpret_cast<void *>(b + sites[i].rva + 3);
    std::memcpy(&old_disp[i], field, sizeof(old_disp[i]));
    const int32_t disp = static_cast<int32_t>(new_rva - (sites[i].rva + 7));
    if (!write_bytes(field, &disp, sizeof(disp))) {
      rollback();
      return false;
    }
    ++done;
  }

  for (size_t i = 0; i < count; ++i) {
    int32_t seen = 0;
    std::memcpy(&seen, reinterpret_cast<const void *>(b + sites[i].rva + 3),
                sizeof(seen));
    if (seen != static_cast<int32_t>(new_rva - (sites[i].rva + 7))) {
      rollback();
      return false;
    }
  }

  new_rva_out = new_rva;
  flag = true;
  return true;
}

// ---- the third screen, WITHOUT moving clientUIActives ---------------
//
// Established 2026-08-24 by reading CG_SetView (PC 0x010F4E00): it does
// NOT touch clientUIActives. It resolves geometry through
// GetLocalClientViewParams (0x010CB520) and writes through the pointer
// at 0x04D17B70 - a structure this project already relocates. So the
// pane pipeline's ONLY dependency on clientUIActives[2] is the
// IsActive(lc) gate in the dispatcher loop at 0x0132E298.
//
// Caving that gate replaces a 250-reference relocation (twice proven
// fatal - see CLAUDE.md) with one 6-instruction function.
//
//   lc 0/1  -> unchanged: clientUIActives[lc].flags & 1
//   lc >= 2 -> relocated clientGameStates[lc].flags & 1
//
// NOTHING IS FAKED. PS4 CL_LocalClient_SetActive sets the active flag
// FROM Com_LocalClient_IsBeingUsed, so "active == beingUsed" is the
// engine's own rule; the cave applies it to the clients the PC build
// forgot to size for. The component already relies on exactly this
// equivalence in install_active_count_fix.
constexpr uint32_t isactive_rva = 0x27C18E0;
constexpr uint8_t isactive_expected[] = {
    0x48, 0x63, 0xC1, // movsxd rax, ecx
    0x48, 0x8D, 0x0D, // lea rcx, [clientUIActives]
};

bool isactive_caved = false;

bool install_isactive_cave() {
  if (isactive_caved) {
    return true;
  }
  if (!signin_relocated) {
    return false; // no three-seat truth to answer from
  }
  const auto b = base();
  auto *fn = reinterpret_cast<uint8_t *>(b + isactive_rva);
  if (!readable(fn, sizeof(isactive_expected)) ||
      std::memcmp(fn, isactive_expected, sizeof(isactive_expected)) != 0) {
    return false;
  }

  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x80));
  if (!cave) {
    return false;
  }
  const auto cave_addr = reinterpret_cast<size_t>(cave);

  std::vector<uint8_t> c;
  const auto rip32 = [&](const size_t tgt) {
    const auto v = static_cast<int32_t>(tgt - (cave_addr + c.size() + 4));
    const auto *p = reinterpret_cast<const uint8_t *>(&v);
    c.insert(c.end(), p, p + 4);
  };
  const auto imm32 = [&](const uint32_t v) {
    const auto *p = reinterpret_cast<const uint8_t *>(&v);
    c.insert(c.end(), p, p + 4);
  };

  c.insert(c.end(), {0x83, 0xF9, 0x02}); // cmp ecx, 2
  c.insert(c.end(), {0x7C, 0x00});       // jl  stock  (patched)
  const auto jl_at = c.size() - 1;

  // THE ENGINE'S OWN GUARD, mirrored. CG_GetLocalClientGlobals
  // (0x010F8020) returns NULL when localClientNum >= cl_maxLocalClients,
  // and its callers do NOT null-check - measured 2026-08-24 as a read from
  // 0x28 with rax = 0 at 0x010F804B. cl_maxLocalClients only becomes 3 at
  // map load, so a widened pane loop visits client 2 while the engine is
  // still sized for two and the NULL is dereferenced.
  //
  // Answering "not active yet" until the engine has sized for the client
  // is not a fake: it is exactly what the engine itself reports through
  // that same bound, so the pane loop simply skips client 2 until its
  // storage exists, then picks it up on the next frame.
  c.insert(c.end(), {0x3B, 0x0D}); // cmp ecx, [cl_maxLocalClients]
  rip32(b + cl_max_local_clients_rva);
  c.insert(c.end(), {0x7C, 0x03}); // jl  +3 (storage exists)
  c.insert(c.end(), {0x33, 0xC0}); // xor eax, eax
  c.insert(c.end(), {0xC3});       // ret  -> not active yet

  // lc >= 2 AND within cl_maxLocalClients: THE ENGINE'S OWN FLAG, i.e.
  // fall through into the stock read below (2026-09-26 18:31).
  //
  // This branch used to answer from the seat table (clientGameStates
  // [lc].flags & 1), because clientUIActives slot 2 was voice_comm's
  // memory then. Since voice_comm moved, slot 2 is real and every loop
  // that sets or clears the active bit walks it (SetAllUsedActive,
  // the frontend setup, the disable commands). The seat answer then
  // became WRONG on the way back to the menu: the frontend setup had
  // cleared lc 2's active bit (flags 0x06, like lc 1) but the seat is
  // still in use, so the frontend connect loop saw "active" and left
  // lc 2 hanging at state 5 - the black loading screen, measured with
  // firstsnap_watch. PS4 CL_LocalClient_IsActive reads the flag.

  // stock path: clientUIActives[lc].flags & 1
  const auto stock_off = c.size();
  c[jl_at] = static_cast<uint8_t>(stock_off - (jl_at + 1));
  c.insert(c.end(), {0x48, 0x63, 0xC1}); // movsxd rax, ecx
  c.insert(c.end(), {0x48, 0x69, 0xC0}); // imul rax, rax, 0x1078
  imm32(uia_stride);
  c.insert(c.end(), {0x48, 0x8D, 0x0D}); // lea rcx, [clientUIActives]
  rip32(b + uia_base_rva);
  c.insert(c.end(), {0x8B, 0x04, 0x08}); // mov eax, [rax+rcx]
  c.insert(c.end(), {0x83, 0xE0, 0x01}); // and eax, 1
  c.insert(c.end(), {0xC3});             // ret

  if (!write_bytes(cave, c.data(), c.size())) {
    return false;
  }

  uint8_t patch[5] = {0xE9};
  const auto rel = static_cast<int32_t>(cave_addr - (b + isactive_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));
  if (!write_bytes(fn, patch, sizeof(patch))) {
    return false;
  }
  isactive_caved = true;
  return true;
}

// ---- Phase 3: the pane count and the four dispatcher bounds --------
//
// GetActiveCount 0x0283AA30 is the DISPATCHER's copy and is a different
// inline expansion from the one install_active_count_fix() caves at
// 0x0283AB7D (which feeds the dvar and the allocator). Both are needed.
// Retail's copy reads bit 0 of clientUIActives[0] (0x053D8BC0 - the
// `test byte [rip+d32],1` ends at 0x0283AA39) and of clientUIActives[1];
// the console instead calls IsActive(i) for i in 0..3. The cave does
// the same over the three elements that exist - the engine's own rule,
// extended, nothing faked.
//
// The bounds are widened ONLY when the storage AND the geometry are
// both live. Widening them alone would make IsActive(2) - which has no
// bounds check of its own - read the foreign array at 0x053DACB0, and
// would index the geometry table past its two rows.
constexpr uint32_t get_active_count_rva = 0x27C18C0;
constexpr uint8_t get_active_count_expected[] = {
    0x33, 0xC0,                               // xor eax, eax
    0xF6, 0x05, 0x37, 0x7C, 0xB9, 0x02, 0x01, // test byte [rip+..], 1
    0xB9, 0x01, 0x00, 0x00, 0x00,             // mov ecx, 1
    0x0F, 0x45, 0xC1,                         // cmovne eax, ecx
};

struct pane_bound {
  uint32_t rva;
  uint8_t offset;
  uint8_t from;
  uint8_t to;
  uint8_t expect[5];
  uint8_t expect_len;
};

// ONLY the CG_SetView dispatcher bound. Disassembled 2026-08-24; the
// other three sites in this function group are NOT safe to widen:
//
//   0x0132E2C4  cmp ebx,2  KEPT. This is the loop at 0x0132E298 whose body
//               is gated by IsActive (0x0283AA50 - our cave) and then calls
//               CG_SetView (0x010F4E00). CG_SetView resolves geometry
//               through structures this component already relocates, so
//               widening it is exactly the third rendered pane and nothing
//               else.
//
//   0x0132E2FA  mov edi,1  REMOVED - it was an out-of-bounds WRITE. That
//               site is not a pane count: it seeds a DOWNWARD walk over
//               clientUIActives (lea rbx,[0x053D9C38] = element[1];
//               sub rbx,0x1078; dec edi; jns). Raising the counter to 3
//               without moving the base makes it step to element[-1] and
//               element[-2] and execute `and [rbx],0xFFFFFFCF` there,
//               corrupting whatever precedes clientUIActives on every call.
//               CLAUDE.md rule 4 in its purest form: the array the bound
//               guards is still [2], so the bound may not move.
//
//   0x0132E220 / 0x0132E283  cmp edi,2  REMOVED pending classification.
//               Their bodies call 0x013E5B00 per local client and nothing
//               has classified slot 2 of what that touches. They are 2D/HUD
//               passes, not the rendered view, so they are not needed for
//               the third screen and must not be widened on a guess.
constexpr pane_bound pane_bounds[] = {
    {0x0132E2E4, 2, 0x02, 0x04, {0x83, 0xFB, 0x02}, 3},
};

bool pane_counts_installed = false;

// Loop #3 - THE LUI ELEMENT REGISTRAR (head 0x01F33160, bound
// 0x01F331D2). Its body does `mov word [r14+lc*2], ax`, storing the
// element handle whose absence for context 2 is the 0x0270D553 NULL
// crash. It may only widen AFTER that word array is relocated, and the
// relocation runs later in try_apply than relocate_lui_roots - which is
// exactly why this is its own function rather than a row in the table
// inside relocate_lui_roots (measured: gated there, it silently never
// applied and the bound read 02 live while the array read RELOCATED).
// Defined with the LUI roots relocation, further down.
extern bool lui_roots_relocated;
extern size_t uiroot_new_base_rva;

// HOLD THE LUI RENDER LOOP AT TWO CONTEXTS.
//
// 0x026FC3F5 is `cmp r15d,[cl_maxLocalClients]` - the back-edge of the
// per-UI-CONTEXT render loop (head 0x026FA885, gated by our IsActive
// cave). It is not a per-client array bound: it decides how many LUI
// CONTEXTS get walked every frame. The moment cl_maxLocalClients held
// at 3 this loop began walking context 2, whose element tree the PC
// never builds - measured twice as 0xC0000005 at 0x0270D553
// (`mov rax,[rdx+0x138]` with rdx = the hardcoded NULL sentinel r14
// from 0x0270D44E, reached when the element-handle salt check fails),
// and as a collapse to ~4 FPS from walking failure paths every frame.
//
// Widening the three constructor loops and relocating the handle array
// (both shipped above, both verified live) was NOT sufficient: the
// context's Lua-side tree is built by the LUI machine, not by those
// loops, and the PC frontend only ever instantiates two.
//
// So scope the UI to what the PC actually supports - two contexts - and
// let the WORLD pane loop (0x0132E2C4, a different loop through
// CG_SetView) run to three. Nothing is faked: we do not claim a third
// context exists, we stop the renderer from walking one that does not.
// Player 3 gets a rendered viewport; the third HUD stays absent until
// the LUI context itself is built, which is its own project.
//
// Length-preserving: 7 bytes `cmp r15d,[rip+d32]` -> `cmp r15d,3` + NOPs.
//
// 2026-09-27: THREE, for pane 3's HUD. The 0x0270D553 NULL-element crash
// that forced the hold was a handle-salt check on context 2's root: the
// per-context element handles (uiElemHandles) were [2] and their ABS32
// reader at 0x02771B42 still read the OLD array - both fixed 2026-09-26
// evening (relocate_flat24 + retarget_uielem_reader), which is also what
// brought the HUD back on panes 1 and 2. zz_probe confirms LUI.roots.UIRoot2
// exists. Contexts that are not active are still skipped by the loop's
// IsActive gate, so the frontend (lc 2 inactive) does not walk it.
constexpr uint32_t lui_ctx_bound_rva = 0x2683285;
constexpr uint8_t lui_ctx_expected[] = {0x44, 0x3B, 0x3D, 0xD4,
                                        0xFD, 0xC9, 0x02};
constexpr uint8_t lui_ctx_patched[] = {0x41, 0x83, 0xFF, 0x03,
                                       0x90, 0x90, 0x90};
bool lui_ctx_held = false;

bool hold_lui_context_count() {
  if (lui_ctx_held) {
    return true;
  }
  auto *at = reinterpret_cast<uint8_t *>(base() + lui_ctx_bound_rva);
  if (!readable(at, sizeof(lui_ctx_expected)) ||
      std::memcmp(at, lui_ctx_expected, sizeof(lui_ctx_expected)) != 0) {
    note("[splitscreen] lui ctx bound: unexpected bytes - skipped\n");
    return false;
  }
  if (!write_bytes(at, lui_ctx_patched, sizeof(lui_ctx_patched))) {
    return false;
  }
  uint8_t back[sizeof(lui_ctx_patched)] = {};
  std::memcpy(back, at, sizeof(back));
  if (std::memcmp(back, lui_ctx_patched, sizeof(back)) != 0) {
    write_bytes(at, lui_ctx_expected, sizeof(lui_ctx_expected));
    return false;
  }
  lui_ctx_held = true;
  return true;
}

bool ui_registrar_widened = false;

bool widen_ui_registrar_bound() {
  if (ui_registrar_widened) {
    return true;
  }
  if (!uielem_relocated || !lui_roots_relocated) {
    return false;
  }
  constexpr uint8_t want[] = {0x83, 0xFF, 0x02}; // cmp edi, 2
  auto *at = reinterpret_cast<uint8_t *>(base() + 0x01F27112);
  if (!readable(at, sizeof(want)) || std::memcmp(at, want, sizeof(want)) != 0) {
    return false;
  }
  const uint8_t four = 0x04;
  if (!write_bytes(at + 2, &four, sizeof(four))) {
    return false;
  }
  uint8_t back = 0;
  std::memcpy(&back, at + 2, sizeof(back));
  if (back != four) {
    const uint8_t two = 0x02;
    write_bytes(at + 2, &two, sizeof(two));
    return false;
  }
  ui_registrar_widened = true;
  return true;
}

// THE SNAPSHOT GUARD - fixes the reproduced 0x01F46FAF crash.
//
// The HUD-refresh reader (fn 0x01F45740) inlines CG_GetLocalClientGlobals
// and then reads cg->nextSnap (+0x30, `snapshot_t* nextSnap` per the PS4
// DWARF). Its guard-FAIL path (`je/jge 0x1F46FAB`) falls into the very
// dereference it was guarding, with a stale rdi - broken in stock, but
// stock never fails the guard. And for local client 2 nextSnap is
// legitimately NULL: cg->snap/nextSnap are written only by
// CG_ProcessSnapshots, whose frame loop the PC bounds at 2 (widening it
// regressed its round to zero panes - build 6D16F5CA - so cgame(2)
// stays off for now).
//
// The instruction after the load is `test byte [rdx],0x10; jne
// 0x1F46FD8` - i.e. "snapshot not usable -> return 0" is this
// function's own answer. The cave gives a NULL nextSnap that same
// answer. This is NOT the closed-dead-end null-guard pattern (masking a
// missing allocation so the crash moves to the next array): the
// function EXITS here, nothing downstream runs, and "no snapshot yet"
// is a state the engine itself models with this exact exit.
constexpr uint32_t snapguard_rva = 0x1F3A82B;
constexpr uint32_t snapguard_ret0 = 0x1F3A858;   // xor eax,eax; ...ret
constexpr uint32_t snapguard_resume = 0x1F3A832; // the jne after the test
constexpr uint8_t snapguard_expected[] = {
    0x48, 0x8B, 0x57, 0x30, // mov rdx, [rdi+0x30]
    0xF6, 0x02, 0x10,       // test byte [rdx], 0x10
};
bool snapguard_installed = false;

bool install_snapguard_cave() {
  if (snapguard_installed) {
    return true;
  }
  const auto b = base();
  auto *site = reinterpret_cast<uint8_t *>(b + snapguard_rva);
  if (!readable(site, sizeof(snapguard_expected)) ||
      std::memcmp(site, snapguard_expected, sizeof(snapguard_expected)) != 0) {
    return false;
  }
  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x40));
  if (!cave) {
    return false;
  }
  const auto cave_addr = reinterpret_cast<size_t>(cave);
  std::vector<uint8_t> c;
  const auto rel32 = [&](const size_t tgt) {
    const auto v = static_cast<int32_t>(tgt - (cave_addr + c.size() + 4));
    const auto *p = reinterpret_cast<const uint8_t *>(&v);
    c.insert(c.end(), p, p + 4);
  };
  c.insert(c.end(), {0x48, 0x8B, 0x57, 0x30}); // mov rdx, [rdi+0x30]
  c.insert(c.end(), {0x48, 0x85, 0xD2});       // test rdx, rdx
  c.insert(c.end(), {0x0F, 0x84});             // jz -> return-0 exit
  rel32(b + snapguard_ret0);
  c.insert(c.end(), {0xF6, 0x02, 0x10}); // test byte [rdx], 0x10
  c.insert(c.end(), {0xE9});             // jmp resume (the jne)
  rel32(b + snapguard_resume);
  if (!write_bytes(cave, c.data(), c.size())) {
    return false;
  }
  uint8_t patch[7] = {0xE9, 0, 0, 0, 0, 0x90, 0x90};
  const auto rel = static_cast<int32_t>(cave_addr - (b + snapguard_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));
  if (!write_bytes(site, patch, sizeof(patch))) {
    return false;
  }
  snapguard_installed = true;
  return true;
}

// PER-CLIENT BUFFER GUARD - the 0x02C3DE7F crash.
//
// 0x01C93850(localClientNum) clears a client's buffers:
//   01C938E9  mov rcx,[rbx+rsi+0x0AE92D48] ; A[lc]      -> memset N
//   01C938FF  mov rcx,[rbx+rsi+0x10615A60] ; C[lc]      -> memset N*8
// with rbx = lc*8 and rsi = the image base. For local client 2 those
// table entries are NULL, so the memset ran on rdi = 0 (`rep stosb`).
//
// The allocator that fills them (0x01C937D0) is COUNT-DRIVEN, not
// literal-2: it loops while `edi < [0x0F4B68C8]`, which a config copy
// at 0x01CB91F2 sets to 2 (measured statically; N = [0x0F4B68CC] =
// 0x700, so the buffers are 0x700 / 0x700 / 0x3800 bytes).
//
// WHY NOT SIMPLY RAISE THE COUNT. Array A holds two ROWS per client and
// the allocator addresses the second row as base+0x10, i.e. base +
// count*8 - so the row stride IS the client count. Its 49 readers do
// NOT recompute that: they index A with a flat index taken from one
// global (0x10615A28, single writer 0x01CE8D83), which already encodes
// row*count + client. Raising the count silently reinterprets every one
// of those reads. Array C's slot 2 (0x10615A70) is separately FOREIGN -
// the base of a stride-0x30 array with its own three users. Widening
// here would corrupt memory in two different ways at once, which is
// exactly what CLAUDE.md rule 4 exists to prevent.
//
// So answer honestly instead: a client this subsystem was never
// configured for has no buffers to clear, and clearing nothing is the
// correct result. The guard is the engine's OWN bound - the same
// `[0x0F4B68C8]` the allocator loops on - so it can never disagree with
// how many buffers actually exist. Nothing is faked and no state is
// invented; the function simply returns for indices it has no data for.
constexpr uint32_t scene_a_rva = 0xAE13DC8;
constexpr uint32_t scene_b_rva = 0xAE13DD8;
constexpr uint32_t scene_c_rva = 0x10596AE0;
constexpr uint32_t scene_size_rva = 0xF43794C;
constexpr uint32_t scene_b_store_rva = 0x1C87448;
constexpr uint8_t scene_b_store_expect[] = {0x48, 0x89, 0x84, 0x33,
                                            0xD8, 0x3D, 0xE1, 0x0A};
constexpr size_t scene_slots = 4;
bool scene_b_relocated = false;
bool scene_buffers_filled = false;
uint32_t scene_b_new_rva = 0;
// C's new 4-slot block (batch 8, 2026-09-26). C[2]/C[3] at the OLD base
// (0x10615A70/78) are NOT padding - three leas (0x01C86C24, 0x01C892A3,
// 0x01C8FF29) use 0x10615A70 as the base of another array - so until C is
// moved, nothing may be written there. 0 = C not relocated.
size_t scene_c_new = 0;
constexpr uint32_t pcbuf_clear_rva = 0x1C87480;
constexpr uint32_t pcbuf_count_rva = 0xF437948;
constexpr uint8_t pcbuf_expected[] = {0x48, 0x89, 0x5C, 0x24, 0x08};
bool pcbuf_guarded = false;

bool install_perclient_buffer_guard() {
  if (pcbuf_guarded) {
    return true;
  }
  const auto b = base();
  auto *fn = reinterpret_cast<uint8_t *>(b + pcbuf_clear_rva);
  if (!readable(fn, sizeof(pcbuf_expected)) ||
      std::memcmp(fn, pcbuf_expected, sizeof(pcbuf_expected)) != 0) {
    return false;
  }
  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x40));
  if (!cave) {
    return false;
  }
  const auto cave_addr = reinterpret_cast<size_t>(cave);
  std::vector<uint8_t> c;
  const auto rel32 = [&](const size_t tgt) {
    const auto v = static_cast<int32_t>(tgt - (cave_addr + c.size() + 4));
    const auto *p = reinterpret_cast<const uint8_t *>(&v);
    c.insert(c.end(), p, p + 4);
  };
  // Gate on the POINTER, not on the count.
  //
  // This used to be `cmp ecx,[count] ; jl work ; ret`, which skipped the
  // whole clear for lc >= 2. That stopped the crash but left client 2's
  // scene buffers permanently uncleared - fine while they did not exist,
  // wrong now that fill_scene_buffers() allocates them, because this
  // clear is what zeroes A[lc] and C[lc] on every call.
  //
  // The thing that actually faults is C[lc] being NULL (the memset at
  // 0x01C938F9 -> rep stosb at 0x02C3DE7F), so test exactly that. The
  // guard then self-disables the moment real buffers exist, and still
  // covers the window before the fill has run.
  //
  //   movsxd rax, ecx            ; lc
  //   mov    r10, &C[0]
  //   mov    rax, [r10+rax*8]    ; C[lc]
  //   test   rax, rax
  //   jnz    work
  //   ret
  //
  // rax/r10 are volatile and hold nothing at entry (args arrive in
  // rcx..r9), so clobbering them ahead of the displaced prologue is safe.
  c.insert(c.end(), {0x48, 0x63, 0xC1});
  c.insert(c.end(), {0x49, 0xBA});
  {
    // the moved C when batch 8 took (it runs before this guard)
    const auto c_base =
        static_cast<uint64_t>(scene_c_new ? scene_c_new : b + scene_c_rva);
    const auto *cp = reinterpret_cast<const uint8_t *>(&c_base);
    c.insert(c.end(), cp, cp + 8);
  }
  c.insert(c.end(), {0x49, 0x8B, 0x04, 0xC2});
  c.insert(c.end(), {0x48, 0x85, 0xC0});
  c.insert(c.end(), {0x75, 0x01});
  c.insert(c.end(), {0xC3});
  // work: displaced prologue, then jump back past it
  c.insert(c.end(), pcbuf_expected, pcbuf_expected + sizeof(pcbuf_expected));
  c.insert(c.end(), {0xE9});
  rel32(b + pcbuf_clear_rva + sizeof(pcbuf_expected));
  if (!write_bytes(cave, c.data(), c.size())) {
    return false;
  }
  uint8_t patch[5] = {0xE9};
  const auto rel = static_cast<int32_t>(cave_addr - (b + pcbuf_clear_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));
  if (!write_bytes(fn, patch, sizeof(patch))) {
    return false;
  }
  pcbuf_guarded = true;
  return true;
}

// ============ R_InitSceneBuffers: the renderer's per-client scene
// buffers, and the reason the third pane crashed ============
//
// Turning the cgame frame loop on for client 2 died at 0x02C3DE7F -
// `rep stosb` with rdi = NULL. Traced to its source:
//
//   01C938F9  mov eax, [0x0F4B68CC]              ; element count, 1792
//   01C938FF  mov rcx, [rbx + rdi + 0x10615A60]  ; C[lc]  -> NULL at lc=2
//   01C93907  shl r8, 3                          ; count*8 bytes
//   01C9390D  call 0x02C3DE60                    ; memset -> 0x02C3DE7F
//
// C[2] is NULL because the allocator that fills it never runs for client
// 2. That allocator is 0x01C937D0, alloc tag "R_InitSceneBuffers":
//
//   loop: A[lc] = alloc(size)     -> [rbx+rsi+0x0AE92D48]
//         B[lc] = alloc(size)     -> [rbx+rsi+0x0AE92D58]
//         C[lc] = alloc(size*8)   -> [rbx+rsi+0x10615A60]
//         inc lc ; cmp lc, [0x0F4B68C8] ; jb loop
//
// and its bound is 2. PS4 R_InitSceneBuffers (0x0093B060) is the same
// function with that bound written as a LITERAL FOUR:
//
//   0093B06F  cmp dword [rbp-4], 4
//   0093B097  call R_AllocGlobalVariable(gfxCfg[4], "R_InitSceneBuffers")
//   0093B0A8  mov [scene + rdx*8 + 0x7AE068], rax
//
// PC's [0x0F4B68C8]/[0x0F4B68CC] pair is PS4's gfxCfg[0]/gfxCfg[4],
// copied by the single writer at 0x01CB91F0 (`mov eax,[rcx] ; mov
// [0x0F4B68C8],eax ; mov eax,[rcx+4] ; mov [0x0F4B68CC],eax`). PC also
// keeps PS4's two singleton buffers, at 0x10615A50/0x10615A58, sitting
// immediately before C[0].
//
// WHY NOT JUST RAISE THE BOUND TO 4, THE WAY THE CONSOLE HAS IT.
// CLAUDE.md rule 4 says find_lea every array a widened bound touches.
// Doing that turned up two reasons not to:
//
//   * [0x0F4B68C8] also bounds a RELEASE loop at 0x01C554C0 walking a
//     DIFFERENT per-client array (rsi = r13 + 0x30 + lc*0xF08, then
//     16 x 5 virtual releases). Raising the count drags that array in
//     as well, and its base is a register - not classifiable statically.
//   * A[2] IS B[0]. A and B are adjacent [2] arrays 0x10 apart, so
//     A[2] = 0x0AE92D48 + 2*8 = 0x0AE92D58 = B's base. At bound 3 the
//     loop's own A[2] store would destroy B[0].
//
// So the bound stays at 2 - every engine loop keeps its present extent,
// zero collateral - and the two slots the loop skips get filled here
// instead. The buffers are real, allocated at the sizes the engine's own
// code uses; no count is faked to make the engine believe in a state.
//
// Measured layout (whole-image displacement scan over
// 0x0AE92D40..0x0AE92DA0 and 0x10615A58..0x10615AB8 - only these three
// addresses are referenced at all in the region):
//
//   A 0x0AE92D48  10 refs  [0]=D48 [1]=D50 | [2] would be D58 = B[0]
//   B 0x0AE92D58   1 ref   [0]=D58 [1]=D60 | [2]=D68 [3]=D70 unref'd
//   C 0x10615A60   4 refs  [0]=A60 [1]=A68 | [2]=A70 [3]=A78 unref'd
//
// C[2]/C[3] are padding and take their buffers directly. A[2]/A[3] are
// foreign - they ARE B - so B is relocated first, which vacates exactly
// the two slots A needs. B is the cheap one to move: ONE reference in
// the whole image (its own store) against A's ten. B is in fact never
// read on PC at all - the clear at 0x01C93850 zeroes A[lc] and C[lc] and
// never touches it - but moving it is still strictly safer than letting
// A[2] land on it, and it is what frees A's slots.

// Move B out of A's way. One store, disp32 at +4. The new array has to
// be reachable as module_base + disp32, because the instruction
// addresses it as [rbx + rsi + disp] with rsi = the module base
// (0x01C937C2 loads it with `lea rsi,[rip-0x1c937c9]`, i.e. RVA 0).
bool relocate_scene_buffer_b() {
  if (scene_b_relocated) {
    return true;
  }
  const auto b = base();
  auto *insn = reinterpret_cast<uint8_t *>(b + scene_b_store_rva);
  if (!readable(insn, sizeof(scene_b_store_expect)) ||
      std::memcmp(insn, scene_b_store_expect, sizeof(scene_b_store_expect)) !=
          0) {
    note("[splitscreen] scene B store mismatch, not relocating\n");
    return false;
  }

  auto *fresh = static_cast<uint8_t *>(
      allocate_near_module(scene_slots * sizeof(void *)));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, scene_slots * sizeof(void *));

  const auto delta = reinterpret_cast<size_t>(fresh) - b;
  if (delta > 0x7FFFFFFF) {
    note("[splitscreen] scene B cave out of disp32 range\n");
    return false;
  }
  const auto new_rva = static_cast<uint32_t>(delta);

  // carry the two live pointers across before anything reads them
  std::memcpy(fresh, reinterpret_cast<const void *>(b + scene_b_rva),
              2 * sizeof(void *));

  if (!write_bytes(insn + 4, &new_rva, sizeof(new_rva))) {
    return false;
  }
  scene_b_new_rva = new_rva;
  scene_b_relocated = true;
  note("[splitscreen] scene buffer B [2]->[4] at RVA 0x%08X\n", new_rva);
  return true;
}

// Allocate what the count-2 loop skips. This sits on the RENDERER
// pipeline - the one that stays alive through a launch, the 2026-08-18
// finding - and early-outs to nothing once the slots are filled.
void fill_scene_buffers() {
  if (scene_buffers_filled || !scene_b_relocated || !raise_local_client_count) {
    return;
  }
  const auto b = base();
  uint32_t elems = 0;
  std::memcpy(&elems, reinterpret_cast<const void *>(b + scene_size_rva),
              sizeof(elems));
  if (elems == 0 || elems > 0x100000) {
    return; // config not copied yet - 0x01CB91F0 has not run
  }

  auto **a = reinterpret_cast<void **>(b + scene_a_rva);
  // CORRECTED 2026-09-26: this used to fill C[2]/C[3] at the OLD base,
  // i.e. write two pointers over the start of the foreign array at
  // 0x10615A70. Only the relocated C (batch 8) has real slots 2/3; if it
  // is not in, nothing here may touch C.
  if (!scene_c_new) {
    return;
  }
  auto **c = reinterpret_cast<void **>(scene_c_new);
  auto **bb = reinterpret_cast<void **>(b + scene_b_new_rva);
  if (!readable(a, scene_slots * sizeof(void *)) ||
      !readable(c, scene_slots * sizeof(void *))) {
    return;
  }
  if (a[0] == nullptr || c[0] == nullptr) {
    return; // R_InitSceneBuffers has not run yet
  }
  if (a[2] != nullptr && c[2] != nullptr) {
    scene_buffers_filled = true;
    return;
  }

  const auto grab = [](const size_t bytes) -> void * {
    // zero-filled by the OS, which is the state the engine's own
    // allocator hands back here
    return VirtualAlloc(nullptr, bytes, MEM_COMMIT | MEM_RESERVE,
                        PAGE_READWRITE);
  };

  for (size_t lc = 2; lc < scene_slots; ++lc) {
    if (a[lc] == nullptr) {
      auto *p = grab(elems);
      if (!p) {
        return;
      }
      a[lc] = p;
    }
    if (bb[lc] == nullptr) {
      auto *p = grab(elems);
      if (!p) {
        return;
      }
      bb[lc] = p;
    }
    if (c[lc] == nullptr) {
      auto *p = grab(static_cast<size_t>(elems) * 8);
      if (!p) {
        return;
      }
      c[lc] = p;
    }
  }
  scene_buffers_filled = true;
  note("[splitscreen] scene buffers allocated for clients 2/3"
       " (%u elements)\n",
       elems);
}

// ============ cgEntCollWorld / cgEntCollNodes: the entity-collision
// pair, and the crash that follows the scene buffers ============
//
// With the scene buffers fixed, the cgame frame loop for client 2 gets
// further and dies here instead (captured 2026-09-14, pid 20864):
//
//   0xC0000005 at RVA 0x02C3DE7F   rdi = 0x4196000041960000   rbx = 2
//
// rdi is FLOATS, not a pointer. The faulting code is PC's inlined
// CG_ClearEntityCollWorld, reached from CG_SetInitialSnapshot
// (0x00FC72BA):
//
//   0058B2DE  lea rdi,[rsi + 0x047E3BA0]   ; cgEntCollWorld, a [2] array
//   0058B2EB  imul rax, rax, 0x401C        ; lc * stride
//   0058B2F8  call memset                  ; memset(world[lc], 0, 0x401C)
//   0058B2FD  mov rcx,[rsi + rbx*8 + 0x032DF8B0]  ; cgEntCollNodes[lc]
//   0058B307  mov r8d, 0xC400
//            call memset                   ; memset(nodes[lc], 0, 0xC400)
//
// and cgEntCollNodes[2] statically contains 0x4196000041960000 - real
// float data belonging to a neighbour, not a buffer. Same for [3]
// (0x419600003F800000). So both arrays are [2] and slot 2 is FOREIGN.
//
// THIS ALSO PROVES CG_SetInitialSnapshot(2) RUNS. The crash is inside
// the initial-snapshot path, so client 2 does receive its first
// snapshot. That matters because PS4 CG_ClearEntityCollWorld (0x189890)
// is the function that BUILDS the sector free-list:
//
//   memset(&cgEntCollWorld[lc], 0, 0x401C)
//   memset(cgEntCollNodes[lc], 0, 0xC400)
//   CM_ModelBounds(...) ; world.header = 2
//   for (i = 2; i < 0x3FF; i++) world.sector[i].next = i + 1
//
// and its only caller is CG_SetInitialSnapshot. So the engine will
// initialize slots 2/3 ITSELF once they point at real storage. Nothing
// here hand-builds a free-list - that would be faking a state.
//
// WHY THE 2026-08-25 ATTEMPT (ae57c92) HUNG, and was reverted.
// It relocated the world with a HAND-BUILT table of 5 sites. An
// exhaustive scan on 2026-09-14 found THIRTEEN. The 8 it missed were
// all field accessors - six at world+0x1C, two at world+0x28 - so part
// of the engine kept reading the OLD array while the rest used the new
// one. That is exactly the "partial rewrite leaves the engine split
// across both arrays" failure CLAUDE.md records for clientUIActives,
// and it is why the node walk at 0x0058B8F4 never reached its
// terminator: sampled 60/60 on one thread by tools/sample_rip.py.
//
// So these tables are GENERATED, never typed - tools/gen_entcoll_sites.py
// re-emits them from the image. Regenerate rather than edit by hand.
// Note the target_off column: a site may point at a FIELD of element 0,
// so a relocation must preserve that offset, not just swap the base.
//
// Neither array can grow in place, both were checked:
//   world[2] would start at 0x047EBBD8, and the 0x8038 span it needs
//   holds 13 separately-referenced addresses.
//   nodes[2]/[3] at 0x032DF8C0/C8 hold the float constants above.
struct entcoll_site {
  uint32_t rva;        // instruction start
  uint8_t disp_off;    // byte offset of the disp32 inside it
  uint8_t insn_len;    // total instruction length
  bool rip;            // true: disp is rip-relative; false: absolute RVA
  uint32_t target_off; // target's offset inside element 0
};

constexpr uint32_t entcoll_world_base = 0x4764BA0;
constexpr uint32_t entcoll_world_stride = 0x401C;
constexpr uint32_t entcoll_nodes_base = 0x32608B0;
constexpr uint32_t entcoll_node_bytes = 0xC400;
constexpr size_t entcoll_slots = 4;

constexpr entcoll_site entcoll_world_sites[] = {
    {0x58B173, 3, 7, true, 0x0028},  // lea rcx,[rip+..]
    {0x58B1F6, 3, 7, true, 0x0000},  // lea rcx,[rip+..]
    {0x58B2DE, 3, 7, false, 0x0000}, // lea rdi,[rsi+0x047E3BA0]
    {0x58B445, 3, 7, true, 0x0000},  // lea rcx,[rip+..]
    {0x58B60C, 3, 7, false, 0x0000}, // lea rdi,[rsi+0x047E3BA0]
    {0x58B710, 3, 7, true, 0x0028},  // lea rax,[rip+..]
    {0x58B886, 3, 7, true, 0x0000},  // lea rcx,[rip+..]
    {0x129DE34, 3, 7, true, 0x001C}, // lea rcx,[rip+..]   MISSED by ae57c92
    {0x129DF28, 3, 7, true, 0x001C}, // lea rcx,[rip+..]   MISSED
    {0x129E784, 3, 7, true, 0x001C}, // lea r10,[rip+..]   MISSED
    {0x129EA61, 3, 7, true, 0x001C}, // lea r10,[rip+..]   MISSED
    {0x12AEC27, 3, 7, true, 0x001C}, // lea rdi,[rip+..]   MISSED
    {0x12AEE47, 3, 7, true, 0x001C}, // lea rdi,[rip+..]   MISSED
};

constexpr entcoll_site entcoll_node_sites[] = {
    {0x58B17A, 3, 7, true, 0x0000},  // lea r10,[rip+..]
    {0x58B2FD, 4, 8, false, 0x0000}, // mov rcx,[rsi+rbx*8+0x032DF8B0]
    {0x58B44F, 3, 7, true, 0x0000},  // lea rcx,[rip+..]
    {0x58B660, 4, 8, false, 0x0000}, // mov rsi,[rsi+rdx*8+..]
    {0x58B721, 4, 8, false, 0x0000}, // mov r9,[rsi+r9*8+..]
    {0x58B853, 4, 8, false, 0x0000}, // lea rdx,[r12+..]
    {0x70DE25, 4, 8, false, 0x0000}, // mov rax,[rcx+rax*8+..]
    {0x70FB49, 4, 8, false, 0x0000}, // mov rax,[rdx+rax*8+..]
    {0x72E424, 4, 8, false, 0x0000}, // mov rax,[r8+r14*8+..]
    {0x72E607, 4, 8, false, 0x0000}, // mov rax,[rcx+r14*8+..]
    {0x731AA3, 4, 8, false, 0x0000}, // mov rax,[r12+r14*8+..]
    {0xFEEED2, 3, 7, true, 0x0000},  // lea rsi,[rip+..]
    {0x129DE45, 3, 7, true, 0x0000}, // lea r8,[rip+..]
    {0x129DECC, 3, 7, true, 0x0000}, // lea r8,[rip+..]
    {0x129DF21, 3, 7, true, 0x0000}, // lea r8,[rip+..]
    {0x129E78B, 3, 7, true, 0x0000}, // lea r8,[rip+..]
    {0x129E820, 3, 7, true, 0x0000}, // lea r8,[rip+..]
    {0x129EA5A, 3, 7, true, 0x0000}, // lea r8,[rip+..]
    {0x12AEC63, 3, 7, true, 0x0000}, // lea rax,[rip+..]
};

bool entcoll_relocated = false;
size_t entcoll_world_new = 0;
size_t entcoll_nodes_new = 0;

// Point one site table at `new_abs`, saving each original disp32 so the
// whole group can be rolled back. All-or-nothing: a half-rewritten table
// is the failure mode that produced the 2026-08-25 hang.
// Every site is VERIFIED before anything is written: its current
// displacement must resolve to old_rva + target_off, the exact reference
// the table was generated from. One mismatch and nothing is written at
// all (CLAUDE.md: fail safe, never fail dirty). Until the 2026-09-25
// audit this overwrote blindly - which is how cl_voiceCommunication got
// relocated TWICE: reloc_tables moved 11 of its refs at startup, then a
// second copy of the relocation rewrote all 12 onto another block that
// held a copy of the already-zeroed original. Nothing flagged it.
bool rewrite_entcoll(const entcoll_site *sites, const size_t count,
                     const uint32_t old_rva, const size_t new_abs,
                     int32_t *saved) {
  const auto b = base();

  for (size_t i = 0; i < count; ++i) {
    auto *insn = reinterpret_cast<uint8_t *>(b + sites[i].rva);
    if (!readable(insn, sites[i].insn_len)) {
      note("[splitscreen] entcoll site 0x%08X unreadable\n", sites[i].rva);
      return false;
    }
    std::memcpy(&saved[i], insn + sites[i].disp_off, sizeof(int32_t));
    const auto want = static_cast<int64_t>(b) + old_rva + sites[i].target_off;
    const auto have =
        sites[i].rip
            ? static_cast<int64_t>(b + sites[i].rva + sites[i].insn_len) +
                  saved[i]
            : static_cast<int64_t>(b) + saved[i];
    if (have != want) {
      note("[splitscreen] site 0x%08X does not reference 0x%08X+0x%X - "
           "nothing written\n",
           sites[i].rva, old_rva, sites[i].target_off);
      return false;
    }
  }

  size_t done = 0;
  const auto rollback = [&] {
    for (size_t j = 0; j < done; ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + sites[j].rva);
      write_bytes(insn + sites[j].disp_off, &saved[j], sizeof(int32_t));
    }
  };

  for (size_t i = 0; i < count; ++i) {
    auto *insn = reinterpret_cast<uint8_t *>(b + sites[i].rva);
    const auto target = new_abs + sites[i].target_off;
    int32_t disp = 0;

    if (sites[i].rip) {
      const auto end =
          static_cast<int64_t>(b + sites[i].rva + sites[i].insn_len);
      const auto delta = static_cast<int64_t>(target) - end;
      if (delta > INT32_MAX || delta < INT32_MIN) {
        note("[splitscreen] entcoll 0x%08X out of rip range\n", sites[i].rva);
        rollback();
        return false;
      }
      disp = static_cast<int32_t>(delta);
    } else {
      const auto rva = static_cast<int64_t>(target) - static_cast<int64_t>(b);
      if (rva < 0 || rva > INT32_MAX) {
        note("[splitscreen] entcoll 0x%08X out of abs range\n", sites[i].rva);
        rollback();
        return false;
      }
      disp = static_cast<int32_t>(rva);
    }

    if (!write_bytes(insn + sites[i].disp_off, &disp, sizeof(disp))) {
      note("[splitscreen] entcoll 0x%08X write failed\n", sites[i].rva);
      rollback();
      return false;
    }
    ++done;
  }
  return true;
}

// ============ COMPLETION of three August-era relocations (2026-09-28)
// ============
//
// tools/audit_reloc_tables.py re-derived the references of all 71 relocated
// [2]-per-local-client arrays with gen_reloc_sites.py v2 and compared them with
// the component's tables (data/reloc_audit_2026-09-28.txt). Every "missing"
// site was then read in the RUNNING game: these 167 still pointed at the OLD
// array while the rest of the engine used the moved one - the aaGlobArray
// failure (SoE aim+fire crash, 3fa25e3) in three more places:
//   playersKb (reloc_tables "players_kb", 213 of 375 refs moved): 162 sites
//     in 8 input functions (0x012F3520 alone 103, CL_ClearKeys' twin, plus
//     0x012FD660, 0x013053D0, 0x01308BF0, 0x0131B6E0, 0x0131B860,
//     0x0131BBFD, 0x0131C050) - key-state bytes read/cleared on the old copy:
//     stale for players 1-2, foreign globals for 3-4.
//   s_gamePads (gamepads_reloc_table, moved in the lobby): 0x022F1BC2
//     `cmp dword [slot1.device], 8` and 0x022F3271 `cmp byte [slot].enabled`.
//   numdestructibles: the audit's 3 `lea reg,[base]` "misses" are NOT its
//     references - they are END MARKERS of the array before it
//     (0x17F37CE0, 128 x 0x108: `lea r9,[arr]; lea r8,[arr+0x8400]; ...
//     add rax,0x108; cmp rax,r8; jl`). Completing them (0dc9905) crashed at
//     boot at 0x0237BF6C - the loop ran off into the new block. Removed.
//     LESSON: "still targets old base+0" cannot tell a base reference from
//     the previous array's end marker - read the loop before rewriting.
// Not completed on purpose (verified false positives): client_ui's two
// +0x22E0 hits are a neighbouring [2] x 0x248 array's vector constructor
// (0x02F09202 `lea rcx,[0x14FB5700]; mov edx,0x248; mov r8d,2`), and the
// cg_clientents30 / scene_pc480 hits decode non-code bytes (data after a
// ret, mid-instruction in flattened Arxan functions).
// Each list holds only sites whose live disp still targets the old array
// (data/reloc_sites/completion_2026-09-28.txt); rewrite_entcoll re-verifies
// every one and writes all or nothing. The new base is derived from a site
// the original relocation already moved.
constexpr entcoll_site players_kb_completion_sites[] = {
    {0x12F3574, 4, 9, false, 0x01A8}, // cmp byte ptr [rbx + r14 + 0x52f2b98], 0
    {0x12F357F, 4, 9, false, 0x01A9}, // cmp byte ptr [rbx + r14 + 0x52f2b99], 0
    {0x12F3591, 4, 9, false, 0x01A9}, // mov byte ptr [rbx + r14 + 0x52f2b99], 0
    {0x12F359A, 4, 9, false, 0x01C0}, // cmp byte ptr [rbx + r14 + 0x52f2bb0], 0
    {0x12F35A5, 4, 9, false, 0x01C1}, // cmp byte ptr [rbx + r14 + 0x52f2bb1], 0
    {0x12F35B7, 4, 9, false, 0x01C1}, // mov byte ptr [rbx + r14 + 0x52f2bb1], 0
    {0x12F35C0, 4, 9, false, 0x0220}, // cmp byte ptr [rbx + r14 + 0x52f2c10], 0
    {0x12F35CB, 4, 9, false, 0x0221}, // cmp byte ptr [rbx + r14 + 0x52f2c11], 0
    {0x12F35DD, 4, 9, false, 0x0221}, // mov byte ptr [rbx + r14 + 0x52f2c11], 0
    {0x12F35E6, 4, 9, false, 0x0238}, // cmp byte ptr [rbx + r14 + 0x52f2c28], 0
    {0x12F35F1, 4, 9, false, 0x0239}, // cmp byte ptr [rbx + r14 + 0x52f2c29], 0
    {0x12F3603, 4, 9, false, 0x0239}, // mov byte ptr [rbx + r14 + 0x52f2c29], 0
    {0x12F360C, 4, 9, false, 0x0250}, // cmp byte ptr [rbx + r14 + 0x52f2c40], 0
    {0x12F3617, 4, 9, false, 0x0251}, // cmp byte ptr [rbx + r14 + 0x52f2c41], 0
    {0x12F3629, 4, 9, false, 0x0251}, // mov byte ptr [rbx + r14 + 0x52f2c41], 0
    {0x12F3632, 4, 9, false, 0x0268}, // cmp byte ptr [rbx + r14 + 0x52f2c58], 0
    {0x12F363D, 4, 9, false, 0x0269}, // cmp byte ptr [rbx + r14 + 0x52f2c59], 0
    {0x12F364F, 4, 9, false, 0x0269}, // mov byte ptr [rbx + r14 + 0x52f2c59], 0
    {0x12F3658, 4, 9, false, 0x0280}, // cmp byte ptr [rbx + r14 + 0x52f2c70], 0
    {0x12F3663, 4, 9, false, 0x0281}, // cmp byte ptr [rbx + r14 + 0x52f2c71], 0
    {0x12F3675, 4, 9, false, 0x0281}, // mov byte ptr [rbx + r14 + 0x52f2c71], 0
    {0x12F367E, 4, 9, false, 0x0298}, // cmp byte ptr [rbx + r14 + 0x52f2c88], 0
    {0x12F3689, 4, 9, false, 0x0299}, // cmp byte ptr [rbx + r14 + 0x52f2c89], 0
    {0x12F369B, 4, 9, false, 0x0299}, // mov byte ptr [rbx + r14 + 0x52f2c89], 0
    {0x12F36A4, 4, 9, false, 0x02B0}, // cmp byte ptr [rbx + r14 + 0x52f2ca0], 0
    {0x12F36AF, 4, 9, false, 0x02B1}, // cmp byte ptr [rbx + r14 + 0x52f2ca1], 0
    {0x12F36C1, 4, 9, false, 0x02B1}, // mov byte ptr [rbx + r14 + 0x52f2ca1], 0
    {0x12F36CA, 4, 9, false, 0x02C8}, // cmp byte ptr [rbx + r14 + 0x52f2cb8], 0
    {0x12F36D5, 4, 9, false, 0x02C9}, // cmp byte ptr [rbx + r14 + 0x52f2cb9], 0
    {0x12F36E7, 4, 9, false, 0x02C9}, // mov byte ptr [rbx + r14 + 0x52f2cb9], 0
    {0x12F36F0, 4, 9, false, 0x0100}, // cmp byte ptr [rbx + r14 + 0x52f2af0], 0
    {0x12F36FB, 4, 9, false, 0x0101}, // cmp byte ptr [rbx + r14 + 0x52f2af1], 0
    {0x12F370D, 4, 9, false, 0x0101}, // mov byte ptr [rbx + r14 + 0x52f2af1], 0
    {0x12F3716, 4, 9, false, 0x02E0}, // cmp byte ptr [rbx + r14 + 0x52f2cd0], 0
    {0x12F3721, 4, 9, false, 0x02E1}, // cmp byte ptr [rbx + r14 + 0x52f2cd1], 0
    {0x12F3733, 4, 9, false, 0x02E1}, // mov byte ptr [rbx + r14 + 0x52f2cd1], 0
    {0x12F373C, 4, 9, false, 0x0328}, // cmp byte ptr [rbx + r14 + 0x52f2d18], 0
    {0x12F3747, 4, 9, false, 0x0329}, // cmp byte ptr [rbx + r14 + 0x52f2d19], 0
    {0x12F3789, 4, 9, false, 0x0329}, // mov byte ptr [rbx + r14 + 0x52f2d19], 0
    {0x12F3792, 4, 9, false, 0x0340}, // cmp byte ptr [rbx + r14 + 0x52f2d30], 0
    {0x12F379D, 4, 9, false, 0x0341}, // cmp byte ptr [rbx + r14 + 0x52f2d31], 0
    {0x12F37AF, 4, 9, false, 0x0341}, // mov byte ptr [rbx + r14 + 0x52f2d31], 0
    {0x12F37B8, 4, 9, false, 0x0358}, // cmp byte ptr [rbx + r14 + 0x52f2d48], 0
    {0x12F37C3, 4, 9, false, 0x0359}, // cmp byte ptr [rbx + r14 + 0x52f2d49], 0
    {0x12F37D5, 4, 9, false, 0x0359}, // mov byte ptr [rbx + r14 + 0x52f2d49], 0
    {0x12F37DE, 4, 9, false, 0x0370}, // cmp byte ptr [rbx + r14 + 0x52f2d60], 0
    {0x12F37E9, 4, 9, false, 0x0371}, // cmp byte ptr [rbx + r14 + 0x52f2d61], 0
    {0x12F37F8, 4, 9, false, 0x0371}, // mov byte ptr [rbx + r14 + 0x52f2d61], 0
    {0x12F3801, 4, 9, false, 0x0388}, // cmp byte ptr [rbx + r14 + 0x52f2d78], 0
    {0x12F380C, 4, 9, false, 0x0389}, // cmp byte ptr [rbx + r14 + 0x52f2d79], 0
    {0x12F381E, 4, 9, false, 0x0389}, // mov byte ptr [rbx + r14 + 0x52f2d79], 0
    {0x12F3827, 4, 9, false, 0x03A0}, // cmp byte ptr [rbx + r14 + 0x52f2d90], 0
    {0x12F3832, 4, 9, false, 0x03A1}, // cmp byte ptr [rbx + r14 + 0x52f2d91], 0
    {0x12F3844, 4, 9, false, 0x03A1}, // mov byte ptr [rbx + r14 + 0x52f2d91], 0
    {0x12F384D, 4, 9, false, 0x03B8}, // cmp byte ptr [rbx + r14 + 0x52f2da8], 0
    {0x12F3858, 4, 9, false, 0x03B9}, // cmp byte ptr [rbx + r14 + 0x52f2da9], 0
    {0x12F386A, 4, 9, false, 0x03B9}, // mov byte ptr [rbx + r14 + 0x52f2da9], 0
    {0x12F3873, 4, 9, false, 0x03D0}, // cmp byte ptr [rbx + r14 + 0x52f2dc0], 0
    {0x12F387E, 4, 9, false, 0x03D1}, // cmp byte ptr [rbx + r14 + 0x52f2dc1], 0
    {0x12F3890, 4, 9, false, 0x03D1}, // mov byte ptr [rbx + r14 + 0x52f2dc1], 0
    {0x12F3899, 4, 9, false, 0x03E8}, // cmp byte ptr [rbx + r14 + 0x52f2dd8], 0
    {0x12F38A4, 4, 9, false, 0x03E9}, // cmp byte ptr [rbx + r14 + 0x52f2dd9], 0
    {0x12F38B3, 4, 9, false, 0x03E9}, // mov byte ptr [rbx + r14 + 0x52f2dd9], 0
    {0x12F38BC, 4, 9, false, 0x0460}, // cmp byte ptr [rbx + r14 + 0x52f2e50], 0
    {0x12F38C7, 4, 9, false, 0x0461}, // cmp byte ptr [rbx + r14 + 0x52f2e51], 0
    {0x12F38D9, 4, 9, false, 0x0461}, // mov byte ptr [rbx + r14 + 0x52f2e51], 0
    {0x12F38E2, 4, 9, false, 0x02F8}, // cmp byte ptr [rbx + r14 + 0x52f2ce8], 0
    {0x12F38ED, 4, 9, false, 0x02F9}, // cmp byte ptr [rbx + r14 + 0x52f2ce9], 0
    {0x12F38FC, 4, 9, false, 0x02F9}, // mov byte ptr [rbx + r14 + 0x52f2ce9], 0
    {0x12F3905, 4, 9, false, 0x0400}, // cmp byte ptr [rbx + r14 + 0x52f2df0], 0
    {0x12F3910, 4, 9, false, 0x0401}, // cmp byte ptr [rbx + r14 + 0x52f2df1], 0
    {0x12F3922, 4, 9, false, 0x0401}, // mov byte ptr [rbx + r14 + 0x52f2df1], 0
    {0x12F392B, 4, 9, false, 0x0418}, // cmp byte ptr [rbx + r14 + 0x52f2e08], 0
    {0x12F3936, 4, 9, false, 0x0419}, // cmp byte ptr [rbx + r14 + 0x52f2e09], 0
    {0x12F3948, 4, 9, false, 0x0419}, // mov byte ptr [rbx + r14 + 0x52f2e09], 0
    {0x12F3951, 4, 9, false, 0x0430}, // cmp byte ptr [rbx + r14 + 0x52f2e20], 0
    {0x12F395C, 4, 9, false, 0x0431}, // cmp byte ptr [rbx + r14 + 0x52f2e21], 0
    {0x12F396E, 4, 9, false, 0x0431}, // mov byte ptr [rbx + r14 + 0x52f2e21], 0
    {0x12F3977, 4, 9, false, 0x0448}, // cmp byte ptr [rbx + r14 + 0x52f2e38], 0
    {0x12F3982, 4, 9, false, 0x0449}, // cmp byte ptr [rbx + r14 + 0x52f2e39], 0
    {0x12F3996, 4, 9, false, 0x0449}, // mov byte ptr [rbx + r14 + 0x52f2e39], 0
    {0x12F39A4, 4, 9, false, 0x0130}, // cmp byte ptr [rbx + r14 + 0x52f2b20], 0
    {0x12F39AF, 4, 9, false, 0x0131}, // cmp byte ptr [rbx + r14 + 0x52f2b21], 0
    {0x12F39C1, 4, 9, false, 0x0131}, // mov byte ptr [rbx + r14 + 0x52f2b21], 0
    {0x12F39CA, 4, 9, false, 0x02F8}, // cmp byte ptr [rbx + r14 + 0x52f2ce8], 0
    {0x12F39D5, 4, 9, false, 0x02F9}, // cmp byte ptr [rbx + r14 + 0x52f2ce9], 0
    {0x12F39E7, 4, 9, false, 0x02F9}, // mov byte ptr [rbx + r14 + 0x52f2ce9], 0
    {0x12F39F0, 4, 9, false, 0x0130}, // cmp byte ptr [rbx + r14 + 0x52f2b20], 0
    {0x12F39FB, 4, 9, false, 0x0131}, // cmp byte ptr [rbx + r14 + 0x52f2b21], 0
    {0x12F3A0D, 4, 9, false, 0x0131}, // mov byte ptr [rbx + r14 + 0x52f2b21], 0
    {0x12F3A16, 4, 9, false, 0x0310}, // cmp byte ptr [rbx + r14 + 0x52f2d00], 0
    {0x12F3A21, 4, 9, false, 0x0311}, // cmp byte ptr [rbx + r14 + 0x52f2d01], 0
    {0x12F3A33, 4, 9, false, 0x0311}, // mov byte ptr [rbx + r14 + 0x52f2d01], 0
    {0x12F3A3C, 4, 9, false, 0x0100}, // cmp byte ptr [rbx + r14 + 0x52f2af0], 0
    {0x12F3A47, 4, 9, false, 0x0101}, // cmp byte ptr [rbx + r14 + 0x52f2af1], 0
    {0x12F3A5E, 4, 9, false, 0x0101}, // mov byte ptr [rbx + r14 + 0x52f2af1], 0
    {0x12F3AB2, 4, 9, false, 0x0130}, // cmp byte ptr [rbx + r14 + 0x52f2b20], 0
    {0x12F3ABD, 4, 9, false, 0x0131}, // cmp byte ptr [rbx + r14 + 0x52f2b21], 0
    {0x12F3ACF, 4, 9, false, 0x0131}, // mov byte ptr [rbx + r14 + 0x52f2b21], 0
    {0x12F3AE1, 4, 9, false, 0x0130}, // cmp byte ptr [rbx + r14 + 0x52f2b20], 0
    {0x12F3AEC, 4, 9, false, 0x0131}, // cmp byte ptr [rbx + r14 + 0x52f2b21], 0
    {0x12F3AFE, 4, 9, false, 0x0131}, // mov byte ptr [rbx + r14 + 0x52f2b21], 0
    {0x12FEEE7, 3, 8, false, 0x0160}, // cmp byte ptr [rdi + rax + 0x52f2b50], 0
    {0x12FEEF1, 3, 8, false, 0x0161}, // cmp byte ptr [rdi + rax + 0x52f2b51], 0
    {0x12FEF41, 4, 9, false, 0x0160}, // cmp byte ptr [rdi + r14 + 0x52f2b50], 0
    {0x12FEF4C, 4, 9, false, 0x0161}, // cmp byte ptr [rdi + r14 + 0x52f2b51], 0
    {0x12FEF65, 4, 9, false, 0x0161}, // mov byte ptr [rdi + r14 + 0x52f2b51], 0
    {0x12FEF83, 4, 9, false, 0x0160}, // cmp byte ptr [rdi + r14 + 0x52f2b50], 0
    {0x12FEF8E, 4, 9, false, 0x0161}, // cmp byte ptr [rdi + r14 + 0x52f2b51], 0
    {0x12FF000, 4, 9, false, 0x0160}, // cmp byte ptr [rdi + r14 + 0x52f2b50], 0
    {0x12FF00B, 4, 9, false, 0x0161}, // cmp byte ptr [rdi + r14 + 0x52f2b51], 0
    {0x12FF0E3, 4, 9, false, 0x0160}, // cmp byte ptr [rdi + r14 + 0x52f2b50], 0
    {0x12FF0EE, 4, 9, false, 0x0161}, // cmp byte ptr [rdi + r14 + 0x52f2b51], 0
    {0x12FF101, 4, 9, false, 0x0161}, // mov byte ptr [rdi + r14 + 0x52f2b51], 0
    {0x12FF114, 4, 9, false, 0x0190}, // cmp byte ptr [rdi + r14 + 0x52f2b80], 0
    {0x12FF11F, 4, 9, false, 0x0191}, // cmp byte ptr [rdi + r14 + 0x52f2b81], 0
    {0x12FF130, 4, 9, false, 0x0178}, // cmp byte ptr [rdi + r14 + 0x52f2b68], 0
    {0x12FF13B, 4, 9, false, 0x0179}, // cmp byte ptr [rdi + r14 + 0x52f2b69], 0
    {0x12FF178, 4, 9, false, 0x0190}, // cmp byte ptr [rdi + r14 + 0x52f2b80], 0
    {0x12FF183, 4, 9, false, 0x0191}, // cmp byte ptr [rdi + r14 + 0x52f2b81], 0
    {0x12FF196, 4, 9, false, 0x0191}, // mov byte ptr [rdi + r14 + 0x52f2b81], 0
    {0x12FF19F, 4, 9, false, 0x0178}, // cmp byte ptr [rdi + r14 + 0x52f2b68], 0
    {0x12FF1AA, 4, 9, false, 0x0179}, // cmp byte ptr [rdi + r14 + 0x52f2b69], 0
    {0x12FF1BD, 4, 9, false, 0x0179}, // mov byte ptr [rdi + r14 + 0x52f2b69], 0
    {0x12FF1EA, 4, 9, false, 0x0190}, // cmp byte ptr [rdi + r14 + 0x52f2b80], 0
    {0x12FF1F5, 4, 9, false, 0x0191}, // cmp byte ptr [rdi + r14 + 0x52f2b81], 0
    {0x12FF208, 4, 9, false, 0x0191}, // mov byte ptr [rdi + r14 + 0x52f2b81], 0
    {0x12FF211, 4, 9, false, 0x0178}, // cmp byte ptr [rdi + r14 + 0x52f2b68], 0
    {0x12FF21C, 4, 9, false, 0x0179}, // cmp byte ptr [rdi + r14 + 0x52f2b69], 0
    {0x12FF22F, 4, 9, false, 0x0179}, // mov byte ptr [rdi + r14 + 0x52f2b69], 0
    {0x1306C23, 3, 8, false, 0x0208}, // cmp byte ptr [rbx + rsi + 0x52f2bf8], 0
    {0x1306C2D, 3, 8, false, 0x0209}, // cmp byte ptr [rbx + rsi + 0x52f2bf9], 0
    {0x1306C45, 3, 8, false, 0x0209}, // mov byte ptr [rbx + rsi + 0x52f2bf9], 0
    {0x1306C50, 3, 8, false, 0x01D8}, // cmp byte ptr [rbx + rsi + 0x52f2bc8], 0
    {0x1306C5A, 3, 8, false, 0x01D9}, // cmp byte ptr [rbx + rsi + 0x52f2bc9], 0
    {0x1306C71, 3, 8, false, 0x01F0}, // cmp byte ptr [rbx + rsi + 0x52f2be0], 0
    {0x1306C7B, 3, 8, false, 0x01F1}, // cmp byte ptr [rbx + rsi + 0x52f2be1], 0
    {0x1306CBF, 3, 8, false, 0x020A}, // cmp byte ptr [rbx + rsi + 0x52f2bfa], 0
    {0x1306CD0, 3, 8, false, 0x020A}, // mov byte ptr [rbx + rsi + 0x52f2bfa], 1
    {0x1306CF0, 4, 10, false,
     0x01D9}, // mov word ptr [rbx + rsi + 0x52f2bc9], 0x100
    {0x1306CFA, 4, 10, false,
     0x01F1}, // mov word ptr [rbx + rsi + 0x52f2be1], 0x100
    {0x1306D11, 3, 8, false, 0x020A}, // mov byte ptr [rbx + rsi + 0x52f2bfa], 0
    {0x1306D19, 3, 8, false, 0x01DA}, // cmp byte ptr [rbx + rsi + 0x52f2bca], 0
    {0x1306D28, 3, 8, false, 0x01D8}, // cmp byte ptr [rbx + rsi + 0x52f2bc8], 0
    {0x1306D32, 3, 8, false, 0x01D9}, // cmp byte ptr [rbx + rsi + 0x52f2bc9], 0
    {0x1306D44, 3, 8, false, 0x01D9}, // mov byte ptr [rbx + rsi + 0x52f2bc9], 0
    {0x1306D59, 3, 8, false, 0x01F2}, // cmp byte ptr [rbx + rsi + 0x52f2be2], 0
    {0x1306D68, 3, 8, false, 0x01F0}, // cmp byte ptr [rbx + rsi + 0x52f2be0], 0
    {0x1306D72, 3, 8, false, 0x01F1}, // cmp byte ptr [rbx + rsi + 0x52f2be1], 0
    {0x1306D84, 3, 8, false, 0x01F1}, // mov byte ptr [rbx + rsi + 0x52f2be1], 0
    {0x1308DD0, 4, 9, false, 0x01A8}, // mov byte ptr [rax + r14 + 0x52f2b98], 1
    {0x1308DD9, 4, 9, false, 0x01A9}, // mov byte ptr [rax + r14 + 0x52f2b99], 1
    {0x131B73B, 4, 9, false, 0x02B0}, // cmp byte ptr [rdx + r8 + 0x52f2ca0], 0
    {0x131B746, 4, 9, false, 0x0118}, // cmp byte ptr [rdx + r8 + 0x52f2b08], 0
    {0x131B8BB, 4, 9, false, 0x02B0}, // cmp byte ptr [rdx + r8 + 0x52f2ca0], 0
    {0x131B8C6, 4, 9, false, 0x0118}, // cmp byte ptr [rdx + r8 + 0x52f2b08], 0
    {0x131BC5E, 4, 9, false, 0x02F8}, // cmp byte ptr [rdi + r15 + 0x52f2ce8], 0
    {0x131BC71, 5, 11, false,
     0x02F8}, // mov word ptr [rdi + r15 + 0x52f2ce8], 0x101
    {0x131BCDA, 4, 9, false, 0x0118}, // cmp byte ptr [rdi + r15 + 0x52f2b08], 0
    {0x131BCF1, 5, 11, false,
     0x0118}, // mov word ptr [rdi + r15 + 0x52f2b08], 0x101
    {0x131BD01, 4, 9, false, 0x02B0}, // cmp byte ptr [rdi + r15 + 0x52f2ca0], 0
    {0x131BD10, 4, 9, false, 0x0118}, // cmp byte ptr [rdi + r15 + 0x52f2b08], 0
};
constexpr entcoll_site gamepads_completion_sites[] = {
    {0x2284AF2, 2, 7, true, 0x0074},  // cmp dword ptr [rip + 0x15b7c7bb], 8
    {0x22861A1, 4, 9, false, 0x0000}, // cmp byte ptr [rcx + r9 + 0x17e6e310], 0
};

// The array's current base, derived from a site that the original relocation
// rewrote: its target minus the field offset. 0 when that site still points at
// the old array (relocation not applied - then nothing may be completed).
size_t moved_base_from_site(const entcoll_site &s, const uint32_t old_base) {
  const auto b = base();
  const auto *insn = reinterpret_cast<const uint8_t *>(b + s.rva);
  if (!readable(insn, s.insn_len)) {
    return 0;
  }
  int32_t disp = 0;
  std::memcpy(&disp, insn + s.disp_off, sizeof(disp));
  const auto target =
      s.rip ? static_cast<int64_t>(b + s.rva + s.insn_len) + disp
            : static_cast<int64_t>(b) + static_cast<uint32_t>(disp);
  const auto moved = static_cast<size_t>(target) - s.target_off;
  return moved == b + old_base ? 0 : moved;
}

void trace_text(const char *text); // defined after trace_write

void complete_relocation(const char *name, const entcoll_site &moved_site,
                         const uint32_t old_base, const entcoll_site *sites,
                         const size_t count, int32_t *saved, bool &done) {
  if (done) {
    return;
  }
  const auto fresh = moved_base_from_site(moved_site, old_base);
  char line[160]{};
  if (!fresh) {
    std::snprintf(line, sizeof(line), "%s completion: base not moved - skipped",
                  name);
  } else if (!rewrite_entcoll(sites, count, old_base, fresh, saved)) {
    std::snprintf(line, sizeof(line),
                  "%s completion: a site did not match - NOTHING written",
                  name);
  } else {
    done = true;
    std::snprintf(
        line, sizeof(line),
        "%s completion: %zu remaining references moved to the new array", name,
        count);
  }
  trace_text(line);
}

bool players_kb_completed = false;
bool gamepads_completed = false;

// A site each relocation is known to rewrite (IN_Attack_Up's kbutton read -
// in reloc_tables' players_kb list; numdestructibles_sites[0]; the gamepad
// table's first ref is handled via its own verified destination instead).
constexpr entcoll_site players_kb_probe = {0x0131B2B1, 4, 8, false,
                                           0x0198}; // reloc.hpp players_kb row

void complete_players_kb() {
  static int32_t saved[std::size(players_kb_completion_sites)]{};
  complete_relocation(
      "playersKb", players_kb_probe, 0x052739F0, players_kb_completion_sites,
      std::size(players_kb_completion_sites), saved, players_kb_completed);
}

void complete_gamepads(const size_t destination_abs) {
  if (gamepads_completed) {
    return;
  }
  static int32_t saved[std::size(gamepads_completion_sites)]{};
  if (rewrite_entcoll(gamepads_completion_sites,
                      std::size(gamepads_completion_sites), 0x17DEF3E0,
                      destination_abs, saved)) {
    gamepads_completed = true;
    trace_text("s_gamePads completion: 2 remaining references moved");
  } else {
    trace_text("s_gamePads completion: a site did not match - NOTHING written");
  }
}

// Retarget one RIP-relative LOOP END MARKER (`lea reg, [rip+d32]` compared
// against an iterator) - verified: the instruction's current target must be
// old_target_rva exactly, otherwise nothing is written. Site tables cannot
// hold these because they point outside [base, base + 2*stride) - see
// tools/endmarker_scan.py. Returns false (and writes nothing) on mismatch.
bool retarget_end_marker(const uint32_t insn_rva, const uint8_t disp_off,
                         const uint8_t insn_len, const uint32_t old_target_rva,
                         const size_t new_target_abs) {
  const auto b = base();
  auto *disp_at = reinterpret_cast<uint8_t *>(b + insn_rva + disp_off);
  int32_t cur = 0;
  if (!readable(disp_at, sizeof(cur))) {
    return false;
  }
  std::memcpy(&cur, disp_at, sizeof(cur));
  const auto end = static_cast<int64_t>(b + insn_rva + insn_len);
  if (end + cur != static_cast<int64_t>(b + old_target_rva)) {
    note("[splitscreen] end marker 0x%08X does not point at 0x%08X - "
         "untouched\n",
         insn_rva, old_target_rva);
    return false;
  }
  const auto delta = static_cast<int64_t>(new_target_abs) - end;
  if (delta > INT32_MAX || delta < INT32_MIN) {
    return false;
  }
  const auto d32 = static_cast<int32_t>(delta);
  return write_bytes(disp_at, &d32, sizeof(d32));
}

bool relocate_entity_collision() {
  if (entcoll_relocated) {
    return true;
  }
  const auto b = base();

  auto *world = static_cast<uint8_t *>(
      allocate_near_module(entcoll_slots * entcoll_world_stride));
  auto *nodes = static_cast<uint8_t *>(
      allocate_near_module(entcoll_slots * sizeof(void *)));
  if (!world || !nodes) {
    return false;
  }
  std::memset(world, 0, entcoll_slots * entcoll_world_stride);
  std::memset(nodes, 0, entcoll_slots * sizeof(void *));

  // carry the two live elements across before any code reads them
  std::memcpy(world, reinterpret_cast<const void *>(b + entcoll_world_base),
              2 * entcoll_world_stride);
  std::memcpy(nodes, reinterpret_cast<const void *>(b + entcoll_nodes_base),
              2 * sizeof(void *));

  // real node storage for the two slots the engine never allocated.
  // The engine memsets these itself in CG_ClearEntityCollWorld; they
  // only have to EXIST and be the right size.
  auto **node_ptrs = reinterpret_cast<void **>(nodes);
  for (size_t lc = 2; lc < entcoll_slots; ++lc) {
    auto *buf = VirtualAlloc(nullptr, entcoll_node_bytes,
                             MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buf) {
      return false;
    }
    node_ptrs[lc] = buf;
  }

  static int32_t saved_world[std::size(entcoll_world_sites)]{};
  static int32_t saved_nodes[std::size(entcoll_node_sites)]{};

  if (!rewrite_entcoll(entcoll_world_sites, std::size(entcoll_world_sites),
                       entcoll_world_base, reinterpret_cast<size_t>(world),
                       saved_world)) {
    return false;
  }
  if (!rewrite_entcoll(entcoll_node_sites, std::size(entcoll_node_sites),
                       entcoll_nodes_base, reinterpret_cast<size_t>(nodes),
                       saved_nodes)) {
    // undo the world group too - either both move or neither does
    for (size_t j = 0; j < std::size(entcoll_world_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + entcoll_world_sites[j].rva);
      write_bytes(insn + entcoll_world_sites[j].disp_off, &saved_world[j],
                  sizeof(int32_t));
    }
    return false;
  }

  entcoll_world_new = reinterpret_cast<size_t>(world);
  entcoll_nodes_new = reinterpret_cast<size_t>(nodes);
  entcoll_relocated = true;
  note("[splitscreen] entity collision [2]->[4]: world RVA 0x%08X,"
       " nodes RVA 0x%08X (%zu + %zu sites)\n",
       static_cast<uint32_t>(entcoll_world_new - b),
       static_cast<uint32_t>(entcoll_nodes_new - b),
       std::size(entcoll_world_sites), std::size(entcoll_node_sites));
  return true;
}

// ============ the clientfield pending-callback array: the crash
// after the entity collision ============
//
// With scene buffers and the collision pair fixed, the frame loop for
// client 2 reaches a brand-new address (pid 5492, 2026-09-14):
//
//   0xC0000005 at RVA 0x00132F94
//   00132F52  movsxd rdx, ecx              ; lc
//   00132F55  lea r9, [r10 + rdx*4]
//   00132F59  mov r8d, [r9 + 0x20000]      ; count[lc]
//   00132F77  shl rdx, 0xb                 ; lc * 2048 entries
//   00132F8A  add rdx, r8
//   00132F8D  shl rdx, 5                   ; * 32 bytes
//   00132F91  add rdx, r10
//   00132F94  and dword [rdx + 0x18], ..   <- FAULT
//
// Its own error strings name it: "Ran out of clientfield callbacks"
// (0x02F9CCC0) and "Cannot add clientfield callbacks when no pending
// callback array allocated" (0x02F9CC70). Layout of that array:
//
//   entries  [0x00000, 0x20000)   2 clients x 2048 x 32 bytes
//   counts   +0x20000 + lc*4       2 x 4 bytes
//   total    0x20008
//
// It is a STATIC buffer at RVA 0x04A115A0. Chain proven without a live
// read: 0x008F26D5 `lea rax,[0x049D9430]` takes the parent struct (size
// 0x38168, three memsets), so `[parent + 0x38160]` IS the pointer
// global 0x04A11590, which statically holds the buffer's preferred VA
// and is filled by 0x008F26E3 `lea rax,[0x04A115A0]` / 0x008F26EA
// `mov [0x04A11590], rax`. Every consumer reads the pointer through
// +0x38160 (13 sites); only those two leas name the buffer itself.
//
// THE TRAP - why enlarging alone would be wrong: for lc=2 the entry
// region starts at +0x20000, exactly where the counts live. The count
// offset is hardcoded, so it must move with the buffer: to 0x40000,
// after four clients' entries. The whole layout is 9 immediates:
//
//   count offset  0x20000 -> 0x40000   6 sites
//   buffer size   0x20008 -> 0x40010   3 sites (two clears + the init
//                                        memset at 0x008F2DB9)
//
// A separate array of up to four 0x20008 blocks exists at
// 0x0133000C/0x01330BFD (`imul rax,rax,0x20008`, count [0x175D2B30]).
// It is NOT this structure and is not touched.
//
// All-or-nothing with rollback, and gated on BO3_CG_FRAME with the rest
// of the third-pane work so the default build is untouched.
struct cf_imm {
  uint32_t rva;
  uint8_t off;
  uint32_t was;
  uint32_t want;
};
constexpr uint32_t cf_buffer_rva = 0x49925A0;
constexpr uint32_t cf_pointer_rva = 0x4992590;
constexpr size_t cf_new_bytes = 0x40010; // 4 * 0x10000 + 4 * 4
constexpr uint32_t cf_lea_sites[] = {0x008F26E3, 0x008F2DB0}; // 7 B, disp @3
constexpr cf_imm cf_imms[] = {
    {0x00132F59, 3, 0x20000, 0x40000}, // mov r8d,[r9+0x20000]
    {0x00132F7F, 3, 0x20000, 0x40000}, // mov [r9+0x20000],eax
    {0x0013304D, 3, 0x20000, 0x40000}, // mov r8d,[r10+0x20000]
    {0x00133079, 3, 0x20000, 0x40000}, // mov [r10+0x20000],eax
    {0x001337F6, 4, 0x20000, 0x40000}, // lea r15,[r8*4+0x20000]
    {0x00136BD4, 4, 0x20000, 0x40000}, // lea r14,[rbx*4+0x20000]
    {0x0013444B, 2, 0x20008, 0x40010}, // mov r8d,0x20008  (clear)
    {0x00137160, 2, 0x20008, 0x40010}, // mov r8d,0x20008  (clear)
    {0x008F2DB9, 2, 0x20008, 0x40010}, // mov r8d,0x20008  (init memset)
};
bool cf_relocated = false;
size_t cf_new_buffer = 0;

bool relocate_clientfield_callbacks() {
  if (cf_relocated) {
    return true;
  }
  const auto b = base();

  // every site must still be stock before anything is written
  for (const auto &s : cf_imms) {
    const auto *at = reinterpret_cast<const uint8_t *>(b + s.rva + s.off);
    uint32_t cur = 0;
    if (!readable(at, sizeof(cur))) {
      return false;
    }
    std::memcpy(&cur, at, sizeof(cur));
    if (cur != s.was) {
      note("[splitscreen] clientfield imm 0x%08X reads 0x%X, want 0x%X"
           " - not patching\n",
           s.rva, cur, s.was);
      return false;
    }
  }
  int32_t saved_lea[std::size(cf_lea_sites)]{};
  for (size_t i = 0; i < std::size(cf_lea_sites); ++i) {
    const auto *insn = reinterpret_cast<const uint8_t *>(b + cf_lea_sites[i]);
    if (!readable(insn, 7) || insn[1] != 0x8D) {
      return false;
    }
    std::memcpy(&saved_lea[i], insn + 3, sizeof(int32_t));
    const auto tgt = static_cast<size_t>(cf_lea_sites[i] + 7 + saved_lea[i]);
    if (tgt != cf_buffer_rva) {
      note("[splitscreen] clientfield lea 0x%08X -> 0x%zX, not the buffer\n",
           cf_lea_sites[i], tgt);
      return false;
    }
  }

  auto *fresh = static_cast<uint8_t *>(allocate_near_module(cf_new_bytes));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, cf_new_bytes);
  // carry the two live client regions and their counts across, in
  // case the engine's init already ran before this did
  std::memcpy(fresh, reinterpret_cast<const void *>(b + cf_buffer_rva),
              0x20000);
  std::memcpy(fresh + 0x40000,
              reinterpret_cast<const void *>(b + cf_buffer_rva + 0x20000), 8);

  size_t done_lea = 0, done_imm = 0;
  const auto rollback = [&] {
    for (size_t j = 0; j < done_lea; ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + cf_lea_sites[j]);
      write_bytes(insn + 3, &saved_lea[j], sizeof(int32_t));
    }
    for (size_t j = 0; j < done_imm; ++j) {
      auto *at =
          reinterpret_cast<uint8_t *>(b + cf_imms[j].rva + cf_imms[j].off);
      write_bytes(at, &cf_imms[j].was, sizeof(uint32_t));
    }
  };

  for (size_t i = 0; i < std::size(cf_lea_sites); ++i) {
    auto *insn = reinterpret_cast<uint8_t *>(b + cf_lea_sites[i]);
    const auto end = static_cast<int64_t>(b + cf_lea_sites[i] + 7);
    const auto delta =
        static_cast<int64_t>(reinterpret_cast<size_t>(fresh)) - end;
    if (delta > INT32_MAX || delta < INT32_MIN) {
      rollback();
      return false;
    }
    const auto d32 = static_cast<int32_t>(delta);
    if (!write_bytes(insn + 3, &d32, sizeof(d32))) {
      rollback();
      return false;
    }
    ++done_lea;
  }
  for (const auto &s : cf_imms) {
    auto *at = reinterpret_cast<uint8_t *>(b + s.rva + s.off);
    if (!write_bytes(at, &s.want, sizeof(s.want))) {
      rollback();
      return false;
    }
    ++done_imm;
  }

  // and the pointer the consumers actually read, so it is right even if
  // the engine's init already stored the old buffer there
  auto *ptr = reinterpret_cast<void **>(b + cf_pointer_rva);
  if (readable(ptr, sizeof(void *))) {
    void *v = fresh;
    write_bytes(ptr, &v, sizeof(v));
  }

  cf_new_buffer = reinterpret_cast<size_t>(fresh);
  cf_relocated = true;
  note("[splitscreen] clientfield callbacks [2]->[4]: buffer RVA 0x%08X,"
       " 2 leas + %zu immediates\n",
       static_cast<uint32_t>(cf_new_buffer - b), std::size(cf_imms));
  return true;
}

// ============ the per-client entity-ref WORD table at 0x16DD3540:
// the fail-fast after the clientfield callbacks ============
//
// With scene buffers, the collision pair and the clientfield array all
// fixed, the round for client 2 ended 15 s after cl_max=3 with NO
// dialog: 0xC0000409 (fail-fast, which bypasses SEH so BOIII's handler
// never sees it) at 0x02C413A4 = `int 29`, subcode 8 =
// FAST_FAIL_RANGE_CHECK_FAILURE. Windows Event 1000 + the WER dump
// (boiii.exe.30852.dmp, 2026-09-14 11:59) gave the caller:
//
//   0214E39E  imul edi, edi, 0x702            ; lc * (MAX_LOCAL_CENTITIES+2)
//   0214E3A4  add  ebx, edi                   ; index = lc*0x702 + entnum
//   0214E41A  movsxd rax, ebx ; add rax, rax  ; WORD index
//   0214E427  movsx rcx, word [rax+r10+0x16DD3540]
//   0214E44F  cmp  rax, 0x1C08                ; = 2 * 0x702 * 2 bytes
//   0214E455  jae  __report_rangecheckfailure ; <- lc=2 always lands here
//
// rbx = 0xEC2 in the dump = 2*0x702 + 0x1E: client 2, entity 30. A
// [2][0x702] word table, and MSVC's own bounds check kills the process
// the first time client 2 registers an entity. The function is a
// callback registered at 0x008F276E, right beside the clientfield init.
//
// Rule 4: it CANNOT grow in place. The [2]/[3] span 0x16DD5148.. is
// foreign - a second table starts at 0x16DD5150 (4 leas + 3 ABS sites)
// with more at +0x1028/+0x1428. So it is relocated, with every
// reference found by scan, not by hand:
//   4 RIP leas   0x0214C7B4 0x0214E149 0x0214E318 0x02151A0C
//   4 ABS32      0x0214E0BD (disp@4) 0x0214E427 0x0214E459 0x0214E773 (disp@5)
//   2 immediates 0x1C08 -> 0x3810: the bounds check at 0x0214E44F and
//                the table's memset size at 0x0214E321 (the clear at
//                0x0214E2xx zeroes it per map).
// The neighbouring tables are NOT per-client (0x16DD5150 is indexed by
// a flat handle, bound 0x1000; 0x16DD2930 likewise, bound 0xC00) and
// are left alone. Same rewrite_entcoll() machinery, all-or-nothing.
//
// 2026-09-27, the first 4-player round: AV at 0x023456AA
// (XAnimSetGoalWeight, PS4 0x110F1E0) on a viewmodel DObj whose tree is
// null, first cgame frame after UIRoot3's first snapshot. The table is
// clientObjMap (PS4 0x101D3470, short[7184] = [4][0x704]). A row holds
// ((1<<10) + 768) + MAX_LOCAL_CLIENTS handles - PS4 Com_GetClientDObj
// 0xE55ED0 asserts handle < 0x704 and indexes lc*0x704 + handle - and the
// viewmodel handle is 0x700 + lc (CG_WeaponDObjHandle 0x161CE0). The PC
// row is 0x702 (MAX_LOCAL_CLIENTS = 2), so lc 2's viewmodel (0x702)
// aliased lc 3's entity 0 and lc 3's (0x703) ran past the table. Three
// players never noticed because row 3 was empty. So the table becomes
// the PS4 layout [4][0x704], and every row-dependent constant moves in
// the same transaction (verified by an imm32 scan for 0x702 / 0x1C08,
// every hit in the image decoded - no other user):
//   stride   0x0214C79F ClearAllSkel(lc), 0x0214E0AC Create,
//            0x0214E140 Get, 0x0214E39E SafeFree, 0x0214E769 rebuild loop
//   count    0x0214C7A5 ClearAllSkel, 0x02151820 rebuild, 0x02151A5F free
//   bytes    0x0214E321 memset, 0x0214E44F /GS bound  -> 0x3820
//   clients  0x0215183D rebuild, 0x02151A69 free: 2 -> 4. Both are the
//            PC-only free-all / rebuild-all pair around a zone unload
//            (callers 0x0141F870 / 0x0142566F and 0x0142401A / 0x01425817).
//            Rule 4: rows 2/3 are zero unless those clients exist; the
//            rebuild body gates lc on cl_maxLocalClients (0x0214FFB8) and
//            uses the heap cg_t array, the free body's 0x014D1300 indexes
//            the heap block 0x09F167D8 sized by the allocation floor.
constexpr uint32_t entword_base = 0x16D545D0;
constexpr uint32_t entword_old_row = 0x702; // PC handles per client
constexpr uint32_t entword_row = 0x704;     // PS4 handles per client
constexpr uint32_t entword_client_bytes = entword_row * 2; // 0xE08
constexpr size_t entword_slots = 4;
constexpr entcoll_site entword_sites[] = {
    {0x20F3CF4, 3, 7, true, 0},  // lea rcx,[rip+..]
    {0x20F55FD, 4, 8, false, 0}, // mov word [rax+rdx*2+0x16DD3540],di
    {0x20F5689, 3, 7, true, 0},  // lea rcx,[rip+..]
    {0x20F5858, 3, 7, true, 0},  // lea rcx,[rip+..]   (the clear)
    {0x20F5967, 5, 9, false, 0}, // movsx rcx,word [rax+r10+..]  (crash site)
    {0x20F5999, 5, 9, false, 0}, // mov word [rax+r10+..],dx
    {0x20F5CB3, 5, 9, false, 0}, // movzx ecx,word cs:[rcx+rax*2+..]
    {0x20F8F4C, 3, 7, true, 0},  // lea rsi,[rip+..]
};
// size = immediate width in bytes (4 = imm32, 1 = imm8)
struct entword_imm {
  uint32_t rva;
  uint8_t off;
  uint8_t size;
  uint32_t was;
  uint32_t want;
};
constexpr entword_imm entword_imms[] = {
    {0x20F5861, 2, 4, 0x1C08, 0x3820}, // mov r8d,0x1c08  - memset size
    {0x20F598F, 2, 4, 0x1C08, 0x3820}, // cmp rax,0x1c08  - the /GS bound
    {0x20F3CDF, 2, 4, 0x702, 0x704},   // imul ecx,ecx,0x702   ClearAllSkel(lc)
    {0x20F3CE5, 1, 4, 0x702, 0x704}, // mov edi,0x702        ClearAllSkel count
    {0x20F55EC, 3, 4, 0x702, 0x704}, // imul rdx,rdx,0x702 Com_ClientDObjCreate
    {0x20F5680, 2, 4, 0x702, 0x704}, // imul edx,edx,0x702   Com_GetClientDObj
    {0x20F58DE, 2, 4, 0x702,
     0x704}, // imul edi,edi,0x702   Com_SafeClientDObjFree
    {0x20F5CA9, 3, 4, 0x702, 0x704}, // imul r13,r13,0x702   rebuild-all row
    {0x20F8D60, 2, 4, 0x702, 0x704}, // cmp esi,0x702        rebuild-all count
    {0x20F8F9F, 2, 4, 0x702, 0x704}, // cmp ebx,0x702        free-all count
    {0x20F8D7D, 3, 1, 0x02, 0x04},   // cmp r12d,2           rebuild-all clients
    {0x20F8FA9, 2, 1, 0x02, 0x04},   // cmp ebp,2            free-all clients
};
bool entword_relocated = false;
size_t entword_new = 0;
const char *entword_result = "clientObjMap: not attempted";

bool entword_imm_reads(const entword_imm &s, uint32_t want) {
  const auto *at = reinterpret_cast<const uint8_t *>(base() + s.rva + s.off);
  uint32_t cur = 0;
  if (!readable(at, s.size)) {
    return false;
  }
  std::memcpy(&cur, at, s.size);
  return cur == want;
}

bool relocate_entword_table() {
  if (entword_relocated) {
    return true;
  }
  const auto b = base();
  for (const auto &s : entword_imms) {
    if (!entword_imm_reads(s, s.was)) {
      note("[splitscreen] entword imm 0x%08X is not 0x%X - not patching\n",
           s.rva, s.was);
      entword_result = "clientObjMap: NOT moved - an immediate did not match";
      return false;
    }
  }

  auto *fresh = static_cast<uint8_t *>(
      allocate_near_module(entword_slots * entword_client_bytes));
  if (!fresh) {
    entword_result = "clientObjMap: NOT moved - allocation failed";
    return false;
  }
  // Re-stride the two stock rows: old row r (0x702 words) -> new row r
  // (0x704 words). Handles 0x702/0x703 of a row start empty.
  std::memset(fresh, 0, entword_slots * entword_client_bytes);
  for (uint32_t r = 0; r < 2; ++r) {
    std::memcpy(fresh + r * entword_client_bytes,
                reinterpret_cast<const void *>(b + entword_base +
                                               r * entword_old_row * 2),
                entword_old_row * 2);
  }

  static int32_t saved[std::size(entword_sites)]{};
  if (!rewrite_entcoll(entword_sites, std::size(entword_sites), entword_base,
                       reinterpret_cast<size_t>(fresh), saved)) {
    return false;
  }
  size_t done = 0;
  for (const auto &s : entword_imms) {
    auto *at = reinterpret_cast<uint8_t *>(b + s.rva + s.off);
    if (!write_bytes(at, &s.want, s.size)) {
      for (size_t j = 0; j < done; ++j) {
        auto *back = reinterpret_cast<uint8_t *>(b + entword_imms[j].rva +
                                                 entword_imms[j].off);
        write_bytes(back, &entword_imms[j].was, entword_imms[j].size);
      }
      for (size_t j = 0; j < std::size(entword_sites); ++j) {
        auto *insn = reinterpret_cast<uint8_t *>(b + entword_sites[j].rva);
        write_bytes(insn + entword_sites[j].disp_off, &saved[j],
                    sizeof(int32_t));
      }
      entword_result =
          "clientObjMap: NOT moved - an immediate write failed (rolled back)";
      return false;
    }
    ++done;
  }
  entword_new = reinterpret_cast<size_t>(fresh);
  entword_relocated = true;
  entword_result =
      "clientObjMap [2][0x702] -> [4][0x704] (8 sites, 12 immediates)";
  note("[splitscreen] clientObjMap [2][0x702] -> [4][0x704] at RVA 0x%08X (8 "
       "sites, %zu imms)\n",
       static_cast<uint32_t>(entword_new - b), std::size(entword_imms));
  return true;
}

// ============ s_exposureAdaptions: one auto-exposure buffer per local
// client plus the extra cam (2026-09-27, the first 4-player round) ============
//
// Pane 4 rendered overexposed in both 4-player rounds. PS4: the renderer
// keeps ExposureAdaption s_exposureAdaptions[5] (0xAE89550 - MAX_LOCAL_CLIENTS
// + 1), and GfxLightingData.exposureAdaptionBuffers is GfxRWTexture*[5] at
// +0x140 (DWARF), filled by R_InitLightingData with &adaption[k].texture.
// RB_FxBloomLDRColorGrade picks one per view (0x8EA690..0x8EA6DD):
//   index = view->isExtraCam ? 4 : view->localClientNum
// PC: [3] x 0x110 at 0x0F6CDB30 (created at
// 0x01CCCA94.."exposureAdaptionBuffer"), the per-entry table is 3 pointers at
// +0x10F0 followed by exposureOutputBuffer at +0x1108, and the one indexed
// reader is 0x01C6BFC0 (`extraCam ? 2 : lc`, then [lightingData + idx*8 +
// 0x10F0]) - the only scaled 0x10F0 access in the image. So player 3 shared the
// extra cam's buffer and player 4 read exposureOutputBuffer as a texture.
// Rule 4: 0x0F6CDE60/0x0F6CDE64 (right after [3]) are foreign ints with
// 4 refs, so the array moves:
//   base leas 0x01CCBB90 (free loop), 0x01CCC65F (table fill), 0x01CCCA94
//   (create loop) -> new; end leas 0x01CCBB97 / 0x01CCCAA2 -> new+5*0x110
//   (free and create all five); the table-fill end 0x01CCC2DC -> new+3*0x110
//   (the per-entry table has three slots and must not overflow into
//   exposureOutputBuffer - nothing reads it any more).
// The selector reads the record directly, PS4's index with PS4's extra-cam
// slot, in the same 30 bytes (the table holds the same pointers for every
// lighting-data entry, so this is equivalent for indices 0..2):
//   mov eax,4 ; jne +6 ; mov eax,[rdx+0x398] ; imul eax,eax,0x110 ;
//   lea rsi,[new+0x108] ; add rsi,rax ; nop
// Runs at post_unpack, before R_InitLightingData creates anything.
constexpr uint32_t exposure_base = 0xF64EBB0;
constexpr uint32_t exposure_stride = 0x110;
constexpr uint32_t exposure_old_count = 3;
constexpr uint32_t exposure_new_count = 5;
constexpr uint32_t exposure_texture_off = 0x108;
constexpr entcoll_site exposure_base_sites[] = {
    {0x1CBF7C0, 3, 7, true, 0}, // lea rbx,[base]  free loop
    {0x1CC028F, 3, 7, true, 0}, // lea rcx,[base]  table fill
    {0x1CC06C4, 3, 7, true, 0}, // lea rbx,[base]  create loop
};
constexpr entcoll_site exposure_fill_end_site[] = {
    {0x1CBFF0C, 3, 7, true,
     exposure_old_count *exposure_stride}, // lea r13,[end]
};
constexpr entcoll_site exposure_all_end_sites[] = {
    {0x1CBF7C7, 3, 7, true,
     exposure_old_count *exposure_stride}, // lea rdi,[end] free
    {0x1CC06D2, 3, 7, true,
     exposure_old_count *exposure_stride}, // lea rdi,[end] create
};
constexpr uint32_t exposure_select_rva = 0x1C5FC0C;
constexpr uint8_t exposure_select_stock[] = {
    0xB8, 0x02, 0x00, 0x00, 0x00,             // mov eax,2
    0x75, 0x06,                               // jne +6
    0x8B, 0x82, 0x98, 0x03, 0x00, 0x00,       // mov eax,[rdx+0x398]
    0x8B, 0xC8,                               // mov ecx,eax
    0x48, 0x8B, 0x82, 0xB0, 0x03, 0x00, 0x00, // mov rax,[rdx+0x3B0]
    0x48, 0x8B, 0xB4, 0xC8, 0xF0, 0x10, 0x00,
    0x00, // mov rsi,[rax+rcx*8+0x10F0]
};
constexpr uint32_t exposure_select_lea_off =
    19; // lea rsi,[rip+d] inside the patch
const char *exposure_result = "exposure adaptions: not attempted";
size_t exposure_new = 0;

bool relocate_exposure_adaptions() {
  if (exposure_new) {
    return true;
  }
  const auto b = base();
  auto *select = reinterpret_cast<uint8_t *>(b + exposure_select_rva);
  if (!readable(select, sizeof(exposure_select_stock)) ||
      std::memcmp(select, exposure_select_stock,
                  sizeof(exposure_select_stock)) != 0) {
    exposure_result =
        "exposure adaptions: NOT moved - selector bytes differ at 0x01C6BFDC";
    return false;
  }
  auto *fresh = static_cast<uint8_t *>(
      allocate_near_module(exposure_new_count * exposure_stride));
  if (!fresh) {
    exposure_result = "exposure adaptions: NOT moved - allocation failed";
    return false;
  }
  std::memset(fresh, 0, exposure_new_count * exposure_stride);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + exposure_base),
              exposure_old_count * exposure_stride);
  const auto fresh_abs = reinterpret_cast<size_t>(fresh);

  // New selector bytes; the lea is rip-relative to its own end.
  uint8_t patch[sizeof(exposure_select_stock)] = {
      0xB8,
      0x04,
      0x00,
      0x00,
      0x00, // mov eax,4 (extra cam = MAX_LOCAL_CLIENTS)
      0x75,
      0x06, // jne +6
      0x8B,
      0x82,
      0x98,
      0x03,
      0x00,
      0x00, // mov eax,[rdx+0x398]
            // (localClientNum)
      0x69,
      0xC0,
      0x10,
      0x01,
      0x00,
      0x00, // imul eax,eax,0x110
      0x48,
      0x8D,
      0x35,
      0x00,
      0x00,
      0x00,
      0x00, // lea rsi,[rip+d] -> new+0x108
      0x48,
      0x01,
      0xC6, // add rsi,rax
      0x90, // nop
  };
  const auto lea_end = static_cast<int64_t>(b + exposure_select_rva +
                                            exposure_select_lea_off + 7);
  const auto lea_disp =
      static_cast<int64_t>(fresh_abs + exposure_texture_off) - lea_end;
  if (lea_disp < INT32_MIN || lea_disp > INT32_MAX) {
    exposure_result =
        "exposure adaptions: NOT moved - new block out of rip range";
    return false;
  }
  const auto d32 = static_cast<int32_t>(lea_disp);
  std::memcpy(patch + exposure_select_lea_off + 3, &d32, sizeof(d32));

  static int32_t saved_base[std::size(exposure_base_sites)]{};
  static int32_t saved_fill[std::size(exposure_fill_end_site)]{};
  static int32_t saved_all[std::size(exposure_all_end_sites)]{};
  if (!rewrite_entcoll(exposure_base_sites, std::size(exposure_base_sites),
                       exposure_base, fresh_abs, saved_base)) {
    exposure_result =
        "exposure adaptions: NOT moved - a base lea did not match";
    return false;
  }
  const auto undo_base = [&] {
    for (size_t j = 0; j < std::size(exposure_base_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + exposure_base_sites[j].rva);
      write_bytes(insn + exposure_base_sites[j].disp_off, &saved_base[j],
                  sizeof(int32_t));
    }
  };
  if (!rewrite_entcoll(exposure_fill_end_site,
                       std::size(exposure_fill_end_site), exposure_base,
                       fresh_abs, saved_fill)) {
    undo_base();
    exposure_result =
        "exposure adaptions: NOT moved - the fill end lea did not match";
    return false;
  }
  const auto undo_fill = [&] {
    auto *insn = reinterpret_cast<uint8_t *>(b + exposure_fill_end_site[0].rva);
    write_bytes(insn + exposure_fill_end_site[0].disp_off, &saved_fill[0],
                sizeof(int32_t));
  };
  // target = new_abs + target_off(3*0x110): passing new + 2*0x110 lands on new
  // + 5*0x110.
  if (!rewrite_entcoll(exposure_all_end_sites,
                       std::size(exposure_all_end_sites), exposure_base,
                       fresh_abs + (exposure_new_count - exposure_old_count) *
                                       exposure_stride,
                       saved_all)) {
    undo_fill();
    undo_base();
    exposure_result =
        "exposure adaptions: NOT moved - a create/free end lea did not match";
    return false;
  }
  if (!write_bytes(select, patch, sizeof(patch))) {
    for (size_t j = 0; j < std::size(exposure_all_end_sites); ++j) {
      auto *insn =
          reinterpret_cast<uint8_t *>(b + exposure_all_end_sites[j].rva);
      write_bytes(insn + exposure_all_end_sites[j].disp_off, &saved_all[j],
                  sizeof(int32_t));
    }
    undo_fill();
    undo_base();
    exposure_result =
        "exposure adaptions: NOT moved - selector write failed (rolled back)";
    return false;
  }
  exposure_new = fresh_abs;
  exposure_result = "exposure adaptions [3] -> [5] (PS4 MAX_LOCAL_CLIENTS+1), "
                    "selector extraCam ? 4 : lc";
  return true;
}

// ============ the UI model node pool: 0x9000 -> 0xFFFF nodes (2026-09-28,
// multiplayer with 3 and 4 players) ============
//
// MP "Fatal Error: LUI out of memory" (Com_ERROR EXE_LUI_OUT_OF_MEMORY) at
// every 3- and 4-player match start. Measured (LOG.md "ROOT CAUSE of the MP
// LUI out of memory"): the model node pool goes 28k free -> 0 within ~200 ms
// while ChooseClass_InGame builds global.controllerN.CustomClassList for
// every player - 11 classes x ~1110 nodes = ~11.1k per controller. Stock 2
// players fit (8.2k + 2 x 11.1k of 36864); 3 need ~41.6k, 4 need ~52.7k.
// Every other LUI budget is the same or larger than PS4's (Lua heap 60 MB,
// 20000 elements, 18000 animation states), so this pool is the wall.
//
// PS4 (RULE ZERO): UI_Model_Init 0xD68140 resets s_modelNodePool[0x9000],
// links node i -> i+1, sets the free head to 1 and allocates "global" and
// controller0..3; UI_Model_ResetNode 0xD682E0 zeroes a node and stores its
// own index. PC node (0x28 B): +0 name hash, +0x18 parent, +0x1A first child,
// +0x1C own index, +0x1E next sibling / next free, +0x20 subscription head,
// +0x22 persistent - UI_Model_FreeModel 0x0200D0A0 resets a node to exactly
// zero + own index + next free. The node index is a u16, so 0xFFFF nodes.
//
// Users of the array in the new build (scratch audit 2026-09-28): every raw
// field image-wide that could address it, decoded linearly from the start
// of its .pdata function OR of the gap between functions - the typed value
// getters are LEAF functions without .pdata, and the first version of this
// table (20 sites) missed all of them: values read back as nil and the boot
// died on "Lobby Error 22439 / nil + number". 22 rip-relative + 6 ABS32
// sites below; every raw candidate at the array head is explained (the one
// leftover is the control word 0x1629314E), none lies in the Arxan section.
// The free-list head 0x16293150 and its neighbours are control words and stay.
// Two 0x9000 constants: GetModel 0x0200D525 `cmp edx,0x9000` is an END
// sentinel (node 0x9000 is therefore never handed out), and
// ResetModelsAndSubscriptions 0x0200DACE `mov esi,0x9000` bounds both of its
// node loops (clear subscription heads, free non-persistent) -> 0xFFFF.
// No other code compares a handle against 0x9000 (image-wide scan).
//
// Runs at post_unpack, before the hidden UI_Model_Init. That init keeps
// initialising the OLD array (its references cannot be rewritten), which
// then lies unused; the NEW array is pre-built here in the reset state with
// the free list 1 -> ... -> 0x8FFF -> 0x9001 -> ... -> 0xFFFE -> 0, so the
// init's head = 1 and its AllocateNode("global") (rewritten) land in it.
// Verify after boot that the old array stays untouched (no hidden user).
constexpr uint32_t model_pool_base = 0x16293160;
constexpr uint32_t model_pool_stride = 0x28;
constexpr uint32_t model_pool_old_count = 0x9000;
constexpr uint32_t model_pool_new_count = 0xFFFF;
constexpr uint32_t model_pool_sentinel = 0x9000;
constexpr uint32_t model_pool_self_off = 0x1C;
constexpr uint32_t model_pool_next_off = 0x1E;
constexpr entcoll_site model_pool_sites[] = {
    {0x200C70E, 3, 7, true, 0x00}, // lea rdx,[base]            AllocateNode
    {0x200C75F, 3, 7, true, 0x00}, // lea rdx,[base]            AllocateNode
    {0x200CA81, 3, 7, true, 0x00}, // lea rbx,[base]            FreeModel
    {0x200CA9E, 3, 7, true, 0x00}, // lea rsi,[base]            FreeModel
    {0x200CAD3, 3, 7, true, 0x00}, // lea rbx,[base]            FreeModel
    {0x200CC9F, 3, 7, true,
     0x00}, // lea rax,[base]            GetBool   (leaf, no .pdata)
    {0x200CCCF, 3, 7, true,
     0x08}, // lea rax,[base+8]          GetDataType (leaf)
    {0x200CCEF, 3, 7, true,
     0x00}, // lea rax,[base]            GetFunction (leaf)
    {0x200CD2F, 3, 7, true, 0x00}, // lea rax,[base]            getter (leaf)
    {0x200CE4D, 3, 7, true, 0x00}, // lea r11,[base]            GetModel
    {0x200CFA0, 3, 7, true, 0x00}, // lea rax,[base]            GetReal   (leaf)
    {0x200CFD4, 3, 7, true, 0x00}, // lea rax,[base]            getter (leaf)
    {0x200CFFF, 3, 7, true, 0x00}, // lea rax,[base]            getter (leaf)
    {0x200D20F, 5, 9, false, 0x20}, // movzx ebx,[r13+rax*8+base+0x20]  notify
    {0x200D413, 3, 7, true,
     0x20}, // lea rax,[base+0x20]       Reset (subscription heads)
    {0x200D438, 3, 7, true,
     0x22}, // lea rdi,[base+0x22]       Reset (persistent)
    {0x200D498, 3, 7, true, 0x00},  // lea r9,[base]             typed get/set
    {0x200D4FC, 3, 7, true, 0x00},  // lea rax,[base]
    {0x200D555, 3, 7, true, 0x00},  // lea rax,[base]
    {0x200D5AC, 3, 7, true, 0x00},  // lea rax,[base]
    {0x200D5F7, 3, 7, true, 0x00},  // lea rax,[base]
    {0x200D661, 3, 7, true, 0x00},  // lea r15,[base]            SetString
    {0x200D74C, 3, 7, true, 0x00},  // lea rax,[base]
    {0x200D7F1, 4, 8, false, 0x20}, // lea rdx,[rcx*8+base+0x20] Subscribe
    {0x200D850, 3, 7, false, 0x20}, // lea rdx,[r10+base+0x20]   (leaf)
    {0x200D97F, 4, 8, false, 0x20}, // movzx ecx,[rax+rbp+base+0x20] unsubscribe
    {0x200D9C7, 4, 8, false, 0x1A}, // movzx ecx,[rax+rbp+base+0x1A]
    {0x200D9E7, 4, 8, false, 0x1E}, // movzx ebx,[rdi+rdx*8+base+0x1E]
};
// ============ command buffers for local clients 2 and 3 (2026-09-28, MP)
// ============
//
// 4-player MP: players 3 and 4 never spawned - their panes read
// "Spectating: <host>" while all four server clients were ACTIVE. The
// class choice is a client command: Lua Engine.SendMenuResponse (PS4
// 0xD11590) builds "cmd mrp %i %i %i" and Cbuf_AddText(localClientNum),
// and Com_Frame executes each local client's buffer. Measured live: the
// records of local clients 2/3 were EMPTY (data 0, maxsize 0) - Cbuf_AddText
// then prints "overflow" and drops the text - and Com_Frame's loop
// 0x020ED461..0x020ED476 runs Cbuf_Execute(lc, GetControllerIndex(lc)) for
// lc < 2 only (deferred since 2026-08, LOG "Com_Frame Cbuf pair loop").
//
// PS4 (RULE ZERO): Cbuf_Init 0xE2FB20 sets up FOUR records,
// cmd_textArray[i] = {&cmd_text_buf[i * 0x10000], maxsize 0x10000,
// cursize 0}, cmd_insideCBufExecute[i] = 0; Com_Frame 0xE4D32B loops
// Cbuf_Execute over lc < 4. The PC records are already [4] (reloc_tables
// "cbuf", slots 2/3 zero); this gives slots 2/3 their buffers the way
// Cbuf_Init would (the static text buffers are [2], so 2 x 64 KB are
// allocated), then widens the two bounds that keep them from running:
//   Cbuf_Execute 0x020E00ED `cmp rbx,2 / jae __report_rangecheckfailure`
//     guards busy[lc] at 0x1681EFA8 - slots 2/3 (0x1681EFAA/AB) have no
//     reference of their own (image-wide field search): padding.
//   Com_Frame 0x020ED473 `cmp esi,2 / jl` -> 4.
// GetControllerIndex already reads the relocated seat records [4] and
// Cbuf_ExecuteInternal the relocated records. Runs at post_unpack, before
// Cbuf_Init and before the first frame; all or nothing.
constexpr uint32_t cbuf_old_records_rva = 0x1681EFB8;
constexpr uint32_t cbuf_exec_lea_rva =
    0x20DFBC8; // lea rax,[records] in Cbuf_ExecuteInternal
constexpr uint8_t cbuf_exec_lea_head[] = {0x48, 0x8D, 0x05};
constexpr uint32_t cbuf_range_check_rva = 0x20DFA2D;
constexpr uint8_t cbuf_range_check_stock[] = {0x48, 0x83, 0xFB,
                                              0x02, 0x73, 0x13};
constexpr uint32_t cbuf_frame_bound_rva = 0x20ECDB3;
constexpr uint8_t cbuf_frame_bound_stock[] = {0x83, 0xFE, 0x02, 0x7C, 0xE9};
constexpr uint32_t cbuf_text_size = 0x10000;
constexpr size_t cbuf_record_stride = 0x10;
const char *cbuf34_result = "command buffers 2/3: not attempted";

bool install_cbuf_for_players34() {
  const auto b = base();
  const auto *lea = reinterpret_cast<const uint8_t *>(b + cbuf_exec_lea_rva);
  if (!readable(lea, 7) ||
      std::memcmp(lea, cbuf_exec_lea_head, sizeof(cbuf_exec_lea_head)) != 0) {
    cbuf34_result =
        "command buffers 2/3: NOT installed - Cbuf_ExecuteInternal lea differs";
    return false;
  }
  int32_t disp = 0;
  std::memcpy(&disp, lea + 3, sizeof(disp));
  auto *records = reinterpret_cast<uint8_t *>(b + cbuf_exec_lea_rva + 7 +
                                              static_cast<int64_t>(disp));
  if (records == reinterpret_cast<uint8_t *>(b + cbuf_old_records_rva)) {
    cbuf34_result = "command buffers 2/3: NOT installed - the cbuf records "
                    "were not relocated";
    return false;
  }
  if (!readable(records, 4 * cbuf_record_stride)) {
    cbuf34_result = "command buffers 2/3: NOT installed - records unreadable";
    return false;
  }
  for (size_t i = 2 * cbuf_record_stride; i < 4 * cbuf_record_stride; ++i) {
    if (records[i] != 0) {
      cbuf34_result =
          "command buffers 2/3: NOT installed - records 2/3 are not empty";
      return false;
    }
  }
  auto *range = reinterpret_cast<uint8_t *>(b + cbuf_range_check_rva);
  auto *bound = reinterpret_cast<uint8_t *>(b + cbuf_frame_bound_rva);
  if (!readable(range, sizeof(cbuf_range_check_stock)) ||
      std::memcmp(range, cbuf_range_check_stock,
                  sizeof(cbuf_range_check_stock)) != 0 ||
      !readable(bound, sizeof(cbuf_frame_bound_stock)) ||
      std::memcmp(bound, cbuf_frame_bound_stock,
                  sizeof(cbuf_frame_bound_stock)) != 0) {
    cbuf34_result = "command buffers 2/3: NOT installed - range check or "
                    "Com_Frame bound bytes differ";
    return false;
  }
  auto *text = static_cast<uint8_t *>(VirtualAlloc(
      nullptr, 2 * cbuf_text_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
  if (!text) {
    cbuf34_result = "command buffers 2/3: NOT installed - allocation failed";
    return false;
  }
  for (size_t lc = 2; lc < 4; ++lc) {
    auto *rec = records + lc * cbuf_record_stride;
    auto *data = text + (lc - 2) * cbuf_text_size;
    const int32_t maxsize = static_cast<int32_t>(cbuf_text_size);
    const int32_t cursize = 0;
    std::memcpy(rec + 0x0, &data, sizeof(data));
    std::memcpy(rec + 0x8, &maxsize, sizeof(maxsize));
    std::memcpy(rec + 0xC, &cursize, sizeof(cursize));
  }
  const uint8_t four = 0x04, two = 0x02;
  if (!write_bytes(range + 3, &four, 1)) {
    std::memset(records + 2 * cbuf_record_stride, 0, 2 * cbuf_record_stride);
    cbuf34_result =
        "command buffers 2/3: NOT installed - range check write failed";
    return false;
  }
  if (!write_bytes(bound + 2, &four, 1)) {
    write_bytes(range + 3, &two, 1);
    std::memset(records + 2 * cbuf_record_stride, 0, 2 * cbuf_record_stride);
    cbuf34_result = "command buffers 2/3: NOT installed - Com_Frame bound "
                    "write failed (rolled back)";
    return false;
  }
  cbuf_range_resting = 0x04;
  cbuf34_result = "command buffers 2/3: 64 KB each, Cbuf_Execute range 2 -> 4, "
                  "Com_Frame Cbuf loop 2 -> 4";
  return true;
}

// ============ lobby join clients [2] -> [4] + the lobby message loop 2 -> 4
// (2026-09-28, "Failed to host lobby" with 3-4 players seated) ============
//
// Entering ZOMBIES / MULTIPLAYER from the offline main menu with 3-4 local
// players already seated (e.g. after Leave Party) ended in "Failed to host
// lobby". The lobby VM publishes its action history into the UI models
// global.lobbyDebug.processQueue (lua/Lobby/LobbyDebug.lua); polled live
// (scratch lobbyqueue_watch.py): CreateGameLobby hosts the game lobby fine,
// then LobbyJoinXUID (the whole party joins it) FAILS after ~6 s with 4
// seated and succeeds at once with 2. The join first asks every party
// member to agree; players 3/4 never answer:
//   PS4 LobbyMsgTransport_Update 0xCD2100: for (c = 0; c < 4; c++) if
//   LiveUser_IsSignedIn(c) { CheckNetChanLobbyGlobalPacket(c);
//   CheckNetChanPerLobbyPacket(c); CheckNetChanLobbyVoicePacket(c); }
//   PC 0x01EECD10: the same three calls, `cmp ebx,2` at 0x01EECD4E.
// Rule 4 before widening: the checks read the per-controller netchan
// (0x0211C570, table relocated [4] by reloc_tables "netchan"), and the
// agreement request handler (PC 0x01ED8930 = PS4
// LobbyJoinClient_MsgAgreementRequest, which asserts ci < 4 on s_joinClient)
// indexes s_joinClient - PS4 JoinClient[4] x 0xB8, PC [2] x 0xB0 at
// 0x156CB4B0, and its slot 2 is FOREIGN (a lobby join object starts at
// +0x160, 40+ refs). So the array moves first. Every reference in the new
// image (functions and leaf gaps, scratch array_refs.py): 8 base/field
// sites, 2 end markers (reset loop +0x160, update loop +0x208 from +0xA8),
// the static constructor 0x02E9104F (+0x6E; already ran - rewritten anyway),
// no range checks. Slot layout: +0 state, +0x66/+0x6E security id/key,
// +0xA8, +0xAC controller index (the init 0x01ED8910 writes 0/1 there).
// Slots 2/3 start as copies of slot 1 with state 0 and controller 2/3.
constexpr uint32_t joinclient_base = 0x156CB4B0;
constexpr uint32_t joinclient_stride = 0xB0;
constexpr uint32_t joinclient_old_count = 2;
constexpr uint32_t joinclient_new_count = 4;
constexpr uint32_t joinclient_ci_off = 0xAC;
constexpr entcoll_site joinclient_sites[] = {
    {0x1ED81CF, 3, 7, true, 0x0}, // lea rbx,[base]          reset loop
    {0x1ED8223, 3, 7, true, 0x0}, // lea rcx,[base]          getter (leaf)
    {0x1ED825C, 2, 6, true, 0x0}, // mov [base],eax          init (leaf)
    {0x1ED8262, 3, 7, true,
     0xAC}, // mov qword [base+0xAC]   init: slot0 ci, slot1 state
    {0x1ED8252, 2, 10, true, 0x15C}, // mov dword [base+0x15C],1  init: slot1 ci
    {0x1ED8298, 3, 7, true,
     0x0}, // lea rax,[base]          agreement request handler
    {0x1ED8438, 3, 7, true, 0x0},  // lea rax,[base]
    {0x1ED86D5, 3, 7, true, 0xA8}, // lea rbx,[base+0xA8]     update loop start
    {0x2E9098F, 3, 7, true,
     0x6E}, // lea rbx,[base+0x6E]     static ctor (ran already)
};
constexpr entcoll_site joinclient_end_sites[] = {
    {0x1ED81D8, 3, 7, true,
     0x160}, // lea rsi,[base+2*0xB0]        reset loop end
    {0x1ED86DE, 3, 7, true,
     0x208}, // lea r14,[base+0xA8+2*0xB0]   update loop end
};
constexpr uint32_t lobbymsg_bound_rva = 0x1EEC68E;
constexpr uint8_t lobbymsg_bound_stock[] = {0x83, 0xFB, 0x02, 0x7C,
                                            0xC5};  // cmp ebx,2 / jl
constexpr uint32_t netchan_get_lea_rva = 0x211BF51; // lea rax,[s_netchan]
constexpr uint32_t netchan_old_base = 0x16DEAEB0;
const char *joinclient_result = "lobby join clients: not attempted";
size_t joinclient_new = 0;

bool relocate_join_clients() {
  if (joinclient_new) {
    return true;
  }
  const auto b = base();
  auto *bound = reinterpret_cast<uint8_t *>(b + lobbymsg_bound_rva);
  if (!readable(bound, sizeof(lobbymsg_bound_stock)) ||
      std::memcmp(bound, lobbymsg_bound_stock, sizeof(lobbymsg_bound_stock)) !=
          0) {
    joinclient_result =
        "lobby join clients: NOT moved - LobbyMsgTransport_Update bytes differ";
    return false;
  }
  // The widened loop reads controllers 2/3 through the netchan table - it
  // must already be the relocated [4] one (reloc_tables "netchan").
  const auto *nlea = reinterpret_cast<const uint8_t *>(b + netchan_get_lea_rva);
  int32_t nd = 0;
  if (!readable(nlea, 7)) {
    joinclient_result =
        "lobby join clients: NOT moved - netchan lea unreadable";
    return false;
  }
  std::memcpy(&nd, nlea + 3, sizeof(nd));
  if (netchan_get_lea_rva + 7 + static_cast<int64_t>(nd) == netchan_old_base) {
    joinclient_result =
        "lobby join clients: NOT moved - the netchan table is still [2]";
    return false;
  }
  auto *fresh = static_cast<uint8_t *>(
      allocate_near_module(joinclient_new_count * joinclient_stride));
  if (!fresh) {
    joinclient_result = "lobby join clients: NOT moved - allocation failed";
    return false;
  }
  const auto *old = reinterpret_cast<const uint8_t *>(b + joinclient_base);
  std::memcpy(fresh, old, joinclient_old_count * joinclient_stride);
  for (uint32_t s = joinclient_old_count; s < joinclient_new_count; ++s) {
    auto *slot = fresh + s * joinclient_stride;
    std::memcpy(slot, old + joinclient_stride,
                joinclient_stride); // like slot 1
    const int32_t idle = 0, ci = static_cast<int32_t>(s);
    std::memcpy(slot + 0, &idle, sizeof(idle));
    std::memcpy(slot + joinclient_ci_off, &ci, sizeof(ci));
  }
  const auto fresh_abs = reinterpret_cast<size_t>(fresh);
  static int32_t saved[std::size(joinclient_sites)]{};
  static int32_t saved_end[std::size(joinclient_end_sites)]{};
  if (!rewrite_entcoll(joinclient_sites, std::size(joinclient_sites),
                       joinclient_base, fresh_abs, saved)) {
    joinclient_result =
        "lobby join clients: NOT moved - a reference did not match";
    return false;
  }
  // end markers: target_off is 2*stride past their start; the new end is
  // 4*stride
  if (!rewrite_entcoll(joinclient_end_sites, std::size(joinclient_end_sites),
                       joinclient_base,
                       fresh_abs +
                           (joinclient_new_count - joinclient_old_count) *
                               joinclient_stride,
                       saved_end)) {
    for (size_t j = 0; j < std::size(joinclient_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + joinclient_sites[j].rva);
      write_bytes(insn + joinclient_sites[j].disp_off, &saved[j],
                  sizeof(int32_t));
    }
    joinclient_result = "lobby join clients: NOT moved - an end marker did not "
                        "match (rolled back)";
    return false;
  }
  const uint8_t four = 0x04;
  if (!write_bytes(bound + 2, &four, 1)) {
    joinclient_result = "lobby join clients [2] -> [4] moved, but the message "
                        "loop widen FAILED";
    joinclient_new = fresh_abs;
    return false;
  }
  joinclient_new = fresh_abs;
  joinclient_result = "lobby join clients [2] -> [4] (9 sites + 2 end "
                      "markers), LobbyMsgTransport_Update 2 -> 4";
  note("[splitscreen] lobby join clients [2] -> [4] at RVA 0x%08X, lobby "
       "message loop 2 -> 4\n",
       static_cast<uint32_t>(fresh_abs - b));
  return true;
}

// ============ Lua Engine.GetClientNum / GetPredictedClientNum for controllers
// 2/3 (2026-09-28, MP HUD of players 3/4) ============
//
// MP panes 3/4 showed no ammo counter / weapon name / scorestreaks. Dev
// probe ui_scripts/zz_hudprobe: every input of AmmoWidgetMPContainer's
// state conditions matched players 1/2 EXCEPT IsSpectatingInvalidPlayer(c)
// (conditions.lua: deadSpectator.playerIndex == Engine.GetClientNum(c)),
// true for controllers 2/3: both read -1. The PC binding GetClientNum
// 0x01F42E90 starts with `cmp ecx,1 / ja -> return -1` (PS4
// Lua_CoD_LuaCall_GetClientNum 0xD18000: controllerIndex < 4, then
// CG_GetLocalClientGlobals(lc)->clientNum). Behind the check the PC maps
// controller -> lc (seat records [4]) and indexes the cg globals only
// while lc < cl_maxLocalClients (`cmp r9d,[0x05323720] / jge`), so the
// widen reads valid data or keeps -1. GetPredictedClientNum 0x01F502C0 has
// the identical shape. Image-wide scan of all 1755 Lua bindings: 10 carry
// this `cmp reg,1 / ja` controller check; the other 8 are frontend/online
// (challenges, probation, timers, menu music) and stay.
struct ctrl_check_patch {
  uint32_t rva;     // the cmp
  uint8_t stock[9]; // cmp ecx,1 ; ja rel32
  const char *what;
};
constexpr ctrl_check_patch lua_ctrl_checks[] = {
    {0x01F42EBE,
     {0x83, 0xF9, 0x01, 0x0F, 0x87, 0x12, 0x18, 0x00, 0x00},
     "Engine.GetClientNum"},
    {0x01F502EE,
     {0x83, 0xF9, 0x01, 0x0F, 0x87, 0x17, 0x18, 0x00, 0x00},
     "Engine.GetPredictedClientNum"},
};
const char *lua_ctrl_result = "lua controller checks: not attempted";

bool widen_lua_controller_checks() {
  const auto b = base();
  for (const auto &p : lua_ctrl_checks) {
    const auto *site = reinterpret_cast<const uint8_t *>(b + p.rva);
    if (!readable(site, sizeof(p.stock)) ||
        std::memcmp(site, p.stock, sizeof(p.stock)) != 0) {
      lua_ctrl_result = "lua controller checks: NOT widened - bytes differ";
      return false;
    }
  }
  uint32_t done = 0;
  for (const auto &p : lua_ctrl_checks) {
    const uint8_t three = 0x03; // ja when controller > 3
    if (write_bytes(reinterpret_cast<void *>(b + p.rva + 2), &three, 1)) {
      ++done;
    }
  }
  lua_ctrl_result = done == std::size(lua_ctrl_checks)
                        ? "lua controller checks 1 -> 3: Engine.GetClientNum, "
                          "GetPredictedClientNum"
                        : "lua controller checks: a write FAILED";
  return done == std::size(lua_ctrl_checks);
}

// ============ the UI model STRING hunk ("UIModelAllocator"): 0xC0000 -> 4 MB
// ============
//
// With the bigger node pool the first 4-player MP start got further and
// crashed in UI_Model_SetString (0x0200DD10): Hunk_UserAlloc (0x022773A0)
// returned NULL for node 0xC016 and the string copy wrote to 0 (minidump
// 2026-09-28 15:27:39, rbp = node, rdi = 0, r15 = the new pool). Every
// string value of every model lives in this hunk; the HunkUser at
// [0x1626C030] read live: name "UIModelAllocator", size 0xC0000 (PS4
// UI_Model_Init: s_modelStringBuffer 0x80000). Four class lists do not fit.
//
// The hunk is created by the hidden UI_Model_Init through the VISIBLE
// Hunk_UserCreateFromBuffer 0x02277460 (PS4 0x10D1C90; PC address from two
// independent callers, "FreeType" 0x01CA06E5 and "serverHunk" 0x021F44D1):
// rcx buffer, rdx size, r8d scheme, r9d flags, [rsp+0x20] ptr,
// [rsp+0x28] name, [rsp+0x30] int. This detour hands exactly that one call
// (name "UIModelAllocator", size 0xC0000) a 4 MB buffer before anything is
// allocated from it. No code references the static buffer (RVA 0x163FB160,
// right after the old node array) except the hidden init itself, so nothing
// compares string pointers against its address.
constexpr uint32_t hunk_create_rva = 0x2276DA0;
constexpr uint8_t hunk_create_prologue[] = {
    0x49, 0x63, 0xC0, // movsxd rax,r8d
    0x4C, 0x8D, 0x1D, 0x86,
    0xB4, 0x14, 0x01, // lea r11,[rip+0x0114B486] (scheme table)
};
constexpr size_t model_string_stock_size = 0xC0000;
constexpr size_t model_string_new_size = 0x400000;
utils::hook::detour hunk_create_hook;
void *model_string_buffer = nullptr;
const char *model_string_result = "ui model string hunk: not installed";

// ============ "ClientCache_ClientPool" 0x3880 -> 0x7100 (2026-09-28)
// ============ 4-player MP (game mode 1) crashed entering the round:
// memset(NULL, 0, 0x118) at 0x009577A2 in the ET_PLAYER handler 0x00957710,
// which gives each player centity (per local client) a 0x118-byte block at
// +0x628 and an 8-byte one at +0x638 from the HunkUser at [0x17CC4108]. Read
// from the dump (boiii-crash-2026-09-28-18-55-14): size 0x3880, used 0x3760 ->
// full. PS4 (RULE ZERO): CG_InitClientEntityCaches(lc) allocates
// cent->clientCache (0xBC) and cent->aimTargetInfo (8) for every client from
// g_ClientCachePool, created in SmallDynHeaps_InitSmallDynamicHeaps (from
// Com_Init) on g_ClientCachePoolBuffer byte[0x4720] - sized for 4 local
// clients. The PC blocks are bigger and its buffer holds two local clients'
// worth, so it doubles here. Its siblings from the same init were measured
// in the same dump: Vehicle_ClientPool 0xE7800 (PS4 0x78000) and
// ScriptMover_ClientPool 0x1D520 (= PS4), both nearly empty - left alone.
constexpr size_t client_cache_stock_size = 0x3880;
constexpr size_t client_cache_new_size = 0x7100;
void *client_cache_buffer = nullptr;
const char *client_cache_result = "client cache pool: not seen";

void *hunk_create_stub(void *buffer, size_t size, int scheme, int flags,
                       void *arg5, const char *name, int arg7) {
  if (name && size == client_cache_stock_size &&
      std::strcmp(name, "ClientCache_ClientPool") == 0) {
    // One buffer for the process: a re-created pool (after its destroy)
    // takes the same memory, like the stock static buffer.
    if (!client_cache_buffer) {
      client_cache_buffer =
          VirtualAlloc(nullptr, client_cache_new_size, MEM_COMMIT | MEM_RESERVE,
                       PAGE_READWRITE);
    }
    if (client_cache_buffer) {
      buffer = client_cache_buffer;
      size = client_cache_new_size;
      client_cache_result =
          "client cache pool 0x3880 -> 0x7100 (ClientCache_ClientPool)";
      note("[splitscreen] client cache pool 0x3880 -> 0x7100 "
           "(ClientCache_ClientPool)\n");
    } else {
      client_cache_result =
          "client cache pool: allocation failed - stock 0x3880 kept";
    }
  }
  if (!model_string_buffer && name && size == model_string_stock_size &&
      std::strcmp(name, "UIModelAllocator") == 0) {
    auto *bigger = VirtualAlloc(nullptr, model_string_new_size,
                                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (bigger) {
      model_string_buffer = bigger;
      buffer = bigger;
      size = model_string_new_size;
      model_string_result =
          "ui model string hunk 0xC0000 -> 0x400000 (UIModelAllocator)";
    } else {
      model_string_result =
          "ui model string hunk: allocation failed - stock 0xC0000 kept";
    }
  }
  return hunk_create_hook.invoke<void *>(buffer, size, scheme, flags, arg5,
                                         name, arg7);
}

bool install_model_string_hunk() {
  const auto *p = reinterpret_cast<const uint8_t *>(base() + hunk_create_rva);
  if (!readable(p, sizeof(hunk_create_prologue)) ||
      std::memcmp(p, hunk_create_prologue, sizeof(hunk_create_prologue)) != 0) {
    model_string_result = "ui model string hunk: NOT hooked - "
                          "Hunk_UserCreateFromBuffer bytes differ";
    return false;
  }
  hunk_create_hook.create(reinterpret_cast<void *>(base() + hunk_create_rva),
                          hunk_create_stub);
  model_string_result =
      "ui model string hunk: hooked, waiting for UI_Model_Init";
  return true;
}

constexpr uint32_t model_pool_bound_rva = 0x0200DACE;
constexpr uint8_t model_pool_bound_stock[] = {0xBE, 0x00, 0x90, 0x00,
                                              0x00}; // mov esi,0x9000
constexpr uint8_t model_pool_bound_new[] = {0xBE, 0xFF, 0xFF, 0x00,
                                            0x00}; // mov esi,0xFFFF
const char *model_pool_result = "ui model pool: not attempted";
size_t model_pool_new = 0;

bool relocate_ui_model_pool() {
  if (model_pool_new) {
    return true;
  }
  const auto b = base();
  auto *bound = reinterpret_cast<uint8_t *>(b + model_pool_bound_rva);
  if (!readable(bound, sizeof(model_pool_bound_stock)) ||
      std::memcmp(bound, model_pool_bound_stock,
                  sizeof(model_pool_bound_stock)) != 0) {
    model_pool_result =
        "ui model pool: NOT moved - reset bound bytes differ at 0x0200DACE";
    return false;
  }
  // The old array must still be in its pre-init state (all zero): if the
  // hidden UI_Model_Init had already run, live nodes would be left behind.
  const auto *old_nodes =
      reinterpret_cast<const uint8_t *>(b + model_pool_base);
  if (!readable(old_nodes, 16 * model_pool_stride)) {
    model_pool_result = "ui model pool: NOT moved - old array unreadable";
    return false;
  }
  for (size_t i = 0; i < 16 * model_pool_stride; ++i) {
    if (old_nodes[i] != 0) {
      model_pool_result =
          "ui model pool: NOT moved - old array already initialised";
      return false;
    }
  }
  auto *fresh = static_cast<uint8_t *>(allocate_near_module(
      static_cast<size_t>(model_pool_new_count) * model_pool_stride));
  if (!fresh) {
    model_pool_result = "ui model pool: NOT moved - allocation failed";
    return false;
  }
  std::memset(fresh, 0,
              static_cast<size_t>(model_pool_new_count) * model_pool_stride);
  for (uint32_t i = 0; i < model_pool_new_count; ++i) {
    auto *node = fresh + static_cast<size_t>(i) * model_pool_stride;
    uint32_t next = i + 1;
    if (next == model_pool_sentinel) {
      next = model_pool_sentinel + 1;
    }
    if (next >= model_pool_new_count || i == 0 || i == model_pool_sentinel) {
      next =
          0; // end of list; node 0 is the null handle, 0x9000 the end sentinel
    }
    const auto self = static_cast<uint16_t>(i);
    const auto link = static_cast<uint16_t>(next);
    std::memcpy(node + model_pool_self_off, &self, sizeof(self));
    std::memcpy(node + model_pool_next_off, &link, sizeof(link));
  }
  const auto fresh_abs = reinterpret_cast<size_t>(fresh);

  static int32_t saved[std::size(model_pool_sites)]{};
  if (!rewrite_entcoll(model_pool_sites, std::size(model_pool_sites),
                       model_pool_base, fresh_abs, saved)) {
    model_pool_result = "ui model pool: NOT moved - a reference did not match";
    return false;
  }
  if (!write_bytes(bound, model_pool_bound_new, sizeof(model_pool_bound_new))) {
    for (size_t j = 0; j < std::size(model_pool_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + model_pool_sites[j].rva);
      write_bytes(insn + model_pool_sites[j].disp_off, &saved[j],
                  sizeof(int32_t));
    }
    model_pool_result =
        "ui model pool: NOT moved - reset bound write failed (rolled back)";
    return false;
  }
  model_pool_new = fresh_abs;
  model_pool_result = "ui model pool 0x9000 -> 0xFFFF nodes (28 sites, reset "
                      "bound, 0x9000 kept as end sentinel)";
  install_model_string_hunk();
  note("[splitscreen] ui model pool 0x9000 -> 0xFFFF nodes at RVA 0x%08X\n",
       static_cast<uint32_t>(fresh_abs - b));
  return true;
}

// ============ the per-view sun-shadow (SST) ring: 4 -> 8 entries
// (2026-09-27 night, the pane-4 shadow flicker) ============
//
// Reported in testing: textures in pane 4 flicker / graphic glitches.
// tools/burst_panes.py (20 frames/s, cameras still): stair-stepped black
// shadow blocks come and go in pane 4, and turning player 2 or 3 ALONE
// changes pane 4's picture (mean change 4-7, spikes to 24; a still pane
// is ~0.6), while player 1 neither disturbs nor is disturbed.
//
// The sun-shadow setup 0x01D1A3E0 takes a per-VIEW record from a ring:
//   01D1A411  mov ecx, [0x10BA01D0]      counter
//   01D1A426  and eax, 3                 4 entries
//   01D1A429  lea rdx, [0x10BA01E0]      x 0x21F0
//   01D1A443  mov [viewInfo+0x1F10], rdx (RB_SunShadowMaps reads it)
// Measured in an active 4-player round: the counter advances evenly every
// ~2.2 ms, ~4 per 9 ms frame - once per view. With two players (stock) a
// record comes back every second frame; with four, every frame, while the
// GPU may still be drawing the previous frame from it (each record holds
// GPU buffers: "DrawSST", 32 x "sunCacheDispatch", "sstDispatch" per
// partition, created by 0x01D19950). The 2015 PS4 build has no SST system;
// its own per-view ring of the same kind, R_AllocLightingData (0x9A0A20),
// is 32 deep. So the ring grows to 8 = 4 views x 2 frames, the stock
// margin. Every reference (find_lea 0x10BA01E0..0x10BA89A8, the rest were
// decodes of data):
//   0x01D1A429 alloc base, 0x01D1A426 `and eax,3` -> 7
//   0x01D19968 / 0x01D19976 buffer creation loop start / end (base+4*0x21F0)
//   0x01D1A2DA free loop start, 0x01D1A2E1 `mov r14d,4` -> 8
//   0x02F03E3A static constructor base, 0x02F03E41 `mov edi,3` (dec/jns) -> 7
// Both constructors (0x01D182A0, element 0x01D18280) only write zeros, so
// the zero-filled new block is a constructed state whatever the order.
// Runs at post_unpack, before the renderer creates the buffers.
constexpr uint32_t sst_base = 0x10B21260;
constexpr uint32_t sst_stride = 0x21F0;
constexpr uint32_t sst_old_count = 4;
constexpr uint32_t sst_new_count = 8;
constexpr entcoll_site sst_sites[] = {
    {0x1D0E059, 3, 7, true, 0}, // lea rdx,[base]        alloc
    {0x1D0D598, 3, 7, true,
     sst_stride}, // lea rbp,[base+0x21F0] buffer creation
    {0x1D0DF0A, 3, 7, true, sst_stride}, // lea rbp,[base+0x21F0] free loop
    {0x2E8ACCA, 3, 7, true, 0}, // lea rbx,[base]        static constructor
};
constexpr entcoll_site sst_end_site[] = {
    {0x1D0D5A6, 3, 7, true,
     sst_old_count *sst_stride}, // lea r14,[end] creation loop end
};
struct sst_imm {
  uint32_t rva;
  uint8_t off;
  uint8_t size;
  uint32_t was;
  uint32_t want;
};
constexpr sst_imm sst_imms[] = {
    {0x1D0E056, 2, 1, 3, sst_new_count - 1}, // and eax,3 -> 7
    {0x1D0DF11, 2, 4, 4, sst_new_count},     // mov r14d,4 -> 8
    {0x2E8ACD1, 1, 4, 3, sst_new_count - 1}, // mov edi,3 (dec/jns) -> 7
};
const char *sst_result = "sun-shadow ring: not attempted";
size_t sst_new = 0;

bool relocate_sst_ring() {
  if (sst_new) {
    return true;
  }
  const auto b = base();
  for (const auto &s : sst_imms) {
    const auto *at = reinterpret_cast<const uint8_t *>(b + s.rva + s.off);
    uint32_t cur = 0;
    if (!readable(at, s.size)) {
      sst_result = "sun-shadow ring: NOT moved - immediate unreadable";
      return false;
    }
    std::memcpy(&cur, at, s.size);
    if (cur != s.was) {
      sst_result = "sun-shadow ring: NOT moved - an immediate differs";
      return false;
    }
  }
  auto *fresh =
      static_cast<uint8_t *>(allocate_near_module(sst_new_count * sst_stride));
  if (!fresh) {
    sst_result = "sun-shadow ring: NOT moved - allocation failed";
    return false;
  }
  std::memset(fresh, 0, sst_new_count * sst_stride);
  const auto fresh_abs = reinterpret_cast<size_t>(fresh);

  static int32_t saved[std::size(sst_sites)]{};
  static int32_t saved_end[std::size(sst_end_site)]{};
  if (!rewrite_entcoll(sst_sites, std::size(sst_sites), sst_base, fresh_abs,
                       saved)) {
    sst_result = "sun-shadow ring: NOT moved - a base lea did not match";
    return false;
  }
  const auto undo_sites = [&] {
    for (size_t j = 0; j < std::size(sst_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + sst_sites[j].rva);
      write_bytes(insn + sst_sites[j].disp_off, &saved[j], sizeof(int32_t));
    }
  };
  // target = new_abs + target_off(4*stride): passing new + 4*stride lands on
  // new + 8*stride.
  if (!rewrite_entcoll(sst_end_site, std::size(sst_end_site), sst_base,
                       fresh_abs + (sst_new_count - sst_old_count) * sst_stride,
                       saved_end)) {
    undo_sites();
    sst_result = "sun-shadow ring: NOT moved - the end lea did not match";
    return false;
  }
  size_t done = 0;
  for (const auto &s : sst_imms) {
    auto *at = reinterpret_cast<uint8_t *>(b + s.rva + s.off);
    if (!write_bytes(at, &s.want, s.size)) {
      for (size_t j = 0; j < done; ++j) {
        write_bytes(
            reinterpret_cast<uint8_t *>(b + sst_imms[j].rva + sst_imms[j].off),
            &sst_imms[j].was, sst_imms[j].size);
      }
      auto *insn = reinterpret_cast<uint8_t *>(b + sst_end_site[0].rva);
      write_bytes(insn + sst_end_site[0].disp_off, &saved_end[0],
                  sizeof(int32_t));
      undo_sites();
      sst_result = "sun-shadow ring: NOT moved - an immediate write failed "
                   "(rolled back)";
      return false;
    }
    ++done;
  }
  sst_new = fresh_abs;
  sst_result =
      "sun-shadow ring [4] -> [8] (per-view records, 5 leas + 3 immediates)";
  return true;
}

// ============ cl_voiceCommunication: the foreign array UNDER
// clientUIActives[2] - moved so slot 2 owns its memory ============
//
// clientUIActives is [2] x 0x1078 at 0x053D8BC0. Its slot 2 would sit at
// 0x053DACB0, and the engine's own [lc]-indexed code has been writing
// client 2's record THERE all along (every crash capture reads it live:
// flags 0x37, connectionState 0x0B = CA_ACTIVE). That works only because
// what really lives at 0x053DACB0 is cl_voiceCommunication (DWARF:
// voiceCommunication_t[4] x 0xA34 on PS4, [2] on PC = 0x053DACB0 ..
// 0x053DC118), which offline play barely touches. "Barely" is not
// "never": the voice code increments/clears the queue count at +0xA28
// and walks the queue at +1 - straight through client 2's UI record.
// A stomped record is a plausible source of the third-seat drops seen
// on 2026-09-14 (seats 110 -> 100 -> 110, [61] "ctrl2 rejected").
//
// Relocating clientUIActives itself is the CLOSED DEAD END (607 refs,
// immediate-form references no scan can find, black screen at launch -
// LOG.md 2026-08-24). The planned alternative - a sidecar plus a dozen
// caved accessors - was never built. Neither is needed for THREE
// players: move the NEIGHBOUR instead. cl_voiceCommunication is
// reached through a single PS4 accessor
// (CL_GetLocalClientVoiceCommunication, 0x1DA6C40) and on PC through
// exactly 12 RIP-relative references, all found by two independent
// scanners (linear disassembly of every .pdata function, and
// find_lea.py's byte scan): 5 leas and 7 direct accesses of the count
// at +0xA28. Move it to a fresh [4] x 0xA34 block and 0x053DACB0 ..
// 0x053DBD28 becomes exclusive storage for clientUIActives[2] with not
// one of clientUIActives' references touched - correct by construction,
// the same inversion that fixed the scene buffers (move B, not A).
//
// The 12 OTHER references landing in this span are clientUIActives'
// own loop-END sentinels (&clientUIActives[2] and +8), all using
// 0x1078/0x20F0 and all owned by widen_client_ui_walker_bounds(). They
// are deliberately NOT in this table.
//
// Player 4 is a different matter: slot 3 (0x053DBD28..0x053DCDA0) runs
// into 0x053DC188, the clientActive base pointer. Not this change.
//
// WHERE THE MOVE HAPPENS: reloc_tables' "voice_comm" entry
// (splitscreen_reloc.hpp, from voicecomm_reloc.validated.txt), at
// startup and ungated, followed by zeroing the vacated original. It
// carried 11 of the 12 references - the v1 scan dropped the one with
// an immediate, 0x013E3BB4 `cmp dword [count], 0` - so on 2026-09-20 a
// second, 12-site relocate_voice_communication() was added to the
// BO3_CG_FRAME group. The 2026-09-25 audit found the two stacked: the
// second rewrote all 12 sites (rewrite_entcoll did not verify then) onto
// another block holding a copy of the ZEROED original, leaving the
// first block orphaned. Now: 0x013E3BB4 is row 12 of the validated
// table, the one relocation owns all 12 references, and the second copy
// is gone.

// ============ DWARF-map batch 1: per-client cgame arrays taken off the
// PS4 map PROACTIVELY, not crash by crash ============
//
// dwarf_localclient.txt lists every PS4 global sized LOCAL_CLIENT_COUNT.
// On 2026-09-20 the untouched ones on the cgame path were located on the
// PC by their element stride (imul reg,reg,STRIDE next to the base lea)
// and their COMPLETE reference sets generated by tools/gen_reloc_sites.py
// - RIP-relative by linear disassembly of every .pdata function, ABS32
// by validated byte scan - and kept in data/reloc_sites/. Rule 4 was run
// on each would-be slot 2..3 span; all three are FOREIGN there, so all
// three are relocated, none widened. Same rewrite_entcoll() machinery,
// all-or-nothing per array, gated on BO3_CG_FRAME with the rest.
// cgDC - the per-client display context (CG_Init memsets cgDC[lc]; lc=2 hit
// foreign memory at 0x04A34D40)
constexpr uint32_t cgdc_base = 0x49B2CD0;
constexpr uint32_t cgdc_stride = 0x1838;
constexpr entcoll_site cgdc_sites[] = {
    {0x8F0ABC, 3, 7, false, 0x0000}, // lea rbx, [rbx + 0x4a31cd0]
    {0x10AAC43, 5, 9, false,
     0x002C}, // movss xmm0, dword ptr [rax + rcx + 0x4a31cfc]
};
bool cgdc_relocated = false;

bool relocate_cgdc() {
  if (cgdc_relocated) {
    return true;
  }
  const auto b = base();
  auto *fresh = static_cast<uint8_t *>(allocate_near_module(4 * cgdc_stride));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, 4 * cgdc_stride);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + cgdc_base),
              2 * cgdc_stride);
  static int32_t saved[std::size(cgdc_sites)]{};
  if (!rewrite_entcoll(cgdc_sites, std::size(cgdc_sites), cgdc_base,
                       reinterpret_cast<size_t>(fresh), saved)) {
    return false;
  }
  cgdc_relocated = true;
  note("[splitscreen] cgdc [2]->[4] at RVA 0x%08X (%zu sites)\n",
       static_cast<uint32_t>(reinterpret_cast<size_t>(fresh) - b),
       std::size(cgdc_sites));
  return true;
}

// cg_pmove was relocated here on 2026-09-20 and REMOVED in the 2026-09-25
// audit: it is NOT a per-local-client array. Two vector constructors,
// 0x02DBFAC2 and 0x02F788B7, build it as `lea rcx,0x09F438F0 ; mov edx,
// 0x1660 ; mov r8d,0x12 ; jmp ehvec_ctor` - EIGHTEEN elements, indexed by
// client number (the accessor at 0x015A815C indexes with [rbx], a
// field of the pmove struct, not lc). Moved into four slots it let
// clients 4..17 write past the end of the allocation. The PS4 stride
// 0x1660 matched by coincidence - the same mistake as cg_zbarriers.
// gen_reloc_sites.py v2 now refuses any array a vector constructor
// sizes differently from [2].

// cg_zbarriers was in this batch and REMOVED the same day: the stride-0x188
// array at 0x05E4A900 is a POOL of 0x2C00 entries indexed by a handle, not
// a per-client [2] - the init loop at 0x013EA0B2 links 11,264 entries and
// ran straight off a 4-element block (boot crash, pid 25404). A stride
// match is not enough: the INDEX must be lc. See gen_reloc_sites.py.

// ============ playerKeys: the per-client key/binding state, and the
// ctrl-2 'dangling string' crash ============
//
// 0x022E9550 (a generic stricmp) with rsi = 0x3280 = 2*0x1940 - local
// client 2's offset INTO THIS ARRAY - (rax = 0x20F0 = 2*0x1078 is the
// same lc scaled by the clientUIActives stride) is the key-event walker
// at 0x01342200 comparing a BINDING STRING of local client 2:
//
//   01342241  lea  r13, [rdx + 0x0539C188]    ; rdx = module base
//   01342253  imul rsi, rsi, 0x1940           ; lc * stride
//   0134225A  add  r13, rsi
//   013422F2  mov  rbx, [r13 + rcx*8 + 0x10]  ; binding pointer
//   01342306  call stricmp(rbx, ...)          ; rbx dangling for lc=2
//
// DWARF names it: playerKeys, PlayerKeyState[4], PS4 element 0x1810; the
// PC stride is 0x1940 and the base is 0x0539C050 - fifteen imul-0x1940
// accessors resolve to it; 0x0539C180 is its field +0x130. It is [2] on
// PC, so client 2's key state, and every binding pointer in it, was
// whatever foreign memory follows the array. That is the LOG.md 7255
// 'per-controller-2 dangling string' (ESC out of the gobblegum menu),
// and it would break player 3's controls in a round the same way.
//
// 82 references (38 RIP, 44 ABS32) from tools/gen_reloc_sites.py v2.1.
// The first table shipped 70 and was INCOMPLETE - found in the 2026-09-25
// audit: v1 never scanned leaf functions (no .pdata entry on x64), losing
// four accessors (0x01346CEB, 0x01347080, 0x0134722D, 0x01347973), and
// only accepted ABS32 instructions ending right after the displacement,
// losing eight stores that carry an immediate (`mov dword [..+0x539D98C],
// 1` etc.). With the relocation on, those twelve kept using the OLD array:
// key state split across two copies for every player. Table
// in data/reloc_sites/sites_playerkeys.txt. Rule 4: 32 references into
// the would-be slots 2..3 (+0x3280 is the next global) - foreign, so
// relocate. The engine initializes key state per client itself
// (CL_ClearKeys(lc), already widened); a fresh zeroed block gives
// client 2 NULL binding pointers until it does, which stricmp's own
// null-guards at 0x022E953C/9541 accept.
constexpr uint32_t playerkeys_base = 0x531D850;
constexpr uint32_t playerkeys_stride = 0x1940;
constexpr entcoll_site playerkeys_sites[] = {
    {0x12F2B9B, 3, 7, false,
     0x1938}, // mov esi, dword ptr [rax + rsi + 0x539d988]
    {0x133A477, 3, 7, true, 0x0000},  // lea rdi, [rip + 0x4061bf2]
    {0x133DE94, 3, 7, true, 0x0000},  // lea rcx, [rip + 0x405e1d5]
    {0x133F27A, 3, 7, false, 0x0138}, // lea r15, [r8 + 0x539c188]
    {0x133F2C6, 4, 8, false, 0x0130}, // inc dword ptr [r8 + rbp + 0x539c180]
    {0x133F2D5, 4, 8, false, 0x0130}, // dec dword ptr [r8 + rbp + 0x539c180]
    {0x133F2DD, 4, 8, false,
     0x0130}, // mov eax, dword ptr [r8 + rbp + 0x539c180]
    {0x133F2E8, 4, 8, false,
     0x0130}, // mov dword ptr [r8 + rbp + 0x539c180], eax
    {0x133F668, 3, 7, false, 0x0138}, // lea r12, [r8 + 0x539c188]
    {0x133F6C1, 4, 8, false, 0x0130}, // inc dword ptr [rbx + r8 + 0x539c180]
    {0x133F6D0, 4, 8, false, 0x0130}, // dec dword ptr [rbx + r8 + 0x539c180]
    {0x133F6D8, 4, 8, false,
     0x0130}, // mov eax, dword ptr [rbx + r8 + 0x539c180]
    {0x133F6E3, 4, 8, false,
     0x0130}, // mov dword ptr [rbx + r8 + 0x539c180], eax
    {0x133FEC2, 4, 8, false, 0x0138}, // lea r12, [r12 + 0x539c188]
    {0x133FF22, 3, 7, false, 0x0130}, // inc dword ptr [rdi + rax + 0x539c180]
    {0x13405A4, 3, 7, true, 0x1938},  // lea rax, [rip + 0x405d3fd]
    {0x1340E1F, 3, 7, true, 0x0000},  // lea rax, [rip + 0x405b24a]
    {0x1341D93, 3, 7, true, 0x0138},  // lea rcx, [rip + 0x405a40e]
    {0x1341E42, 3, 7, true, 0x0000},  // lea r12, [rip + 0x405a227]
    {0x1341ED9, 3, 7, true, 0x0000},  // lea r13, [rip + 0x405a190]
    {0x1341F93, 3, 7, true, 0x0000},  // lea r13, [rip + 0x405a0d6]
    {0x1342029, 3, 7, true, 0x0000},  // lea r13, [rip + 0x405a040]
    {0x1342261, 3, 7, false, 0x0138}, // lea r13, [rdx + 0x539c188]
    {0x13422C6, 3, 7, false, 0x0130}, // inc dword ptr [rsi + rdx + 0x539c180]
    {0x13422D4, 3, 7, false, 0x0130}, // dec dword ptr [rsi + rdx + 0x539c180]
    {0x13422DB, 3, 7, false,
     0x0130}, // mov eax, dword ptr [rsi + rdx + 0x539c180]
    {0x13422E6, 3, 7, false,
     0x0130}, // mov dword ptr [rsi + rdx + 0x539c180], eax
    {0x1343E68, 4, 8, false,
     0x1938}, // mov dword ptr [rsi + r9 + 0x539d988], eax
    {0x1343E7E, 4, 12, false,
     0x1938},                         // mov dword ptr [rsi + r9 + 0x539d988], 3
    {0x1343E9D, 4, 9, false, 0x1938}, // cmp dword ptr [rsi + r9 + 0x539d988], 2
    {0x1343EB8, 4, 8, false,
     0x1938}, // mov dword ptr [rsi + r9 + 0x539d988], r8d
    {0x1343EC2, 4, 8, false,
     0x1938}, // mov dword ptr [rsi + r9 + 0x539d988], r12d
    {0x134499C, 4, 8, false,
     0x1038}, // mov ebp, dword ptr [rax + r15 + 0x539d088]
    {0x13449A4, 4, 8, false,
     0x1020}, // mov edi, dword ptr [rax + r15 + 0x539d070]
    {0x13449AC, 4, 8, false,
     0x1008}, // mov r14d, dword ptr [rax + r15 + 0x539d058]
    {0x1345375, 3, 7, true, 0x012C}, // lea rax, [rip + 0x4056e20]
    {0x1345650, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4056a19]
    {0x13456F0, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4056979]
    {0x13457EF, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x405687a]
    {0x13459E0, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4056689]
    {0x1345DC2, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x40562a7]
    {0x1345F97, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x40560d2]
    {0x13462E1, 4, 8, false,
     0x0140}, // cmp r14d, dword ptr [rax + r11 + 0x539c190]
    {0x134638D, 4, 8, false,
     0x0144}, // cmp r11d, dword ptr [rax + r14 + 0x539c194]
    {0x13463D8, 4, 8, false,
     0x0144}, // mov dword ptr [rax + r14 + 0x539c194], r15d
    {0x13463E0, 4, 12, false,
     0x193C}, // mov dword ptr [rdx + r14 + 0x539d98c], 1
    {0x134640C, 4, 8, false,
     0x0144}, // mov dword ptr [rax + r14 + 0x539c194], r15d
    {0x1346414, 4, 12, false,
     0x193C}, // mov dword ptr [rdx + r14 + 0x539d98c], 1
    {0x1346468, 3, 7, false, 0x0148}, // lea rcx, [rax + 0x539c198]
    {0x134648B, 4, 8, false,
     0x0140}, // mov dword ptr [rsi + rax + 0x539c190], r14d
    {0x134649A, 4, 12, false,
     0x193C}, // mov dword ptr [rdi + r14 + 0x539d98c], 1
    {0x13464F5, 4, 8, false,
     0x0140}, // cmp r10d, dword ptr [rcx + r14 + 0x539c190]
    {0x1346537, 4, 8, false,
     0x0144}, // mov dword ptr [rax + r14 + 0x539c194], r11d
    {0x134653F, 4, 12, false,
     0x193C}, // mov dword ptr [rdx + r14 + 0x539d98c], 1
    {0x1346566, 4, 8, false,
     0x0144}, // mov dword ptr [rax + r14 + 0x539c194], r11d
    {0x134656E, 4, 12, false,
     0x193C},                        // mov dword ptr [rdx + r14 + 0x539d98c], 1
    {0x134674A, 3, 7, true, 0x0138}, // lea rcx, [rip + 0x4055a57]
    {0x1346917, 3, 7, false, 0x0148}, // lea rcx, [rsi + 0x539c198]
    {0x1346928, 4, 8, false,
     0x0140}, // mov qword ptr [rbx + rsi + 0x539c190], rax
    {0x1346930, 3, 11, false,
     0x193C},                        // mov dword ptr [rdi + rsi + 0x539d98c], 1
    {0x1346B24, 3, 7, true, 0x0144}, // lea rax, [rip + 0x4055689]
    {0x1346BD2, 3, 7, true, 0x0138}, // lea rax, [rip + 0x40555cf]
    {0x1346C62, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4055407]
    {0x1346D0B, 3, 7, true, 0x0140}, // lea rcx, [rip + 0x405549e]
    {0x1346E37, 3, 7, true, 0x0138}, // lea rax, [rip + 0x405536a]
    {0x1346EE2, 3, 7, true, 0x0140}, // lea r13, [rip + 0x40552c7]
    {0x134700D, 3, 7, true, 0x0140}, // lea rax, [rip + 0x405519c]
    {0x13470A0, 3, 7, true, 0x0138}, // lea rax, [rip + 0x4055101]
    {0x134724D, 3, 7, true, 0x0138}, // lea rcx, [rip + 0x4054f54]
    {0x13475E6, 3, 7, true, 0x193C}, // lea rdi, [rip + 0x40563bf]
    {0x13478B4, 3, 7, true, 0x0000}, // lea r15, [rip + 0x40547b5]
    {0x1347993, 3, 7, true, 0x193C}, // lea rcx, [rip + 0x4056012]
    {0x13479FA, 3, 7, true, 0x0148}, // lea rsi, [rip + 0x40547b7]
    {0x1347C68, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4054401]
    {0x1347D9C, 3, 7, true, 0x0000}, // lea r10, [rip + 0x40542cd]
    {0x1347E2F, 3, 7, true, 0x0000}, // lea r12, [rip + 0x405423a]
    {0x1347E3B, 3, 7, true, 0x0148}, // lea rax, [rip + 0x4054376]
    {0x1347E77, 3, 7, true, 0x0148}, // lea rax, [rip + 0x405433a]
    {0x1347F7F, 3, 7, true, 0x0138}, // lea rax, [rip + 0x4054222]
    {0x13481CE, 3, 7, true, 0x0000}, // lea r8, [rip + 0x4053e9b]
    {0x1DDE9AA, 3, 7, true, 0x0000}, // lea rax, [rip + 0x35b12cf]
    // 0x0219DA4D `adc bh, [rdi+0x539e9e4]` REMOVED (2026-09-25 pdata
    // cross-check): inside the 370 KB Arxan-flattened blob
    // 0x02195450..0x021EFAE4, right after two unconditional jmps and a nop, and
    // no jmp/call/jcc anywhere in the image targets it or the byte after it -
    // unreachable filler whose 4 bytes happen to equal playerKeys+0x2994. Never
    // write unproven bytes (the audit excluded cgDC's 0x021D4754 in the same
    // blob for the same reason).
};
bool playerkeys_relocated = false;

bool relocate_playerkeys() {
  if (playerkeys_relocated) {
    return true;
  }
  const auto b = base();
  auto *fresh =
      static_cast<uint8_t *>(allocate_near_module(4 * playerkeys_stride));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, 4 * playerkeys_stride);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + playerkeys_base),
              2 * playerkeys_stride);
  static int32_t saved[std::size(playerkeys_sites)]{};
  if (!rewrite_entcoll(playerkeys_sites, std::size(playerkeys_sites),
                       playerkeys_base, reinterpret_cast<size_t>(fresh),
                       saved)) {
    return false;
  }
  // The binding-clear loop at 0x013479C0 walks every client's key table
  // by pointer: rsi = &playerKeys[0]+0x148 (a site above), r14 =
  // &playerKeys[2]+0x148 = END MARKER (lea r14 at 0x013479E3), `add rsi,
  // 0x1940 / cmp rsi,r14 / jl`. Not in the table (it points past the two
  // slots), so from 09-20 until this fix the loop compared the new array
  // against the old end and cleared player 1's bindings only. Found by
  // tools/endmarker_scan.py. New end = &new[4]+0x148, as PS4 loops four.
  if (!retarget_end_marker(
          0x01347A03, 3, 7, playerkeys_base + 2 * playerkeys_stride + 0x148,
          reinterpret_cast<size_t>(fresh) + 4 * playerkeys_stride + 0x148)) {
    for (size_t j = 0; j < std::size(playerkeys_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + playerkeys_sites[j].rva);
      write_bytes(insn + playerkeys_sites[j].disp_off, &saved[j],
                  sizeof(int32_t));
    }
    return false;
  }
  playerkeys_relocated = true;
  note("[splitscreen] playerKeys [2]->[4] at RVA 0x%08X (%zu sites)\n",
       static_cast<uint32_t>(reinterpret_cast<size_t>(fresh) - b),
       std::size(playerkeys_sites));
  return true;
}

// ============ g_notetrackLerps: the crash after playerKeys (2026-09-25)
// ============
//
// First 3-player lobby on the audited build: the round started and died in
// the cgame frame for client 2 (dump boiii-crash-2026-09-25-18-10-53):
//
//   0xC0000005 at RVA 0x01CF94EF  `or dword [r10+0x180], eax`  WRITE to heap
//   r12 = 2 (lc), r8 = 0x047CAA34 = 0x047CA144 + (2*16 + 12)*0x34
//
// The caller 0x002758D0 is PC's CG_UpdateNotetrackLerps(lc), Arxan-flattened.
// It walks a per-client table at 0x047CA130 indexed (lc*16 + i)*0x34, reads
// an entity number out of each entry and forms `r9 = entities + ent*0x900`.
// For lc = 2 the row is FOREIGN memory, so the 'entity number' is garbage
// and r9 lands ~90 MB off - the write through it faults.
//
// PS4 names it: g_notetrackLerps, notetrackLerp_t 0x34, and every accessor
// takes a LocalClientNum_t (CG_UpdateNotetrackLerps, CG_InitNotetrackLerps,
// CG_AllocNotetrackLerps, CG_SkipNotetrackLerps, CG_GetFreeNotetrackLerpIndex).
// CG_InitNotetrackLerps (0x13C6A0): `imul rcx, 0x340 ; memset ; cmp i, 0x10`
// - [LOCAL_CLIENT_COUNT][16] x 0x34, a 0x340-byte row per client. [2] on PC.
//
// 34 references (5 RIP, 29 ABS32), generated by tools/gen_reloc_sites.py
// v2.1 (raw-byte scan of both code sections, convergence-validated, no
// vector constructor, all targets inside row 0), table in
// data/reloc_sites/sites_notetracklerps.txt. Rule 4: slots 2..3 are
// referenced by other code - foreign - so relocate, never widen in place.
// CG_InitNotetrackLerps(2) initializes the new row itself.
constexpr uint32_t notetracklerps_base = 0x474B130;
constexpr uint32_t notetracklerps_stride = 0x340;
constexpr entcoll_site notetracklerps_sites[] = {
    {0x248E3F, 3, 7, true, 0x0000},  // lea rdx, [rip + 0x45812ea]
    {0x249355, 3, 7, true, 0x0034},  // lea rcx, [rip + 0x4580e08]
    {0x24E039, 3, 7, true, 0x0000},  // lea rcx, [rip + 0x457c0f0]
    {0x26EE8A, 4, 9, false, 0x0000}, // cmp dword ptr [rdi + r10 + 0x47ca130], 3
    {0x26EE99, 4, 8, false,
     0x002C}, // movsxd r8, dword ptr [rdi + r10 + 0x47ca15c]
    {0x2735C3, 4, 8, false,
     0x0000}, // mov eax, dword ptr [rdi + r10 + 0x47ca130]
    {0x2735CF, 4, 8, false,
     0x0030}, // mov edx, dword ptr [rdi + r10 + 0x47ca160]
    {0x2735D7, 3, 7, true, 0x0014}, // lea rax, [rip + 0x4556b66]
    {0x273605, 6, 10, false,
     0x0014}, // movss xmm1, dword ptr [rdi + r10 + 0x47ca144]
    {0x27361B, 4, 8, false,
     0x0014}, // mov eax, dword ptr [rdi + r10 + 0x47ca144]
    {0x27366C, 4, 12, false,
     0x0000},                        // mov dword ptr [rdi + r10 + 0x47ca130], 3
    {0x275949, 4, 9, false, 0x0000}, // cmp dword ptr [rdi + r10 + 0x47ca130], 3
    {0x27595F, 4, 8, false,
     0x002C}, // movsxd r8, dword ptr [rdi + r10 + 0x47ca15c]
    {0x27A063, 4, 8, false,
     0x0024}, // mov edx, dword ptr [rdi + r10 + 0x47ca154]
    {0x27A06B, 4, 8, false,
     0x0028}, // mov r8d, dword ptr [rdi + r10 + 0x47ca158]
    {0x27A07F, 4, 8, false,
     0x0000}, // mov eax, dword ptr [rdi + r10 + 0x47ca130]
    {0x27A0AA, 6, 10, false,
     0x0004}, // movss xmm0, dword ptr [rdi + r10 + 0x47ca134]
    {0x27A0B4, 6, 10, false,
     0x0008}, // movss xmm2, dword ptr [rdi + r10 + 0x47ca138]
    {0x27A0BE, 6, 10, false,
     0x0014}, // movss xmm1, dword ptr [rdi + r10 + 0x47ca144]
    {0x27A0C8, 4, 8, false,
     0x0030}, // mov edx, dword ptr [rdi + r10 + 0x47ca160]
    {0x27A0E7, 6, 10, false,
     0x0018}, // movss xmm0, dword ptr [rdi + r10 + 0x47ca148]
    {0x27A0FA, 6, 10, false,
     0x000C}, // movss xmm1, dword ptr [rdi + r10 + 0x47ca13c]
    {0x27A10C, 6, 10, false,
     0x001C}, // movss xmm2, dword ptr [rdi + r10 + 0x47ca14c]
    {0x27A11F, 6, 10, false,
     0x0010}, // movss xmm0, dword ptr [rdi + r10 + 0x47ca140]
    {0x27A131, 6, 10, false,
     0x0020}, // movss xmm1, dword ptr [rdi + r10 + 0x47ca150]
    {0x27A167, 6, 10, false,
     0x0004}, // movss xmm0, dword ptr [rdi + r10 + 0x47ca134]
    {0x27A171, 6, 10, false,
     0x0014}, // movss xmm1, dword ptr [rdi + r10 + 0x47ca144]
    {0x27A1A8, 6, 10, false,
     0x0004}, // movss xmm0, dword ptr [rdi + r10 + 0x47ca134]
    {0x27A1B2, 6, 10, false,
     0x0014}, // movss xmm1, dword ptr [rdi + r10 + 0x47ca144]
    {0x27A1D9, 4, 8, false,
     0x0030}, // mov edx, dword ptr [rdi + r10 + 0x47ca160]
    {0x27A1F5, 3, 7, true, 0x0014}, // lea rax, [rip + 0x454ff48]
    {0x27A216, 6, 10, false,
     0x0014}, // movss xmm1, dword ptr [rdi + r10 + 0x47ca144]
    {0x27A22C, 4, 8, false,
     0x0014}, // mov eax, dword ptr [rdi + r10 + 0x47ca144]
    {0x27A273, 4, 12, false,
     0x0000}, // mov dword ptr [rdi + r10 + 0x47ca130], 3
};
bool notetracklerps_relocated = false;

bool relocate_notetracklerps() {
  if (notetracklerps_relocated) {
    return true;
  }
  const auto b = base();
  auto *fresh =
      static_cast<uint8_t *>(allocate_near_module(4 * notetracklerps_stride));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, 4 * notetracklerps_stride);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + notetracklerps_base),
              2 * notetracklerps_stride);
  static int32_t saved[std::size(notetracklerps_sites)]{};
  if (!rewrite_entcoll(notetracklerps_sites, std::size(notetracklerps_sites),
                       notetracklerps_base, reinterpret_cast<size_t>(fresh),
                       saved)) {
    return false;
  }
  notetracklerps_relocated = true;
  note("[splitscreen] g_notetrackLerps [2]->[4] at RVA 0x%08X (%zu sites)\n",
       static_cast<uint32_t>(reinterpret_cast<size_t>(fresh) - b),
       std::size(notetracklerps_sites));
  return true;
}

// ============ DWARF-map batch 1b (2026-09-25): the per-client arrays the
// cgame frame touches for player 3, taken off the PS4 map, not crash by crash
// ============
//
// g_notetrackLerps was the sixth crash in a row caused by one of these, and it
// had been on data/perclient_sweep.txt the whole time - the 08-25 'CG family'
// list was filtered by NAME (cg*), so g_*, s_*, *GlobArray never made it. This
// batch comes from scratchpad ps4_perclient_users.py: every PS4 [4] global, the
// functions that really reference it (decoded, not pattern-matched), matched to
// the PC by row stride and CONFIRMED by reading the PC index register (it must
// be the lc argument or the register that indexes cg_t by 0x342720). Tables:
// tools/gen_reloc_sites.py v2.2, copied to data/reloc_sites/sites_<name>.txt.
// Every site is verified against the old address before anything is written.
struct perclient_array {
  const char *name;
  uint32_t base;   // old RVA of slot 0
  uint32_t stride; // bytes per local client
  const entcoll_site *sites;
  size_t count;
  uint32_t ctor_rva;   // 0: zero-fill IS the initial state
  uint8_t ctor_sig[8]; // the constructor's first bytes, verified
};

// cg_pmove - pmove_t[LOCAL_CLIENT_COUNT] (PS4 0x0451EFE0, 0x1660). THE real
// one: the vector constructor at 0x02D47742 builds `lea rcx,0x04D18740 ; mov
// edx,0x1660 ; mov r8d,2` and the accessor 0x0269AF30 indexes it with the lc
// argument. (The 0x09F438F0 array removed on 09-25 was an [18] per-CLIENT
// array.) The element constructor 0x009B4720 stores a VTABLE at +0x2C0 - a
// zero-filled slot would call through NULL - so it is run on slots 2/3. Used
// every frame by CG_PredictPlayerState_Internal (PS4: 84 references). 47 sites
// (20 rip, 27 abs).
constexpr entcoll_site cg_pmove_sites[] = {
    {0x926DC4, 2, 6, true, 0x0340}, // mov dword ptr [rip + 0x43f1cb6], esi
    {0x926DCA, 3, 7, true, 0x0344}, // mov byte ptr [rip + 0x43f1cb3], sil
    {0x926DD1, 2, 10, true,
     0x02D0}, // mov dword ptr [rip + 0x43f1c35], 0x7e967699
    {0x926DDB, 2, 10, true,
     0x02D4}, // mov dword ptr [rip + 0x43f1c2f], 0x7e967699
    {0x926DE5, 2, 10, true,
     0x02D8}, // mov dword ptr [rip + 0x43f1c29], 0x7e967699
    {0x926DEF, 2, 10, true,
     0x02E0}, // mov dword ptr [rip + 0x43f1c27], 0xfe967699
    {0x926DF9, 2, 10, true,
     0x02E4}, // mov dword ptr [rip + 0x43f1c21], 0xfe967699
    {0x926E03, 2, 10, true,
     0x02E8}, // mov dword ptr [rip + 0x43f1c1b], 0xfe967699
    {0x926E0D, 2, 6, true, 0x19A0}, // mov dword ptr [rip + 0x43f32cd], esi
    {0x926E14, 2, 6, true, 0x19A4}, // mov byte ptr [rip + 0x43f32ca], dh
    {0x926E1A, 2, 10, true,
     0x1930}, // mov dword ptr [rip + 0x43f324c], 0x7e967699
    {0x926E24, 2, 10, true,
     0x1934}, // mov dword ptr [rip + 0x43f3246], 0x7e967699
    {0x926E2E, 2, 10, true,
     0x1938}, // mov dword ptr [rip + 0x43f3240], 0x7e967699
    {0x926E38, 2, 10, true,
     0x1940}, // mov dword ptr [rip + 0x43f323e], 0xfe967699
    {0x926E42, 2, 10, true,
     0x1944}, // mov dword ptr [rip + 0x43f3238], 0xfe967699
    {0x926E4C, 2, 10, true,
     0x1948}, // mov dword ptr [rip + 0x43f3232], 0xfe967699
    {0x9D18A0, 4, 8, false,
     0x02B0}, // mov dword ptr [rsi + r15 + 0x4d189f0], ebx
    {0x9D18A8, 3, 7, false, 0x00A8}, // lea rbx, [r15 + 0x4d187e8]
    {0x9D18AF, 4, 8, false,
     0x0000}, // mov qword ptr [r15 + rsi + 0x4d18740], r13
    {0x9D18BA, 4, 9, false, 0x02AC}, // mov byte ptr [rsi + r15 + 0x4d189ec], 0
    {0x9D1915, 4, 12, false,
     0x0294}, // mov dword ptr [rsi + r15 + 0x4d189d4], 0
    {0x9D1927, 4, 8, false,
     0x0290}, // mov dword ptr [rsi + r15 + 0x4d189d0], eax
    {0x9D1C16, 3, 7, false, 0x0008}, // lea rbx, [r15 + 0x4d18748]
    {0x9D1C6D, 3, 7, false, 0x0000}, // lea rcx, [r15 + 0x4d18740]
    {0x9D1CD0, 3, 7, false, 0x0008}, // lea r13, [rax + 0x4d18748]
    {0x9D1CF4, 3, 7, false,
     0x0008}, // mov eax, dword ptr [rsi + rcx + 0x4d18748]
    {0x9D1D10, 3, 7, false, 0x0058}, // lea r8, [rcx + 0x4d18798]
    {0x9D1E56, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rsi + rcx + 0x4d18740]
    {0x9D1E5E, 3, 7, false, 0x0000}, // lea rcx, [rcx + 0x4d18740]
    {0x9D1E8D, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rsi + rax + 0x4d18740]
    {0x9D1F36, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rsi + rax + 0x4d18740]
    {0x9D1F7A, 3, 7, false, 0x0000}, // lea rcx, [r14 + 0x4d18740]
    {0x9D1FE1, 6, 10, false,
     0x0294}, // movss xmm8, dword ptr [rsi + r14 + 0x4d189d4]
    {0x9D2000, 4, 8, false,
     0x0290}, // cmp dword ptr [rsi + r14 + 0x4d189d0], eax
    {0x9D2086, 4, 8, false,
     0x0290}, // mov eax, dword ptr [rsi + r14 + 0x4d189d0]
    {0x9D2106, 4, 8, false,
     0x0290}, // mov eax, dword ptr [rsi + r14 + 0x4d189d0]
    {0x10BBA0C, 4, 9, false,
     0x1618}, // cmp dword ptr [r14 + rax + 0x4d19d58], 0
    {0x10BBA17, 3, 7, false, 0x161C}, // lea rbx, [rax + 0x4d19d5c]
    {0x10BBA53, 4, 8, false,
     0x1618}, // cmp edi, dword ptr [r14 + r15 + 0x4d19d58]
    {0x10C153D, 4, 9, false,
     0x1618}, // cmp dword ptr [r12 + rax + 0x4d19d58], 0
    {0x10C1548, 3, 7, false, 0x161C}, // lea rdi, [rax + 0x4d19d5c]
    {0x10C1583, 4, 8, false,
     0x1618}, // cmp r14d, dword ptr [r12 + r13 + 0x4d19d58]
    {0x23A0ACE, 3, 7, true, 0x0000}, // mov rdx, qword ptr [rip + 0x28feafb]
    {0x23AAE5A, 4, 9, false,
     0x0000},                        // cmp qword ptr [rax + r15 + 0x4d18740], 0
    {0x2621DD8, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x267d7f1]
    {0x2CCE5D2, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x1fd0ff7]
    {0x2EF9AE7, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x1da0082]
};

// s_cameraShakeSet - CameraShakeSet[4] (PS4 0x03F61EF0, 0x104).
// CG_ClearCameraShakes (0x005830A0) memsets [lc]; CG_ShakeCamera reads it every
// frame. 3 sites (3 rip, 0 abs).
constexpr entcoll_site camerashake_sites[] = {
    {0x5830A3, 3, 7, true, 0x0000}, // lea rax, [rip + 0x42608e6]
    {0x58491C, 3, 7, true, 0x0000}, // lea rax, [rip + 0x425f06d]
    {0x58651E, 3, 7, true, 0x0000}, // lea rax, [rip + 0x425d46b]
};

// moverInfos - mover_info_t[4] (PS4 0x03F60F50, 0x390), camera-tween mover
// records. The static init loop at 0x02D46D5A runs a no-op constructor
// (0x004C6C70 is `mov rax,rcx ; ret`), so zero is the initial state. 3 sites (3
// rip, 0 abs).
constexpr entcoll_site moverinfos_sites[] = {
    {0x4CB9D4, 3, 7, true, 0x0000},  // lea rax, [rip + 0x43177d5]
    {0x4F0FDB, 3, 7, true, 0x0000},  // lea rcx, [rip + 0x42f21ce]
    {0x2CCDBEA, 3, 7, true, 0x0000}, // lea rbx, [rip + 0x1a9c44f]
};

// moveInfoEntNum - int[4] (PS4 0x03F61D90), read next to moverInfos at
// 0x004F0FCA. Its slot 2 (0x047E3148) is taken by 3 foreign leas. 1 sites (0
// rip, 1 abs).
constexpr entcoll_site moveinfoentnum_sites[] = {
    {0x4F0FCA, 3, 7, false, 0x0000}, // lea rsi, [r11 + 0x47e3140]
};

// rumbleGlobArray - RumbleGlobals[4] (PS4 0x045269F0, 0x410). GetRumbleGlobals
// is inlined 8 times; every one indexes with the lc argument. 8 sites (8 rip, 0
// abs).
constexpr entcoll_site rumble_sites[] = {
    {0x9E6E50, 3, 7, true, 0x0000}, // lea rax, [rip + 0x4336619]
    {0x9E6E99, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x43365d0]
    {0x9E6F03, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4336566]
    {0x9E6F98, 3, 7, true, 0x0000}, // lea rax, [rip + 0x43364d1]
    {0x9E7033, 3, 7, true, 0x0000}, // lea rax, [rip + 0x4336436]
    {0x9E70AB, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x43363be]
    {0x9E7112, 3, 7, true, 0x0000}, // lea rax, [rip + 0x4336357]
    {0x9EF6A1, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x432ddc8]
};

// atGlobArray - AimTargetGlob[4] (PS4 0x03159380, 0x1604).
// AimTarget_GetGlobArray (0x000771E0) and the clear (0x0007E100) take lc; aim
// assist runs per frame for every gamepad player. 4 sites (3 rip, 1 abs).
constexpr entcoll_site atglob_sites[] = {
    {0x771E3, 3, 7, true, 0x0000}, // lea rax, [rip + 0x36085d6]
    {0x7AA64, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x3604d55]
    {0x7E103, 3, 7, true, 0x0000}, // lea rax, [rip + 0x36016b6]
    {0x86724, 4, 8, false,
     0x1600}, // mov dword ptr [rax + rdx + 0x3680dc0], r14d
};

// g_aimtarget_cmd - AimTarget_Cmd[4] (PS4 0x031592E0, 0x10), indexed lc*16 next
// to atGlobArray at 0x0008672C. Slot 2 lands on a foreign global (static init
// movups at 0x02CE3DE2 writes 0x0367F7AC).
// 4 sites (1 rip, 3 abs).
constexpr entcoll_site aimtargetcmd_sites[] = {
    {0x7A991, 3, 7, true, 0x0000}, // lea r12, [rip + 0x3604de8]
    {0x8672C, 4, 8, false,
     0x0004}, // mov dword ptr [rdx + r11*8 + 0x367f784], r14d
    {0x8978A, 4, 8, false,
     0x0004}, // movsxd rdx, dword ptr [rax + r11*8 + 0x367f784]
    {0x897BA, 4, 8, false,
     0x0004}, // mov dword ptr [rdx + r11*8 + 0x367f784], eax
};

// gArcData - ARC_DATA[4] (PS4 0x03FF7A10, 0xEEC), grenade arc prediction
// (CG_ArcPrediction_Update/Render). Indexed by lc next to cg_t (0x342720).
// 13 sites (6 rip, 7 abs).
constexpr entcoll_site arcdata_sites[] = {
    {0x5F11E7, 3, 7, true, 0x0000}, // lea r14, [rip + 0x42270c2]
    {0x5F66A0, 3, 7, true, 0x0000}, // lea r9, [rip + 0x4221c09]
    {0x5F6852, 3, 7, true, 0x0000}, // lea r9, [rip + 0x4221a57]
    {0x5F6981, 3, 7, true, 0x0000}, // lea r9, [rip + 0x4221928]
    {0x5F8562, 3, 7, true, 0x0000}, // lea r9, [rip + 0x421fd47]
    {0x5F86BE, 3, 7, true, 0x0000}, // lea r9, [rip + 0x421fbeb]
    {0x5FBB81, 4, 8, false,
     0x0E60}, // mov dword ptr [rsi + r12 + 0x4819110], r15d
    {0x5FBBBD, 4, 8, false,
     0x0E60}, // mov dword ptr [rsi + r12 + 0x4819110], r15d
    {0x5FBBC5, 5, 11, false,
     0x0EE8}, // mov word ptr [rsi + r12 + 0x4819198], 0x100
    {0x5FF962, 3, 8, false, 0x0EE8}, // mov byte ptr [rsi + rax + 0x4819198], 1
    {0x5FF97B, 3, 8, false, 0x0EE9}, // mov byte ptr [rsi + rax + 0x4819199], 1
    {0x5FF98C, 3, 8, false, 0x0E60}, // cmp dword ptr [rsi + rax + 0x4819110], 1
    {0x5FF996, 3, 8, false, 0x0EE9}, // mov byte ptr [rsi + rax + 0x4819199], 0
};

// cg_zbarriers - cgZBarrier_t[4][128] (PS4 0x03F2FE90, row 0xC400 = 128 x
// 0x188). CG_InitZBarrier(lc, cent) (0x00461040) hands out &cg_zbarriers[lc][n]
// and stores the pointer in the centity; CG_InitZBarriers (0x004616C0) clears
// the whole array at map start. PC row 2 is foreign memory (553 ABS32 refs) -
// every zombies window barrier player 3 sees was being built on top of it. The
// clear is widened with it: memset 0x18800 -> 0x31000 (PS4 clears 0x31000), see
// below. 2 sites (2 rip, 0 abs).
constexpr entcoll_site zbarriers_sites[] = {
    {0x461048, 3, 7, true, 0x0000}, // lea r10, [rip + 0x43698a1]
    {0x4616C4, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4369225]
};

constexpr perclient_array batch1b[] = {
    {"cg_pmove",
     0x04C99740,
     0x1660,
     cg_pmove_sites,
     std::size(cg_pmove_sites),
     0x009B4720,
     {0x33, 0xD2, 0x0F, 0x57, 0xC0, 0x48, 0x8D, 0x05}},
    {"camerashake",
     0x04764990,
     0x104,
     camerashake_sites,
     std::size(camerashake_sites),
     0x00000000,
     {}},
    {"moverinfos",
     0x047641B0,
     0x390,
     moverinfos_sites,
     std::size(moverinfos_sites),
     0x00000000,
     {}},
    {"moveinfoentnum",
     0x04764140,
     0x4,
     moveinfoentnum_sites,
     std::size(moveinfoentnum_sites),
     0x00000000,
     {}},
    {"rumble",
     0x04C9E470,
     0x410,
     rumble_sites,
     std::size(rumble_sites),
     0x00000000,
     {}},
    {"atglob",
     0x036007C0,
     0x1604,
     atglob_sites,
     std::size(atglob_sites),
     0x00000000,
     {}},
    {"aimtargetcmd",
     0x03600780,
     0x10,
     aimtargetcmd_sites,
     std::size(aimtargetcmd_sites),
     0x00000000,
     {}},
    {"arcdata",
     0x047992B0,
     0xEEC,
     arcdata_sites,
     std::size(arcdata_sites),
     0x00000000,
     {}},
    {"zbarriers",
     0x0474B8F0,
     0xC400,
     zbarriers_sites,
     std::size(zbarriers_sites),
     0x00000000,
     {}},
};
size_t batch1b_new[std::size(batch1b)] = {};

// One [2]->[4] move: fresh 4-slot block near the module, the two live slots
// copied, 2..3 zero-filled and - where the engine constructs the element -
// constructed by the engine's own constructor, then every site rewritten
// (verified first, all or nothing). Returns the new block or 0.
// ---- per-local-client [2][18] x 0x132 array at 0x168CEA20 (2026-09-27) ----
//
// Found while widening the seat table to four (player 4): the August
// seat table (signin_reloc.txt) had taken two of this array's four
// references for clientGameStates END MARKERS - it lies right after the
// old seat array - and pointed them into the reserved .data window
// (0x1A8A7594), while the other two accessors stayed. So the clear
// wiped the reserved window (profile table 0x1A8A7600, caves) instead
// of the array, and the array was split across two places.
// What it is, from the code: 0x020F10C0 turns a controller into a local
// client (0x020EF7C0), gates on lc < cl_maxLocalClients and a
// clientActive flag, then walks the game session's 18 member slots and
// indexes [lc*18 + i] x 0x132 (0x020F2987..0x020F2996). A per-local-
// client [2] array: lc 2 already ran past it, and its slot 2 is the
// static cmd_function_t node 0x168CF528 (Cmd_AddCommand at 0x020F0930).
// Four references (gen_reloc_sites; the Arxan-section hits are junk)
// and one clear, `memset(arr, 0, 0x2B08)` at 0x020F0900/0x020F0909.
// Moved to [4]; the clear widens to four rows.
constexpr entcoll_site session_member_sites[] = {
    {0x20E4180, 3, 7, true, 0x0000}, // lea rcx, [arr]           clear
    {0x20E6224, 3, 7, false,
     0x0000}, // lea rdx, [rax + arr]     rax = image base
    {0x20E6260, 3, 7, true, 0x0021}, // lea rax, [arr + 0x21]
    {0x20E6271, 3, 7, false,
     0x0001}, // lea rsi, [rsi + arr + 1] image-base relative
};
constexpr uint32_t session_member_base = 0x1684FAA0;
constexpr uint32_t session_member_stride = 18 * 0x132; // 0x1584
constexpr uint32_t session_member_clear_rva = 0x20E4189;
size_t session_member_new = 0;

bool relocate_session_members() {
  if (session_member_new) {
    return true;
  }
  const auto b = base();
  auto *len = reinterpret_cast<uint8_t *>(b + session_member_clear_rva);
  constexpr uint8_t len_old[] = {0x41, 0xB8, 0x08,
                                 0x2B, 0x00, 0x00}; // mov r8d, 0x2B08
  constexpr uint8_t len_new[] = {0x41, 0xB8, 0x10,
                                 0x56, 0x00, 0x00}; // mov r8d, 0x5610
  if (!readable(len, sizeof(len_old)) ||
      std::memcmp(len, len_old, sizeof(len_old)) != 0) {
    return false;
  }
  auto *fresh =
      static_cast<uint8_t *>(allocate_near_module(4 * session_member_stride));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, 4 * session_member_stride);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + session_member_base),
              2 * session_member_stride);
  int32_t saved[std::size(session_member_sites)] = {};
  if (!rewrite_entcoll(session_member_sites, std::size(session_member_sites),
                       session_member_base, reinterpret_cast<size_t>(fresh),
                       saved)) {
    return false;
  }
  if (!write_bytes(len, len_new, sizeof(len_new))) {
    for (size_t j = 0; j < std::size(session_member_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + session_member_sites[j].rva);
      write_bytes(insn + session_member_sites[j].disp_off, &saved[j],
                  sizeof(int32_t));
    }
    return false;
  }
  session_member_new = reinterpret_cast<size_t>(fresh);
  return true;
}

size_t relocate_perclient(const perclient_array &a) {
  const auto b = base();
  if (a.ctor_rva) {
    const auto *at = reinterpret_cast<const uint8_t *>(b + a.ctor_rva);
    if (!readable(at, sizeof(a.ctor_sig)) ||
        std::memcmp(at, a.ctor_sig, sizeof(a.ctor_sig)) != 0) {
      note("[splitscreen] %s: constructor bytes differ - nothing moved\n",
           a.name);
      return 0;
    }
  }
  auto *fresh = static_cast<uint8_t *>(allocate_near_module(4 * a.stride));
  if (!fresh) {
    return 0;
  }
  std::memset(fresh, 0, 4 * a.stride);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + a.base), 2 * a.stride);
  if (a.ctor_rva) {
    using ctor_t = void *(*)(void *);
    const auto ctor = reinterpret_cast<ctor_t>(b + a.ctor_rva);
    for (size_t lc = 2; lc < 4; ++lc) {
      ctor(fresh + lc * a.stride);
    }
  }
  std::vector<int32_t> saved(a.count);
  if (!rewrite_entcoll(a.sites, a.count, a.base,
                       reinterpret_cast<size_t>(fresh), saved.data())) {
    return 0;
  }
  note("[splitscreen] %s [2]->[4] at RVA 0x%08X (%zu sites)\n", a.name,
       static_cast<uint32_t>(reinterpret_cast<size_t>(fresh) - b), a.count);
  return reinterpret_cast<size_t>(fresh);
}

// aaGlobArray, COMPLETE table (2026-09-28). The first 4-player round on
// Shadows of Evil died when player 4 aimed and fired: AV at 0x0110CE7F
// reading centity (entnum 0x5685C180 * 0x900) - the entnum came from
// 0x000520F0 (`lea rcx,[0x036753E4]` = aaGlobArray+0x214, lc*0x4E30 +
// i*0x4C) and the count from 0x000520D0 (+0x2814). The flat24 table
// above moved only the 23 `lea reg,[base]` sites, so these accessors -
// and 20 absolute field accesses in 0x0006DBA0, the +0x4E18/+0x4E20
// readers, the +0x1B4 site - still read the OLD [2] array: stale data
// for players 1-2, foreign memory (0x0367EE30..) for players 3-4. Its
// exclusion of 0x00039BB9 was wrong too: `lea rcx,[rip-0x39BB5]` is the
// module base, so `lea rsi,[rcx+0x36751D0]` IS the array.
// tools/gen_reloc_sites.py v2 (data/reloc_sites/sites_aimglob.txt): 50
// sites (28 rip, 22 abs), 8 byte coincidences rejected, rule 4: 309
// raw ABS32 hits into slots 2..3 -> foreign. pdata_xcheck: the 7
// table-only sites are leaf accessors / no .pdata (both crash accessors
// among them). Zero-fill is the initial state (AimTarget_Init 0x00052170
// memsets each slot).
constexpr entcoll_site aaglob_v2_sites[] = {
    {0x2D7B6, 3, 7, true, 0x0000},  // lea rax, [rip + 0x3647a13]
    {0x2DAD3, 3, 7, true, 0x0000},  // lea rax, [rip + 0x36476f6]
    {0x2F70F, 3, 7, true, 0x0000},  // lea rax, [rip + 0x3645aba]
    {0x2FC45, 3, 7, true, 0x0000},  // lea rax, [rip + 0x3645584]
    {0x2FFD1, 3, 7, true, 0x0000},  // lea rax, [rip + 0x36451f8]
    {0x34D16, 3, 7, true, 0x0000},  // lea rax, [rip + 0x36404b3]
    {0x369C8, 3, 7, true, 0x0000},  // lea rax, [rip + 0x363e801]
    {0x39BB9, 3, 7, false, 0x0000}, // lea rsi, [rcx + 0x36751d0]
    {0x3FF57, 3, 7, false,
     0x4E18},                      // mov ebx, dword ptr [rax + rbx + 0x3679fe8]
    {0x43773, 3, 7, true, 0x4E20}, // lea rdx, [rip + 0x3636876]
    {0x43793, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x3631a36]
    {0x43E7D, 3, 7, true, 0x0000}, // lea rax, [rip + 0x363134c]
    {0x457A3, 3, 7, true, 0x4E20}, // lea rcx, [rip + 0x3634846]
    {0x4EDEE, 3, 7, true, 0x0000}, // lea rax, [rip + 0x36263db]
    {0x520D3, 3, 7, true, 0x2814}, // lea rcx, [rip + 0x362590a]
    {0x52104, 3, 7, true, 0x0214}, // lea rcx, [rip + 0x36232d9]
    {0x5219A, 3, 7, true, 0x0000}, // lea rax, [rip + 0x362302f]
    {0x56D33, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x361e496]
    {0x5B969, 3, 7, true, 0x0000}, // lea rax, [rip + 0x3619860]
    {0x5BA60, 3, 7, true, 0x0000}, // lea rax, [rip + 0x3619769]
    {0x5D383, 3, 7, true, 0x0000}, // lea rax, [rip + 0x3617e46]
    {0x604DE, 3, 7, true, 0x0000}, // lea rax, [rip + 0x3614ceb]
    {0x639B3, 3, 7, true, 0x0000}, // lea rax, [rip + 0x3611816]
    {0x65353, 3, 7, true, 0x0000}, // lea rax, [rip + 0x360fe76]
    {0x66CC7, 3, 7, true, 0x0000}, // lea rax, [rip + 0x360e502]
    {0x66D50, 3, 7, true, 0x0000}, // lea rax, [rip + 0x360e479]
    {0x6DC2A, 3, 7, false, 0x00C8}, // lea rsi, [r13 + 0x3675298]
    {0x6DC47, 6, 10, false,
     0x01C0}, // movss dword ptr [rdi + r13 + 0x3675390], xmm6
    {0x6DC6E, 3, 7, false, 0x00CC}, // lea r12, [r13 + 0x367529c]
    {0x6DC8E, 6, 10, false,
     0x01C4}, // movss dword ptr [rdi + r13 + 0x3675394], xmm6
    {0x6DC98, 4, 9, false, 0x018D}, // cmp byte ptr [rdi + r13 + 0x367535d], 0
    {0x6DCA3, 6, 10, false,
     0x0194}, // movss xmm6, dword ptr [rdi + r13 + 0x3675364]
    {0x6DCE4, 6, 10, false,
     0x00B4}, // mulss xmm6, dword ptr [rdi + r13 + 0x3675284]
    {0x6DCEE, 6, 10, false,
     0x01C8}, // movss dword ptr [rdi + r13 + 0x3675398], xmm6
    {0x6DD24, 6, 10, false,
     0x00B4}, // mulss xmm6, dword ptr [rdi + r13 + 0x3675284]
    {0x6DD2E, 6, 10, false,
     0x01CC}, // movss dword ptr [rdi + r13 + 0x367539c], xmm6
    {0x6DD64, 6, 10, false,
     0x00B4}, // mulss xmm6, dword ptr [rdi + r13 + 0x3675284]
    {0x6DD6E, 6, 10, false,
     0x01D0}, // movss dword ptr [rdi + r13 + 0x36753a0], xmm6
    {0x6DDA7, 6, 10, false,
     0x00B4}, // mulss xmm6, dword ptr [rdi + r13 + 0x3675284]
    {0x6DDB1, 6, 10, false,
     0x01D4}, // movss dword ptr [rdi + r13 + 0x36753a4], xmm6
    {0x6DDE7, 6, 10, false,
     0x00B4}, // mulss xmm6, dword ptr [rdi + r13 + 0x3675284]
    {0x6DDF1, 6, 10, false,
     0x01D8}, // movss dword ptr [rdi + r13 + 0x36753a8], xmm6
    {0x6DE64, 6, 10, false,
     0x00B4}, // mulss xmm6, dword ptr [rdi + r13 + 0x3675284]
    {0x6DE6E, 6, 10, false,
     0x01DC}, // movss dword ptr [rdi + r13 + 0x36753ac], xmm6
    {0x6F5EB, 5, 9, false,
     0x01E0}, // movss dword ptr [rax + rdx + 0x36753b0], xmm6
    {0x70E6D, 5, 9, false,
     0x01E4}, // movss dword ptr [rax + rcx + 0x36753b4], xmm6
    {0x71B48, 3, 7, true, 0x0000},   // lea rax, [rip + 0x3603681]
    {0x73644, 3, 7, true, 0x0000},   // lea rax, [rip + 0x3601b85]
    {0x753E0, 3, 7, true, 0x0000},   // lea rax, [rip + 0x35ffde9]
    {0x2C6AC35, 3, 7, true, 0x01B4}, // lea rax, [rip + 0x9915d8]
};
constexpr perclient_array aaglob_array = {"aaGlobArray",
                                          0x035F61D0,
                                          0x4E30,
                                          aaglob_v2_sites,
                                          std::size(aaglob_v2_sites),
                                          0,
                                          {}};

// CG_InitZBarriers (PC 0x004616C0, PS4 0x1697B0) - PS4 clears all four rows
// (memset 0x31000) and loops numcgZBarriers[0..3] = 0. The PC clears two rows
// and resets the two counts with ONE qword store:
//   004616C4  lea  rcx, cg_zbarriers          (a batch-1b site)
//   004616CD  mov  r8d, 0x18800               -> 0x31000
//   004616D3  call memset
//   004616D8  mov  qword [numcgZBarriers], 0  (11 bytes)
// numcgZBarriers (0x047E3100) is NOT moved: its slots 2..3 (0x047E3108..0F)
// have zero references - padding (find_lea; the next global's static init
// starts at 0x047E3110). So the qword store becomes a 16-byte store in the
// same 11 bytes: xorps xmm0,xmm0 / movups [numcgZBarriers], xmm0 / nop.
// xmm0 is volatile in the x64 ABI and the function returns right after.
// Without this, player 3's count would never reset and the second map would
// run it past 128 (PS4 asserts numcgZBarriers[lc] < 128).
bool widen_zbarrier_clear(const size_t zb_new) {
  const auto b = base();
  auto *imm = reinterpret_cast<uint8_t *>(b + 0x004616CD);
  auto *clr = reinterpret_cast<uint8_t *>(b + 0x004616D8);
  constexpr uint8_t imm_old[] = {0x41, 0xB8, 0x00, 0x88, 0x01, 0x00};
  constexpr uint8_t clr_old[] = {0x48, 0xC7, 0x05, 0x1D, 0x2A, 0x30,
                                 0x04, 0x00, 0x00, 0x00, 0x00};
  // the memset's rcx must already point at the MOVED rows
  const auto *lea = reinterpret_cast<const uint8_t *>(b + 0x004616C4);
  int32_t lea_disp = 0;
  std::memcpy(&lea_disp, lea + 3, sizeof(lea_disp));
  if (!zb_new || b + 0x004616CB + lea_disp != zb_new ||
      !readable(imm, sizeof(imm_old)) ||
      std::memcmp(imm, imm_old, sizeof(imm_old)) != 0 ||
      !readable(clr, sizeof(clr_old)) ||
      std::memcmp(clr, clr_old, sizeof(clr_old)) != 0) {
    note("[splitscreen] zbarrier clear: bytes differ - not widened\n");
    return false;
  }
  const auto disp = static_cast<int32_t>(0x04764100 - (0x004616DB + 7));
  uint8_t clr_new[11] = {0x0F, 0x57, 0xC0, 0x0F, 0x11, 0x05, 0, 0, 0, 0, 0x90};
  std::memcpy(clr_new + 6, &disp, sizeof(disp));
  constexpr uint8_t imm_new[] = {0x41, 0xB8, 0x00, 0x10, 0x03, 0x00};
  if (!write_bytes(clr, clr_new, sizeof(clr_new))) {
    return false;
  }
  if (!write_bytes(imm, imm_new, sizeof(imm_new))) {
    write_bytes(clr, clr_old, sizeof(clr_old));
    return false;
  }
  return true;
}

void relocate_batch1b() {
  for (size_t i = 0; i < std::size(batch1b); ++i) {
    if (!batch1b_new[i]) {
      batch1b_new[i] = relocate_perclient(batch1b[i]);
    }
    if (batch1b_new[i] && std::strcmp(batch1b[i].name, "zbarriers") == 0) {
      widen_zbarrier_clear(batch1b_new[i]);
    }
  }
}

// ============ DWARF-map batch 2 (2026-09-25) ============
//
// totalCoverageArea_s - totalCoverageArea_t[4][18] (PS4 0x04E73820, row 0x360),
// CG_TotalCoverage_Frame(LocalClientNum_t). PC 0x04D42420, every site indexes
// with the register that also indexes cg_t (0x342720); slot 2 overlaps four
// foreign globals (find_lea: leas at 0x0123A7D4, 0x01241981, 0x02DA646F).
// 5 sites, identical to the .pdata decode (tools/pdata_xcheck.py).
constexpr entcoll_site totalcoverage_sites[] = {
    {0x125F881, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x3ae2bb8]
    {0x1262A2B, 3, 7, true, 0x0008}, // lea rax, [rip + 0x3adfa16]
    {0x1262B66, 3, 7, true, 0x0000}, // lea rsi, [rip + 0x3adf8d3]
    {0x1262B71, 3, 7, true, 0x0018}, // lea rax, [rip + 0x3adf8e0]
    {0x12633A3, 3, 7, true,
     0x0018}, // lea rcx, [rip + ..] - found by the new-build audit 2026-09-28;
              // the old build had it too (0x01263383) and both scans missed it
    {0x1264CA6, 3, 7, true, 0x0004}, // lea r10, [rip + 0x3add797]
};
// gaGlobs - GpadAxesGlob[4] x 0x48 (PS4 0x05A4F090): the per-client gamepad
// AXIS bindings (+0x18 + axis*8), read by CL_GamepadAxisValue(lc, axis) for
// every stick movement. PC 0x0539B6D0 x 0x48, found through Axis_Unbindall_f
// (0x0133F050: six `mov [gaGlobs + lc*0x48 + 0x18 + k*8], -1`). Slot 2 is
// foreign (s_rightStickModels, then s_gamepadButtons) - player 3's sticks were
// bound through whatever those held.
// CL_InitGamepadAxisBindings (0x01340120) is a LEAF loop that ends on a
// pointer, not a count:
//   01340120  lea r8,  &gaGlobs[0] + 0x1C
//   0134012A  lea r10, &gaGlobs[2] + 0x1C     <- END MARKER
//   ...       add r8, 0x48 / cmp r8, r10 / jl
// Moving the array without the marker would end that loop after client 0
// (new block > old marker) and leave even player 2 unbound. The marker moves
// to &new[4] + 0x1C: all four clients initialized, as PS4's loop does.
constexpr entcoll_site gaglobs_sites[] = {
    {0x133F00B, 3, 7, true, 0x0000},  // lea rax, [rip + 0x405c6de]
    {0x133F085, 3, 7, true, 0x0000},  // lea rcx, [rip + 0x405c664]
    {0x133F5AA, 3, 7, true, 0x0000},  // lea rax, [rip + 0x405c13f]
    {0x133FB06, 3, 7, false, 0x0000}, // lea r14, [r13 + 0x539b6d0]
    {0x133FD4E, 3, 7, false, 0x0000}, // lea r9, [r10 + 0x539b6d0]
    {0x1340140, 3, 7, true, 0x001C},  // lea r8, [rip + 0x405b5c5]
    {0x134067B, 3, 7, false, 0x0000}, // lea rdi, [r15 + 0x539b6d0]
};
// s_rightStickModels - word[5] per controller (PS4 RightStickModel 0xA), PC
// 0x0539B760 x 0xA, read for the controller of the calling client (0x013403DB,
// 0x013404D8). Its slot 3 overlaps s_gamepadButtons[0] (0x0539B780), so it
// moves.
//
// s_gamepadButtons (0x0539B780 x 0x2E, PS4 [4] x 0x2A) moves too. Its slots
// 2..3 have zero CODE references, and that was once written down here as
// "padding" - WRONG, measured 2026-09-27 15:27 and 15:30: with
// CL_InitGamepadModels widened to 3, both launches died at 0x020EE7E0
// (`mov byte [rax+0x28], 0` walking the list at [0x1689DF58]) with rax =
// 0x0055005400530052 - four consecutive UI-model handles 0x52..0x55, i.e.
// ButtonBits.N of controller 2. The dumped image shows why: static list
// nodes {next, name, .., flag +0x28} sit at 0x0539B7E0 and 0x0539B810,
// inside slot 2, reached only through the list links - no lea, no ABS32.
// Six sites, all in 0x013401B0..0x01340425 (range scan rip + ABS32):
constexpr entcoll_site gamepadbuttons_sites[] = {
    {0x13402B6, 4, 8, false, 0x0002}, // lea rdi, [r12 + 0x539b782] init
    {0x13402C7, 5, 9, false, 0x0000}, // mov word [r14 + r12 + 0x539b780], r13w
    {0x1340323, 5, 9, false,
     0x002C}, // mov word [r14 + r12 + 0x539b7ac], ax  KeyPressBits
    {0x134036D, 3, 7, true,
     0x0000}, // lea rcx, [rip -> 0x0539B780]        CL_ModelForButton
    {0x1340383, 3, 7, true,
     0x002C}, // lea rcx, [rip -> 0x0539B7AC]        KeyPressBits getter
    {0x13403D2, 3, 7, true,
     0x0002}, // lea rax, [rip -> 0x0539B782]        per-controller reset
};
constexpr entcoll_site rightstick_sites[] = {
    {0x1340235, 5, 9, false,
     0x0000}, // mov word ptr [r12 + rbx*2 + 0x539b760], ax
    {0x1340243, 5, 9, false,
     0x0000}, // movzx ecx, word ptr [r12 + rbx*2 + 0x539b760]
    {0x1340253, 5, 9, false,
     0x0002}, // mov word ptr [r12 + rbx*2 + 0x539b762], ax
    {0x1340261, 5, 9, false,
     0x0000}, // movzx ecx, word ptr [r12 + rbx*2 + 0x539b760]
    {0x1340271, 5, 9, false,
     0x0004}, // mov word ptr [r12 + rbx*2 + 0x539b764], ax
    {0x134027F, 5, 9, false,
     0x0000}, // movzx ecx, word ptr [r12 + rbx*2 + 0x539b760]
    {0x134028F, 5, 9, false,
     0x0006}, // mov word ptr [r12 + rbx*2 + 0x539b766], ax
    {0x13402A8, 5, 9, false,
     0x0008}, // mov word ptr [r12 + rbx*2 + 0x539b768], ax
    {0x13403FB, 3, 7, true, 0x0000}, // lea rdi, [rip + 0x405b37e]
    {0x13404F8, 3, 7, true, 0x0000}, // lea rdi, [rip + 0x405b281]
};
size_t gaglobs_new = 0;

bool relocate_gaglobs() {
  if (gaglobs_new) {
    return true;
  }
  constexpr uint32_t ga_base = 0x531C6D0;
  constexpr uint32_t ga_stride = 0x48;
  constexpr uint32_t end_rva = 0x134014A; // lea r10, [rip + d32], 7 bytes
  const auto b = base();
  auto *end_disp = reinterpret_cast<uint8_t *>(b + end_rva + 3);
  int32_t end_old = 0;
  if (!readable(end_disp, sizeof(end_old))) {
    return false;
  }
  std::memcpy(&end_old, end_disp, sizeof(end_old));
  const auto *end_insn = reinterpret_cast<const uint8_t *>(b + end_rva);
  if (end_insn[0] != 0x4C || end_insn[1] != 0x8D || end_insn[2] != 0x15 ||
      static_cast<int64_t>(end_rva) + 7 + end_old !=
          static_cast<int64_t>(ga_base) + 2 * ga_stride + 0x1C) {
    note("[splitscreen] gaGlobs: end marker bytes differ - nothing moved\n");
    return false;
  }
  auto *fresh = static_cast<uint8_t *>(allocate_near_module(4 * ga_stride));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, 4 * ga_stride);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + ga_base),
              2 * ga_stride);
  std::vector<int32_t> saved(std::size(gaglobs_sites));
  if (!rewrite_entcoll(gaglobs_sites, std::size(gaglobs_sites), ga_base,
                       reinterpret_cast<size_t>(fresh), saved.data())) {
    return false;
  }
  const auto end_new = static_cast<int64_t>(reinterpret_cast<size_t>(fresh) +
                                            4 * ga_stride + 0x1C) -
                       static_cast<int64_t>(b + end_rva + 7);
  const auto end_new32 = static_cast<int32_t>(end_new);
  if (end_new != end_new32 ||
      !write_bytes(end_disp, &end_new32, sizeof(end_new32))) {
    for (size_t j = 0; j < std::size(gaglobs_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + gaglobs_sites[j].rva);
      write_bytes(insn + gaglobs_sites[j].disp_off, &saved[j], sizeof(int32_t));
    }
    return false;
  }
  gaglobs_new = reinterpret_cast<size_t>(fresh);
  return true;
}

constexpr perclient_array batch2[] = {
    {"totalcoverage",
     0x04CC3420,
     0x360,
     totalcoverage_sites,
     std::size(totalcoverage_sites),
     0,
     {}},
    {"rightstick",
     0x0531C760,
     0xA,
     rightstick_sites,
     std::size(rightstick_sites),
     0,
     {}},
    {"gamepadbuttons",
     0x0531C780,
     0x2E,
     gamepadbuttons_sites,
     std::size(gamepadbuttons_sites),
     0,
     {}},
};
size_t batch2_new[std::size(batch2)] = {};

// cgExploderTriggers - ExploderTrigger[4][1000] x 0x30 (PS4 0x03E9C550, row
// 0xBB80) and cgExploderTriggerCount - int[4] (PS4, next to it). PC: triggers
// 0x047136E0 (right behind RadiantExploderData, which already moved), counts
// 0x043806C8. CG_FindTrigger / CG_AddFxTrigger take LocalClientNum_t; entry =
// base + lc*0xBB80 + i*0x30, count = counts[lc]. Row 2 of the triggers is
// foreign (1557 ABS32 hits), and count slot 3 (0x043806D4) is written by a
// static-init movups at 0x02D20B72.
//
// Trigger table: the 2026-08-21 validated set (exploder_triggers_reloc.
// validated.txt) - NOT the generator's: gen_reloc_sites dropped the init lea
// 0x001FD776 (00-padding right before it defeats convergence) and accepted
// junk at 0x01DB7EF5. pdata_xcheck: identical to the .pdata decode.
//
// CG_ExplodersInit (PC 0x001FD770) clears the counts with ONE qword store
// (`mov qword [counts], rax`, 7 bytes - no room for 16) and memsets the
// triggers with length 0x17700. So both live in ONE block, the four counts
// directly behind the four rows, and the memset length becomes 0x2EE10:
// one clear covers every row and every count, as PS4's loop does.
constexpr entcoll_site exploder_trig_sites[] = {
    {0x1FD4B8, 3, 7, true, 0x0010}, // lea rax,[rip+..]  +0x10 CG_ExploderUpdate
                                    // walk, imul lc,0xBB80
    {0x1FD776, 3, 7, true, 0x0000}, // lea rcx,[rip+..] CG_ExplodersInit memset
                                    // (length widened below)
    {0x2008EA, 3, 7, true,
     0x0018}, // lea rdi,[rip+..]  +0x18   CG_FindTrigger(lc, ...)
    {0x205679, 3, 7, true,
     0x0000}, // lea rcx,[rip+..]          (lc*1000 + i) * 0x30
    {0x2070EA, 3, 7, true,
     0x0000}, // lea rcx,[rip+..]          (lc*1000 + i) * 0x30
    {0x208B38, 3, 7, true,
     0x0000}, // lea rcx,[rip+..]          (lc*1000 + i) * 0x30
};
constexpr entcoll_site exploder_count_sites[] = {
    {0x1FD466, 3, 7, false, 0x0000}, // lea r12, [r10 + 0x43806c8]
    {0x1FD78F, 3, 7, true, 0x0000},  // mov qword ptr [rip + 0x4182f32], rax
    {0x20090A, 3, 7, true, 0x0000},  // lea rdi, [rip + 0x417fdb7]
    {0x205612, 3, 7, false, 0x0000}, // lea rax, [rax + 0x43806c8]
    {0x20706C, 4, 8, false, 0x0000}, // lea rcx, [rax*4 + 0x43806c8]
    {0x208AAC, 4, 8, false, 0x0000}, // lea rcx, [rax*4 + 0x43806c8]
};
constexpr uint32_t exploder_trig_base = 0x46946E0;
constexpr uint32_t exploder_trig_stride = 0xBB80;
constexpr uint32_t exploder_count_base = 0x43016C8;
size_t exploder_trig_new = 0;

bool relocate_exploder_triggers() {
  if (exploder_trig_new) {
    return true;
  }
  const auto b = base();
  auto *len = reinterpret_cast<uint8_t *>(b + 0x001FD77F);
  constexpr uint8_t len_old[] = {0x41, 0xB8, 0x00,
                                 0x77, 0x01, 0x00}; // mov r8d, 0x17700
  constexpr uint8_t len_new[] = {0x41, 0xB8, 0x10,
                                 0xEE, 0x02, 0x00}; // mov r8d, 0x2EE10
  if (!readable(len, sizeof(len_old)) ||
      std::memcmp(len, len_old, sizeof(len_old)) != 0) {
    note("[splitscreen] exploder triggers: memset length bytes differ - "
         "nothing moved\n");
    return false;
  }
  constexpr size_t rows = 4 * exploder_trig_stride; // 0x2EE00
  auto *fresh = static_cast<uint8_t *>(allocate_near_module(rows + 0x10));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, rows + 0x10);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + exploder_trig_base),
              2 * exploder_trig_stride);
  std::memcpy(fresh + rows,
              reinterpret_cast<const void *>(b + exploder_count_base),
              2 * sizeof(int32_t));
  std::vector<int32_t> saved_trig(std::size(exploder_trig_sites));
  std::vector<int32_t> saved_count(std::size(exploder_count_sites));
  if (!rewrite_entcoll(exploder_trig_sites, std::size(exploder_trig_sites),
                       exploder_trig_base, reinterpret_cast<size_t>(fresh),
                       saved_trig.data())) {
    return false;
  }
  const auto undo_trig = [&] {
    for (size_t j = 0; j < std::size(exploder_trig_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + exploder_trig_sites[j].rva);
      write_bytes(insn + exploder_trig_sites[j].disp_off, &saved_trig[j],
                  sizeof(int32_t));
    }
  };
  if (!rewrite_entcoll(exploder_count_sites, std::size(exploder_count_sites),
                       exploder_count_base,
                       reinterpret_cast<size_t>(fresh + rows),
                       saved_count.data())) {
    undo_trig();
    return false;
  }
  if (!write_bytes(len, len_new, sizeof(len_new))) {
    undo_trig();
    for (size_t j = 0; j < std::size(exploder_count_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + exploder_count_sites[j].rva);
      write_bytes(insn + exploder_count_sites[j].disp_off, &saved_count[j],
                  sizeof(int32_t));
    }
    return false;
  }
  exploder_trig_new = reinterpret_cast<size_t>(fresh);
  note("[splitscreen] cgExploderTriggers + counts [2]->[4] at RVA 0x%08X\n",
       static_cast<uint32_t>(exploder_trig_new - b));
  return true;
}

void relocate_batch2() {
  for (size_t i = 0; i < std::size(batch2); ++i) {
    if (!batch2_new[i]) {
      batch2_new[i] = relocate_perclient(batch2[i]);
    }
  }
  relocate_exploder_triggers();
  relocate_gaglobs();
}

// ============ DWARF-map batch 3 (2026-09-25): screen effects + compass
// ============
//
// s_screenBlur ScreenBlur[4] x0x1C, s_screenElectrified / s_screenBurn
// ScreenBurn[4] x0xC (PS4 0x04001E50 / EC0 / EF0 - consecutive, as on PC:
// 0x0481D410 / 0x0481D448 / 0x0481D460, each one's slot 2 is the next one's
// base, the burn's is foreign data at 0x0481D478). CG_ClearBlur/Electrified/
// Burn (0x0060C330/60/90) and CG_GetBlurRadius take lc in ecx; the setters
// index with the cg_t register. Zombies uses both electric and burn.
//
// Compass: CG_CompassUpdateActors(lc) runs EVERY FRAME from CG_Draw2DInternal
// (PS4 callers), so player 3's 2D pass wrote 0x2C00 bytes past s_compassActors
// straight across the other compass tables. PC CG_ClearCompassPingData
// (0x00598880, takes no lc - clears every client) memsets each table with a
// length of TWO rows; each length below is widened to four rows, but only for
// a table that actually moved (widening an unmoved [2] would overflow it).
// Not moved: fake-fire 0x04809D80 is already [4] on PC (clear 0x2100 =
// 4 x 0x840, next table starts exactly there); 0x0480D080 and 0x0480DBF0
// have no accessor but the clear. The actor clear lea 0x00598884 follows junk
// bytes and was found by pdata_xcheck, not the generator.
constexpr entcoll_site screenblur_sites[] = {
    {0x60136E, 3, 7, true, 0x0000}, // lea rax, [rip + 0x421c09b]
    {0x60C333, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x42110d6]
    {0x641853, 3, 7, true, 0x0018}, // lea rcx, [rip + 0x41dbbce]
    {0x6467B2, 3, 7, true, 0x0000}, // lea rax, [rip + 0x41d6c57]
    {0x652BA0, 3, 7, true, 0x0000}, // lea rax, [rip + 0x41ca869]
};
constexpr entcoll_site screenelec_sites[] = {
    {0x602C3B, 4, 8, false, 0x0000}, // lea rdx, [rcx*4 + 0x481d448]
    {0x60C363, 3, 7, true, 0x0000},  // lea r8, [rip + 0x42110de]
    {0x61136D, 4, 8, false,
     0x0004}, // cmp dword ptr [r13 + rdi*4 + 0x481d44c], r12d
    {0x61137F, 4, 8, false,
     0x0004}, // cmp dword ptr [r13 + rdi*4 + 0x481d44c], eax
    {0x611394, 4, 8, false,
     0x0008}, // mov dword ptr [r13 + rdi*4 + 0x481d450], eax
    {0x61139C, 4, 8, false,
     0x0000}, // mov qword ptr [r13 + rdi*4 + 0x481d448], r12
};
constexpr entcoll_site screenburn_sites[] = {
    {0x60C393, 3, 7, true, 0x0000}, // lea r8, [rip + 0x42110c6]
    {0x6113A9, 4, 8, false,
     0x0004}, // cmp dword ptr [r13 + rdi*4 + 0x481d464], r12d
    {0x6113BB, 4, 8, false,
     0x0004}, // cmp dword ptr [r13 + rdi*4 + 0x481d464], eax
    {0x6113D0, 4, 8, false,
     0x0008}, // mov dword ptr [r13 + rdi*4 + 0x481d468], eax
    {0x6113D8, 4, 8, false,
     0x0000}, // mov qword ptr [r13 + rdi*4 + 0x481d460], r12
    {0x63FECB, 4, 8, false, 0x0000}, // lea rdx, [rcx*4 + 0x481d460]
};
constexpr entcoll_site compass_actors_sites[] = {
    {0x598884, 3, 7, true,
     0x0000}, // lea rcx, [rip + 0x426bcf5]   CG_ClearCompassPingData (added:
              // pdata_xcheck)
    {0x5A1EC3, 3, 7, true, 0x0000}, // lea rax, [rip + 0x42626b6]
    {0x5A3B0D, 3, 7, true, 0x0000}, // lea rax, [rip + 0x4260a6c]
    {0x5A5343, 3, 7, true, 0x0000}, // lea rax, [rip + 0x425f236]
    {0x5A6CFF, 3, 7, true, 0x0000}, // lea rax, [rip + 0x425d87a]
    {0x5B3452, 3, 7, true, 0x0000}, // lea rax, [rip + 0x4251127]
    {0x5B4DB4, 3, 7, true, 0x0000}, // lea r11, [rip + 0x424f7c5]
    {0x5B7FDB, 3, 7, true, 0x0000}, // lea r11, [rip + 0x424c59e]
    {0x5CC464, 3, 7, true, 0x0000}, // lea rax, [rip + 0x4238115]
    {0x5D2B8D, 3, 7, true, 0x0000}, // lea rax, [rip + 0x42319ec]
    {0x5D2BBE, 3, 7, true, 0x0000}, // lea rax, [rip + 0x42319bb]
    {0x5D445E, 3, 7, true, 0x0000}, // lea rax, [rip + 0x423011b]
};
constexpr entcoll_site compass_vehicles_sites[] = {
    {0x5988AC, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x42735cd]
    {0x5A20A3, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4269dd6]
    {0x5AB961, 3, 7, true, 0x0000}, // lea rax, [rip + 0x4260518]
    {0x5D95D5, 3, 7, true, 0x0000}, // lea rsi, [rip + 0x42328a4]
};
constexpr entcoll_site compass_artillery_sites[] = {
    {0x593D7F, 3, 7, true, 0x0010}, // lea rax, [rip + 0x427950a]
    {0x593DD2, 4, 8, false,
     0x0010}, // mov dword ptr [rsi + rbx*4 + 0x480d290], r9d
    {0x593DDE, 3, 7, false,
     0x0000}, // mov dword ptr [rsi + rbx*4 + 0x480d280], eax
    {0x593DE8, 3, 7, false,
     0x0004}, // mov dword ptr [rsi + rbx*4 + 0x480d284], eax
    {0x593DF1, 3, 7, false,
     0x0008}, // mov dword ptr [rsi + rbx*4 + 0x480d288], eax
    {0x593DFB, 3, 7, false,
     0x000C}, // mov dword ptr [rsi + rbx*4 + 0x480d28c], eax
    {0x593E18, 5, 9, false,
     0x0000}, // addss xmm0, dword ptr [rsi + rbx*4 + 0x480d280]
    {0x593E21, 5, 9, false,
     0x0000}, // movss dword ptr [rsi + rbx*4 + 0x480d280], xmm0
    {0x593E36, 5, 9, false,
     0x0004}, // addss xmm0, dword ptr [rsi + rbx*4 + 0x480d284]
    {0x593E3F, 5, 9, false,
     0x0004}, // movss dword ptr [rsi + rbx*4 + 0x480d284], xmm0
    {0x5988E8, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4274991]
    {0x5A1FC3, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x426b2b6]
};
constexpr entcoll_site compass_heli_sites[] = {
    {0x5988FC, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4274a6d]
    {0x5A2043, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x426b326]
    {0x5D9285, 3, 7, true, 0x0000}, // lea rsi, [rip + 0x42340e4]
};
constexpr entcoll_site compass_0240_sites[] = {
    {0x598910, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4274c19]
    {0x5A2023, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x426b506]
    {0x5D9145, 3, 7, true, 0x0000}, // lea rsi, [rip + 0x42343e4]
};
constexpr entcoll_site compass_0120_sites[] = {
    {0x598924, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4275085]
    {0x5A2063, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x426b946]
    {0x5D8DC8, 3, 7, true, 0x0000}, // lea rax, [rip + 0x4234be1]
};
constexpr entcoll_site compass_0500_sites[] = {
    {0x598938, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x42762b1]
    {0x5A1FE3, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x426cc06]
    {0x5CC5B0, 3, 7, true, 0x0004}, // lea rax, [rip + 0x424263d]
};
constexpr entcoll_site compass_0400_sites[] = {
    {0x59894C, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x4276c9d]
    {0x5A2083, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x426d566]
    {0x5D94A5, 3, 7, true, 0x0000}, // lea rsi, [rip + 0x4236144]
};
constexpr perclient_array batch3[] = {
    {"screenblur",
     0x0479E410,
     0x1C,
     screenblur_sites,
     std::size(screenblur_sites),
     0,
     {}},
    {"screenelec",
     0x0479E448,
     0xC,
     screenelec_sites,
     std::size(screenelec_sites),
     0,
     {}},
    {"screenburn",
     0x0479E460,
     0xC,
     screenburn_sites,
     std::size(screenburn_sites),
     0,
     {}},
    {"compass_actors",
     0x04785580,
     0x2C00,
     compass_actors_sites,
     std::size(compass_actors_sites),
     0,
     {}},
    {"compass_vehicles",
     0x0478CE80,
     0x900,
     compass_vehicles_sites,
     std::size(compass_vehicles_sites),
     0,
     {}},
    {"compass_artillery",
     0x0478E280,
     0x78,
     compass_artillery_sites,
     std::size(compass_artillery_sites),
     0,
     {}},
    {"compass_heli",
     0x0478E370,
     0xE0,
     compass_heli_sites,
     std::size(compass_heli_sites),
     0,
     {}},
    {"compass_0240",
     0x0478E530,
     0x240,
     compass_0240_sites,
     std::size(compass_0240_sites),
     0,
     {}},
    {"compass_0120",
     0x0478E9B0,
     0x120,
     compass_0120_sites,
     std::size(compass_0120_sites),
     0,
     {}},
    {"compass_0500",
     0x0478FBF0,
     0x500,
     compass_0500_sites,
     std::size(compass_0500_sites),
     0,
     {}},
    {"compass_0400",
     0x047905F0,
     0x400,
     compass_0400_sites,
     std::size(compass_0400_sites),
     0,
     {}},
};
constexpr uint32_t batch3_clear_len[] = {
    0x00000000, // screenblur (no clear site)
    0x00000000, // screenelec (no clear site)
    0x00000000, // screenburn (no clear site)
    0x0059888D, // compass_actors
    0x005988B5, // compass_vehicles
    0x005988F1, // compass_artillery
    0x00598905, // compass_heli
    0x00598919, // compass_0240
    0x0059892D, // compass_0120
    0x00598941, // compass_0500
    0x00598955, // compass_0400
};
size_t batch3_new[std::size(batch3)] = {};

void relocate_batch3() {
  const auto b = base();
  for (size_t i = 0; i < std::size(batch3); ++i) {
    if (batch3_new[i]) {
      continue;
    }
    const auto &a = batch3[i];
    auto *len = batch3_clear_len[i]
                    ? reinterpret_cast<uint8_t *>(b + batch3_clear_len[i])
                    : nullptr;
    const uint32_t len_old = 2 * a.stride;
    const uint32_t len_new = 4 * a.stride;
    if (len) {
      uint32_t cur = 0;
      if (!readable(len, 6) || len[0] != 0x41 || len[1] != 0xB8) {
        continue;
      }
      std::memcpy(&cur, len + 2, sizeof(cur));
      if (cur != len_old) {
        note("[splitscreen] %s: clear length differs - not moved\n", a.name);
        continue;
      }
    }
    batch3_new[i] = relocate_perclient(a);
    if (batch3_new[i] && len) {
      write_bytes(len + 2, &len_new, sizeof(len_new));
    }
  }
}

// ============ DWARF-map batch 4 (2026-09-25): CG_AllocateClientMemory's
// per-client POINTER tables and the destructible cluster ============
//
// PC CG_AllocateClientMemory loops lc = 0 .. r12d (the local-client count,
// 3 in a three-player round) at 0x00843AA0 and, per client, allocates and
// stores (PS4 0x21FD70 names them):
//   [lc*8 + 0x049D9410] = Hunk alloc 0x7000     cg_weaponsArray
//   [lc*8 + 0x17F00FF0] = Hunk alloc 0x11880    cg_destructibles
//   [lc*8 + 0x04A315C0] = Hunk alloc 0xDB7F0    cg_ikBuf ->
//   IK_AllocateLocalClientMemory
//                         (0x02470CB0: [lc*8 + 0x17FA86F8] = buf, ikStates)
// The engine ALLOCATES client 2's buffers itself - but all four tables are
// [2], so the pointers were stored on top of foreign globals: weapons[2] is
// the pointer at 0x049D9420 allocated right before the loop (9 readers) -
// player 3's weapon-info buffer replaced it for everyone; destructibles[2]
// is s_destructible_gamestates[0][0]; ikBuf[2] and ikStates[2] overlap
// static-init globals. Moving the tables is the whole fix - the engine then
// fills slot 2 on its own.
// Destructible counts: cg_numDestructibles 0x17F400E0 [2] is followed by
// cg_updateTime 0x17F400E8 [2] (CG_InitDestructibles zeroes both per lc,
// 0x0236F0C5) - numDestructibles[2] WAS updateTime[0] (read live: 83) and
// updateTime[2] a foreign list at 0x17F400F0. s_destructible_gamestates
// 0x17F01000 [2][32] x 0x84 (row 0x1080, PS4 Destructible_FindGameState
// (int, LocalClientNum_t)) is followed by s_num_destructible_gamestates
// 0x17F03100 [2], whose slot 2 is another global (0x0236F12E writes it).
// 0x0237BD11 (count read) was missed by the generator; pdata_xcheck found it.
constexpr entcoll_site cg_weaponsarray_sites[] = {
    {0x44D1AA, 4, 8, false,
     0x0000}, // add r13, qword ptr [rax + rcx*8 + 0x49d9410]
    {0x843ACF, 4, 8, false,
     0x0000}, // mov qword ptr [rsi + r13 + 0x49d9410], rax
    {0x853DE9, 4, 8, false,
     0x0000}, // mov rdx, qword ptr [r14 + rdi*8 + 0x49d9410]
    {0x856EE2, 3, 7, true, 0x0000}, // mov qword ptr [rip + 0x4182527], rax
    {0x856EE9, 3, 7, true, 0x0008}, // mov qword ptr [rip + 0x4182528], rax
    {0x8F258B, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rcx + r14*8 + 0x49d9410]
    {0x119A273, 4, 8, false,
     0x0000}, // add r15, qword ptr [rdx + r14*8 + 0x49d9410]
    {0x11CA1FD, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x380f22c]
    {0x122C649, 4, 8, false,
     0x0000}, // add rdi, qword ptr [rbp + r14*8 + 0x49d9410]
    {0x126671A, 4, 8, false,
     0x0000}, // add rdi, qword ptr [r10 + rsi*8 + 0x49d9410]
    {0x26D13F4, 4, 8, false,
     0x0000}, // add r13, qword ptr [rcx + rax*8 + 0x49d9410]
    {0x271F6BE, 4, 8, false,
     0x0000}, // add rbx, qword ptr [r14 + rax*8 + 0x49d9410]
};
constexpr entcoll_site cg_ikbuf_sites[] = {
    {0x843B03, 4, 8, false,
     0x0000}, // mov qword ptr [rsi + r13 + 0x4a315c0], rax
    {0x853DC0, 4, 8, false,
     0x0000}, // mov rdx, qword ptr [r14 + rdi*8 + 0x4a315c0]
};
// ikStates is NOT moved (it was in 80F6350B, and that split it): the real
// table is [3] at 0x17FA86F0 - slot 0 the server's IK buffer, then
// [1 + lc] per client (IK_AllocateLocalClientMemory 0x02470CB0 stores at
// 0x17FA86F8 + lc*8) - and the IK reset loop at 0x02471603 walks it
// contiguously from 0x17FA86F0 to its end marker 0x17FA8708
// (`add rsi,8 / cmp rsi,r14`). Client 2's slot IS that end, 0x17FA8708,
// and nothing else references it (find_lea; the 14 raw ABS32 values there
// decode as no instruction) - usable storage for THREE players as it is.
// Only the reset loop's end moves one slot (widen_ik_reset_loop) so it
// resets player 3's IK state too. Player 4's slot 0x17FA8710 is foreign.
constexpr entcoll_site cg_destructibles_sites[] = {
    {0x843AF1, 4, 8, false,
     0x0000}, // mov qword ptr [rsi + r13 + 0x17f00ff0], rax
    {0x853DD9, 4, 8, false,
     0x0000}, // mov rdx, qword ptr [r14 + rdi*8 + 0x17f00ff0]
    {0x856F0C, 3, 7, true, 0x0000}, // mov qword ptr [rip + 0x176aa0dd], rax
    {0x856F13, 3, 7, true, 0x0008}, // mov qword ptr [rip + 0x176aa0de], rax
    {0x22F28C0, 4, 8, false,
     0x0000}, // mov rax, qword ptr [r13 + rdi*8 + 0x17f00ff0]
    {0x22F5C96, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x15b921e3]
    {0x22F5CB3, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x15b921c6]
    {0x22F5CF3, 4, 8, false,
     0x0000}, // mov r10, qword ptr [rdi + r11*8 + 0x17f00ff0]
    {0x22F5D7D, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rdi + r11*8 + 0x17f00ff0]
    {0x22F5D8D, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rdi + r11*8 + 0x17f00ff0]
    {0x22F5D9D, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rdi + r11*8 + 0x17f00ff0]
    {0x22F5DAA, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rdi + r11*8 + 0x17f00ff0]
    {0x22F5DB7, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rdi + r11*8 + 0x17f00ff0]
    {0x22F5DDB, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rdi + r11*8 + 0x17f00ff0]
    {0x22F5EF1, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r14 + rsi*8 + 0x17f00ff0]
    {0x22F5F10, 4, 8, false,
     0x0000}, // mov rdi, qword ptr [r14 + rsi*8 + 0x17f00ff0]
    {0x22F6067, 4, 8, false,
     0x0000}, // add r8, qword ptr [rdi + r12*8 + 0x17f00ff0]
    {0x22F60B4, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdx + r12*8 + 0x17f00ff0]
    {0x22F60C8, 4, 8, false,
     0x0000}, // mov r8, qword ptr [rdx + r12*8 + 0x17f00ff0]
    {0x22F60EA, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rcx + r12*8 + 0x17f00ff0]
    {0x22F6110, 4, 8, false,
     0x0000}, // mov rax, qword ptr [r14 + r12*8 + 0x17f00ff0]
    {0x22F6154, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdi + r12*8 + 0x17f00ff0]
    {0x22F6161, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rdi + r12*8 + 0x17f00ff0]
    {0x22FAB99, 4, 8, false,
     0x0000}, // add rdx, qword ptr [r12 + r15*8 + 0x17f00ff0]
    {0x22FAC3A, 3, 7, true, 0x0000},  // lea rcx, [rip + 0x15b8d23f]
    {0x22FC8DF, 4, 8, false, 0x0000}, // lea rsi, [rdi*8 + 0x17f00ff0]
    {0x22FD47C, 4, 8, false,
     0x0000}, // add rdi, qword ptr [rsi + r15*8 + 0x17f00ff0]
    {0x230064F, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x15b8782a]
};
constexpr entcoll_site numdestructibles_sites[] = {
    // 0x0235CB02 `lea r8` REMOVED: end marker of the s_destructibles loop (see
    // below)
    {0x22F5A58, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x15bd1511]
    {0x22F5D62, 4, 8, false,
     0x0000}, // inc dword ptr [rdi + r11*4 + 0x17f400e0]
    {0x22F5F55, 4, 12, false,
     0x0000}, // mov dword ptr [r14 + rsi*4 + 0x17f400e0], 0
    {0x22F601F, 4, 8, false,
     0x0000}, // mov dword ptr [rdi + r14*4 + 0x17f400e0], eax
    {0x22F91CB, 4, 8, false,
     0x0000}, // cmp r13d, dword ptr [rdi + r12*4 + 0x17f400e0]
    {0x22FAB84, 4, 8, false,
     0x0000}, // cmp dword ptr [r12 + r15*4 + 0x17f400e0], ebx
    {0x22FABC2, 4, 8, false,
     0x0000}, // cmp ebx, dword ptr [r12 + r15*4 + 0x17f400e0]
    {0x22FC94D, 4, 8, false,
     0x0000}, // cmp dword ptr [r14 + r15 + 0x17f400e0], ebx
    {0x22FC98F, 4, 8, false,
     0x0000}, // cmp ebx, dword ptr [r14 + r15 + 0x17f400e0]
    // 0x0237B56E `lea rdx` and 0x0237BF49 `lea r13` REMOVED - with 0x0235CB02
    // they
    // are END MARKERS of loops over s_destructibles (0x17F37CE0, 0x80 x 0x108),
    // which ends exactly where cg_numDestructibles begins: `add rbx,0x108 /
    // cmp rbx,r13 / jl`. Build 80F6350B moved them with this array and the
    // menu map's destructible walk (0x0237BF20) ran off the end - 0xC0000005 at
    // 0x0237BF6C, 2026-09-25 20:08. Caught since by tools/sentinel_check.py.
};
constexpr entcoll_site cg_updatetime_sites[] = {
    {0x22F5F61, 4, 12, false,
     0x0000}, // mov dword ptr [r14 + rsi*4 + 0x17f400e8], 0
    {0x22FC920, 4, 8, false,
     0x0000}, // mov eax, dword ptr [r14 + r15 + 0x17f400e8]
    {0x22FC92A, 4, 8, false,
     0x0000}, // mov dword ptr [r14 + r15 + 0x17f400e8], eax
    {0x22FC945, 4, 8, false,
     0x0000}, // mov dword ptr [r14 + r15 + 0x17f400e8], eax
};
constexpr entcoll_site destr_gamestates_sites[] = {
    {0x230200C, 3, 7, true, 0x0002},  // lea rax, [rip + 0x15b85e7f]
    {0x2302043, 3, 7, true, 0x0000},  // lea rax, [rip + 0x15b85e46]
    {0x2302B07, 3, 7, true, 0x0000},  // lea r15, [rip + 0x15b85382]
    {0x2302BB1, 3, 7, true, 0x0002},  // lea rax, [rip + 0x15b852da]
    {0x2302BE3, 3, 7, false, 0x0000}, // lea rdx, [r11 + 0x17f01000]
    {0x2302BEA, 3, 7, false, 0x0000}, // lea r9, [r11 + 0x17f01000]
};
constexpr entcoll_site destr_numgamestates_sites[] = {
    {0x2301FF3, 3, 7, true, 0x0000},  // lea rax, [rip + 0x15b87f96]
    {0x2302ACD, 3, 7, true, 0x0000},  // lea rax, [rip + 0x15b874bc]
    {0x2302BA1, 4, 8, false, 0x0000}, // mov r9d, dword ptr [r11 + r10*4 +
                                      // 0x17f03100]   (added: pdata_xcheck)
    {0x2302BF1, 4, 8, false,
     0x0000}, // mov dword ptr [r11 + r10*4 + 0x17f03100], eax
};
constexpr perclient_array batch4[] = {
    {"cg_weaponsarray",
     0x0495A410,
     0x8,
     cg_weaponsarray_sites,
     std::size(cg_weaponsarray_sites),
     0,
     {}},
    {"cg_ikbuf",
     0x049B25C0,
     0x8,
     cg_ikbuf_sites,
     std::size(cg_ikbuf_sites),
     0,
     {}},
    {"cg_destructibles",
     0x17E820C0,
     0x8,
     cg_destructibles_sites,
     std::size(cg_destructibles_sites),
     0,
     {}},
    {"numdestructibles",
     0x17EC11B0,
     0x4,
     numdestructibles_sites,
     std::size(numdestructibles_sites),
     0,
     {}},
    {"cg_updatetime",
     0x17EC11B8,
     0x4,
     cg_updatetime_sites,
     std::size(cg_updatetime_sites),
     0,
     {}},
    {"destr_gamestates",
     0x17E820D0,
     0x1080,
     destr_gamestates_sites,
     std::size(destr_gamestates_sites),
     0,
     {}},
    {"destr_numgamestates",
     0x17E841D0,
     0x4,
     destr_numgamestates_sites,
     std::size(destr_numgamestates_sites),
     0,
     {}},
};
size_t batch4_new[std::size(batch4)] = {};

bool ik_reset_widened = false;

// ikStates -> [5] for PLAYER 4 (2026-09-27, static, untested in game).
// PS4 DWARF: `IKState* ikStates[5]` (0x120DB5D0) - the server's state and
// one per local client. The PC table at 0x17FA86F0 has room for three:
// client 2 used its end slot (0x17FA8708, unreferenced), but client 3's
// slot 0x17FA8710 is a byte flag with 7 references (`mov byte [..],1`,
// `cmp byte [..],al` x2, ...) followed by another array (0x024A1E0E) -
// foreign. So the table moves: gen_reloc_sites over 0x17FA86F0..+0x28 -
// 9 references with targets inside the three slots (the +0x20 hits are
// the foreign flag, not ours) plus the reset loop's end marker
// (0x02471623 `lea r14`, stock end 0x17FA8708 -> new + 5*8).
constexpr entcoll_site ikstates_sites[] = {
    {0x23F7B43, 3, 7, true,
     0x0008}, // lea rdx, [ikStates+8]   IK_AllocateLocalClientMemory
    {0x23F7D0D, 3, 7, true, 0x0000}, // lea rdx, [ikStates]
    {0x23F7D3F, 3, 7, true, 0x0000}, // lea rdx, [ikStates]
    {0x23F7E15, 3, 7, true, 0x0008}, // lea rax, [ikStates+8]
    {0x23F8200, 3, 7, true, 0x0000}, // lea rsi, [ikStates]
    {0x23F84AC, 3, 7, true,
     0x0000}, // lea rsi, [ikStates]      reset loop start
    {0x23F8594, 3, 7, true, 0x0000},  // lea rsi, [ikStates]
    {0x23F9260, 3, 7, true, 0x0008},  // lea rcx, [ikStates+8]
    {0x245A539, 4, 9, false, 0x0008}, // cmp qword [rbx+rcx*8+ikStates+8], 0
};
constexpr uint32_t ikstates_base = 0x17F297C0;
constexpr uint32_t ikstates_old_slots = 3;
constexpr uint32_t ikstates_new_slots = 5;
size_t ikstates_new = 0;

bool relocate_ikstates() {
  if (ikstates_new) {
    return true;
  }
  const auto b = base();
  auto *fresh =
      static_cast<uint8_t *>(allocate_near_module(ikstates_new_slots * 8));
  if (!fresh) {
    return false;
  }
  std::memset(fresh, 0, ikstates_new_slots * 8);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + ikstates_base),
              ikstates_old_slots * 8);
  int32_t saved[std::size(ikstates_sites)] = {};
  if (!rewrite_entcoll(ikstates_sites, std::size(ikstates_sites), ikstates_base,
                       reinterpret_cast<size_t>(fresh), saved)) {
    return false;
  }
  if (!retarget_end_marker(
          0x023F8B73, 3, 7, ikstates_base + ikstates_old_slots * 8,
          reinterpret_cast<size_t>(fresh) + ikstates_new_slots * 8)) {
    for (size_t j = 0; j < std::size(ikstates_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + ikstates_sites[j].rva);
      write_bytes(insn + ikstates_sites[j].disp_off, &saved[j],
                  sizeof(int32_t));
    }
    return false;
  }
  ikstates_new = reinterpret_cast<size_t>(fresh);
  return true;
}

void relocate_batch4() {
  for (size_t i = 0; i < std::size(batch4); ++i) {
    if (!batch4_new[i]) {
      batch4_new[i] = relocate_perclient(batch4[i]);
    }
  }
  // The IK reset loop (0x02471603) ends at 0x17FA8708 = client 2's slot;
  // one slot further includes it (lea r14 at 0x02471623, 7 bytes, disp @3).
  // With two players that slot is NULL and the loop's `test rbx,rbx` skips it.
  // Player 4: the table moves to [5] instead (its end marker included);
  // the one-slot widen stays the fallback if any reference does not verify.
  if (!ik_reset_widened) {
    ik_reset_widened =
        relocate_ikstates() ||
        retarget_end_marker(0x023F8B73, 3, 7, 0x17F297D8, base() + 0x17F297E0);
  }
}

// ============ batch 5 (2026-09-25 20:32 round crash) ============
//
// First round on 809C85D6 started (cl_max 3, pane 3 can draw) and died ~13 s
// in: 0xC0000005 at 0x0009EA8B (dump boiii-crash-2026-09-25-20-32-46), `mov
// rax,[rbx] / cmp byte [rax+0x2ad],0` with rax = NULL, rbx = 0x0429E600,
// r12 = rsi = 2. The caller 0x0019A99C(lc, flag) walks a per-client block
// `lea rax,0x0425B500 ; imul rdi,lc,0x21840` - 30 entries of 0x11E0 per client
// (getter 0x001989A0: (lc*0x1E + idx)*0x11E0) - and hands entry+0x80 to the
// interpolation update 0x0009E980. Row 2 (0x0429E580) is foreign: four
// pointer globals the same code stores and reads (0x0019A5AE, 0x0019B3AE ..)
// and another at 0x0429E5A0. It was lead #1 on perclient_sweep.txt (0x0425B500
// x0x21840, 29 slot-2 refs) - unnamed: no PS4 [4] global has this shape.
// Static init 0x02D209F0 only zeroes a 16-byte field in all 60 (2 x 30)
// entries, so zero-fill is the initial state. 12 RIP leas; the generator's 6
// ABS rows are coincidences in the 0x43080-byte span (5 never decoded from a
// function start; 0x010B2F33 `add [rdx+rax+0x427010B],ebx` is a real
// instruction whose odd displacement is a register offset). No end markers
// (endmarker_scan), the flagged cmp at 0x00198952 is `cmp r14b, al` after
// rax was consumed.
constexpr entcoll_site cg_clientents30_sites[] = {
    {0x19891D, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c2bdc]
    {0x1989B0, 3, 7, true, 0x0080},  // lea rcx, [rip + 0x40c2bc9]
    {0x1989EB, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c2b0e]
    {0x198A89, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c2a70]
    {0x19A3BC, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c113d]
    {0x19A462, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c1097]
    {0x19A63D, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c0ebc]
    {0x19A754, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c0da5]
    {0x19A8A4, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c0c55]
    {0x19A9D6, 3, 7, true, 0x0000},  // lea rax, [rip + 0x40c0b23]
    {0x19B831, 3, 7, true, 0x0000},  // lea rdx, [rip + 0x40bfcc8]
    {0x2CA7888, 3, 7, true, 0x00F0}, // lea rax, [rip + 0x153abf1]
};
// 0x0481CC80 x 0x3C0 - 8 entries of 0x78 per client ((lc, idx) accessors
// 0x0060F6A0 / 0x0060F830 / 0x00641870; caller 0x00643580 takes lc and indexes
// cg_t's base pointer 0x04D17C80 with it). Right before s_screenBlur; its row 2
// ran over 0x0481D400..0x0481D7C0 (the moved screen-effect arrays and foreign
// globals at 0x0481D478/80). No PS4 [4] global has this shape. 6 sites,
// identical to the .pdata decode, no end markers, no constructor.
constexpr entcoll_site cg_perclient_3c0_sites[] = {
    {0x60F6B5, 3, 7, true, 0x0000},  // lea rax, [rip + 0x420d5c4]
    {0x60F844, 3, 7, true, 0x0000},  // lea rax, [rip + 0x420d435]
    {0x641878, 3, 7, true, 0x0000},  // lea rax, [rip + 0x41db401]
    {0x643596, 3, 7, true, 0x0000},  // lea rax, [rip + 0x41d96e3]
    {0x65785F, 3, 7, false, 0x0000}, // lea rcx, [rcx + 0x481cc80]
    {0x662085, 3, 7, true, 0x0000},  // lea rcx, [rip + 0x41babf4]
};
constexpr perclient_array batch5[] = {
    {"cg_clientents30",
     0x041DC500,
     0x21840,
     cg_clientents30_sites,
     std::size(cg_clientents30_sites),
     0,
     {}},
    {"cg_perclient_3c0",
     0x0479DC80,
     0x3C0,
     cg_perclient_3c0_sites,
     std::size(cg_perclient_3c0_sites),
     0,
     {}},
};
size_t batch5_new[std::size(batch5)] = {};

void relocate_batch5() {
  for (size_t i = 0; i < std::size(batch5); ++i) {
    if (!batch5_new[i]) {
      batch5_new[i] = relocate_perclient(batch5[i]);
    }
  }
}

// ---- BATCH 6: the cgame threaded-notify queues (2026-09-26) -----------
//
// PS4 CG_ThreadedNotifyList_* (Init 0x2956D0 from CG_Init, ClaimJob 0x2958C0,
// Add* 0x295C70.., ProcessQueue 0x2961B0 from CG_AddPacketEntities): per LOCAL
// CLIENT 100 items of 0x48 (s_threadedNotifyList [4] 0x04538F50) plus the
// queue pointers s_processQueueHead/Tail and s_firstFree [4]. PC twin: Init
// 0x00A21A90 - 100 items of 0x50, stride 0x1F40 - and every array is [2]:
//
//   s_threadedNotifyList 0x04D24820 [2] x 0x1F40  -> ends 0x04D286A0
//   s_processQueueHead   0x04D286C0 [2] x 8   slot 2 = Tail[0]
//   s_processQueueTail   0x04D286D0 [2] x 8   slot 2 = FirstFree[0]
//   s_firstFree          0x04D286E0 [2] x 8   slot 2 = the next global
//
// So CG_Init(2) at map load zeroed player 1's queue tail and free list and
// linked 100 items (8000 bytes) over 0x04D286A0.. - the three pointer arrays
// and ~8 KB of live globals behind them (find_lea: 110 loads/stores there).
// Every notify after that pushed and popped through corrupted lists: a wild
// writer, and a fit for the three symptoms of the 3-player rounds - Arxan
// working from damaged state (.idata shifted by 8), a zone asset gone
// (default_aitype) and the fastfile checksum ("Data is corrupt"). It runs
// with CG_FRAME on AND off, which is why it is NOT gated on BO3_CG_FRAME.
// The sweep had filed list+0x44 (0x04D24864) as "padding", so it was missed.
//
// Tables: gen_reloc_sites + pdata_xcheck. head/tail/free identical to the
// .pdata decode (6 each). list: 5 identical + 6 table-only rows, each read
// by hand - the leaf item-flag 0x00A18760, the four leaf Add* writers
// 0x00A21790/0x00A217CF/0x00A21810/0x00A218B0 and the static initializer
// 0x02DA5E50. sentinel_check: no suspect end markers. endmarker_scan's four
// hits (0x0103AABB/C2, 0x0103F830/39) are loops over OTHER arrays living in
// the overrun region (0x04D2893C..0x04D2A2BC) - they stay as they are.
constexpr entcoll_site tnotify_list_sites[] = {
    {0xA18777, 3, 7, true, 0x0044},  // lea rcx, [rip + 0x430c0e6]
    {0xA2179C, 3, 7, true, 0x0000},  // lea rax, [rip + 0x430307d]
    {0xA217DC, 3, 7, true, 0x0000},  // lea rax, [rip + 0x430303d]
    {0xA2181C, 3, 7, true, 0x0000},  // lea rax, [rip + 0x4302ffd]
    {0xA21865, 3, 7, true, 0x0000},  // lea rax, [rip + 0x4302fb4]
    {0xA218BC, 3, 7, true, 0x0000},  // lea rax, [rip + 0x4302f5d]
    {0xA21A0B, 3, 7, true, 0x0000},  // lea rax, [rip + 0x4302e0e]
    {0xA21B09, 3, 7, true, 0x0000},  // lea r11, [rip + 0x4302d10]
    {0xA21B10, 3, 7, true, 0x0048},  // lea rdx, [rip + 0x4302d51]
    {0xA21B69, 3, 7, true, 0x0044},  // lea rsi, [rip + 0x4302cf4]
    {0x2D2CCE5, 3, 7, true, 0x0040}, // lea rax, [rip + 0x1f7ea04]
};
constexpr entcoll_site tnotify_head_sites[] = {
    {0xA219F1, 4, 8, false,
     0x0000}, // mov qword ptr [r12 + rdi*8 + 0x4d286c0], rbx
    {0xA21B22, 4, 8, false,
     0x0000}, // mov qword ptr [r15 + rsi*8 + 0x4d286c0], r10
    {0xA21CC4, 4, 8, false,
     0x0000}, // mov r15, qword ptr [r9 + rbx*8 + 0x4d286c0]
    {0xA21CDB, 4, 12, false,
     0x0000}, // mov qword ptr [r9 + rbx*8 + 0x4d286c0], 0
    {0xA21FA2, 4, 8, false,
     0x0000}, // mov rax, qword ptr [r12 + rdi*8 + 0x4d286c0]
    {0xA21FBB, 4, 8, false,
     0x0000}, // mov qword ptr [r12 + rdi*8 + 0x4d286c0], rbx
};
constexpr entcoll_site tnotify_tail_sites[] = {
    {0xA219DE, 4, 8, false,
     0x0000}, // mov rax, qword ptr [r12 + rdi*8 + 0x4d286d0]
    {0xA219F9, 4, 8, false,
     0x0000}, // mov qword ptr [r12 + rdi*8 + 0x4d286d0], rbx
    {0xA21B31, 4, 8, false,
     0x0000}, // mov qword ptr [r15 + rsi*8 + 0x4d286d0], r10
    {0xA21CE7, 4, 12, false,
     0x0000}, // mov qword ptr [r9 + rbx*8 + 0x4d286d0], 0
    {0xA21FB2, 4, 9, false,
     0x0000}, // cmp qword ptr [r12 + rdi*8 + 0x4d286d0], 0
    {0xA21FC5, 4, 8, false,
     0x0000}, // mov qword ptr [r12 + rdi*8 + 0x4d286d0], r13
};
constexpr entcoll_site tnotify_free_sites[] = {
    {0xA21915, 4, 9, false,
     0x0000}, // cmp qword ptr [r12 + rdi*8 + 0x4d286e0], 0
    {0xA219A9, 4, 8, false,
     0x0000}, // mov rbx, qword ptr [r12 + rdi*8 + 0x4d286e0]
    {0xA219D2, 4, 8, false,
     0x0000}, // mov qword ptr [r12 + rdi*8 + 0x4d286e0], rax
    {0xA21B5E, 4, 8, false,
     0x0000}, // mov qword ptr [r15 + rsi*8 + 0x4d286e0], rax
    {0xA21EEE, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rcx + rax*8 + 0x4d286e0]
    {0xA21F02, 4, 8, false,
     0x0000}, // mov qword ptr [r12 + rdi*8 + 0x4d286e0], rsi
};
constexpr perclient_array batch6[] = {
    {"tnotify_list",
     0x04CA5820,
     0x1F40,
     tnotify_list_sites,
     std::size(tnotify_list_sites),
     0,
     {}},
    {"tnotify_head",
     0x04CA96C0,
     0x8,
     tnotify_head_sites,
     std::size(tnotify_head_sites),
     0,
     {}},
    {"tnotify_tail",
     0x04CA96D0,
     0x8,
     tnotify_tail_sites,
     std::size(tnotify_tail_sites),
     0,
     {}},
    {"tnotify_free",
     0x04CA96E0,
     0x8,
     tnotify_free_sites,
     std::size(tnotify_free_sites),
     0,
     {}},
};
size_t batch6_new[std::size(batch6)] = {};

// The engine's static initializer 0x02DA5E50 sets up all 200 items (two
// clients) once at startup:  mov ecx,0xC7 / lea rax,[list+0x40] / loop:
// [rax-0x40]=0, [rax-0x3C]=0x3FF, [rax]=0, [rax+8]=0, [rax+4]=0, rax+=0x50.
// Its lea is in the table, so it initialises slots 0/1 of the new block; the
// new slots 2/3 get exactly the same values here (everything zero except
// +0x04 = 0x3FF). The bytes are verified first - on any other build, the
// list is not moved at all.
constexpr uint8_t tnotify_static_init[] = {
    0xB9, 0xC7, 0x00, 0x00, 0x00, // mov ecx, 0xC7
    0x48, 0x8D, 0x05,             // lea rax, [rip+..]
};
constexpr uint8_t tnotify_static_init_body[] = {
    0x89, 0x50, 0xC0,                         // mov [rax-0x40], edx
    0xC7, 0x40, 0xC4, 0xFF, 0x03, 0x00, 0x00, // mov dword [rax-0x3C], 0x3FF
};

void relocate_batch6() {
  const auto b = base();
  const auto bytes_at = [b](const uint32_t rva, const uint8_t *expect,
                            const size_t n) {
    const auto *p = reinterpret_cast<const void *>(b + rva);
    return readable(p, n) && std::memcmp(p, expect, n) == 0;
  };
  if (!batch6_new[0]) {
    if (!bytes_at(0x02D2D3A0, tnotify_static_init,
                  sizeof(tnotify_static_init)) ||
        !bytes_at(0x02D2D3B2, tnotify_static_init_body,
                  sizeof(tnotify_static_init_body))) {
      note("[splitscreen] tnotify: static initializer differs - queues not "
           "moved\n");
      return;
    }
    batch6_new[0] = relocate_perclient(batch6[0]);
    if (!batch6_new[0]) {
      return; // the pointer arrays only make sense with the list moved
    }
    auto *items = reinterpret_cast<uint8_t *>(batch6_new[0]);
    for (size_t lc = 2; lc < 4; ++lc) {
      for (size_t i = 0; i < 100; ++i) {
        *reinterpret_cast<uint32_t *>(items + lc * 0x1F40 + i * 0x50 + 0x04) =
            0x3FF;
      }
    }
  }
  for (size_t i = 1; i < std::size(batch6); ++i) {
    if (!batch6_new[i]) {
      batch6_new[i] = relocate_perclient(batch6[i]);
    }
  }
}

// ---- BATCH 7: THE 190 MB SLIDE (2026-09-26) ---------------------------
//
// Every 3-player round died 13-26 s in - Arxan on an import table shifted
// by one slot, "Cannot find AI Type for 'default_aitype'", "Data is
// corrupt". tools/dump_shift_map.py on the 12:16 WER dump: the image is
// intact up to 0x0F50BC78 and from 0x0F50BC80 to the end of .idata EVERY
// byte holds what used to be 8 bytes higher. One slide, ~190 MB.
//
// 0x0F50BC80 = element [2] of the [2] x 0x240 array at 0x0F50B800
// (renderer; indexed by [ctx+0x398] at 0x01CC9946, built for two by the
// static initializer 0x02F033A0). An element is 16 entries of {int id,
// int age} at +0x00, a qword count at +0x80, an id bitmask at +0xC0 and a
// flag at +0x238. 0x01CCEB20 drops stale entries by hand:
//     r9 = &e[count-1];  do { [rax] = [rax+8]; rax += 8 } while (rax != r9)
// With index 2 the element is foreign memory and the count is whatever
// sits at 0x0F50BD00 - so the loop slid everything above 0x0F50BC80 down
// by 8 until it hit Arxan's read-only code. That one instance explains
// all three symptoms, with and without the third pane.
//
// Two references only (gen_reloc_sites, identical to the .pdata decode;
// sentinel_check clean). endmarker_scan's six hits (+0x480/+0x4E0) are
// loops over the renderer buffer records that START at 0x0F50BC80 - the
// foreign owner's own table, which stays where it is. The initializer
// only writes zeros, so zero-filled slots 2/3 are its exact initial state.
constexpr entcoll_site fxgpu_client_sites[] = {
    {0x1CBD588, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd841ea1]
    {0x2E8A244, 3, 7, true, 0x00C0}, // lea rbx, [rip + 0xc608505]
};
constexpr perclient_array batch7[] = {
    {"view_idsets_240",
     0x0F48C880,
     0x240,
     fxgpu_client_sites,
     std::size(fxgpu_client_sites),
     0,
     {}},
};
size_t batch7_new[std::size(batch7)] = {};

void relocate_batch7() {
  for (size_t i = 0; i < std::size(batch7); ++i) {
    if (!batch7_new[i]) {
      batch7_new[i] = relocate_perclient(batch7[i]);
    }
  }
}

// ---- BATCH 8: the renderer scene-buffer overrun (2026-09-26 12:31) ----
//
// With batch 7 in, the 3-player round connected all three players and
// then died in several renderer workers at once: 0x01C90464 and
// 0x01C93EEA, both `cmp byte [idx + entVisData[lc]]` with the pointer =
// 0xBF4684D4, a FLOAT. entVisData is PS4 GfxSceneDpvs (+0x08 byte*[4],
// indexed by localClientNum; dpvs itself = PC 0x0AE92D40, its fields
// localClientNum / entVisData[4] / sceneXModelIndex / sceneDObjIndex /
// entInfo[5] up to 0x0AE92DA0).
//
// The overwriter sits right in front of it: a per-client array at
// 0x0AE92420, 4 entries of 0x120 per client ((lc*4+i)*0x120; accessors
// 0x01C91440 / 0x01C91470 / 0x01C91C70, copier 0x01C93FF1 indexed by the
// client global 0x10615A28), sized for two: 0x0AE92420..0x0AE92D20. Client
// 2's slot is 0x0AE92D20..0x0AE931A0 - straight over dpvs. 20 sites:
// 13 identical to the .pdata decode, 7 leaf accessors read by hand; one
// generator row (0x021F8758 `sbb r10,[rax+0xae924b0]`) was a coincidence
// inside a .pdata function and is dropped. No end markers, no sentinels.
//
// C (0x10615A60, the per-client pointer table R_InitSceneBuffers fills,
// PS4 dpvsGlob) moves too: its slot 2 is the base of another array, and
// fill_scene_buffers used to write client 2/3's pointers into it. 4 ABS
// sites (3 identical, 1 leaf read by hand), no end markers. The allocator's
// store 0x01C9382D is among them, so C[0]/C[1] land in the new block.
constexpr entcoll_site scene_pc480_sites[] = {
    {0x1C84F2F, 3, 7, true, 0x001C}, // lea rax, [rip + 0x9201136]
    {0x1C85077, 3, 7, true, 0x001C}, // lea rax, [rip + 0x9200fee]
    {0x1C850AE, 3, 7, true, 0x001C}, // lea r8, [rip + 0x9200fb7]
    {0x1C854D8, 3, 7, true, 0x001C}, // lea rax, [rip + 0x9200b8d]
    {0x1C855E7, 3, 7, true, 0x0138}, // lea rdi, [rip + 0x9200b9a]
    {0x1C856A2, 3, 7, true, 0x001C}, // lea rax, [rip + 0x92009c3]
    {0x1C858A7, 3, 7, true, 0x001C}, // lea rax, [rip + 0x92007be]
    {0x1C8593E, 3, 7, true, 0x001C}, // lea rax, [rip + 0x9200727]
    {0x1C87AF0, 4, 8, false,
     0x001C}, // mov ecx, dword ptr [r8 + rbx + 0xae9243c]
    {0x1C87B9A, 4, 8, false,
     0x001C}, // mov ecx, dword ptr [r8 + rbx + 0xae9243c]
    {0x1C87C2E, 3, 7, true, 0x0020}, // lea rax, [rip + 0x91fe43b]
    {0x1C87C6A, 3, 7, true, 0x0034}, // lea rax, [rip + 0x91fe413]
    {0x1C87CAA, 3, 7, true, 0x0048}, // lea rax, [rip + 0x91fe3e7]
    {0x1C87CEA, 3, 7, true, 0x005C}, // lea rax, [rip + 0x91fe3bb]
    {0x1C87D80, 3, 7, true, 0x001C}, // lea rax, [rip + 0x91fe2e5]
    {0x1C8A9D8, 3, 7, true, 0x001C}, // lea rax, [rip + 0x91fb68d]
    {0x1C8AF48, 3, 7, true, 0x001C}, // lea rax, [rip + 0x91fb11d]
    {0x1C8B094, 3, 7, true, 0x001C}, // lea rax, [rip + 0x91fafd1]
    {0x1CE1DA1, 3, 7, true, 0x037C}, // lea rax, [rip + 0x91a4624]
    {0x1D0DB6F, 3, 7, true, 0x001C}, // lea rax, [rip + 0x91784f6]
};
constexpr entcoll_site scene_c_sites[] = {
    {0x1C85A0C, 4, 8, false,
     0x0000}, // mov rax, qword ptr [r9 + r10 + 0x10615a60]
    {0x1C85A4E, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r8 + rcx*8 + 0x10615a60]
    {0x1C8745D, 4, 8, false,
     0x0000}, // mov qword ptr [rbx + rsi + 0x10615a60], rax
    {0x1C8752F, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rbx + rdi + 0x10615a60]
};
constexpr perclient_array batch8[] = {
    {"scene_pc480",
     0x0AE134A0,
     0x480,
     scene_pc480_sites,
     std::size(scene_pc480_sites),
     0,
     {}},
    {"scene_c",
     0x10596AE0,
     0x8,
     scene_c_sites,
     std::size(scene_c_sites),
     0,
     {}},
};
size_t batch8_new[std::size(batch8)] = {};

void relocate_batch8() {
  for (size_t i = 0; i < std::size(batch8); ++i) {
    if (!batch8_new[i]) {
      batch8_new[i] = relocate_perclient(batch8[i]);
    }
  }
  scene_c_new = batch8_new[1];
}

// ---- BATCH 9: the 3-FPS freeze (2026-09-26 13:01) ---------------------
//
// First round ever with THREE panes drawing (batch 8 in) - and it ran at
// 3 FPS. The main thread sat in the frame limiter 0x01CF3F20 (`while
// (Sys_Milliseconds() < [0x0F4E5418]) Sys_Sleep(1)`), whose target is
// set only by 0x01CED4F0 from Com_Frame's 3-frame pacing ring
// (0x16CEEF30 = 8, 8, 8 ms - correct). Live, the target read 1128046900
// = 0x433C4E34, i.e. the FLOAT 188.3: someone else's float on an int.
//
// perclient_sweep lead: 0x0F4E3F4C x 0xA24 [2] (renderer, per client;
// accessor 0x01CA8C20 `lc * 0xA24`), client 2's slot 0x0F4E5394..
// 0x0F4E5DB8 covers the limiter target (+0x84) and the globals around
// it. 24 sites: gen_reloc_sites (23) + 0x01CA8559, which the .pdata
// decode found and the generator missed; the table-only rows are one
// leaf copying element 0's fields (0x01CA8800..) and the accessor. No end
// markers, no suspect sentinels. The static initializer 0x02F03210 builds
// each element as 6 x 0x1B0 through 0x01CB8740, which only writes
// [+0x1AC] = 0 - so zero-filled slots 2/3 ARE the constructed state.
constexpr entcoll_site rview_a24_sites[] = {
    {0x1C9C189, 3, 7, true, 0x0000}, // lea rcx, [rip + 0xd83b9ec]
    {0x1C9C3C6, 3, 7, true, 0x0A20}, // lea rdi, [rip + 0xd83c1cf]
    {0x1C9C3CD, 3, 7, true, 0x0000}, // lea rbp, [rip + 0xd83b7a8]
    {0x1C9C440, 4, 8, true, 0x01C0}, // movss xmm0, dword ptr [rip + 0xd83b8f4]
    {0x1C9C457, 4, 8, true, 0x01C4}, // divss xmm0, dword ptr [rip + 0xd83b8e1]
    {0x1C9C464, 4, 8, true, 0x01C8}, // movss xmm0, dword ptr [rip + 0xd83b8d8]
    {0x1C9C471, 4, 8, true, 0x01CC}, // divss xmm1, dword ptr [rip + 0xd83b8cf]
    {0x1C9C47E, 4, 8, true, 0x01B0}, // movss xmm0, dword ptr [rip + 0xd83b8a6]
    {0x1C9C48B, 4, 8, true, 0x01B4}, // movss xmm1, dword ptr [rip + 0xd83b89d]
    {0x1C9C498, 4, 8, true, 0x01B8}, // movss xmm0, dword ptr [rip + 0xd83b894]
    {0x1C9C4A5, 4, 8, true, 0x01BC}, // movss xmm1, dword ptr [rip + 0xd83b88b]
    {0x1C9C4B2, 4, 8, true, 0x01D8}, // movss xmm0, dword ptr [rip + 0xd83b89a]
    {0x1C9C4BF, 4, 8, true, 0x01DC}, // movss xmm1, dword ptr [rip + 0xd83b891]
    {0x1C9C4CC, 4, 8, true, 0x01E0}, // movss xmm0, dword ptr [rip + 0xd83b888]
    {0x1C9C4D9, 4, 8, true, 0x01E8}, // movss xmm1, dword ptr [rip + 0xd83b883]
    {0x1C9C4E6, 4, 8, true, 0x01EC}, // movss xmm0, dword ptr [rip + 0xd83b87a]
    {0x1C9C4F3, 4, 8, true, 0x01F0}, // movss xmm1, dword ptr [rip + 0xd83b871]
    {0x1C9C500, 4, 8, true, 0x01F4}, // movss xmm0, dword ptr [rip + 0xd83b868]
    {0x1C9C50D, 4, 8, true, 0x01F8}, // movss xmm1, dword ptr [rip + 0xd83b85f]
    {0x1C9C51A, 4, 8, true, 0x01E4}, // movss xmm0, dword ptr [rip + 0xd83b83e]
    {0x1C9C85F, 3, 7, true, 0x0000}, // lea rcx, [rip + 0xd83b316]
    {0x1C9CA22, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd83b153]
    {0x1C9CB0F, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd83b066]
    {0x2E8A0AA, 3, 7, true, 0x0000}, // lea rbx, [rip + 0xc5e0d2b]
};
constexpr perclient_array batch9[] = {
    {"rview_a24",
     0x0F464FCC,
     0xA24,
     rview_a24_sites,
     std::size(rview_a24_sites),
     0,
     {}},
};
size_t batch9_new[std::size(batch9)] = {};

void relocate_batch9() {
  for (size_t i = 0; i < std::size(batch9); ++i) {
    if (!batch9_new[i]) {
      batch9_new[i] = relocate_perclient(batch9[i]);
    }
  }
}

// ---- BATCH 10: the frame-limiter stall (2026-09-26 14:43) -------------
//
// Batch 9 fixed nothing here: the limiter target [0x0F4E5418] still read
// a float (0x433B9746 / 0x433BA01E, ~187.6) in every three-player round,
// and tools/sample_stacks.py put the main thread inside the limiter
// 0x01CF3F20 in 15/15 samples - `while (Sys_Milliseconds() <
// [0x0F4E5418]) Sys_Sleep(1)` waiting ~13 days. No main-thread frames
// means no LUI tick (every RootData.lastSystemUpdateTime froze the moment
// the third view went active), no first_snapshot processing, frozen
// players: pane 1 on "Awaiting textures", pane 2 without HUD.
//
// The relocated rview block holds zeros at those offsets, so the data is
// not rview-shaped. The writer is the renderer's per-view array right
// behind it: base 0x0F4E53B0, stride 0x30 (every accessor computes
// `lea r,[i+i*2]; shl r,4`, which is why perclient_sweep never listed
// it), indexed by the view's local client number ([viewInfo+0x1a680c] at
// 0x01CE97AD, which hands &elem[lc] to 0x01CF1600 to fill; a vec3 sits
// at +0). Element 2 = 0x0F4E5410..0x0F4E543F - over the qword pointer
// global at 0x0F4E5410 (0x01C94142 writes it, 0x01C815F6 reads it) and
// the limiter target 0x0F4E5418 = element 2's z. PS4 has no top-level
// [4] global of 0x30-byte renderer elements (dwarf_localclient.txt), so
// on console it is a member of a larger renderer struct; the PC evidence
// (index = view local client, element 2 over two foreign globals) is
// the same shape as batches 1-9.
//
// 13 sites, all `lea base` (gen_reloc_sites v2); pdata_xcheck identical
// (13/13), 0 end markers, 0 suspect sentinels. No static initializer
// references the array (all sites 0x01C8..0x01CE), so zero-filled slots
// 2/3 are the constructed state.
constexpr entcoll_site rview_org30_sites[] = {
    {0x1C73DFD, 3, 7, true, 0x0000}, // lea rcx, [rip + 0xd8651dc]
    {0x1C747DB, 3, 7, true, 0x0000}, // lea rcx, [rip + 0xd8647fe]
    {0x1C74EDC, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd8640fd]
    {0x1C75012, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd863fc7]
    {0x1C7AB76, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd85e463]
    {0x1C7AFB0, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd85e029]
    {0x1C7C8CC, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd85c70d]
    {0x1C7D057, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd85bf82]
    {0x1C7E2E4, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd85acf5]
    {0x1C7E4E5, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd85aaf4]
    {0x1C7E8BA, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd85a71f]
    {0x1CB2BC5, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd826414]
    {0x1CDD3EC, 3, 7, true, 0x0000}, // lea rax, [rip + 0xd7fbbed]
};
constexpr perclient_array batch10[] = {
    {"rview_org30",
     0x0F466430,
     0x30,
     rview_org30_sites,
     std::size(rview_org30_sites),
     0,
     {}},
};
size_t batch10_new[std::size(batch10)] = {};

void relocate_batch10() {
  for (size_t i = 0; i < std::size(batch10); ++i) {
    if (!batch10_new[i]) {
      batch10_new[i] = relocate_perclient(batch10[i]);
    }
  }
}

// ---- BATCH 11: the aim-target actor lists (2026-09-26 15:41) ----------
//
// With the client-script bound widened, the three-player round RAN for
// the first time (player 2's HUD, name tags, player 3's weapon) and
// then crashed: 0xC0000005 at RVA 0x00084E88 on a worker thread,
// `movzx edx, byte [rdi+2]` with rdi = 0x2C77DC43620 (unmapped), rsi = 2.
// The worker job is the `aim_target` pass (job fn 0x899C0 - chained
// unwind parent of 0x899E8 - fetches its data by the name "aim_target",
// 0x02F93D88) and calls 0x83620(lc, cent) for every pointer in
// AimTarget_Cmd[lc].list. Its submitter 0x7A970 builds that list as
// `0x0367F380 + lc*0x200` (64 centity pointers per client) - PS4's
// aim_target_actors, a [4] global there (dwarf_localclient.txt). For
// lc 2 that is 0x0367F780: AimTarget_Cmd's OLD home (batch 1b moved the
// command away), so player 3's pass read stale bytes as entities.
//
// 2 sites (gen_reloc_sites v2): the submitter's lea and the filler's
// ABS store at 0x000897A8 (`[base + (lc*64+i)*8 + 0x367F380]`).
// pdata_xcheck identical, 0 end markers, 0 suspect sentinels. No
// constructor - zero-filled slots are "no actors".
constexpr entcoll_site aimactors_sites[] = {
    {0x7A998, 3, 7, true, 0x0000}, // lea rax, [rip + 0x36049e1]
    {0x897A8, 4, 8, false,
     0x0000}, // mov qword ptr [rax + rcx*8 + 0x367f380], r8
};
constexpr perclient_array batch11[] = {
    {"aimactors",
     0x03600380,
     0x200,
     aimactors_sites,
     std::size(aimactors_sites),
     0,
     {}},
};
size_t batch11_new[std::size(batch11)] = {};

void relocate_batch11() {
  for (size_t i = 0; i < std::size(batch11); ++i) {
    if (!batch11_new[i]) {
      batch11_new[i] = relocate_perclient(batch11[i]);
    }
  }
}

// ---- UI TRACE (diagnostic) ------------------------------------------------
//
// Pane 1 stuck on "Awaiting textures", pane 2 without HUD, players frozen
// (2026-09-26, every three-player round). This trace was built on a WRONG
// first reading, kept here so it is not repeated: s_rootData+0x84 is not an
// element handle but RootData.lastSystemUpdateTime (PS4 DWARF RootData:
// +0x80 viewParmIndex, +0x84 lastSystemUpdateTime, +0x88
// lastGameUpdateTime, +0x8C name, +0xAC inUse). The roots were never
// rebuilt - they stopped UPDATING. The first build of this trace also
// restarted HUDs after a "late UI_CoD_Init"; the trace proved no such init
// happens (only the load-time one, clients at state 6), so that recovery
// was removed again.
//
// The real cause (batch 10 below): the main thread sits in the frame
// limiter 0x01CF3F20 in 15/15 stack samples (tools/sample_stacks.py),
// waiting for Sys_Milliseconds() to reach [0x0F4E5418] = 0x433BA01E - a
// float written there once, when the third view went active. No frames,
// so no LUI tick, no first_snapshot processing, frozen players.
//
// What stays: both call sites of UI_CoD_Init (PC 0x01F29010,
// BOIII-detoured - calling the original ADDRESS keeps that chain) and the
// three first_snapshot LUIScopedEvent sites append one line each to
// %LOCALAPPDATA%\boiii\splitscreen_ui_trace.txt (tick, call site, client
// states, root, stack). Cold paths only, plain Win32 file calls, no CRT
// (see note()). CG_LUIHUDRestart (PC 0x00F7E970 = PS4 0x29AD90) is
// verified and documented here in case a real HUD restart is ever needed.
constexpr uint32_t ui_cod_init_rva = 0x1F1C890;
constexpr uint32_t ui_cod_init_callsites[] = {
    0x01F2651F, // UI_CoD_RunFrame        (if !UI_IsInitialized)
    0x01F26F48, // UI_CoD_ShutdownAndInit (CL_InitUI,
                // Com_InitUIAndCommonXAssets, devmap)
};
constexpr uint32_t lui_scoped_event_rva = 0x2685620;
constexpr uint32_t first_snapshot_event_callsites[] = {
    0x00F7E9F6, // CG_LUIHUDRestart
    0x01321058, // CL_FirstSnapshot
    0x013CFB09, // SCR_DrawScreenField (CL_CheckKeepDrawingConnectScreen,
                // inlined)
};
constexpr uint32_t cg_lui_hud_restart_rva = 0xF7E970;
constexpr uint8_t cg_lui_hud_restart_prologue[] = {
    0x48, 0x8B, 0xC4,                         // mov rax, rsp
    0x57,                                     // push rdi
    0x48, 0x81, 0xEC, 0xA0, 0x00, 0x00, 0x00, // sub rsp, 0xA0
};
constexpr uint32_t server_initial_players_connected_rva = 0xA1C272A;
constexpr uint32_t connection_state_rva =
    0x5359BC8; // clientUIActives[lc]+8, stride 0x1078
bool ui_trace_installed = false;

struct trace_line {
  char b[1536];
  size_t n = 0;

  void str(const char *s) {
    while (s && *s && n < sizeof(b) - 3) {
      b[n++] = *s++;
    }
  }

  void hex(uint64_t v) {
    char t[16];
    int k = 0;
    do {
      t[k++] = "0123456789ABCDEF"[v & 0xF];
      v >>= 4;
    } while (v && k < 16);
    str("0x");
    while (k > 0 && n < sizeof(b) - 3) {
      b[n++] = t[--k];
    }
  }

  void dec(uint64_t v) {
    char t[20];
    int k = 0;
    do {
      t[k++] = static_cast<char>('0' + v % 10);
      v /= 10;
    } while (v && k < 20);
    while (k > 0 && n < sizeof(b) - 3) {
      b[n++] = t[--k];
    }
  }
};

void trace_write(trace_line &l) {
  l.b[l.n++] = '\r';
  l.b[l.n++] = '\n';
  char path[MAX_PATH]{};
  const auto len = GetEnvironmentVariableA("LOCALAPPDATA", path, MAX_PATH);
  constexpr char leaf[] = "\\boiii\\splitscreen_ui_trace.txt";
  if (len == 0 || len + sizeof(leaf) > MAX_PATH) {
    return;
  }
  std::memcpy(path + len, leaf, sizeof(leaf));
  const auto file =
      CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                  nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    return;
  }
  DWORD written{};
  WriteFile(file, l.b, static_cast<DWORD>(l.n), &written, nullptr);
  CloseHandle(file);
}

void trace_text(const char *text) {
  trace_line l;
  l.str(text);
  trace_write(l);
}

void log_gamepad_slots(const char *what) {
  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" ");
  l.str(what);
  l.str(" slot device/connected:");
  for (size_t slot = 0; slot < 4; ++slot) {
    l.str(" ");
    l.dec(
        static_cast<uint64_t>(static_cast<uint32_t>(gamepad_device_of(slot))));
    l.str(gamepad_connected(slot) ? "/1" : "/0");
  }
  trace_write(l);
}

uint32_t connection_state(const int lc) {
  // [0]/[1] stock; [2] is the voice_comm-vacated block the component
  // already uses for clientUIActives[2] (run_cl_init_for_local_client2).
  return *reinterpret_cast<const volatile uint32_t *>(
      base() + connection_state_rva + static_cast<size_t>(lc) * 0x1078);
}

void trace_head(trace_line &l, const char *what, const size_t call_site) {
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" ");
  l.str(what);
  l.str(" site=");
  l.hex(call_site);
  l.str(" cl_max=");
  l.dec(*reinterpret_cast<const volatile uint32_t *>(base() +
                                                     cl_max_local_clients_rva));
  l.str(" st=");
  for (int lc = 0; lc < 4; ++lc) // lc 3: owned head of clientUIActives[3]
  {
    l.dec(connection_state(lc));
    l.str(lc < 3 ? "," : "");
  }
  l.str(" svIPC=");
  l.dec(*reinterpret_cast<const volatile uint8_t *>(
      base() + server_initial_players_connected_rva));
  l.str(" uiLevel=");
  l.dec(game::com::Com_IsRunningUILevel() ? 1 : 0);
}

void trace_stack(trace_line &l) {
  void *frames[20]{};
  const auto count = RtlCaptureStackBackTrace(1, 20, frames, nullptr);
  const auto b = base();
  l.str(" stack:");
  for (USHORT i = 0; i < count; ++i) {
    const auto a = reinterpret_cast<size_t>(frames[i]);
    l.str(" ");
    if (a >= b && a < b + 0x20000000) {
      l.hex(a - b);
    } else {
      l.str("x");
      l.hex(a);
    }
  }
}

size_t call_site_of(void *return_address) {
  return reinterpret_cast<size_t>(return_address) - base() - 5;
}

void *lui_event_trace_thunk(void *self, void *lua, const char *root,
                            const char *event) {
  trace_line l;
  trace_head(l, "LUIEvent", call_site_of(_ReturnAddress()));
  l.str(" root=");
  l.str(root);
  l.str(" event=");
  l.str(event);
  trace_write(l);
  return reinterpret_cast<void *(*)(void *, void *, const char *,
                                    const char *)>(
      base() + lui_scoped_event_rva)(self, lua, root, event);
}

void ui_cod_init_trace_thunk(const bool frontend) {
  const auto site = call_site_of(_ReturnAddress());
  {
    trace_line l;
    trace_head(l, "UI_CoD_Init", site);
    l.str(" frontend=");
    l.dec(frontend ? 1 : 0);
    trace_stack(l);
    trace_write(l);
  }

  reinterpret_cast<void (*)(bool)>(base() + ui_cod_init_rva)(frontend);
}

bool call_site_targets(const uint32_t site, const uint32_t target) {
  const auto *p = reinterpret_cast<const uint8_t *>(base() + site);
  if (!readable(p, 5) || p[0] != 0xE8) {
    return false;
  }
  int32_t rel{};
  std::memcpy(&rel, p + 1, sizeof(rel));
  return site + 5 + static_cast<int64_t>(rel) == target;
}

void install_ui_trace() {
  if (ui_trace_installed) {
    return;
  }
  const auto b = base();
  if (std::memcmp(reinterpret_cast<const void *>(b + cg_lui_hud_restart_rva),
                  cg_lui_hud_restart_prologue,
                  sizeof(cg_lui_hud_restart_prologue)) != 0) {
    return;
  }
  for (const auto site : ui_cod_init_callsites) {
    if (!call_site_targets(site, ui_cod_init_rva)) {
      return;
    }
  }
  for (const auto site : first_snapshot_event_callsites) {
    if (!call_site_targets(site, lui_scoped_event_rva)) {
      return;
    }
  }
  try {
    for (const auto site : first_snapshot_event_callsites) {
      utils::hook::call(b + site, lui_event_trace_thunk);
    }
    for (const auto site : ui_cod_init_callsites) {
      utils::hook::call(b + site, ui_cod_init_trace_thunk);
    }
  } catch (...) {
    return;
  }
  ui_trace_installed = true;
  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" ---- ui trace installed ----");
  trace_write(l);
}

// ---- GUESTS START FROM PLAYER 1'S CLASSES AND STATS (2026-09-27) ------
//
// Wanted: every added controller starts from a copy of player 1's data and
// can then edit it. The console does exactly this for a guest: PS4
// Live_HandleClientSplitscreenSignin (0xC16080) calls, for a guest
// without stats, LiveStats_CopyFromSponsor (0xC6A6B0 ->
// LiveStorage_CopyStatsBuffer(ctrl, sponsor) + DisableStatsUpload), so a
// guest plays with the sponsor's level, unlocks and classes and keeps
// nothing. Live_SignedInGuestUser (0xC2E7A0) calls it too.
//
// On the PC every controller has its own save set, read from disk by ONE
// function (all measured 2026-09-27, not taken from older notes):
//   0x02274B9E  call 0x01C20880 - the only caller; a storage task with
//               [task] = controller, [task+0x208] = count,
//               task+0x210 = descriptors (0x58 each). The write twin is
//               0x02274B79 -> 0x01C20990.
//   0x01C20880  for each descriptor: Com_sprintf("%s_%d.%s", name (inline
//               at +0), controller, "cgp"), dir "players" (BOIII makes it
//               "boiii_players", client_patches.cpp) unless byte +0x4C is
//               set, base = Dvar_GetString(0x022BF590) of [0x17A652E0],
//               then a synchronous read and close.
// So the copy is made one step before the game's own read: for a
// controller >= 1, player 1's `<name>_0.cgp` replaces `<name>_N.cgp` for
// the loadout and stats files, and the game then reads a real file through
// its own path - no storage record or buffer is touched. The guest's edits
// are saved to his own slot by the game as usual and are replaced by the
// next copy, which is the console's "keeps nothing".
// BO3_GUEST_COPY=off disables it.
constexpr uint32_t save_read_callsite = 0x221806E;
constexpr uint32_t save_read_rva = 0x1C144B0;
constexpr size_t save_desc_stride = 0x58;
constexpr size_t save_desc_name_max = 0x40;
constexpr size_t save_desc_other_dir = 0x4C;
// 0x01C208EE is the branch on that byte. BOIII (client_patches.cpp
// patch_players_folder_name) turns it into `jmp` (0xEB), so every file
// goes to boiii_players whatever the byte says - measured live
// 2026-09-27: `eb 08`. The first build honoured the byte and matched
// nothing ("copied 0, missing 0" for all four controllers at boot).
constexpr uint32_t save_dir_branch_rva = 0x1C1451E;
constexpr uint32_t save_base_dvar_rva = 0x179E63E0;
constexpr uint32_t dvar_get_string_rva = 0x2262A70;
constexpr const char *sponsor_copy_names[] = {
    "loadouts_zm_offline",        "loadouts_mp_offline", "loadouts_cp_offline",
    "stats_zm_offline",           "stats_mp_offline",    "stats_cp_offline",
    "stats_cp_nightmare_offline", "stats_fr_offline",
};
bool guest_copy_installed = false;
constexpr uint32_t save_write_callsite =
    0x2218049; // call 0x1C20990, same task layout
constexpr uint32_t save_write_rva = 0x1C145C0;

// Trace only: which controller's files the game writes, and when.
void save_write_stub(const int controller, uint8_t *files, const int count) {
  reinterpret_cast<void (*)(int, uint8_t *, int)>(base() + save_write_rva)(
      controller, files, count);
  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" save write: controller ");
  l.dec(static_cast<uint64_t>(controller < 0 ? 0 : controller));
  l.str(",");
  for (int i = 0; files && i < count && i < 16; ++i) {
    const auto *desc = files + static_cast<size_t>(i) * save_desc_stride;
    if (strnlen(reinterpret_cast<const char *>(desc), save_desc_name_max) <
        save_desc_name_max) {
      l.str(" ");
      l.str(reinterpret_cast<const char *>(desc));
    }
  }
  trace_write(l);
}

bool is_sponsor_copy_name(const char *name) {
  for (const auto *n : sponsor_copy_names) {
    if (std::strcmp(name, n) == 0) {
      return true;
    }
  }
  return false;
}

void save_read_stub(const int controller, uint8_t *files, const int count) {
  size_t copied = 0;
  size_t missing = 0;
  size_t failed = 0;
  trace_line names;
  if (controller >= 1 && controller < 4 && files && count > 0) {
    const auto b = base();
    const bool dir_forced =
        *reinterpret_cast<const uint8_t *>(b + save_dir_branch_rva) == 0xEB;
    const auto *dvar = *reinterpret_cast<void *const *>(b + save_base_dvar_rva);
    const auto get_string = reinterpret_cast<const char *(*)(const void *)>(
        b + dvar_get_string_rva);
    const char *root = dvar ? get_string(dvar) : nullptr;
    for (int i = 0; root && *root && i < count; ++i) {
      const auto *desc = files + static_cast<size_t>(i) * save_desc_stride;
      if (strnlen(reinterpret_cast<const char *>(desc), save_desc_name_max) >=
          save_desc_name_max) {
        names.str(" ?");
        continue;
      }
      const auto *name = reinterpret_cast<const char *>(desc);
      names.str(" ");
      names.str(name);
      if (!dir_forced && desc[save_desc_other_dir] != 0) {
        names.str("(other dir)");
        continue;
      }
      if (!is_sponsor_copy_name(name)) {
        continue;
      }
      names.str("*");
      char src[MAX_PATH]{};
      char dst[MAX_PATH]{};
      const auto ns = std::snprintf(src, sizeof(src),
                                    "%s\\boiii_players\\%s_0.cgp", root, name);
      const auto nd =
          std::snprintf(dst, sizeof(dst), "%s\\boiii_players\\%s_%d.cgp", root,
                        name, controller);
      if (ns <= 0 || nd <= 0 || ns >= MAX_PATH || nd >= MAX_PATH) {
        ++failed;
        continue;
      }
      if (GetFileAttributesA(src) == INVALID_FILE_ATTRIBUTES) {
        ++missing; // player 1 has no such file yet - leave the guest's alone
        continue;
      }
      if (CopyFileA(src, dst, FALSE)) {
        ++copied;
      } else {
        ++failed;
      }
    }
  }

  reinterpret_cast<void (*)(int, uint8_t *, int)>(base() + save_read_rva)(
      controller, files, count);

  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" save read: controller ");
  l.dec(static_cast<uint64_t>(controller < 0 ? 0 : controller));
  l.str(controller < 0 ? " (negative)" : "");
  l.str(", ");
  l.dec(static_cast<uint64_t>(count < 0 ? 0 : count));
  l.str(" files; from player 1: copied ");
  l.dec(copied);
  l.str(", player 1 missing ");
  l.dec(missing);
  l.str(", failed ");
  l.dec(failed);
  if (names.n) {
    l.str(" |");
    names.b[names.n] = 0;
    l.str(names.b);
  }
  trace_write(l);
}

void install_guest_copy() {
  trace_line l;
  char env[16] = {};
  GetEnvironmentVariableA("BO3_GUEST_COPY", env, sizeof(env));
  if (std::strcmp(env, "off") == 0) {
    l.str("guest copy: OFF (BO3_GUEST_COPY=off)");
    trace_write(l);
    return;
  }
  if (guest_copy_installed) {
    return;
  }
  if (!call_site_targets(save_read_callsite, save_read_rva)) {
    l.str("guest copy: NOT installed - 0x02274B9E is not call 0x01C20880");
    trace_write(l);
    return;
  }
  try {
    utils::hook::call(base() + save_read_callsite, save_read_stub);
    if (call_site_targets(save_write_callsite, save_write_rva)) {
      utils::hook::call(base() + save_write_callsite, save_write_stub);
    }
  } catch (...) {
    l.str("guest copy: NOT installed - hook failed");
    trace_write(l);
    return;
  }
  guest_copy_installed = true;
  l.str("guest copy: installed - controllers 1..3 read player 1's loadouts + "
        "stats");
  trace_write(l);
}

// The game reads all four controllers' saves ONCE, at boot - measured
// 2026-09-27 21:13: four "save read" batches before the title screen and
// none at any join. So a guest would get player 1's classes as they were
// at boot, and a guest who leaves and rejoins would keep his session's
// edits. The goal is a fresh copy every time a
// controller is added, so the join re-reads the eight files itself:
//   PS4 Storage_Read 0xF7E860 -> RequestOperation -> CanPerformFileOp
//   0xF80F80 (only: DDL buffer present, target active, no write to a
//   read-only file - nothing refuses a second READ) ->
//   StorageTarget_QueueFile.
//   PC Storage_Read 0x022775A0 (ecx controller, edx type, r8d slot ->
//   bool; finds props->type == type && slot among the 0x40 files),
//   entered at its own address so the component's guest read filter
//   (storage_read_stub) still applies.
// Type numbers read today from the PC property table (0x0343ADF0, 0x78
// each: +0 type, +8 name, +0x20 target; all eight target 0 = local):
//   11 loadouts_cp_offline  15 loadouts_mp_offline  20 loadouts_zm_offline
//    7 stats_cp_offline      9 stats_cp_nightmare_offline
//   13 stats_mp_offline     18 stats_zm_offline     22 stats_fr_offline
constexpr int sponsor_copy_types[] = {11, 15, 20, 7, 9, 13, 18, 22};

void reread_guest_saves(const int controller) {
  if (!guest_copy_installed || controller < 1 || controller > 3) {
    return;
  }
  const auto read =
      reinterpret_cast<bool (*)(int, int, int)>(base() + storage_read_rva);
  uint32_t queued = 0;
  for (size_t i = 0; i < std::size(sponsor_copy_types); ++i) {
    if (read(controller, sponsor_copy_types[i], 0)) {
      queued |= 1u << i;
    }
  }
  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" guest copy: controller ");
  l.dec(static_cast<uint64_t>(controller));
  l.str(" joined - re-read queued mask ");
  l.hex(queued);
  l.str(" of 0xFF (types 11 15 20 7 9 13 18 22)");
  trace_write(l);
}

// ---- CLIENT-SCRIPT LOCAL-CLIENT BOUND (2026-09-26) --------------------
//
// With batch 10 the three-player round reached the client-script VM and
// hung there: tools/script_hang.py named zm_zod_traps.csc
// update_chain_anims (a clientfield callback), and the script error
// buffer (PC 0x052E50F0) read "Trying to get a local client index for a
// client '2' that is not a local client." (format string 0x02FA4740).
// The error aborts the clientfield callback dispatcher 0x0013380D
// before client 2's pending queue is cleared (count[2] = 77, measured,
// never draining), so it re-dispatches the same callbacks forever: a
// main-thread hang, VM <-> longjmp (0x02C433A0), Com_Frame frozen.
//
// PS4 has the check as ONE function, CScr_GetLocalClientNum (0x1D926A0):
//   lc = Scr_GetInt(SCRIPTINSTANCE_CLIENT, arg)
//   if (lc < 0 || lc >= 4) Scr_Error(va(fmt, lc))
// The PC inlines it into every builtin with the bound 2:
//   call Scr_GetInt (0x012EB7F0, ecx=1) / cmp reg, 1 / jbe ok / lea fmt
// So the console accepts local clients 0..3 in every client-script
// builtin; this raises each inlined `cmp reg, 1` to 2 (three players).
// tools/gen_csc_lc_checks.py: 220 xrefs, 217 verified (compare decodes
// as 83 /7 ib with imm 1, jbe skips past the error or ja enters it);
// three sites compare against a register holding 1 (0x004259CF,
// 0x00A187C2, 0x00D8B0BF) and are left alone. Rule 4 audit (every
// enclosing function vs the FOREIGN sweep, relocated bases excluded):
// 0 functions touch an unrelocated FOREIGN array directly.
// All-or-nothing: every byte must read 0x01 before any is written.
constexpr uint32_t csc_lc_check_imms[] = {
    0x0028A308, 0x00293969, 0x00293A1A, 0x00293A7C, 0x002B41CB, 0x002DDF33,
    0x002E5B63, 0x002E7413, 0x002E8D03, 0x002EC2A6, 0x002EC2F3, 0x002EF4FA,
    0x002EF59C, 0x002F0E6A, 0x002F0EE3, 0x003284FA, 0x0032855A, 0x00368C1A,
    0x00368FE8, 0x003694DC, 0x0036C73C, 0x0036FB2A, 0x0036FBA1, 0x00372E1A,
    0x0037AAC4, 0x0037AB6A, 0x0039294A, 0x003929AA, 0x00392A0A, 0x00395CAF,
    0x00395D2A, 0x00395D91, 0x003AC01A, 0x003AC0DA, 0x003AC13A, 0x003B24BA,
    0x003B3E16, 0x003B3F6F, 0x003B592F, 0x003B8B8A, 0x003B8C16, 0x003B8FAF,
    0x003BA90A, 0x003BC21A, 0x003BC27A, 0x003BC308, 0x003BC463, 0x003D8293,
    0x003DE4EF, 0x003DE564, 0x003DE5AA, 0x003DE60A, 0x003DE66A, 0x003DE6CA,
    0x003DE75A, 0x003DE82F, 0x003DE8E4, 0x003DE9AF, 0x003DEA54, 0x003E984C,
    0x003ED234, 0x003EEDD9, 0x003EF00A, 0x003FF0D2, 0x00412A33, 0x004209C2,
    0x00422265, 0x00423CA5, 0x004255B4, 0x004257FA, 0x00425925, 0x00425A7D,
    0x00425B61, 0x004274EA, 0x004275D1, 0x00428FE4, 0x0042924C, 0x0042AAE4,
    0x0042AB66, 0x00435E84, 0x00A3630C, 0x00A3F43C, 0x00A8444B, 0x00A8DA73,
    0x00A9A624, 0x00AA0A94, 0x00AAD334, 0x00AB686A, 0x00AB9E71, 0x00ACCA84,
    0x00ACFBB4, 0x00B582E8, 0x00B583B9, 0x00B61869, 0x00B7A497, 0x00B7EF11,
    0x00B824E1, 0x00BC53D4, 0x00BC542C, 0x00BC6CDC, 0x00BE8296, 0x00BF095B,
    0x00C07E54, 0x00C224DC, 0x00C30536, 0x00C383D1, 0x00C4041F, 0x00C6728A,
    0x00C7BC62, 0x00C7EDE2, 0x00C82289, 0x00C8252A, 0x00C86DFE, 0x00C8D28C,
    0x00C8EB2C, 0x00C9FBB8, 0x00C9FCAC, 0x00CA5F19, 0x00CA909A, 0x00CBA12E,
    0x00CBA24D, 0x00CBA304, 0x00CBA3E4, 0x00CBA4B4, 0x00CBA58E, 0x00CBA6B4,
    0x00CBA76A, 0x00CBA84F, 0x00CBA8CF, 0x00CBA93F, 0x00CBA9E3, 0x00CBCC90,
    0x00CC1873, 0x00CC3123, 0x00CCADCB, 0x00CCB38C, 0x00CE56B4, 0x00CE5723,
    0x00CE57E4, 0x00CE5834, 0x00CE588C, 0x00CE715C, 0x00CF86AB, 0x00CFD1CB,
    0x00D254CA, 0x00D2E913, 0x00D301E1, 0x00D31AB3, 0x00D333B8, 0x00D46288,
    0x00D46318, 0x00D463A8, 0x00D46488, 0x00D46518, 0x00D4DF98, 0x00D4E07C,
    0x00D4F938, 0x00D4F9F8, 0x00D4FA78, 0x00D4FAEB, 0x00D5136D, 0x00D53289,
    0x00D57C31, 0x00D5C6A8, 0x00D5C714, 0x00D5C78B, 0x00D5E1EA, 0x00D6475B,
    0x00D647DC, 0x00D6C1CC, 0x00D6DA64, 0x00D73DBC, 0x00D97681, 0x00D9F424,
    0x00D9F484, 0x00D9F504, 0x00DF9DA4, 0x00DF9E0C, 0x00DFE8AC, 0x00E00144,
    0x00E001A4, 0x00E001F4, 0x00E0025C, 0x00E04C2F, 0x00E2183E, 0x00E2946F,
    0x00E2AEA1, 0x00E2C83A, 0x00E2E186, 0x00E2FB6E, 0x00E314CD, 0x00E32DFD,
    0x00E34711, 0x00E360AA, 0x00E36226, 0x00E363D7, 0x00E36469, 0x00E44376,
    0x00E45F85, 0x00E63981, 0x00E65289, 0x00E69CFC, 0x00E6B5DC, 0x00E6CECC,
    0x00E6E7BC, 0x00E93371, 0x00EA12F3, 0x00EE3D51, 0x00EE560A, 0x00F0175C,
    0x00F0C47E, 0x00F0DD86, 0x00F29D51, 0x00F2B7D0, 0x00F2B8D5, 0x00F41B44,
    0x00F5101B,
};
bool csc_lc_widened = false;

void widen_csc_lc_checks() {
  if (csc_lc_widened) {
    return;
  }
  const auto b = base();
  for (const auto rva : csc_lc_check_imms) {
    const auto *at = reinterpret_cast<const uint8_t *>(b + rva);
    if (!readable(at, 1) || *at != 0x01) {
      trace_line l;
      l.str("csc lc bound: 0x");
      l.hex(rva);
      l.str(" is not stock - nothing written");
      trace_write(l);
      return;
    }
  }
  // PLAYER 4 (2026-09-27): 0x03 = local clients 0..3, the console's own bound.
  // Rule 4 for slot 3: tools/audit_csc_slot3.py on a live code dump - the only
  // [2] arrays still in place with a referenced slot 3 are clientUIActives
  // +4 / +0x10 (the owned head of slot 3) and a jump-table false positive.
  const uint8_t three_clients = 0x03;
  for (const auto rva : csc_lc_check_imms) {
    write_bytes(reinterpret_cast<uint8_t *>(b + rva), &three_clients, 1);
  }
  csc_lc_widened = true;
  trace_line l;
  l.str("csc lc bound: ");
  l.dec(std::size(csc_lc_check_imms));
  l.str(" client-script local-client checks widened 1 -> 3");
  trace_write(l);
}

// ---- BATCH 12: the zombies HUD player list (2026-09-26 16:02) -----------
//
// With lens flares gated the round ran ~55 s (Com_Frame and all three
// LUI roots advancing), then crashed: 0xC0000005 in strcmp 0x022E93B0
// reading a garbage string pointer 0x12A00000008. Chain: the per-
// controller HUD update 0x02766960 -> PlayerList update 0x027451B0
// ('PlayerList', '%d.playerScore', '%d.zombiePlayerIcon') -> the UI
// model string setter 0x02019DD0 -> strcmp(old, new).
//
// The player list keeps six arrays per LOCAL CLIENT, all [2], packed
// back to back in .data, so every slot 2 lands on the next array:
//   0x1A8759D0  [lc][8] int   scores          stride 0x20
//   0x1A875A10  [lc][8] int   (second score)  stride 0x20
//   0x1A875A50  [lc][8] int   shown flags     stride 0x20
//   0x1A875A90  [lc][8] int   client ids (-1) stride 0x20
//   0x1A875AD0  [lc][8] char* icon names      stride 0x40
//   0x1A875B50  [lc]    int   own index       stride 4
// For lc 2 the icon row IS 0x1A875B50.. - ints read as string
// pointers, exactly the crash. Reset leaves 0x0273FEB0 / 0x0273FF20
// (no .pdata, read by hand) set icons to "blacktransparent" and ids to
// -1 per lc, so zero-filled slots 2/3 are fixed by the engine's own
// reset before use. gen_reloc_sites v2 for all six; pdata_xcheck
// table-only rows are exactly those two leaves; 0 end markers, 0
// suspect sentinels. The icon lea at 0x0273FEB3 was rejected by the
// generator's convergence test (leaf right after int3 padding) and is
// added by hand - pe_xref's exhaustive scan lists it.
constexpr entcoll_site hudpl_score_sites[] = {
    {0x26A4375, 4, 8, false,
     0x0000}, // cmp dword ptr [r14 + rsi + 0x1a8759d0], eax
    {0x26A4388, 4, 8, false,
     0x0000}, // mov dword ptr [r14 + rsi + 0x1a8759d0], eax
    {0x26A74F2, 3, 7, false,
     0x0000}, // mov dword ptr [rcx + rsi + 0x1a8759d0], eax
    {0x26C6E0B, 3, 7, false, 0x0000}, // lea rcx, [r10 + 0x1a8759d0]
    {0x26CC141, 4, 8, false,
     0x0000}, // mov edx, dword ptr [r14 + rax + 0x1a8759d0]
};
constexpr entcoll_site hudpl_gap_sites[] = {
    {0x26A4396, 4, 8, false,
     0x0000}, // mov dword ptr [r14 + rsi + 0x1a875a10], eax
};
constexpr entcoll_site hudpl_flags_sites[] = {
    {0x26A4349, 4, 8, false,
     0x0000}, // cmp dword ptr [r14 + rsi + 0x1a875a50], eax
    {0x26A4362, 4, 8, false,
     0x0000}, // mov dword ptr [r14 + rsi + 0x1a875a50], eax
    {0x26A74D7, 3, 8, false,
     0x0000}, // cmp dword ptr [rcx + rsi + 0x1a875a50], 1
    {0x26A74F9, 3, 11, false,
     0x0000}, // mov dword ptr [rcx + rsi + 0x1a875a50], 0
    {0x26C6DDC, 4, 8, false,
     0x0000}, // mov qword ptr [rax + r10 + 0x1a875a50], r9
    {0x26C6DE4, 4, 8, false,
     0x0008}, // mov qword ptr [rax + r10 + 0x1a875a58], r9
    {0x26C6DEC, 4, 8, false,
     0x0010}, // mov qword ptr [rax + r10 + 0x1a875a60], r9
    {0x26C6DFB, 4, 8, false,
     0x0018}, // mov qword ptr [rax + r10 + 0x1a875a68], r9
    {0x26CC0D5, 4, 9, false,
     0x0000}, // cmp dword ptr [r14 + rdx + 0x1a875a50], 0
};
constexpr entcoll_site hudpl_ids_sites[] = {
    {0x26A7434, 4, 8, false,
     0x0000}, // cmp ebx, dword ptr [r14 + rax + 0x1a875a90]
    {0x26A74A6, 4, 8, false,
     0x0000}, // mov dword ptr [r14 + rsi + 0x1a875a90], ebx
    {0x26C6D58, 3, 7, true, 0x0000},  // lea rcx, [rip + 0x18135bc1]
    {0x26C6DD2, 3, 7, false, 0x0000}, // lea rdx, [r10 + 0x1a875a90]
};
constexpr entcoll_site hudpl_icons_sites[] = {
    {0x26A43A2, 3, 8, false, 0x0000}, // cmp qword ptr [rsi + 0x1a875ad0], 0
    {0x26A43AA, 3, 7, false, 0x0000}, // lea rsi, [rsi + 0x1a875ad0]
    {0x26A74E1, 4, 8, false,
     0x0000}, // mov qword ptr [rsi + rax*8 + 0x1a875ad0], rbx
    {0x26C6DC0, 3, 7, true, 0x0000}, // lea rdx, [rip + 0x18135b99]
    {0x26CC0A9, 3, 7, true, 0x0000}, // lea rax, [rip + 0x181308b0]
    {0x26C6D43, 3, 7, true,
     0x0000}, // lea rcx, [rip + 0x18135c16]  (reset leaf 0x0273FEB0;
              // convergence-rejected, hand-verified)
};
constexpr entcoll_site hudpl_self_sites[] = {
    {0x26A42DB, 4, 8, false,
     0x0000}, // mov dword ptr [rsi + r11*4 + 0x1a875b50], eax
    {0x26CC08D, 3, 7, false, 0x0000}, // lea rax, [rdx + 0x1a875b50]
};
constexpr perclient_array batch12[] = {
    {"hudpl_score",
     0x1A7F6A50,
     0x20,
     hudpl_score_sites,
     std::size(hudpl_score_sites),
     0,
     {}},
    {"hudpl_gap",
     0x1A7F6A90,
     0x20,
     hudpl_gap_sites,
     std::size(hudpl_gap_sites),
     0,
     {}},
    {"hudpl_flags",
     0x1A7F6AD0,
     0x20,
     hudpl_flags_sites,
     std::size(hudpl_flags_sites),
     0,
     {}},
    {"hudpl_ids",
     0x1A7F6B10,
     0x20,
     hudpl_ids_sites,
     std::size(hudpl_ids_sites),
     0,
     {}},
    {"hudpl_icons",
     0x1A7F6B50,
     0x40,
     hudpl_icons_sites,
     std::size(hudpl_icons_sites),
     0,
     {}},
    {"hudpl_self",
     0x1A7F6BD0,
     0x4,
     hudpl_self_sites,
     std::size(hudpl_self_sites),
     0,
     {}},
};
size_t batch12_new[std::size(batch12)] = {};

void relocate_batch12() {
  for (size_t i = 0; i < std::size(batch12); ++i) {
    if (!batch12_new[i]) {
      batch12_new[i] = relocate_perclient(batch12[i]);
    }
  }
}

// ---- BATCH 13: view 2's lighting and previous-frame view (2026-09-26) ---
//
// Pane 3 draws its world as a white void. Two [2] per-view renderer
// arrays have a FOREIGN slot 2, both written by view 2 every frame:
//
// s_sunVolumeTransitions - PS4 SunVolumeTransition[4] x 0x2A28 at
// 0x04546CE0, used by CG_SetLightingState(lc) (PS4 0x2CA430: blends the
// sun of the volume the view stands in) and CG_InitView. PC 0x04D2CB40
// x 0x2BB8 (GfxSunVolume 0x11B8, blend 0x824): CG_SetLightingState
// 0x010F3050 does `lea base; imul lc,0x2bb8`, CG_InitView's tail
// 0x010CD0EC resets +0x2BA0/+0x2BA8/+0x2BB0 with ABS32 stores, and the
// leaf 0x010EB4D0 (no .pdata, read by hand) resets +0x828/+0x2BB0.
// Slot 2 = 0x04D322B0: PS4 puts s_testEffect[4] x 0x54 right after the
// array, and the PC does the same at 0x04D322C0 - so player 3's sun
// transition overwrote s_testEffect and what follows (47 refs).
// The static initializer 0x02DA6200 zeroes each element and stores
// 0xFFFFFFFF at +0x2BB0; slots 2/3 get the same state after the move.
// 6 sites (gen_reloc_sites v2), pdata_xcheck: the leaf is the only
// table-only row; 0 end markers, 0 suspect sentinels; find_lea lists
// the same three leas plus one Arxan-section `movsxd r15d,[rip]` that
// is not a compiler encoding (junk, not kept).
//
// g_prevFrameViewParmsDraw - PS4 GfxViewParms[4] x 0x290 at 0x0D740B30.
// R_RenderScene (PS4 0xA0B822) takes &prev[viewInfo->localClientNum],
// passes it to R_GenerateSortedDrawSurfs and then memcpy's this frame's
// parms into it. PC 0x0FE4B780 x 0x290, one lea at 0x01CEB7A5, index
// [viewInfo+0x1a680c] - the same field batch 10 found. Slot 2 =
// 0x0FE4BCA0..0x0FE4BF30 covers the first 0x230 bytes of the renderer
// object at 0x0FE4BD00 (22 leas, getter 0x01CEA5D0), so view 2 wrote
// its view parms over that object every frame. pdata_xcheck identical,
// 0 end markers, 0 sentinels, find_lea: the one lea. Zero-filled slots
// are what the first frame of a new view finds on PS4 as well.
constexpr entcoll_site sunvol_sites[] = {
    {0x10CD118, 4, 12, false,
     0x2BB0}, // mov dword ptr [rax + r14 + 0x4d2f6f0], 0xffffffff
    {0x10CD124, 4, 8, false,
     0x2BA0}, // mov qword ptr [rax + r14 + 0x4d2f6e0], rcx
    {0x10CD12C, 4, 8, false,
     0x2BA8}, // mov qword ptr [rax + r14 + 0x4d2f6e8], rcx
    {0x10EB4F0, 3, 7, true, 0x0000}, // lea rdx, [rip + 0x3c41669]  (reset leaf,
                                     // no .pdata, read by hand)
    {0x10F30C2, 3, 7, true, 0x0000}, // lea rax, [rip + 0x3c39a97]
    {0x2D2D0AF, 3, 7, true,
     0x0830}, // lea rbx, [rip + 0x1f8714a]  (static initializer)
};
constexpr entcoll_site prevview_sites[] = {
    {0x1CDF3D5, 3, 7, true, 0x0000}, // lea rax, [rip + 0xe15ffd4]
};
constexpr perclient_array batch13[] = {
    {"sunvol",
     0x04CADB40,
     0x2BB8,
     sunvol_sites,
     std::size(sunvol_sites),
     0,
     {}},
    {"prevview",
     0x0FDCC800,
     0x290,
     prevview_sites,
     std::size(prevview_sites),
     0,
     {}},
};
size_t batch13_new[std::size(batch13)] = {};

// ---- BATCH 14: LiveStats' per-controller stat-change cache (2026-09-26) ---
//
// Two rounds in a row crashed at GAME OVER in Cmd_RemoveCommand (leaf
// 0x020EE770, from CG_RemoveCommands 0x005DE5A0): the static cmd node
// of "statReadDDLExt" (0x11423140, LiveStats_InitOnce) had its name
// pointer overwritten with 0x0014A0000100000F. tools/region_watch.py
// caught the store at ROUND START (17:25:02): 0x11423080..0x11423170
// rewritten at once with packed {u16 0, u16 1, u16 offset} records.
//
// PS4 LiveStats_SetStatChanged (0xC63D00): cache =
// &s_cachedStatsChanges[controller] ([4] x 0x1984: 0x60 entries of
// {u8 data[0x40]; int len} + count), YEnc-decodes the change message
// into the next entry. PC 0x01EA5F70 does the same with 0x100 entries:
// base 0x1141A7D0, stride 0x4404, count at +0x4400. [2] on PC, so
// controller 2's cache begins at 0x11422FD8 - the statics after the
// array (the "statscache2" high-water id, LiveStats' own globals, the
// cmd nodes at 0x11423080.., an [18] x 0x858 array at 0x11423B10).
//
// PS4 LiveStats_ResetCache (0xC63CD0) clears ALL FOUR slots with one
// memset of 0x6610; the PC's (0x01EA191F) clears 0x8808 = two. Once
// the array has moved, that immediate is widened to 0x11010 so slots
// 2/3 reset with the rest (an uncleared count reaching 0x100 is
// EXE_PATCH_STATSOVERFLOW).
//
// 8 sites (gen_reloc_sites v2), pdata_xcheck identical, 0 suspect
// sentinels. endmarker_scan's one hit (0x02F089BE) is the vector
// constructor of the foreign [18] x 0x858 array at 0x11423B10, not an
// end marker of this one. 0x01EA53CF.. read cache[0] with fixed
// addresses (the primary controller's flush) - rip sites like the rest.
constexpr entcoll_site statscache_sites[] = {
    {0x1E94E8F, 3, 7, true,
     0x0000}, // lea rcx, [rip + 0xf578eaa]  (LiveStats_ResetCache memset)
    {0x1E9893F, 2, 6, true, 0x4400}, // mov edx, dword ptr [rip + 0xf5797fb]
    {0x1E9894F, 2, 6, true, 0x4400}, // mov eax, dword ptr [rip + 0xf5797eb]
    {0x1E98959, 3, 7, true, 0x0040}, // lea r14, [rip + 0xf575420]
    {0x1E98960, 3, 7, true, 0x0000}, // lea rbp, [rip + 0xf5753d9]
    {0x1E989A3, 2, 6, true, 0x4400}, // mov eax, dword ptr [rip + 0xf579797]
    {0x1E989AD, 3, 7, true, 0x4400}, // mov dword ptr [rip + 0xf57978c], r15d
    {0x1E9952B, 3, 7, true,
     0x0000}, // lea rax, [rip + 0xf57480e]  (LiveStats_SetStatChanged)
};
constexpr perclient_array batch14[] = {
    {"statscache",
     0x1139B860,
     0x4404,
     statscache_sites,
     std::size(statscache_sites),
     0,
     {}},
};
size_t batch14_new[std::size(batch14)] = {};

// ---- BATCH 15: the per-client UI visibility bits (2026-09-26 18:05) -----
//
// No HUD in panes 1 and 2 although both HUD menus exist and processed
// their first snapshot (ui_scripts/zz_probe: fs=true, gm=T7Hud_ZM). The
// zombie HUD shows its widgets only through the models
// "UIVisibilityBit.<n>" (T7Hud_zm_factory.lua: BIT_HUD_VISIBLE,
// BIT_WEAPON_HUD_VISIBLE, ...). Measured live: the bits themselves are
// right (lc 0/1 = 0x4000D90000008003: HUD_VISIBLE and
// WEAPON_HUD_VISIBLE set), the MODELS never change.
//
// PS4 keeps the bits in sharedUiInfo +0x14240 as u64[4] (UI_Set/
// CheckUIVisibilityBit). The PC keeps them at 0x179DBDC8 as u64[2]:
// Engine.IsVisibilityBitSet (0x01FE71A0) reads [0x179DBDC8 + lc*8].
// Local client 2's u64 therefore lands on 0x179DBDD8..DF - the per-
// client u16 "UIVisibilityBit" model handles (UI_CoD_VisibilityBitModel
// _Init 0x01F33130) and the scoreboard's team-model handles
// (0x01F328FB). Every frame CL_UpdateUIVisibilityBits(2) (0x013D1FE0)
// wrote player 3's bits over them. 101 sites (gen_reloc_sites v2: 9
// rip, 92 ABS32), pdata_xcheck identical, 0 end markers, 0 suspect
// sentinels. The reset loop at 0x01F32EB5 (bits[lc] = 0 plus the
// batch-12 player-list reset, bound 0x01F32ED0) widens to three with
// it. Known and left: 0x0262A370 clears match bits for lc 0/1 with one
// 16-byte and/andn over the array (a two-client unroll); client 2's
// bits are recomputed every frame by CL_UpdateUIVisibilityBits anyway.
constexpr entcoll_site visbits_sites[] = {
    {0x61D911, 3, 7, false,
     0x0000}, // mov ebx, dword ptr [rbx + rsi*8 + 0x179dbdc8]
    {0x6D3CAD, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r9 + rbx*8 + 0x179dbdc8]
    {0x8635A1, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r14 + rsi*8 + 0x179dbdc8]
    {0xA141F4, 4, 8, false,
     0x0000}, // mov eax, dword ptr [r13 + r15*8 + 0x179dbdc8]
    {0xE477F9, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r15 + rsi*8 + 0x179dbdc8]
    {0xFB5B24, 3, 7, false,
     0x0000}, // mov ecx, dword ptr [rdx + rsi*8 + 0x179dbdc8]
    {0x135FAF1, 4, 8, false,
     0x0000}, // mov rax, qword ptr [r14 + rdi*8 + 0x179dbdc8]
    {0x13D2030, 4, 8, false,
     0x0000}, // mov rax, qword ptr [r14 + rdx + 0x179dbdc8]
    {0x13D2047, 4, 12, false,
     0x0000}, // mov qword ptr [r14 + rdx + 0x179dbdc8], 0
    {0x13D38B2, 4, 8, false,
     0x0000}, // or qword ptr [r14 + rdx + 0x179dbdc8], rax
    {0x13D38CB, 4, 8, false,
     0x0000}, // or rcx, qword ptr [r14 + rdx + 0x179dbdc8]
    {0x13D38D3, 4, 8, false,
     0x0000}, // mov qword ptr [r14 + rdx + 0x179dbdc8], rcx
    {0x13D38FA, 4, 8, false,
     0x0000}, // mov qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3919, 4, 12, false,
     0x0000}, // or qword ptr [rax + rcx + 0x179dbdc8], 0x40000000
    {0x13D3946, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3972, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3991, 4, 12, false,
     0x0000}, // or qword ptr [rax + rcx + 0x179dbdc8], 0x40000000
    {0x13D39A6, 4, 12, false,
     0x0000}, // or qword ptr [rax + rcx + 0x179dbdc8], 0x800000
    {0x13D39CD, 4, 12, false,
     0x0000}, // or qword ptr [rax + rcx + 0x179dbdc8], 0x1000000
    {0x13D3A12, 4, 12, false,
     0x0000}, // or qword ptr [rax + rcx + 0x179dbdc8], 0x2000000
    {0x13D3A54, 4, 8, false,
     0x0000}, // or qword ptr [rcx + rdx + 0x179dbdc8], rax
    {0x13D3A81, 4, 8, false,
     0x0000}, // or qword ptr [rcx + rdx + 0x179dbdc8], rax
    {0x13D3AAA, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3ADA, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3B03, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3B22, 4, 12, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], 0x4000000
    {0x13D3B3B, 4, 12, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], 0x8000000
    {0x13D3B5E, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3B79, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3B94, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3BBD, 4, 8, false,
     0x0000}, // or qword ptr [rax + rdx + 0x179dbdc8], rcx
    {0x13D3BF1, 4, 12, false,
     0x0000}, // or qword ptr [rax + rcx + 0x179dbdc8], 0x10000000
    {0x13D6D42, 4, 12, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], 0x20000000
    {0x13D6DDF, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D6E04, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D6E26, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D6E44, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D6E7E, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D6EDC, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D6EFD, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D6F78, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D6F80, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r15 + r13 + 0x179dbdc8]
    {0x13D6FE1, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r15 + r13 + 0x179dbdc8]
    {0x13D7033, 4, 8, false,
     0x0000}, // mov qword ptr [r15 + r13 + 0x179dbdc8], rcx
    {0x13D7055, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D709D, 4, 8, false,
     0x0000}, // and qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D70BF, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D70DD, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D7103, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D711E, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D7139, 4, 8, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D714A, 4, 12, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], 0x200000
    {0x13D715F, 4, 12, false,
     0x0000}, // or qword ptr [r15 + r13 + 0x179dbdc8], 0x400000
    {0x13D7173, 4, 8, false,
     0x0000}, // mov rax, qword ptr [r15 + r13 + 0x179dbdc8]
    {0x13D71D5, 4, 8, false,
     0x0000}, // mov qword ptr [r15 + r13 + 0x179dbdc8], rax
    {0x13D72CC, 4, 8, false,
     0x0000}, // mov r8, qword ptr [r15 + r13 + 0x179dbdc8]
    {0x1F23CB7, 4, 8, false,
     0x0000}, // mov esi, dword ptr [rax + r12*8 + 0x179dbdc8]
    {0x1F24013, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rbx + r12*8 + 0x179dbdc8]
    {0x1F26735, 3, 7, true, 0x0000}, // lea rsi, [rip + 0x15aa8f0c]
    {0x1FDAA77, 3, 7, true, 0x0000}, // lea r8, [rip + 0x159f4bca]
    {0x1FF7347, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rsi + rdx*8 + 0x179dbdc8]
    {0x1FF7537, 4, 8, false,
     0x0000}, // mov r9, qword ptr [r14 + rdi*8 + 0x179dbdc8]
    {0x1FF770F, 4, 8, false,
     0x0000}, // mov r9, qword ptr [rdx + rdi*8 + 0x179dbdc8]
    {0x1FF785E, 4, 8, false,
     0x0000}, // mov r9, qword ptr [rcx + rdi*8 + 0x179dbdc8]
    {0x1FF79B7, 4, 8, false,
     0x0000}, // mov r9, qword ptr [r14 + rdi*8 + 0x179dbdc8]
    {0x200C283, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r14 + rbx*8 + 0x179dbdc8]
    {0x201127F, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x159be3c2]
    {0x201153E, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x159be103]
    {0x201C4B7, 5, 9, false,
     0x0000}, // movzx eax, byte ptr [r13 + rdi*8 + 0x179dbdc8]
    {0x2035F70, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r11 + rsi*8 + 0x179dbdc8]
    {0x2037FE1, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rcx + r14*8 + 0x179dbdc8]
    {0x2038163, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rcx + r14*8 + 0x179dbdc8]
    {0x203838E, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rcx + r14*8 + 0x179dbdc8]
    {0x203A820, 4, 8, false,
     0x0000}, // mov rax, qword ptr [rdx + r12*8 + 0x179dbdc8]
    {0x20462F0, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rcx + r14*8 + 0x179dbdc8]
    {0x2054DD8, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdx + r15*8 + 0x179dbdc8]
    {0x205BF00, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r8 + r13*8 + 0x179dbdc8]
    {0x20668E6, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdx + rsi*8 + 0x179dbdc8]
    {0x206F333, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rsi + rdi*8 + 0x179dbdc8]
    {0x206F711, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r14 + rdi*8 + 0x179dbdc8]
    {0x2074E6E, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r8 + rbx*8 + 0x179dbdc8]
    {0x207824F, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdx + r14*8 + 0x179dbdc8]
    {0x2079CBC, 4, 8, false,
     0x0000}, // mov r9, qword ptr [r12 + r14*8 + 0x179dbdc8]
    {0x207B8ED, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdx + r12*8 + 0x179dbdc8]
    {0x2080A84, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdx + r14*8 + 0x179dbdc8]
    {0x2084436, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdx + rsi*8 + 0x179dbdc8]
    {0x2087672, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r10 + rdi*8 + 0x179dbdc8]
    {0x20877EE, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rbx + rdi*8 + 0x179dbdc8]
    {0x2092456, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rax + rbx*8 + 0x179dbdc8]
    {0x2098F85, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r12 + rsi*8 + 0x179dbdc8]
    {0x209920F, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rdx + r13*8 + 0x179dbdc8]
    {0x20A0DDF, 3, 7, true, 0x0000}, // lea r9, [rip + 0x1592e862]
    {0x20A0ED7, 3, 7, true, 0x0000}, // lea r9, [rip + 0x1592e76a]
    {0x25B1223, 4, 8, true,
     0x0000}, // movdqu xmm0, xmmword ptr [rip + 0x153b1a2d]
    {0x25B126B, 4, 8, true,
     0x0000}, // movdqu xmmword ptr [rip + 0x153b19e5], xmm0
    {0x25D17BE, 3, 7, true, 0x0000}, // lea rcx, [rip + 0x15391493]
    {0x25D40C8, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r14 + rbx*8 + 0x179dbdc8]
    {0x26B72CE, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [rsi + rdi*8 + 0x179dbdc8]
    {0x26EBC6A, 4, 8, false,
     0x0000}, // movzx ecx, byte ptr [rdx + rsi*8 + 0x179dbdc8]
    {0x26EF0C3, 4, 8, false,
     0x0000}, // mov eax, dword ptr [rax + r13*8 + 0x179dbdc8]
    {0x26EF10A, 4, 8, false,
     0x0000}, // mov eax, dword ptr [rax + r13*8 + 0x179dbdc8]
};
constexpr perclient_array batch15[] = {
    {"visbits",
     0x1795CEC8,
     0x8,
     visbits_sites,
     std::size(visbits_sites),
     0,
     {}},
};
size_t batch15_new[std::size(batch15)] = {};

void relocate_batch15() {
  for (size_t i = 0; i < std::size(batch15); ++i) {
    if (batch15_new[i]) {
      continue;
    }
    batch15_new[i] = relocate_perclient(batch15[i]);
    if (!batch15_new[i]) {
      continue;
    }
    // the per-client reset loop that zeroes bits[lc]: 2 -> 4 (player 4,
    // 2026-09-27; visbits is relocated to [4] by relocate_perclient)
    auto *bound = reinterpret_cast<uint8_t *>(base() + 0x01F26E10);
    constexpr uint8_t bound_old[] = {0x83, 0xFF, 0x02};
    if (readable(bound, sizeof(bound_old)) &&
        std::memcmp(bound, bound_old, sizeof(bound_old)) == 0) {
      const uint8_t four = 0x04;
      write_bytes(bound + 2, &four, 1);
    }
  }
}

// ---- CONSOLE MESSAGE BUFFERS: con.messageBuffer [2] -> [4] (2026-09-26 19:40)
//
// Next crash after the sun-shadow fix (19:18, main thread): a read of
// 0xFFFFFFFFFFFFFFFF at 0x0133ADC6 in Con_NudgeMessageWindowTimes
// (0x0133AD80), called by Con_TimeNudged (0x0133D1C0, PS4 0x3F1C60) from
// CL_AdjustTimeDelta for local client 2 - the "window" it walked held
// text (rax = "onnected"). PS4 DWARF, struct Console (cl_console.cpp):
// messageBuffer is MessageBuffer[4] at +0x11078 (stride 0x48A0),
// followed by color and operationBuffer[0x8000], the console text. The
// PC keeps MessageBuffer[2] at con (0x05366780) + 0x11078 = 0x053777F8,
// stride 0x2BC0 (Con_GetGameMsgWindow 0x0133A7B0 = (lc*0xAF + i) << 6),
// so client 2's windows ARE con.color and the operation buffer - r14 in
// the crash was 0x5780 = 2 * 0x2BC0.
//
// Three reference forms, every one verified before anything is written:
//   15 RIP/ABS32 sites into the array (tools/gen_reloc_sites.py v2);
//   1 loop END MARKER (tools/endmarker_scan.py): Con_InitMessageBuffer
//     (0x0133A9F0) steps element pointers up to &messageBuffer[2]+0x2B10
//     (0x0133AB5E) - retargeted to &new[4]+0x2B10, since PS4 initialises
//     all four (its loop bound is 4);
//   16 accesses through the address of the WHOLE con struct plus a
//     displacement or immediate inside the array (`lea r15,[con]` ...
//     `lea rbx,[r15+0x13078]`, `add r8,0x13B78`). They carry no address
//     of the array, so no range scan can see them; they were found by
//     decoding every console function (0x01330000..0x01345000) for
//     disp/imm in [0x11078, 0x167F8) - the undecodable gaps raw-scanned
//     - and each base register was checked to be loaded from con. Their
//     new value is old + (new block - old array), so every field offset
//     is kept.
// Nothing in the game has initialised con at post_unpack.
constexpr uint32_t conmsgbuf_base = 0x52F87F8;
constexpr uint32_t conmsgbuf_stride = 0x2BC0;

constexpr entcoll_site conmsgbuf_sites[] = {
    {0x133930F, 4, 8, false,
     0x2030}, // mov qword ptr [rax + rdi + 0x5379828], rcx
    {0x1339317, 4, 8, false,
     0x2038}, // mov qword ptr [rax + rdi + 0x5379830], rcx
    {0x133931F, 4, 8, false,
     0x2070}, // mov qword ptr [rax + rdi + 0x5379868], rcx
    {0x1339327, 4, 8, false,
     0x2078}, // mov qword ptr [rax + rdi + 0x5379870], rcx
    {0x133932F, 4, 8, false,
     0x20B0}, // mov qword ptr [rax + rdi + 0x53798a8], rcx
    {0x1339337, 4, 8, false,
     0x20B8}, // mov qword ptr [rax + rdi + 0x53798b0], rcx
    {0x133933F, 4, 8, false,
     0x20F0}, // mov qword ptr [rax + rdi + 0x53798e8], rcx
    {0x1339347, 4, 8, false,
     0x20F8}, // mov qword ptr [rax + rdi + 0x53798f0], rcx
    {0x133934F, 4, 8, false,
     0x2B30}, // mov qword ptr [rbx + rdi + 0x537a328], rcx
    {0x1339357, 4, 8, false,
     0x2B38}, // mov qword ptr [rbx + rdi + 0x537a330], rcx
    {0x133A666, 3, 7, true, 0x201C}, // lea rcx, [rip + 0x403f1c7]
    {0x133A7E0, 3, 7, true,
     0x2000}, // lea rcx, [rip + 0x403f031]  Con_GetGameMsgWindow
    {0x133AA39, 3, 7, true,
     0x2B10}, // lea rdi, [rip + 0x403f8e8]  Con_InitMessageBuffer
    {0x133D8E4, 3, 7, true, 0x2000}, // lea r13, [rip + 0x403bf2d]
    {0x133DA8E, 3, 7, true, 0x2000}, // lea rax, [rip + 0x403bd83]
};

struct con_rel_site {
  uint32_t rva;
  uint8_t off;        // where the disp32 / imm32 sits in the instruction
  uint32_t old_value; // offset from con it holds
};

constexpr con_rel_site conmsgbuf_con_rel[] = {
    {0x0133D107, 3,
     0x13094}, // lea rcx, [rsi + 0x13094]            rsi = con (0x0133D0D8)
    {0x0133D172, 4, 0x13BB0}, // cmp dword ptr [rdi + rsi + 0x13bb0], r11d
    {0x0133D180, 3, 0x13BAC}, // mov eax, dword ptr [rdi + rsi + 0x13bac]
    {0x0133D18E, 3, 0x13B94}, // idiv dword ptr [rdi + rsi + 0x13b94]
    {0x0133D19C, 4, 0x13B78}, // mov rax, qword ptr [rdi + rsi + 0x13b78]
    {0x0133D1A8, 4, 0x13B80}, // mov rax, qword ptr [rdi + rsi + 0x13b80]
    {0x0133D1C3, 4, 0x13BB0}, // cmp r11d, dword ptr [rdi + rsi + 0x13bb0]
    {0x0133D22C, 3,
     0x13078}, // lea rbx, [r15 + 0x13078]            r15 = con (0x0133D1FE)
    {0x0133D256, 3, 0x13B78}, // lea rcx, [r15 + 0x13b78]
    {0x0133D930, 4,
     0x13B78}, // lea rdx, [r12 + 0x13b78]            r12 = con (0x0133D8CB)
    {0x0133D987, 4, 0x13B78}, // lea rdx, [r12 + 0x13b78]
    {0x0133DABC, 3,
     0x13B78}, // add r8, 0x13b78                     r8 = con (0x0133DA95)
    {0x0133DB8B, 4, 0x13BB4}, // mov eax, dword ptr [rcx + r8 + 0x13bb4]   r8 =
                              // con (0x0133DB56)
    {0x0133DB93, 4, 0x13B80}, // mov rdi, qword ptr [rcx + r8 + 0x13b80]
    {0x0133DB9E, 4, 0x13B94}, // idiv dword ptr [rcx + r8 + 0x13b94]
    {0x0133DBA6, 4, 0x13BB4}, // mov dword ptr [rcx + r8 + 0x13bb4], edx
};

constexpr uint32_t conmsgbuf_end_marker_rva =
    0x133AB7E; // lea rcx, [&messageBuffer[2]+0x2B10]
constexpr uint32_t conmsgbuf_end_field = 0x2B10;

constexpr perclient_array batch16[] = {
    {"conmsgbuf",
     conmsgbuf_base,
     conmsgbuf_stride,
     conmsgbuf_sites,
     std::size(conmsgbuf_sites),
     0,
     {}},
};
size_t batch16_new[std::size(batch16)] = {};

void relocate_batch16() {
  if (batch16_new[0]) {
    return;
  }
  const auto b = base();
  for (const auto &s : conmsgbuf_con_rel) {
    uint32_t have = 0;
    const auto *at = reinterpret_cast<const uint8_t *>(b + s.rva + s.off);
    if (!readable(at, sizeof(have))) {
      return;
    }
    std::memcpy(&have, at, sizeof(have));
    if (have != s.old_value) {
      note("[splitscreen] conmsgbuf: 0x%08X holds 0x%X - nothing moved\n",
           s.rva, have);
      return;
    }
  }
  {
    const auto *at =
        reinterpret_cast<const uint8_t *>(b + conmsgbuf_end_marker_rva + 3);
    int32_t d32 = 0;
    if (!readable(at, sizeof(d32))) {
      return;
    }
    std::memcpy(&d32, at, sizeof(d32));
    if (conmsgbuf_end_marker_rva + 7 + d32 !=
        conmsgbuf_base + 2 * conmsgbuf_stride + conmsgbuf_end_field) {
      note("[splitscreen] conmsgbuf: end marker differs - nothing moved\n");
      return;
    }
  }

  const auto fresh = relocate_perclient(batch16[0]);
  if (!fresh) {
    return;
  }
  batch16_new[0] = fresh;

  // allocate_near_module keeps the block within 0x60000000 of the base,
  // so every new value below fits in 32 bits; checked anyway.
  const auto delta =
      static_cast<int64_t>(fresh) - static_cast<int64_t>(b + conmsgbuf_base);
  uint32_t rewritten = 0;
  for (const auto &s : conmsgbuf_con_rel) {
    const auto v = static_cast<int64_t>(s.old_value) + delta;
    if (v > INT32_MAX || v < INT32_MIN) {
      break;
    }
    const auto v32 = static_cast<int32_t>(v);
    if (!write_bytes(reinterpret_cast<void *>(b + s.rva + s.off), &v32,
                     sizeof(v32))) {
      break;
    }
    ++rewritten;
  }
  const bool marker = retarget_end_marker(
      conmsgbuf_end_marker_rva, 3, 7,
      conmsgbuf_base + 2 * conmsgbuf_stride + conmsgbuf_end_field,
      fresh + 4 * conmsgbuf_stride + conmsgbuf_end_field);

  trace_line l;
  l.str("conmsgbuf [2]->[4]: ");
  l.dec(std::size(conmsgbuf_sites));
  l.str(" array sites, ");
  l.dec(rewritten);
  l.str("/");
  l.dec(std::size(conmsgbuf_con_rel));
  l.str(" con-relative, end marker ");
  l.str(marker ? "moved" : "FAILED");
  trace_write(l);
}

// ---- UI CONTEXT INFO: uiInfoArray [2] -> [4] (2026-09-26 20:10) -------------
//
// PS4 ui_main.cpp: `uiInfoArray[MAX_UI_CONTEXTS]`, MAX_UI_CONTEXTS = 4
// (UI_UIContext_GetInfo 0xF99940 asserts contextIndex < 4), element
// 0x1B68 - the SAME size on PC, one of the rare strides that transfer.
// The PC array is [2] at 0x179DC170 (accessor 0x0228D6E0 = base +
// uictx(lc) * 0x1B68, uictx from 0x020EF970). Its slot 2 (0x179DF840)
// is foreign: pe_xref finds dvar-pointer loads throughout, and the
// process-exit crash of 19:47 (static std::stringstream destructor
// 0x0292CEE0 via atexit 0x01DEEDB0, vbtable pointer at RVA 0x179E1088
// overwritten with 8) is exactly uiInfo[2]+0x1848 - UI code for local
// client 2 (uictx 2) was already writing its menu state there in game.
//
// 10 rip sites (gen_reloc_sites v2; endmarker_scan 0, sentinel_check 0,
// pdata_xcheck identical). UI_InitUIInfos (0x0228DB70) loops
// `cmp ebp,2` at 0x0228DC30 over memset + Menu_Setup; with the array
// moved it runs to 4 like PS4. The Com_ShutdownInternal UI-close loops
// (see widen_client_shutdown_loops) may only follow once this moved.
constexpr entcoll_site uiinfo_sites[] = {
    {0x22304A1, 3, 7, true, 0x0000}, // lea rdi, [rip + 0x1574f1a8]
    {0x223085C, 3, 7, true, 0x184C}, // lea rax, [rip + 0x15750639]
    {0x223088C, 3, 7, true, 0x002C}, // lea rax, [rip + 0x1574ede9]
    {0x2230BC9, 3, 7, true,
     0x0000}, // lea rcx, [rip + 0x1574ea80]  UI_UIContext_GetInfo
    {0x2231087, 3, 7, true,
     0x001C}, // lea rsi, [rip + 0x1574e5de]  UI_InitUIInfos
    {0x22312C9, 3, 7, true, 0x184C}, // lea rdx, [rip + 0x1574fbcc]
    {0x223157F, 3, 7, true, 0x0000}, // lea rsi, [rip + 0x1574e0ca]
    {0x223268B, 3, 7, true, 0x0030}, // lea rax, [rip + 0x1574cfee]
    {0x22328F5, 3, 7, true, 0x0000}, // lea rax, [rip + 0x1574cd54]
    {0x22329A0, 3, 7, true, 0x0000}, // lea rax, [rip + 0x1574cca9]
};
constexpr perclient_array batch17[] = {
    {"uiinfo",
     0x1795D270,
     0x1B68,
     uiinfo_sites,
     std::size(uiinfo_sites),
     0,
     {}},
};
size_t batch17_new[std::size(batch17)] = {};

void relocate_batch17() {
  if (batch17_new[0]) {
    return;
  }
  const auto b = base();
  auto *bound = reinterpret_cast<uint8_t *>(b + 0x022317D0);
  constexpr uint8_t bound_old[] = {0x83, 0xFD, 0x02}; // cmp ebp, 2
  if (!readable(bound, sizeof(bound_old)) ||
      std::memcmp(bound, bound_old, sizeof(bound_old)) != 0) {
    note("[splitscreen] uiinfo: init loop bound differs - nothing moved\n");
    return;
  }
  batch17_new[0] = relocate_perclient(batch17[0]);
  if (!batch17_new[0]) {
    return;
  }
  const uint8_t four = 0x04;
  const bool widened = write_bytes(bound + 2, &four, 1);
  trace_line l;
  l.str("uiinfo [2]->[4]: ");
  l.dec(std::size(uiinfo_sites));
  l.str(" sites, init loop ");
  l.str(widened ? "-> 4" : "FAILED");
  trace_write(l);
}

// ---- LIGHT QUEUE: per-client records [2][1024] + head/tail [2] -> [4]
// (2026-09-26 20:35)
//
// Crash 20:16:15 on Revelations, one second after all three local
// clients went CA_ACTIVE: READ 0x10000000D at 0x00436F83 (`lddqu
// xmm4,[r13+0xC]`), main thread, SCR_UpdateFrame -> active draw ->
// cgame 0x010B62A0 -> 0x006F7010 -> 0x00436E10(lc). That function
// drains a per-client ring: records at 0x106175F0, 1024 x 0x28 per
// client (stride 0xA000; each record holds a pointer, a GfxLight-
// Description asset - XAssetType 81 on PS4 - and a sub-pointer into it),
// indexed `lc << 10`, with read/write counters int[2] at 0x1062B5F0
// and int[2] at 0x1062B5F8. Written by the renderer (0x01CF9E00),
// reset by 0x01CFA4B0 (two qword stores = two clients), saved and
// restored by 0x000B1C70 / 0x000B1540 (loops `cmp r?d,2`, restore
// memsets 0x14000 = two clients). Client 2's records ARE the counters
// and whatever follows (gen_reloc_sites: 277 raw ABS32 hits into slots
// 2..3) - hence the garbage pointer. Revelations drives scripted
// lights; Shadows of Evil and The Giant never filled the queue.
//
// New block: records[4] (0x28000) then A[4] at +0x28000 and B[4] at
// +0x28010 - A and B are separate [2] arrays 8 bytes apart, so they
// cannot be copied flat. 23 record sites + 7 A + 8 B sites
// (gen_reloc_sites v2; pdata_xcheck identical for both; sentinel_check
// 0; endmarker_scan's one hit, 0x01CFA406, walks the 0xC0000 buffer
// that follows at 0x1062B610 - not this array - and is left alone).
// The reset's `mov qword [rip+d],rax` pair becomes `movups
// [rip+d],xmm0` (same length; xmm0 is zeroed by the xorps right before
// and only stored as 0.0 afterwards) so all four clients' counters
// clear.
constexpr uint32_t lightq_base = 0x10598670;
constexpr uint32_t lightq_stride = 0xA000;
constexpr uint32_t lightq_a = 0x105AC670;
constexpr uint32_t lightq_b = 0x105AC678;
constexpr size_t lightq_records_new = 4 * lightq_stride; // 0x28000

constexpr entcoll_site lightq_sites[] = {
    {0xB15FE, 3, 7, true,
     0x0000}, // lea rcx, [rip + 0x10565feb]   restore memset
    {0xB1665, 3, 7, true, 0x0000},  // lea r12, [rip + 0x10565f84]   restore
    {0xB1E7B, 3, 7, false, 0x0000}, // lea rdx, [r13 + 0x106175f0]   save
    {0xB1EDE, 4, 9, false,
     0x0020}, // test byte ptr [r13 + rbp*8 + 0x10617610], 1
    {0xB1EF4, 4, 8, false,
     0x0000}, // mov rcx, qword ptr [r13 + rbp*8 + 0x106175f0]
    {0xB1F73, 4, 8, false,
     0x0008}, // mov rcx, qword ptr [r13 + rbp*8 + 0x106175f8]
    {0xB1F8B, 4, 8, false,
     0x0008}, // mov rax, qword ptr [r13 + rbp*8 + 0x106175f8]
    {0xB1F93, 4, 8, false,
     0x0010}, // mov rcx, qword ptr [r13 + rbp*8 + 0x10617600]
    {0xB2008, 4, 8, false,
     0x001C}, // mov ecx, dword ptr [r13 + rbp*8 + 0x1061760c]
    {0x436EF7, 4, 8, false,
     0x0008}, // mov rax, qword ptr [r9 + rcx*8 + 0x106175f8]  consumer
    {0x436EFF, 4, 8, false,
     0x0020}, // mov rsi, qword ptr [r9 + rcx*8 + 0x10617610]
    {0x436F07, 4, 8, false,
     0x0010}, // mov r14, qword ptr [r9 + rcx*8 + 0x10617600]
    {0x436F0F, 4, 8, false,
     0x0000}, // mov r13, qword ptr [r9 + rcx*8 + 0x106175f0]
    {0x436F1C, 4, 8, false,
     0x0018}, // mov rax, qword ptr [r9 + rcx*8 + 0x10617608]
    {0x1CEDEFE, 4, 8, false,
     0x0010}, // mov qword ptr [r13 + r8*8 + 0x10617600], r12  producer
    {0x1CEDF09, 4, 8, false,
     0x0008}, // mov qword ptr [r13 + r8*8 + 0x106175f8], rax
    {0x1CEDF1F, 5, 9, false,
     0x0020}, // mov word ptr [r13 + r8*8 + 0x10617610], ax
    {0x1CEDF2E, 4, 8, false,
     0x001C}, // mov dword ptr [r13 + r8*8 + 0x1061760c], ecx
    {0x1CEDF3A, 4, 8, false,
     0x0018}, // mov dword ptr [r13 + r8*8 + 0x10617608], eax
    {0x1CEDF52, 5, 9, false,
     0x0000}, // mov word ptr [r13 + r8*8 + 0x106175f0], ax
    {0x1CEDF5B, 5, 10, false,
     0x0020}, // or word ptr [r13 + r8*8 + 0x10617610], 1
    {0x1CEDF6B, 4, 8, false,
     0x0000}, // mov qword ptr [r13 + r8*8 + 0x106175f0], rax
    {0x1CEE6EE, 3, 7, false, 0x0000}, // lea rcx, [rdi + 0x106175f0]
};
constexpr entcoll_site lightq_a_sites[] = {
    {0xB174B, 4, 8, false, 0},   // mov dword ptr [r12 + rbx + 0x1062b5f0], r13d
    {0xB1DB0, 4, 8, false, 0},   // mov r14d, dword ptr [rax + rdi + 0x1062b5f0]
    {0x436E47, 4, 8, false, 0},  // cmp eax, dword ptr [r9 + r15*4 + 0x1062b5f0]
    {0x43A80D, 4, 8, false, 0},  // cmp eax, dword ptr [r9 + r15*4 + 0x1062b5f0]
    {0x1CEDECE, 4, 8, false, 0}, // lea r10, [rcx*4 + 0x1062b5f0]
    {0x1CEE158, 3, 7, true, 0},  // mov qword ptr [rip + 0xe9310c1], rax (reset)
    {0x1CEE6C5, 4, 8, false, 0}, // lea rdx, [rcx*4 + 0x1062b5f0]
};
constexpr entcoll_site lightq_b_sites[] = {
    {0xB1753, 4, 8, false, 0},  // mov dword ptr [r12 + rbx + 0x1062b5f8], eax
    {0xB1DB8, 3, 7, false, 0},  // mov esi, dword ptr [rax + rdi + 0x1062b5f8]
    {0x436E3C, 4, 8, false, 0}, // mov eax, dword ptr [r9 + r15*4 + 0x1062b5f8]
    {0x43A7DE, 4, 8, false, 0}, // mov eax, dword ptr [r9 + r15*4 + 0x1062b5f8]
    {0x43A805, 4, 8, false, 0}, // mov dword ptr [r9 + r15*4 + 0x1062b5f8], eax
    {0x1CEDEE5, 4, 8, false,
     0},                        // cmp r9d, dword ptr [r13 + rcx*4 + 0x1062b5f8]
    {0x1CEE15F, 3, 7, true, 0}, // mov qword ptr [rip + 0xe9310c2], rax (reset)
    {0x1CEE6DA, 3, 7, false,
     0}, // cmp eax, dword ptr [rdi + rcx*4 + 0x1062b5f8]
};
bool lightq_relocated = false;

void relocate_lightq() {
  if (lightq_relocated) {
    return;
  }
  const auto b = base();
  struct fixed_bytes {
    uint32_t rva;
    uint8_t len;
    uint8_t old_bytes[6];
    uint8_t new_bytes[6];
  };
  constexpr fixed_bytes extras[] = {
      {0x01CEE155,
       3,
       {0x0F, 0x57, 0xC0},
       {0x0F, 0x57, 0xC0}}, // xorps xmm0,xmm0 - must be there
      {0x01CEE158,
       3,
       {0x48, 0x89, 0x05},
       {0x0F, 0x11, 0x05}}, // mov qword -> movups (A)
      {0x01CEE15F,
       3,
       {0x48, 0x89, 0x05},
       {0x0F, 0x11, 0x05}}, // mov qword -> movups (B)
      {0x000B15F8,
       6,
       {0x41, 0xB8, 0x00, 0x40, 0x01, 0x00},
       {0x41, 0xB8, 0x00, 0x80, 0x02, 0x00}}, // restore memset
      {0x000B176E,
       4,
       {0x41, 0x83, 0xFF, 0x02},
       {0x41, 0x83, 0xFF, 0x03}}, // restore loop
      {0x000B2060,
       4,
       {0x41, 0x83, 0xFD, 0x02},
       {0x41, 0x83, 0xFD, 0x03}}, // save loop
  };
  for (const auto &e : extras) {
    const auto *at = reinterpret_cast<const uint8_t *>(b + e.rva);
    if (!readable(at, e.len) || std::memcmp(at, e.old_bytes, e.len) != 0) {
      note("[splitscreen] lightq: bytes at 0x%08X differ - nothing moved\n",
           e.rva);
      return;
    }
  }

  auto *fresh =
      static_cast<uint8_t *>(allocate_near_module(lightq_records_new + 0x20));
  if (!fresh) {
    return;
  }
  std::memset(fresh, 0, lightq_records_new + 0x20);
  std::memcpy(fresh, reinterpret_cast<const void *>(b + lightq_base),
              2 * lightq_stride);
  std::memcpy(fresh + lightq_records_new,
              reinterpret_cast<const void *>(b + lightq_a), 8);
  std::memcpy(fresh + lightq_records_new + 0x10,
              reinterpret_cast<const void *>(b + lightq_b), 8);

  int32_t saved_r[std::size(lightq_sites)] = {};
  int32_t saved_a[std::size(lightq_a_sites)] = {};
  int32_t saved_b[std::size(lightq_b_sites)] = {};
  const auto restore = [&](const entcoll_site *sites, const size_t n,
                           const int32_t *saved) {
    for (size_t i = 0; i < n; ++i) {
      write_bytes(
          reinterpret_cast<void *>(b + sites[i].rva + sites[i].disp_off),
          &saved[i], sizeof(int32_t));
    }
  };
  const auto fresh_abs = reinterpret_cast<size_t>(fresh);
  if (!rewrite_entcoll(lightq_sites, std::size(lightq_sites), lightq_base,
                       fresh_abs, saved_r)) {
    return;
  }
  if (!rewrite_entcoll(lightq_a_sites, std::size(lightq_a_sites), lightq_a,
                       fresh_abs + lightq_records_new, saved_a)) {
    restore(lightq_sites, std::size(lightq_sites), saved_r);
    return;
  }
  if (!rewrite_entcoll(lightq_b_sites, std::size(lightq_b_sites), lightq_b,
                       fresh_abs + lightq_records_new + 0x10, saved_b)) {
    restore(lightq_a_sites, std::size(lightq_a_sites), saved_a);
    restore(lightq_sites, std::size(lightq_sites), saved_r);
    return;
  }
  uint32_t extras_done = 0;
  for (const auto &e : extras) {
    if (write_bytes(reinterpret_cast<void *>(b + e.rva), e.new_bytes, e.len)) {
      ++extras_done;
    }
  }
  lightq_relocated = true;
  trace_line l;
  l.str("lightq [2]->[4]: ");
  l.dec(std::size(lightq_sites) + std::size(lightq_a_sites) +
        std::size(lightq_b_sites));
  l.str(" sites, ");
  l.dec(extras_done);
  l.str("/");
  l.dec(std::size(extras));
  l.str(" reset/memset/loop patches");
  trace_write(l);
}

// ---- UMBRA: the per-client culling state, [2] -> [4] (2026-09-26 18:40) --
//
// Pane 3 draws no world: walls, streets and buildings missing, only
// entities in the fog. The occlusion culler keeps per-local-client state
// inside its heap object sUmbra (pointer 0x0AE94B78, allocated once by
// 0x01C996F0 with size 0x470210). PS4: sUmbra + 0x14fd1c holds
// UmbraQueryParameters[4] x 0x14 (distanceScale, accurateOcclusion-
// Threshold, minRelativeContribution, grid width/height) read by
// R_Umbra_SelectTome(lc) (0x945980, `imul lc, 0x14`). The PC keeps
// [2]:
//   +0x12DC0C  params[2] x 0x14      (defaults 1.0, 1024.0, 0, 4, 4)
//   +0x12DC48  tome trigger[2] x 4   (-1 = none)
//   +0x12DC50  persistent tome trigger[2] x 4
// Measured live, 3 players: params[0] = params[1] = {1.0, 1024, 0, 4, 4},
// "params[2]" = all zero (it is the counters at +0x12DC34/+0x12DC38),
// and client 2's tome triggers alias [0] of the persistent array and
// the field after it.
//
// The object is on the heap, so the arrays cannot move to a static
// block without caving every accessor. Instead the OBJECT grows: the
// allocation (and its memset) goes 0x470210 -> 0x470300 and the three
// arrays get [4] at the new tail - params at 0x470220 (a multiple of
// 0x14, because the distance-scale setter 0x01C9A3E0 addresses them as
// `(lc + 0xF167) * 0x14`), triggers at 0x470270 and 0x470280. Every
// access is a displacement off the object pointer (26 instructions,
// found by decoding every .pdata function in 0x01C98000..0x01C9E000
// plus the four leaf accessors), and the init loops that fill two
// entries (defaults, the five per-level UmbraLevel settings, the -1
// trigger resets) fill four. Only applied while sUmbra is still NULL:
// the object must be allocated with the new size or the tail does not
// exist. The old two-entry region simply goes unused.
struct umbra_disp {
  uint32_t rva;
  uint8_t off; // where the imm32/disp32 sits in the instruction
  uint32_t old_value;
  uint32_t new_value;
};

constexpr uint32_t umbra_params_new = 0x470220;
constexpr uint32_t umbra_trig_new = 0x470270;
constexpr uint32_t umbra_ptrig_new = 0x470280;
constexpr uint32_t umbra_params_delta = umbra_params_new - 0x12DC0C;
static_assert(umbra_params_new % 0x14 == 0,
              "the distance-scale setter indexes params as (lc + bias) * 0x14");

constexpr umbra_disp umbra_disps[] = {
    // params (field offsets 0x00..0x10 of each 0x14 entry)
    {0x01C8D417, 5, 0x12DC0C, 0x12DC0C + umbra_params_delta},
    {0x01C8D420, 5, 0x12DC10, 0x12DC10 + umbra_params_delta},
    {0x01C8D42F, 5, 0x12DC14, 0x12DC14 + umbra_params_delta},
    {0x01C8D43E, 5, 0x12DC18, 0x12DC18 + umbra_params_delta},
    {0x01C8D44D, 5, 0x12DC1C, 0x12DC1C + umbra_params_delta},
    {0x01C8DDDE, 5, 0x12DC10,
     0x12DC10 + umbra_params_delta}, // SetAccurateOcclusionThreshold
    {0x01C8E045, 5, 0x12DC14,
     0x12DC14 + umbra_params_delta}, // SetMinimumContributionThreshold
    {0x01C8EC55, 4, 0x12DC10, 0x12DC10 + umbra_params_delta},
    {0x01C8EFF6, 3, 0x12DC0C, umbra_params_new}, // defaults init
    {0x01C8F084, 3, 0x12DC0C, umbra_params_new}, // UmbraLevel settings x5
    {0x01C8F0D0, 3, 0x12DC0C, umbra_params_new},
    {0x01C8F11C, 3, 0x12DC0C, umbra_params_new},
    {0x01C8F16C, 3, 0x12DC0C, umbra_params_new},
    {0x01C8F1B3, 3, 0x12DC0C, umbra_params_new},
    {0x01C8E013, 2, 0xF167,
     umbra_params_new / 0x14}, // SetDistanceScale index bias
    // tome trigger
    {0x01C8C81D, 2, 0x12DC48, umbra_trig_new},
    {0x01C8CAFA, 3, 0x12DC48, umbra_trig_new},
    {0x01C8CBFE, 3, 0x12DC48, umbra_trig_new},
    {0x01C8CC72, 3, 0x12DC48, umbra_trig_new},
    {0x01C8DB20, 4, 0x12DC48, umbra_trig_new},
    {0x01C8DB68, 4, 0x12DC48, umbra_trig_new},
    {0x01C8F3FC, 3, 0x12DC48, umbra_trig_new},
    // persistent tome trigger
    {0x01C8CB3E, 3, 0x12DC50, umbra_ptrig_new},
    {0x01C8CBB2, 3, 0x12DC50, umbra_ptrig_new},
    {0x01C8DA64, 4, 0x12DC50, umbra_ptrig_new},
    {0x01C8F41F, 3, 0x12DC50, umbra_ptrig_new},
    // the allocation and its memset
    {0x01C8D345, 1, 0x470210, 0x470300},
    {0x01C8D35E, 2, 0x470210, 0x470300},
};

// init-loop end bounds: `lea reg, [base + disp8]`, disp8 at +3
struct umbra_bound {
  uint32_t rva;
  uint8_t old_value;
  uint8_t new_value;
};

constexpr umbra_bound umbra_bounds[] = {
    {0x01C8EFFD, 0x28, 0x50}, // defaults: 2 x 0x14 -> 4 x 0x14
    {0x01C8F08B, 0x28, 0x50},
    {0x01C8F0D7, 0x28, 0x50},
    {0x01C8F123, 0x28, 0x50},
    {0x01C8F173, 0x28, 0x50},
    {0x01C8F1BA, 0x28, 0x50},
    {0x01C8F403, 0x08, 0x10}, // tome triggers: 2 x 4 -> 4 x 4
    {0x01C8F426, 0x08, 0x10},
};

bool umbra_grown = false;

bool grow_umbra_client_arrays() {
  if (umbra_grown) {
    return true;
  }
  const auto b = base();
  const auto *object = reinterpret_cast<const uint64_t *>(b + 0x0AE15BF8);
  if (!readable(object, sizeof(*object)) || *object != 0) {
    note("[splitscreen] umbra: object already allocated - not grown\n");
    return false;
  }
  for (const auto &s : umbra_disps) {
    uint32_t have = 0;
    const auto *at = reinterpret_cast<const uint8_t *>(b + s.rva + s.off);
    if (!readable(at, sizeof(have))) {
      return false;
    }
    std::memcpy(&have, at, sizeof(have));
    if (have != s.old_value) {
      note("[splitscreen] umbra: 0x%08X holds 0x%X - nothing written\n", s.rva,
           have);
      return false;
    }
  }
  for (const auto &u : umbra_bounds) {
    const auto *at = reinterpret_cast<const uint8_t *>(b + u.rva + 3);
    if (!readable(at, 1) || *at != u.old_value) {
      note("[splitscreen] umbra: bound 0x%08X differs - nothing written\n",
           u.rva);
      return false;
    }
  }

  size_t done_disps = 0;
  size_t done_bounds = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < done_disps; ++i) {
      write_bytes(
          reinterpret_cast<void *>(b + umbra_disps[i].rva + umbra_disps[i].off),
          &umbra_disps[i].old_value, sizeof(uint32_t));
    }
    for (size_t i = 0; i < done_bounds; ++i) {
      write_bytes(reinterpret_cast<void *>(b + umbra_bounds[i].rva + 3),
                  &umbra_bounds[i].old_value, 1);
    }
  };
  for (const auto &s : umbra_disps) {
    if (!write_bytes(reinterpret_cast<void *>(b + s.rva + s.off), &s.new_value,
                     sizeof(uint32_t))) {
      rollback();
      return false;
    }
    ++done_disps;
  }
  for (const auto &u : umbra_bounds) {
    if (!write_bytes(reinterpret_cast<void *>(b + u.rva + 3), &u.new_value,
                     1)) {
      rollback();
      return false;
    }
    ++done_bounds;
  }
  umbra_grown = true;
  return true;
}

// LiveStats_ResetCache: `mov r8d, 0x8808` -> `mov r8d, 0x11010`, only
// when its lea already points at the moved array.
bool widen_statscache_reset(const size_t cache_new) {
  const auto b = base();
  auto *imm = reinterpret_cast<uint8_t *>(b + 0x01E95558);
  constexpr uint8_t imm_old[] = {0x41, 0xB8, 0x08, 0x88, 0x00, 0x00};
  constexpr uint8_t imm_new[] = {0x41, 0xB8, 0x10, 0x10, 0x01, 0x00};
  const auto *lea = reinterpret_cast<const uint8_t *>(b + 0x01E9554F);
  int32_t lea_disp = 0;
  std::memcpy(&lea_disp, lea + 3, sizeof(lea_disp));
  if (!cache_new || b + 0x01E95556 + lea_disp != cache_new ||
      !readable(imm, sizeof(imm_old)) ||
      std::memcmp(imm, imm_old, sizeof(imm_old)) != 0) {
    note("[splitscreen] statscache reset: bytes differ - not widened\n");
    return false;
  }
  return write_bytes(imm, imm_new, sizeof(imm_new));
}

void relocate_batch14() {
  for (size_t i = 0; i < std::size(batch14); ++i) {
    if (batch14_new[i]) {
      continue;
    }
    batch14_new[i] = relocate_perclient(batch14[i]);
    if (batch14_new[i]) {
      widen_statscache_reset(batch14_new[i]);
    }
  }
}

void relocate_batch13() {
  for (size_t i = 0; i < std::size(batch13); ++i) {
    if (batch13_new[i]) {
      continue;
    }
    batch13_new[i] = relocate_perclient(batch13[i]);
    if (batch13_new[i] && std::strcmp(batch13[i].name, "sunvol") == 0) {
      // the static initializer's per-element state: all zero but
      // the dword at +0x2BB0 (0x02DA62A5 `mov dword [rbx-0x838], -1`)
      for (size_t lc = 2; lc < 4; ++lc) {
        const uint32_t none = 0xFFFFFFFF;
        std::memcpy(reinterpret_cast<uint8_t *>(batch13_new[i]) + lc * 0x2BB8 +
                        0x2BB0,
                    &none, sizeof(none));
      }
    }
  }
}

// ---- LENS FLARES: off for local clients >= 2 (2026-09-26 15:51) -------
//
// Next crash after batch 11: 0xC0000005 NULL read at RVA 0x014B9397,
// r13 = r14 = 2. Chain: FX spawn -> FxLensFlaresManager::SpawnInstance
// (0x014BBD20, names itself in its error string) -> 0x014BAA60 ->
// 0x014B97C0 `state = this->perClient[lc]` (this+0xA058+lc*8) -> the
// instance-pool pop 0x014B9380 on state+0x3440, whose data is NULL.
//
// PS4 FxLensFlaresManager (cg_lensflare.cpp, 0xF50 bytes) holds eight
// per-client arrays of FOUR (persistentData, visible/rendered instance
// lists, source and instance pool memory, dynamic buffers). On the PC
// they are arrays of TWO embedded in one static object: +0xA058
// (persistent data), +0xB070 / +0xB080 (pool memory) and more. For lc 2
// every one of them reads or WRITES the neighbouring member
// (SetPersistentData 0x014BAC20 stores to this+0xA068). The object cannot
// grow, so porting the console layout means re-laying the whole class -
// not a relocation. Until then player 3 simply has no lens flares.
//
// The five entry points that index those members all take lc in edx
// (verified prologues, no rip-relative bytes stolen):
//   0x014BA7F0  per-client pool setup      (void)  -> ret
//   0x014BAC20  SetPersistentData          (void)  -> ret
//   0x014BB990  per-client update          (void)  -> ret
//   0x014BBD20  SpawnInstance              (int)   -> -1, its own
//                                                    failure value (0x014BC094)
//   0x014BC9C0  per-view render (renderer) (void)  -> ret
// Callers ignore eax after the void ones (checked at each call site).
// Each entry gets `jmp cave`; the cave does `cmp edx,2 / jl original`,
// the neutral return otherwise, and the stolen prologue + jmp back.
struct lc_gate {
  uint32_t rva;
  uint8_t prologue[9];
  uint8_t len;
  bool returns_minus_one;
};
constexpr lc_gate lensflare_gates[] = {
    {0x014BA810,
     {0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08},
     9,
     false},
    {0x014BAC40,
     {0x40, 0x55, 0x56, 0x57, 0x41, 0x54},
     6,
     false}, // 40 55 = push rbp (REX)
    {0x014BB9B0, {0x48, 0x8B, 0xC4, 0x57, 0x41, 0x54}, 6, false},
    {0x014BBD40, {0x48, 0x89, 0x4C, 0x24, 0x08}, 5, true},
    {0x014BC9E0, {0x48, 0x8B, 0xC4, 0x55, 0x53}, 5, false},
};
bool lensflare_gated = false;

void gate_lensflares_for_extra_clients() {
  if (lensflare_gated) {
    return;
  }
  const auto b = base();
  for (const auto &g : lensflare_gates) {
    const auto *at = reinterpret_cast<const uint8_t *>(b + g.rva);
    if (!readable(at, g.len) || std::memcmp(at, g.prologue, g.len) != 0) {
      trace_line l;
      l.str("lensflare gate: 0x");
      l.hex(g.rva);
      l.str(" prologue not stock - no gate installed");
      trace_write(l);
      return;
    }
  }
  uint32_t installed = 0;
  for (const auto &g : lensflare_gates) {
    auto *cave = static_cast<uint8_t *>(allocate_near_module(0x40));
    if (!cave) {
      break;
    }
    std::vector<uint8_t> c;
    c.insert(c.end(), {0x83, 0xFA, 0x02}); // cmp edx, 2
    c.insert(c.end(), {0x7C, 0x00});       // jl original (patched)
    const auto jl_at = c.size() - 1;
    if (g.returns_minus_one) {
      c.insert(c.end(), {0xB8, 0xFF, 0xFF, 0xFF, 0xFF}); // mov eax, -1
    }
    c.insert(c.end(), {0xC3}); // ret
    c[jl_at] = static_cast<uint8_t>(c.size() - (jl_at + 1));
    c.insert(c.end(), g.prologue, g.prologue + g.len);
    c.insert(c.end(), {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00}); // jmp [rip+0]
    const uint64_t back = b + g.rva + g.len;
    const auto *back_bytes = reinterpret_cast<const uint8_t *>(&back);
    c.insert(c.end(), back_bytes, back_bytes + 8);
    if (!write_bytes(cave, c.data(), c.size())) {
      break;
    }
    uint8_t patch[9];
    std::memset(patch, 0x90, sizeof(patch));
    patch[0] = 0xE9;
    const auto rel =
        static_cast<int32_t>(reinterpret_cast<size_t>(cave) - (b + g.rva + 5));
    std::memcpy(patch + 1, &rel, sizeof(rel));
    if (!write_bytes(reinterpret_cast<uint8_t *>(b + g.rva), patch, g.len)) {
      break;
    }
    ++installed;
  }
  lensflare_gated = installed == std::size(lensflare_gates);
  trace_line l;
  l.str("lensflare gate: ");
  l.dec(installed);
  l.str(" of ");
  l.dec(std::size(lensflare_gates));
  l.str(" entry points gated for local clients >= 2");
  trace_write(l);
}

// ---- QUIT HANG: lens-flare manager destructor at process exit (2026-09-26
// 20:50)
//
// Quitting from the menu hung the game ("froze") and then killed it: WER
// dump boiii.exe.4924.dmp, main thread in exit() -> onexit table ->
// thunk 0x02F788A0 (`lea rcx,[FxLensFlaresManager 0x0332DC10]; jmp
// 0x014BBC50`) -> Shutdown -> PMem_Free("LensFlareManager") ->
// Com_Error "attempting to PMem_Free 'LensFlareManager' but it is not
// at the top of the stack" -> the crash handler re-faulted until the
// stack overflowed (~970 frames). The PMem stack in the dump held only
// the twelve core zones: the manager's block had already gone with the
// frontend zone, but its "shut down" byte (+0xA050, written only by
// Init 0x014BB2FB and Shutdown 0x014BBD0C) still read 0, because the
// quit bypassed every FX_ShutdownLensFlareSystem caller (0x0135D330,
// 0x021492AD, 0x02149CED, 0x0214A3B0). The 19:47 exit crash was the
// destructor before this one in the table (the stringstream, fixed by
// the uiInfoArray move); this one was hidden behind it.
//
// At process exit the shutdown only returns memory the OS reclaims
// anyway, so the EXIT thunk returns at once. The level-end shutdown is
// untouched (it is reached through 0x014ED6A0, not this thunk).
constexpr uint32_t lensflare_exit_thunk_rva = 0x2EF9CD0;
constexpr uint8_t lensflare_exit_thunk_expected[] = {
    0x48, 0x8D, 0x0D, 0x79, 0x48, 0x3B, 0x00, // lea rcx, [rip -> 0x0332DC10]
    0xE9, 0xD4, 0x18, 0x5C, 0xFE,             // jmp 0x014BBC50
};

void skip_lensflare_exit_shutdown() {
  auto *at = reinterpret_cast<uint8_t *>(base() + lensflare_exit_thunk_rva);
  if (!readable(at, sizeof(lensflare_exit_thunk_expected)) ||
      std::memcmp(at, lensflare_exit_thunk_expected,
                  sizeof(lensflare_exit_thunk_expected)) != 0) {
    note("[splitscreen] lensflare exit thunk: bytes differ - left alone\n");
    return;
  }
  const uint8_t ret = 0xC3;
  if (write_bytes(at, &ret, 1)) {
    trace_line l;
    l.str("lensflare exit thunk: returns at process exit (quit hang)");
    trace_write(l);
  }
}

// ---- PER-CONTROLLER UI MODEL ROOTS 2..3 (2026-09-27) --------------------
//
// Pane 3 drew only the static HUD parts (gear frame, crosshair, name
// tags, "GAME OVER") - never ammo, weapon name, score list or the
// game-over scoreboard. The probe said why: Engine.GetModelForController(2)
// is nil, so every data-bound widget of controller 2 has no model to
// subscribe to.
//
// PS4 UI_Model_Init (0xD68140, called once from Com_Init) creates
//     s_controllerModel[i] = UI_Model_AllocateNode(global, "controller%d",
//     true)
// for FOUR controllers. It is the only writer of s_controllerModel;
// UI_Model_GetModelForController (0xD68A90) is the only reader.
// On the PC the getter is 0x02019660 (found through the Lua binding
// table: "GetModelForController" -> 0x01FAFB50 -> call 0x02019660) and
// reads uint16 s_controllerModel at 0x162EAFBC. Measured live: [0]=2,
// [1]=3, [2]=[3]=0, 0x162EAFC4 holds another word (0x030A). The PC
// init is hidden (no visible code references "controller%d" or
// "UIModelAllocator"), so its bound cannot be widened.
//
// Rule 4: find_lea over 0x162EAFB8..0x162EAFC8 finds the getter's lea,
// UI_Model_GetGlobalModel's load of 0x162EAFB8 (0x02019490) and six
// word accesses to 0x162EAFC4 - nothing touches 0x162EAFC0..C3, so
// slots 2 and 3 are padding. (Nearby hazard, NOT fixed here: the
// setupArmBladeTarget / setupRocketLauncherTarget tables at
// 0x162EAEE0 [2*4]x0x18 and 0x162EAED8 [2]x4 are indexed by
// localClientNum; PS4 sizes them for 4 local clients. lc 2 would
// write over 0x162EAFA0.. - the global model and these roots.)
//
// So the roots are produced the way the console produces them, not
// faked: UI_Model_CreatePersistentModelFromPath (0x02019080, the PC
// form of PS4 0xD68E30 = GetModelFromPath(parent, path, create=1,
// persistent=1), which for a one-segment path is exactly
// AllocateNode(global, "controllerN", true)). Persistent matters:
// UI_Shutdown -> UI_Model_ResetModelsAndSubscriptions frees every node
// without that bit, and a freed root would leave a stale handle here.
//
// WHEN: at the entry of Com_LocalClient_LastInput_Init (0x020EFA60).
// PS4 Com_Init runs UI_Model_Init (+0x10C) and then this function
// (+0x22C) exactly once each, on the main thread - so the pool exists,
// no other thread allocates, and the function's own lc loop (already
// widened to 3 by the signin relocation) then finds the new roots for
// "LastInput"/"ControllerType". The cave creates a root only while the
// global model is non-zero (init has run) and the slot is still zero
// (idempotent if the function ever runs again).
constexpr uint32_t lastinput_init_rva =
    0x20E32E0; // Com_LocalClient_LastInput_Init
constexpr uint8_t lastinput_init_expected[] = {
    0x48, 0x89, 0x5C, 0x24, 0x10, // mov [rsp+0x10], rbx
};
constexpr uint32_t ui_controller_model_getter_rva = 0x200CEE0;
constexpr uint8_t ui_controller_model_getter_expected[] = {
    0x48, 0x63, 0xC1,                         // movsxd rax, ecx
    0x48, 0x8D, 0x0D, 0x92, 0xEA, 0x25, 0x14, // lea rcx, [rip -> 0x162EAFBC]
    0x0F, 0xB7, 0x04, 0x41,                   // movzx eax, word [rcx+rax*2]
    0xC3,
};
constexpr uint32_t ui_global_model_getter_rva = 0x200CD10;
constexpr uint8_t ui_global_model_getter_expected[] = {
    0x0F, 0xB7, 0x05, 0x61,
    0xEC, 0x25, 0x14, // movzx eax, word [rip -> 0x162EAFB8]
    0xC3,
};
constexpr uint32_t ui_create_persistent_rva = 0x200C900;
constexpr uint8_t ui_create_persistent_expected[] = {
    0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, // prologue
    0x57, 0x48, 0x83, 0xEC, 0x70,
};
constexpr uint32_t ui_create_persistent_alloc_rva = 0x200C965;
constexpr uint8_t ui_create_persistent_alloc_expected[] = {
    0x48, 0x8D, 0x54, 0x24, 0x20, // lea rdx, [rsp+0x20]  (key)
    0x41, 0xB0, 0x01,             // mov r8b, 1           (persistent)
    0x0F, 0xB7, 0xCF,             // movzx ecx, di        (parent)
    0xE8, 0xCB, 0xFC, 0xFF, 0xFF, // call AllocateNode 0x02018DC0
};
constexpr uint32_t ui_global_model_rva = 0x1626C038;
constexpr uint32_t ui_controller_model_rva = 0x1626C03C; // uint16[2] on the PC
bool controller_models_hooked = false;

void create_extra_controller_models() {
  if (controller_models_hooked) {
    return;
  }
  const auto b = base();
  const struct {
    uint32_t rva;
    const uint8_t *bytes;
    size_t len;
  } checks[] = {
      {lastinput_init_rva, lastinput_init_expected,
       sizeof(lastinput_init_expected)},
      {ui_controller_model_getter_rva, ui_controller_model_getter_expected,
       sizeof(ui_controller_model_getter_expected)},
      {ui_global_model_getter_rva, ui_global_model_getter_expected,
       sizeof(ui_global_model_getter_expected)},
      {ui_create_persistent_rva, ui_create_persistent_expected,
       sizeof(ui_create_persistent_expected)},
      {ui_create_persistent_alloc_rva, ui_create_persistent_alloc_expected,
       sizeof(ui_create_persistent_alloc_expected)},
  };
  for (const auto &c : checks) {
    const auto *p = reinterpret_cast<const uint8_t *>(b + c.rva);
    if (!readable(p, c.len)) {
      note(
          "[splitscreen] controller models: 0x%08X not readable - not hooked\n",
          c.rva);
      return;
    }
    if (std::memcmp(p, c.bytes, c.len) == 0) {
      continue;
    }
    // UI_CreatePersistent is only CALLED from the cave below, never
    // patched. BOIII 1.1.0.1445 hooks its entry with `jmp rel32`
    // (measured 2026-09-28 in the game build 0x06517980 dump), so
    // accept exactly that: a 5-byte jmp over the first instruction
    // and every later expected byte unchanged. The call then runs
    // through BOIII's hook into the original, as it does for the game.
    const bool client_hooked_entry =
        c.rva == ui_create_persistent_rva && c.len > 5 && p[0] == 0xE9 &&
        std::memcmp(p + 5, c.bytes + 5, c.len - 5) == 0;
    if (!client_hooked_entry) {
      note("[splitscreen] controller models: bytes differ at 0x%08X - not "
           "hooked\n",
           c.rva);
      return;
    }
  }
  const auto *slots =
      reinterpret_cast<const uint16_t *>(b + ui_controller_model_rva);
  if (!readable(slots, 8) || slots[2] != 0 || slots[3] != 0) {
    note("[splitscreen] controller models: slots 2/3 not zero - not hooked\n");
    return;
  }

  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x100));
  if (!cave) {
    return;
  }
  const auto put64 = [](std::vector<uint8_t> &v, uint64_t x) {
    const auto *p = reinterpret_cast<const uint8_t *>(&x);
    v.insert(v.end(), p, p + 8);
  };
  std::vector<uint8_t> c;
  c.insert(c.end(), {0x48, 0x83, 0xEC, 0x28}); // sub rsp, 0x28
  size_t name_fixups[2] = {};
  for (int slot = 2; slot <= 3; ++slot) {
    const uint64_t slot_va = b + ui_controller_model_rva + slot * 2;
    c.insert(c.end(), {0x48, 0xB8});
    put64(c, slot_va);                           // mov rax, &slot
    c.insert(c.end(), {0x66, 0x83, 0x38, 0x00}); // cmp word [rax], 0
    const size_t jne_at = c.size();
    c.insert(c.end(), {0x75, 0x00}); // jne next
    c.insert(c.end(), {0x48, 0xB8});
    put64(c, b + ui_global_model_rva);     // mov rax, &global
    c.insert(c.end(), {0x0F, 0xB7, 0x08}); // movzx ecx, word [rax]
    c.insert(c.end(), {0x85, 0xC9});       // test ecx, ecx
    const size_t jz_at = c.size();
    c.insert(c.end(), {0x74, 0x00}); // jz next
    c.insert(c.end(), {0x48, 0xBA}); // mov rdx, name
    name_fixups[slot - 2] = c.size();
    put64(c, 0);
    c.insert(c.end(), {0x48, 0xB8});
    put64(c, b + ui_create_persistent_rva); // mov rax, create
    c.insert(c.end(), {0xFF, 0xD0});        // call rax
    c.insert(c.end(), {0x48, 0xB9});
    put64(c, slot_va);                     // mov rcx, &slot
    c.insert(c.end(), {0x66, 0x89, 0x01}); // mov word [rcx], ax
    const size_t next = c.size();
    c[jne_at + 1] = static_cast<uint8_t>(next - (jne_at + 2));
    c[jz_at + 1] = static_cast<uint8_t>(next - (jz_at + 2));
  }
  c.insert(c.end(), {0x48, 0x83, 0xC4, 0x28}); // add rsp, 0x28
  c.insert(c.end(), lastinput_init_expected,
           lastinput_init_expected +
               sizeof(lastinput_init_expected));           // replayed
  c.insert(c.end(), {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00}); // jmp [rip+0]
  put64(c, b + lastinput_init_rva + sizeof(lastinput_init_expected));
  static const char names[] = "controller2\0controller3";
  for (int i = 0; i < 2; ++i) {
    const uint64_t at = reinterpret_cast<uint64_t>(cave) + c.size();
    std::memcpy(c.data() + name_fixups[i], &at, sizeof(at));
    const char *s = names + i * 12;
    c.insert(c.end(), s, s + std::strlen(s) + 1);
  }
  if (c.size() > 0x100 || !write_bytes(cave, c.data(), c.size())) {
    return;
  }

  uint8_t patch[sizeof(lastinput_init_expected)];
  patch[0] = 0xE9;
  const auto rel = static_cast<int32_t>(reinterpret_cast<size_t>(cave) -
                                        (b + lastinput_init_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));
  if (!write_bytes(reinterpret_cast<uint8_t *>(b + lastinput_init_rva), patch,
                   sizeof(patch))) {
    return;
  }
  controller_models_hooked = true;
  trace_line l;
  l.str(
      "controller models: roots 2/3 created at Com_LocalClient_LastInput_Init");
  trace_write(l);
}

// ---- PLAYER 3'S BUTTON MODELS: the console join reads them (2026-09-27) --
//
// Stock CoDMenu.lua subscribes an anyControllerAllowed menu to
// "ButtonBits.<button>" of controllers 0..GetMaxLocalControllers()-1 and
// turns an unused controller's press into "unused_gamepad_button" ->
// LobbyAddLocalClient. Measured: controller 1's A joined that way;
// controller 2's A reached UIRootFull as a gamepad_button (hud_probe
// "c2 primary") and HandleButtonPress never ran - GetModel(controller2,
// "ButtonBits.1") is nil, so the subscription is on nothing.
//
// PS4: CL_InitGamepadModels (0x3FF160, once from Com_Init right after
// Com_LocalClient_LastInput_Init) loops lc 0..3, controller =
// Com_LocalClient_GetControllerIndex(lc), and creates RightStick.* into
// s_rightStickModels[c] and ButtonBits.1..N + KeyPressBits into
// s_gamepadButtons[c]. The only writer of the values is UI_CoD_KeyEvent ->
// CL_ModelForButton(controller, button) (0x3FF130), no bound of its own.
//
// PC: the same function at 0x013401B0, loop `cmp esi, 2` at 0x01340319.
// Rule 4, every array the loop touches for lc 2:
//   s_rightStickModels  moved to [4] by batch2 ("rightstick")
//   s_gamepadButtons    moved to [4] by batch2 ("gamepadbuttons") - slot 2
//                       holds static list nodes (see gamepadbuttons_sites);
//                       the first build widened without moving it and
//                       died at launch, twice
//   s_controllerModel   root 2 created by create_extra_controller_models
//   controller index    seat record 2 holds lc 2 / controller 2 from the
//                       signin relocation (write_signin_seat), before
//                       Com_Init - the same fact LastInput_Init's widened
//                       loop already relies on.
// 3, not 4: lc 3 has no seat record. Applied only when all three
// preconditions held; otherwise stock (player 3 joins by the component).
constexpr uint32_t gamepad_models_bound_rva = 0x1340339;
constexpr uint8_t gamepad_models_bound_expected[] = {
    0x83, 0xFE, 0x02,                   // cmp esi, 2
    0x0F, 0x8C, 0xBE, 0xFE, 0xFF, 0xFF, // jl 0x013401E0
};

bool gamepad_models_widened = false;

void widen_gamepad_button_models() {
  if (!batch2_new[1] || !batch2_new[2] || !signin_relocated ||
      !controller_models_hooked) {
    trace_line l;
    l.str("gamepad button models: stock, missing rightstick=");
    l.dec(batch2_new[1] ? 1 : 0);
    l.str(" buttons=");
    l.dec(batch2_new[2] ? 1 : 0);
    l.str(" signin=");
    l.dec(signin_relocated ? 1 : 0);
    l.str(" roots=");
    l.dec(controller_models_hooked ? 1 : 0);
    trace_write(l);
    return;
  }
  auto *p = reinterpret_cast<uint8_t *>(base() + gamepad_models_bound_rva);
  if (!readable(p, sizeof(gamepad_models_bound_expected)) ||
      std::memcmp(p, gamepad_models_bound_expected,
                  sizeof(gamepad_models_bound_expected)) != 0) {
    trace_line l;
    l.str("gamepad button models: stock, bytes differ at 0x01340319");
    trace_write(l);
    return;
  }
  // Four since player 4 (2026-09-27): seat record 3, model root 3 and
  // s_gamepadButtons/s_rightStickModels [4] all exist.
  const uint8_t four = 0x04;
  if (write_bytes(p + 2, &four, 1)) {
    gamepad_models_widened = true;
    trace_line l;
    l.str("gamepad button models: CL_InitGamepadModels bound 2 -> 4");
    trace_write(l);
  }
}

// ---- lobby_maxLocalPlayers: the console's own room rule (2026-09-27) ----
//
// With controller 2's button models in place his A ran the stock path as
// far as LobbyAddLocalClient (splitscreen_lua.txt "LobbyAddLocalClient c2")
// and stopped at `GetLobbyLocalClientCount < Dvar.lobby_maxLocalPlayers`
// (actions.lua; SplitscreenLobbyRoomAvailable has the same rule). The
// offline ModeSelect calls Lobby_SetMaxLocalPlayers(4), which clamps to
// GetMaxLocalControllers() (3 here) - but the DVAR RANGE refuses it:
//   PS4 LobbyConfig_Init 0xCC0437  Dvar_RegisterInt(.., 1, 1, 4, ..)
//   PC  0x01EE8872  mov ecx, 0x44D2F069 (hash) ; default rbx+2 = 2,
//       min rbx+1 = 1 ; 0x01EE8882 mov dword [rsp+0x20], 2  <- max
// Max 2 -> 3 (the seats). No native code reads the dvar (dvar_users.py:
// 0 readers of its global 0x1574D3B8) - only that Lua, which then applies
// the console's own numbers. Default stays 2.
constexpr uint32_t lobby_max_local_reg_rva = 0x1EDBF02;
constexpr uint8_t lobby_max_local_reg_expected[] = {
    0xB9, 0x69, 0xF0, 0xD2, 0x44,             // mov ecx, hash
    0x89, 0x5C, 0x24, 0x28,                   // mov [rsp+0x28], ebx  flags
    0x48, 0x89, 0x05, 0x6E, 0x1E, 0x7F, 0x13, // mov [rip+..], rax
    0xC7, 0x44, 0x24, 0x20, 0x02, 0x00, 0x00,
    0x00, // mov dword [rsp+0x20], 2  max
};
constexpr size_t lobby_max_local_max_off = 20;

void widen_lobby_max_local_players() {
  if (!gamepad_models_widened) {
    return; // no stock join for controller 2 - keep the stock range
  }
  auto *p = reinterpret_cast<uint8_t *>(base() + lobby_max_local_reg_rva);
  trace_line l;
  if (!readable(p, sizeof(lobby_max_local_reg_expected)) ||
      std::memcmp(p, lobby_max_local_reg_expected,
                  sizeof(lobby_max_local_reg_expected)) != 0) {
    l.str("lobby_maxLocalPlayers: stock, bytes differ at 0x01EE8872");
    trace_write(l);
    return;
  }
  const uint8_t four = 0x04; // PS4's own maximum; 4 seats since player 4
  if (write_bytes(p + lobby_max_local_max_off, &four, 1)) {
    l.str("lobby_maxLocalPlayers: range max 2 -> 4");
    trace_write(l);
  }
}

// ---- SUN SHADOW: a view's slice set must exist (2026-09-26 19:20) --------
//
// Same crash at 18:52 and 19:12, the first frame all three views drew
// after the loading screen: 0xC0000005 READ 0x8 inside d3d11
// OMSetRenderTargets on the render thread. Unwound from the minidump
// with tools/dump_unwind.py:
//   RB_SunShadowMaps 0x01C7C330 -> partition draw 0x01C7BFC0
//   -> R_SetRenderTarget 0x01CFEE70(state, rt [viewInfo+0x1F18] = 5,
//                                   slice [viewInfo+0x1668] + partition = 6)
//   -> 0x01D018B0: dsv = rt 5's view array [6]
// Render targets 5 (sun shadow depth) and 9 (its colour twin, id at
// viewInfo+0x1F1A, drawn by 0x01C59C50 with the same slice base) are
// created with a FIXED 6 slices (0x01CDD67D `mov r8d,6`, descriptor
// +0x14) = 3 partitions (RB_SunShadowMaps loop bound, 0x01C7C3CD
// `cmp ebx,3`) for 2 views. Measured live at the menu: rt 5's view array
// holds 6 depth-stencil views (vtable d3d11+0x1E09C0) followed by 6
// views of another type (d3d11+0x1D7870), and the crashing r14 was
// exactly entry 6 of it. Rt 9 keeps its render-target views INLINE -
// eight slots at +8, R_SetRenderTarget clamps that index to 7 - so the
// targets cannot simply be created with 9 or 12 slices.
//
// The slot is picked in the sun-shadow setup 0x01D19B60 as
//     min(max(CL_SplitscreenPlayerCount(), 1) - 1, viewInfo+0x398)
// with viewInfo+0x398 = the view's localClientNum (written at
// 0x01CECAB4). The engine's own domain for splitscreen_playerCount is
// 1..2, so that clamp always stayed inside the 2 slots; with the dvar
// at 3 (the allocator needs it, see hold_splitscreen_player_count)
// player 3 got slot 2 = slices 6..8, which do not exist.
//
// So the slot is also bounded by the slots the targets own (slices /
// partitions - 1, read from the two verified instructions, not
// assumed): player 3 draws into slot 1. PS4 needs no slots at all -
// R_DrawSunShadowMapCallback renders every view into slices 0..2 - and
// the PC's per-view render 0x01C685A0 draws a view's shadow partitions
// as its first pass. What stays open: if sun-shadow caching reuses a
// slot's content across frames, players 2 and 3 share that cache - a
// possible shadow artifact, never an out-of-bounds view.
//
// 15 bytes at 0x01D19CF0 (straight-line, no branch lands inside, flags
// dead at 0x01D19CFF) become `jmp cave` + NOPs; the cave replays them
// and adds the bound.
constexpr uint32_t sun_slot_site_rva = 0x1D0D920;
constexpr uint8_t sun_slot_site_expected[] = {
    0x41, 0x8B, 0x85, 0x98, 0x03, 0x00, 0x00, // mov eax, [r13+0x398]
    0x3B, 0xC8,                               // cmp ecx, eax
    0x0F, 0x4D, 0xC8,                         // cmovge ecx, eax
    0x89, 0x4D, 0x14,                         // mov [rbp+0x14], ecx
};
constexpr uint32_t sun_slices_rva = 0x1CD12AD;     // mov r8d, <slices>
constexpr uint32_t sun_partitions_rva = 0x1C6FFFD; // cmp ebx, <partitions>
bool sun_slot_clamped = false;

// ---- SUN SHADOW: one slot per view, 4 views (2026-09-28) -----------------
//
// Goal: separate sun shadows for every view ("4 suns").
// With the clamp above players 2-4 share slot 1 and each view's shadow
// render overwrites the others' (burst test, and blocky shadow patches in
// MP). PS4 (RULE ZERO) creates SHADOWMAP_SUN / _TRANS with 3 slices and
// renders every view into them, finishing one view before the next; the
// PC does not (the all-in-slot-0 experiment, 4eb3731, spread the
// cross-talk to player 1), so the PC way is kept and extended: 4 slots x 3
// partitions = 12 slices for RT 5 (SUN_1P, depth) and RT 9 (TRANS_1P,
// colour; its depth views are RT 5's, shared at 0x01CD6066).
//   Descriptors (R_InitRenderTargets, stack table at rbp+0xCF0, stride
//   0x2C = PS4 GfxRenderTargetConfiguration): RT 5 slices+colorFormat
//   `mov [rbp+0xD04],r8` 0x01CD1357, RT 9 slices `mov [rbp+0xD5C],r8d`
//   0x01CD140A. Both take r8 = the `mov r8d,6` at 0x01CD12AD, which also
//   stores RT 6's id (0x01CD138B) - so the immediate stays and the two
//   stores are replaced by calls into caves that store 12.
//   Depth: every per-slice array is allocated from the slice count
//   (0x01CD0D70, 8 sets x slices; DSVs 0x01CD5570, SRVs 0x01CD5680), so
//   RT 5 needs nothing else. Live at the menu the only multi-slice
//   targets are 5 (6), 7 SPOT_ARRAY (12, depth only), 8 OMNI_ARRAY (144,
//   depth only) and 9 (6, colour).
//   Colour: 0x01CD53D0 creates per-slice RTVs only for 2..7 slices
//   (`cmp ax,5 / ja` 0x01CD5449) into the 8 inline slots of the view set
//   (set+8 .. set+0x40; set+0x48 is the depth-view array pointer), and
//   R_SetRenderTargetSlice 0x01CF54E0 clamps the colour index to 7. So:
//   the check admits 2..12, the creation loop bound (0x01CD5494) becomes
//   min(slices, 8), slices 8..11 get RTVs in a sidecar made here from
//   slice 0's own desc, and the setter takes the sidecar view for RT 9
//   slices 8..11 (falls back to the stock clamp while one is missing).
//   Only RT 9 has colour and more than one slice, so the widened check
//   changes no other target. Readers of the current slice (+0x80C8,
//   all 8 sites): the setter above, and the clear 0x01CF33A0, which for
//   a multi-slice target calls ClearRenderTargetView(inline[slice]) at
//   0x01CF367D WITHOUT a clamp - the first 12-slice build (401d038)
//   crashed there in d3d11 (READ 0, inline[9] is past the 8 slots).
//   That site gets the same sidecar pick. The rest index depth only.
// Applied in try_apply before R_Init (the descriptors are built there).
constexpr uint32_t sun_desc_rt5_rva = 0x1CD1357;
constexpr uint8_t sun_desc_rt5_stock[] = {0x4C, 0x89, 0x85, 0x04,
                                          0x0D, 0x00, 0x00};
constexpr uint32_t sun_desc_rt9_rva = 0x1CD140A;
constexpr uint8_t sun_desc_rt9_stock[] = {0x44, 0x89, 0x85, 0x5C,
                                          0x0D, 0x00, 0x00};
constexpr uint32_t sun_view_check_rva = 0x1CD5449; // cmp ax,5
constexpr uint8_t sun_view_check_stock[] = {0x66, 0x83, 0xF8, 0x05};
constexpr uint32_t sun_view_loop_rva = 0x1CD5494; // movzx eax,[rsi+0xA86]
constexpr uint8_t sun_view_loop_stock[] = {0x0F, 0xB7, 0x86, 0x86,
                                           0x0A, 0x00, 0x00};
constexpr uint32_t sun_setter_rva = 0x1CF550D; // colour index clamp
constexpr uint8_t sun_setter_stock[] = {
    0xB9, 0x07, 0x00, 0x00, 0x00, 0x3B, 0xD9, 0x44, 0x8B, 0xCB,
    0x0F, 0xB7, 0xD6, 0x44, 0x0F, 0x4D, 0xC9, 0x33, 0xC9, 0x45,
    0x85, 0xC9, 0x44, 0x0F, 0x4E, 0xC9, 0x4E, 0x8B, 0x04, 0xC8,
};
constexpr uint32_t sun_clear_rva =
    0x1CF367D; // colour clear of the current slice
constexpr uint8_t sun_clear_stock[] = {
    0x8B, 0x93, 0xC8, 0x80, 0x00, 0x00, // mov edx,[rbx+0x80C8]   current slice
    0x48, 0x8B, 0x07,                   // mov rax,[rdi]
    0x4C, 0x8B, 0xC6,                   // mov r8,rsi
    0x48, 0x8B, 0x54, 0xD5, 0x00,       // mov rdx,[rbp+rdx*8]    inline[slice]
    0x48, 0x8B, 0xCF,                   // mov rcx,rdi
}; // then call [rax+0x190] (ClearRenderTargetView)
constexpr uint32_t rt_records_ptr_rva =
    0xFBBE758; // GfxRenderTarget records, stride 0xAE0
constexpr uint32_t rt_record_stride = 0xAE0;
constexpr uint32_t sun_trans_rt = 9;
constexpr uint32_t sun_slices_wanted = 12;
bool sun_slices_grown = false;
const char *sun_grow_result = "sun shadow 12 slices: not attempted";
ID3D11RenderTargetView *sun_trans_extra[4] =
    {}; // RT 9 slices 8..11 (read by the setter cave)
ID3D11RenderTargetView *sun_trans_retired[4] = {}; // released one tick later
void *sun_trans_seen_v0 = nullptr;

bool grow_sun_shadow_slices() {
  if (sun_slices_grown) {
    return true;
  }
  const auto b = base();
  const auto matches = [&](uint32_t rva, const uint8_t *stock, size_t n) {
    const auto *p = reinterpret_cast<const void *>(b + rva);
    return readable(p, n) && std::memcmp(p, stock, n) == 0;
  };
  if (!matches(sun_desc_rt5_rva, sun_desc_rt5_stock,
               sizeof(sun_desc_rt5_stock)) ||
      !matches(sun_desc_rt9_rva, sun_desc_rt9_stock,
               sizeof(sun_desc_rt9_stock)) ||
      !matches(sun_view_check_rva, sun_view_check_stock,
               sizeof(sun_view_check_stock)) ||
      !matches(sun_view_loop_rva, sun_view_loop_stock,
               sizeof(sun_view_loop_stock)) ||
      !matches(sun_setter_rva, sun_setter_stock, sizeof(sun_setter_stock)) ||
      !matches(sun_clear_rva, sun_clear_stock, sizeof(sun_clear_stock))) {
    sun_grow_result = "sun shadow 12 slices: NOT applied - bytes differ";
    return false;
  }
  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x180));
  if (!cave) {
    sun_grow_result = "sun shadow 12 slices: NOT applied - allocation failed";
    return false;
  }
  const uint8_t n = static_cast<uint8_t>(sun_slices_wanted);
  // +0x00: mov qword [rbp+0xD04], 12 ; ret      (RT 5 slices, colorFormat 0)
  const uint8_t c_rt5[] = {0x48, 0xC7, 0x85, 0x04, 0x0D, 0x00,
                           0x00, n,    0x00, 0x00, 0x00, 0xC3};
  // +0x10: mov dword [rbp+0xD5C], 12 ; ret      (RT 9 slices)
  const uint8_t c_rt9[] = {0xC7, 0x85, 0x5C, 0x0D, 0x00, 0x00,
                           n,    0x00, 0x00, 0x00, 0xC3};
  // +0x20: movzx eax,[rsi+0xA86] ; cmp eax,8 ; jbe +5 ; mov eax,8 ; ret
  const uint8_t c_loop[] = {0x0F, 0xB7, 0x86, 0x86, 0x0A, 0x00,
                            0x00, 0x83, 0xF8, 0x08, 0x76, 0x05,
                            0xB8, 0x08, 0x00, 0x00, 0x00, 0xC3};
  // +0x40: the setter's colour pick.
  std::vector<uint8_t> s;
  s.insert(s.end(), {0x83, 0xFB, 0x08, 0x7C, 0x00}); // cmp ebx,8 ; jl orig
  const size_t j1 = s.size() - 1;
  s.insert(s.end(), {0x83, 0xFB, 0x0C, 0x73, 0x00}); // cmp ebx,12 ; jae orig
  const size_t j2 = s.size() - 1;
  s.insert(s.end(), {0x66, 0x83, 0xFE, static_cast<uint8_t>(sun_trans_rt), 0x75,
                     0x00}); // cmp si,9 ; jne orig
  const size_t j3 = s.size() - 1;
  s.insert(s.end(), {0x49, 0xB8}); // mov r8, &sun_trans_extra
  {
    const auto a = reinterpret_cast<uint64_t>(&sun_trans_extra[0]);
    const auto *p = reinterpret_cast<const uint8_t *>(&a);
    s.insert(s.end(), p, p + 8);
  }
  s.insert(s.end(), {0x4D, 0x8B, 0x44, 0xD8, 0xC0}); // mov r8,[r8+rbx*8-0x40]
  s.insert(s.end(), {0x4D, 0x85, 0xC0, 0x75, 0x00}); // test r8,r8 ; jnz done
  const size_t j4 = s.size() - 1;
  const size_t orig = s.size();
  s.insert(s.end(), sun_setter_stock,
           sun_setter_stock + sizeof(sun_setter_stock));
  const size_t done = s.size();
  s.insert(s.end(),
           {0x0F, 0xB7, 0xD6, 0x33, 0xC9}); // movzx edx,si ; xor ecx,ecx
  s.insert(s.end(), {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00}); // jmp [rip+0]
  {
    const uint64_t back = b + sun_setter_rva + sizeof(sun_setter_stock);
    const auto *p = reinterpret_cast<const uint8_t *>(&back);
    s.insert(s.end(), p, p + 8);
  }
  s[j1] = static_cast<uint8_t>(orig - (j1 + 1));
  s[j2] = static_cast<uint8_t>(orig - (j2 + 1));
  s[j3] = static_cast<uint8_t>(orig - (j3 + 1));
  s[j4] = static_cast<uint8_t>(done - (j4 + 1));
  // +0x100: the clear's colour pick (rbp = view set, rbx = cmd state,
  // rdi = context). Sidecar for RT 9 slices 8..11, otherwise the stock
  // inline[slice] with the index clamped to 7 like the setter.
  std::vector<uint8_t> k;
  k.insert(k.end(),
           {0x8B, 0x93, 0xC8, 0x80, 0x00, 0x00});    // mov edx,[rbx+0x80C8]
  k.insert(k.end(), {0x83, 0xFA, 0x08, 0x7C, 0x00}); // cmp edx,8 ; jl orig
  const size_t k1 = k.size() - 1;
  k.insert(k.end(), {0x83, 0xFA, 0x0C, 0x73, 0x00}); // cmp edx,12 ; jae clamp
  const size_t k2 = k.size() - 1;
  k.insert(k.end(), {0x66, 0x83, 0xBB, 0xC0, 0x80, 0x00, 0x00,
                     static_cast<uint8_t>(sun_trans_rt)});
  k.insert(k.end(), {0x75, 0x00}); // cmp word [rbx+0x80C0],9 ; jne clamp
  const size_t k3 = k.size() - 1;
  k.insert(k.end(), {0x48, 0xB8}); // mov rax, &sun_trans_extra
  {
    const auto a = reinterpret_cast<uint64_t>(&sun_trans_extra[0]);
    const auto *p = reinterpret_cast<const uint8_t *>(&a);
    k.insert(k.end(), p, p + 8);
  }
  k.insert(k.end(), {0x48, 0x8B, 0x54, 0xD0, 0xC0}); // mov rdx,[rax+rdx*8-0x40]
  k.insert(k.end(), {0x48, 0x85, 0xD2, 0x75, 0x00}); // test rdx,rdx ; jnz done
  const size_t k4 = k.size() - 1;
  const size_t kclamp = k.size();
  k.insert(k.end(), {0xBA, 0x07, 0x00, 0x00, 0x00}); // mov edx,7
  const size_t korig = k.size();
  k.insert(k.end(), {0x48, 0x8B, 0x54, 0xD5, 0x00}); // mov rdx,[rbp+rdx*8]
  const size_t kdone = k.size();
  k.insert(k.end(), {0x48, 0x8B, 0x07, 0x4C, 0x8B, 0xC6, 0x48, 0x8B,
                     0xCF}); // rax, r8, rcx as stock
  k.insert(k.end(), {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00}); // jmp [rip+0]
  {
    const uint64_t back = b + sun_clear_rva + sizeof(sun_clear_stock);
    const auto *p = reinterpret_cast<const uint8_t *>(&back);
    k.insert(k.end(), p, p + 8);
  }
  k[k1] = static_cast<uint8_t>(korig - (k1 + 1));
  k[k2] = static_cast<uint8_t>(kclamp - (k2 + 1));
  k[k3] = static_cast<uint8_t>(kclamp - (k3 + 1));
  k[k4] = static_cast<uint8_t>(kdone - (k4 + 1));
  if (0x40 + s.size() > 0x100 || 0x100 + k.size() > 0x180 ||
      !write_bytes(cave + 0x0, c_rt5, sizeof(c_rt5)) ||
      !write_bytes(cave + 0x10, c_rt9, sizeof(c_rt9)) ||
      !write_bytes(cave + 0x20, c_loop, sizeof(c_loop)) ||
      !write_bytes(cave + 0x40, s.data(), s.size()) ||
      !write_bytes(cave + 0x100, k.data(), k.size())) {
    sun_grow_result = "sun shadow 12 slices: NOT applied - cave write failed";
    return false;
  }

  // Sites, all-or-nothing. The setter first: with no sidecar views yet it
  // behaves exactly like the stock clamp.
  const auto call_site = [&](uint32_t rva, size_t len, size_t cave_off,
                             uint8_t *out) {
    std::memset(out, 0x90, len);
    out[0] = 0xE8;
    const auto rel = static_cast<int32_t>(
        reinterpret_cast<size_t>(cave + cave_off) - (b + rva + 5));
    std::memcpy(out + 1, &rel, sizeof(rel));
  };
  uint8_t p_setter[sizeof(sun_setter_stock)];
  std::memset(p_setter, 0x90, sizeof(p_setter));
  p_setter[0] = 0xE9;
  {
    const auto rel = static_cast<int32_t>(
        reinterpret_cast<size_t>(cave + 0x40) - (b + sun_setter_rva + 5));
    std::memcpy(p_setter + 1, &rel, sizeof(rel));
  }
  uint8_t p_clear[sizeof(sun_clear_stock)];
  std::memset(p_clear, 0x90, sizeof(p_clear));
  p_clear[0] = 0xE9;
  {
    const auto rel = static_cast<int32_t>(
        reinterpret_cast<size_t>(cave + 0x100) - (b + sun_clear_rva + 5));
    std::memcpy(p_clear + 1, &rel, sizeof(rel));
  }
  uint8_t p_loop[sizeof(sun_view_loop_stock)];
  call_site(sun_view_loop_rva, sizeof(p_loop), 0x20, p_loop);
  const uint8_t p_check[] = {0x66, 0x83, 0xF8,
                             static_cast<uint8_t>(sun_slices_wanted - 2)};
  uint8_t p_rt5[sizeof(sun_desc_rt5_stock)];
  call_site(sun_desc_rt5_rva, sizeof(p_rt5), 0x00, p_rt5);
  uint8_t p_rt9[sizeof(sun_desc_rt9_stock)];
  call_site(sun_desc_rt9_rva, sizeof(p_rt9), 0x10, p_rt9);

  struct w {
    uint32_t rva;
    const uint8_t *stock;
    const uint8_t *patch;
    size_t n;
  };
  const w writes[] = {
      {sun_setter_rva, sun_setter_stock, p_setter, sizeof(p_setter)},
      {sun_clear_rva, sun_clear_stock, p_clear, sizeof(p_clear)},
      {sun_view_loop_rva, sun_view_loop_stock, p_loop, sizeof(p_loop)},
      {sun_view_check_rva, sun_view_check_stock, p_check, sizeof(p_check)},
      {sun_desc_rt5_rva, sun_desc_rt5_stock, p_rt5, sizeof(p_rt5)},
      {sun_desc_rt9_rva, sun_desc_rt9_stock, p_rt9, sizeof(p_rt9)},
  };
  size_t done_w = 0;
  for (const auto &x : writes) {
    if (!write_bytes(reinterpret_cast<void *>(b + x.rva), x.patch, x.n)) {
      for (size_t k = 0; k < done_w; ++k) {
        write_bytes(reinterpret_cast<void *>(b + writes[k].rva),
                    writes[k].stock, writes[k].n);
      }
      sun_grow_result = "sun shadow 12 slices: NOT applied - a site write "
                        "failed (rolled back)";
      return false;
    }
    ++done_w;
  }
  sun_slices_grown = true;
  sun_grow_result = "sun shadow: RT 5/9 6 -> 12 slices (4 view slots), RT 9 "
                    "slices 8..11 via sidecar views";
  return true;
}

// Renderer loop: RT 9's views for slices 8..11, remade whenever the target
// is (re)created (slice 0's view pointer changes). Old views are released
// one tick later, never while the render thread may still pick them.
void maintain_sun_trans_views() {
  for (auto *&r : sun_trans_retired) {
    if (r) {
      r->Release();
      r = nullptr;
    }
  }
  if (!sun_slices_grown) {
    return;
  }
  const auto b = base();
  const auto recs = *reinterpret_cast<const uint64_t *>(b + rt_records_ptr_rva);
  if (!recs) {
    return;
  }
  const auto *rec =
      reinterpret_cast<const uint8_t *>(recs + sun_trans_rt * rt_record_stride);
  if (!readable(rec, rt_record_stride)) {
    return;
  }
  const uint16_t slices = *reinterpret_cast<const uint16_t *>(rec + 0xA86);
  auto *v0 = *reinterpret_cast<ID3D11RenderTargetView *const *>(rec + 8);
  if (v0 == sun_trans_seen_v0) {
    return;
  }
  for (size_t i = 0; i < std::size(sun_trans_extra); ++i) {
    sun_trans_retired[i] = sun_trans_extra[i];
    sun_trans_extra[i] = nullptr;
  }
  sun_trans_seen_v0 = v0;
  if (!v0 || slices <= 8) {
    return;
  }
  D3D11_RENDER_TARGET_VIEW_DESC d{};
  v0->GetDesc(&d);
  if (d.ViewDimension != D3D11_RTV_DIMENSION_TEXTURE2DARRAY) {
    return;
  }
  ID3D11Resource *res = nullptr;
  ID3D11Device *dev = nullptr;
  v0->GetResource(&res);
  v0->GetDevice(&dev);
  uint32_t made = 0;
  if (res && dev) {
    for (uint32_t sl = 8; sl < slices && sl < 8 + std::size(sun_trans_extra);
         ++sl) {
      d.Texture2DArray.FirstArraySlice = sl;
      d.Texture2DArray.ArraySize = 1;
      ID3D11RenderTargetView *v = nullptr;
      if (SUCCEEDED(dev->CreateRenderTargetView(res, &d, &v)) && v) {
        sun_trans_extra[sl - 8] = v;
        ++made;
      }
    }
  }
  if (res) {
    res->Release();
  }
  if (dev) {
    dev->Release();
  }
  trace_line l;
  l.str("sun shadow: RT 9 sidecar views for slices 8..");
  l.dec(slices - 1u);
  l.str(": ");
  l.dec(made);
  l.str(" made");
  trace_write(l);
}

void clamp_sun_shadow_slot() {
  if (sun_slot_clamped) {
    return;
  }
  const auto b = base();
  const auto *slices_at = reinterpret_cast<const uint8_t *>(b + sun_slices_rva);
  const auto *parts_at =
      reinterpret_cast<const uint8_t *>(b + sun_partitions_rva);
  auto *site = reinterpret_cast<uint8_t *>(b + sun_slot_site_rva);
  if (!readable(slices_at, 6) || slices_at[0] != 0x41 || slices_at[1] != 0xB8 ||
      !readable(parts_at, 3) || parts_at[0] != 0x83 || parts_at[1] != 0xFB ||
      !readable(site, sizeof(sun_slot_site_expected)) ||
      std::memcmp(site, sun_slot_site_expected,
                  sizeof(sun_slot_site_expected)) != 0) {
    note("[splitscreen] sun shadow slot: bytes differ - not clamped\n");
    return;
  }
  uint32_t slices = 0;
  std::memcpy(&slices, slices_at + 2, sizeof(slices));
  // grow_sun_shadow_slices() leaves that immediate at 6 (it is RT 6's id
  // too) and stores 12 through its own caves.
  if (sun_slices_grown) {
    slices = sun_slices_wanted;
  }
  const uint32_t partitions = parts_at[2];
  if (partitions == 0 || slices < partitions || slices % partitions != 0 ||
      slices / partitions > 0x80) {
    note("[splitscreen] sun shadow slot: %u slices / %u partitions - not "
         "clamped\n",
         slices, partitions);
    return;
  }
  uint32_t max_slot = slices / partitions - 1;
  // EXPERIMENT (2026-09-28, question: how does PS4 handle this, can the
  // PC copy it?). PS4 R_DrawSunShadowMapCallback (0x9589B0) sets the
  // sun target slice = partition with no per-view offset: every view
  // shares slices 0..2 and finishes its sun-shadow work before the next.
  // BO3_SUN_SLOT_MAX=0 puts all four views into slot 0 the same way, to
  // measure whether the PC sequences views like the console.
  {
    char env[8] = {};
    GetEnvironmentVariableA("BO3_SUN_SLOT_MAX", env, sizeof(env));
    if (env[0] >= '0' && env[0] <= '9' && env[1] == 0 &&
        static_cast<uint32_t>(env[0] - '0') < max_slot) {
      max_slot = static_cast<uint32_t>(env[0] - '0');
    }
  }

  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x40));
  if (!cave) {
    return;
  }
  std::vector<uint8_t> c(sun_slot_site_expected, sun_slot_site_expected + 12);
  c.insert(c.end(),
           {0x83, 0xF9, static_cast<uint8_t>(max_slot)}); // cmp ecx, max_slot
  c.insert(c.end(), {0x7E, 0x05});                        // jle +5
  c.insert(c.end(), {0xB9});                              // mov ecx, max_slot
  {
    const auto *p = reinterpret_cast<const uint8_t *>(&max_slot);
    c.insert(c.end(), p, p + 4);
  }
  c.insert(c.end(), {0x89, 0x4D, 0x14}); // mov [rbp+0x14], ecx
  c.insert(c.end(), {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00}); // jmp [rip+0]
  const uint64_t back = b + sun_slot_site_rva + sizeof(sun_slot_site_expected);
  const auto *back_bytes = reinterpret_cast<const uint8_t *>(&back);
  c.insert(c.end(), back_bytes, back_bytes + 8);
  if (!write_bytes(cave, c.data(), c.size())) {
    return;
  }

  uint8_t patch[sizeof(sun_slot_site_expected)];
  std::memset(patch, 0x90, sizeof(patch));
  patch[0] = 0xE9;
  const auto rel = static_cast<int32_t>(reinterpret_cast<size_t>(cave) -
                                        (b + sun_slot_site_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));
  if (!write_bytes(site, patch, sizeof(patch))) {
    return;
  }
  sun_slot_clamped = true;
  trace_line l;
  l.str("sun shadow slot: bounded to 0..");
  l.dec(max_slot);
  l.str(" (");
  l.dec(slices);
  l.str(" slices / ");
  l.dec(partitions);
  l.str(" partitions)");
  trace_write(l);
}

bool install_pane_counts_and_bounds() {
  if (pane_counts_installed) {
    return true;
  }
  // The storage relocation is a CLOSED DEAD END. The pane loop's only
  // need for clientUIActives[2] is the IsActive gate, which is caved
  // instead; geometry must still be reshaped or total=3 indexes past
  // the stock table's two rows.
  // scrPlaceView and the 0x54 state are indexed at 2 by the pane
  // path itself (2026-08-24 19:10 crash), so widened bounds without
  // their relocations are a guaranteed corruption.
  // aaGlobArray joins the gate: AimAssist writes slot 2 as soon as
  // CG_SetView(2) runs, so widening the pane bound without it is a
  // guaranteed corruption of foreign globals.
  if (!isactive_caved || !view_params_relocated || !scrplace_relocated ||
      !perclient54_relocated || !aaglob_relocated) {
    return false;
  }
  const auto b = base();

  auto *fn = reinterpret_cast<uint8_t *>(b + get_active_count_rva);
  if (std::memcmp(fn, get_active_count_expected,
                  sizeof(get_active_count_expected)) != 0) {
    return false;
  }
  for (const auto &f : pane_bounds) {
    const auto *p = reinterpret_cast<const uint8_t *>(b + f.rva);
    if (!readable(p, f.expect_len) ||
        std::memcmp(p, f.expect, f.expect_len) != 0) {
      return false;
    }
  }

  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x80));
  if (!cave) {
    return false;
  }
  const auto cave_addr = reinterpret_cast<size_t>(cave);
  std::vector<uint8_t> c;
  const auto rip32 = [&](const size_t tgt) {
    const auto v = static_cast<int32_t>(tgt - (cave_addr + c.size() + 4));
    const auto *p = reinterpret_cast<const uint8_t *>(&v);
    c.insert(c.end(), p, p + 4);
  };
  // PS4 CL_LocalClient_GetActiveCount (0x1516A20) is
  //     for (i = 0; i < 4; ++i) count += CL_LocalClient_IsActive(i);
  // and the stock PC copy is that loop unrolled to two elements:
  // clientUIActives[0].flags & 1 + clientUIActives[1].flags & 1.
  // The cave adds element 2 under THE SAME rule the IsActive cave
  // applies to it (read only while lc 2 < cl_maxLocalClients), so the
  // count and the gate can never disagree. Element 3 is not read: slot
  // 3 of clientUIActives is foreign memory (0x053DBD28..), and with
  // cl_maxLocalClients <= 3 IsActive(3) is 0 anyway.
  //
  // This used to count SEATS (clientGameStates[i].flags & 1). That was
  // right while clientUIActives slot 2 was voice_comm's memory; since
  // voice_comm moved it is WRONG at the frontend (2026-09-26 18:45):
  // back in the lobby after GAME OVER the seats still read 1110 while
  // the active flags read lc0 0x37, lc1 0x06, lc2 0x06 - the engine
  // had deactivated the guests - so the count said 3 and
  // CL_SetupScreenPlacements (a GetActiveCount caller, PS4 xref) kept
  // the lobby split into three panes with the menu in the top-left one.
  c.insert(c.end(), {0x33, 0xC0});       // xor eax, eax
  c.insert(c.end(), {0x48, 0x8D, 0x0D}); // lea rcx, [clientUIActives]
  rip32(b + uia_base_rva);
  // PLAYER 4 (2026-09-27): element 3 under the same rule, read only while
  // cl_maxLocalClients > 3 - clientUIActives[3] +0 (flags) lies in the part
  // of slot 3 the component owns. With <= 3 clients allocated it is never
  // read, so the count is bit-identical for 1-3 players.
  for (uint32_t i = 0; i < 4; ++i) {
    const uint32_t off = i * uia_stride;
    if (i >= 2) {
      c.insert(c.end(), {0x83, 0x3D}); // cmp dword [cl_maxLocalClients], i
      {
        const auto v = static_cast<int32_t>((b + cl_max_local_clients_rva) -
                                            (cave_addr + c.size() + 4 + 1));
        const auto *p = reinterpret_cast<const uint8_t *>(&v);
        c.insert(c.end(), p, p + 4);
      }
      c.insert(c.end(), {static_cast<uint8_t>(i)});
      c.insert(c.end(), {0x7E, 0x0B}); // jle next (skips the 11 bytes below)
    }
    if (off == 0) {
      c.insert(c.end(), {0x8B, 0x11}); // mov edx, [rcx]
    } else {
      c.insert(c.end(), {0x8B, 0x91}); // mov edx, [rcx+off]
      const auto *p = reinterpret_cast<const uint8_t *>(&off);
      c.insert(c.end(), p, p + 4);
    }
    c.insert(c.end(), {0x83, 0xE2, 0x01}); // and edx, 1
    c.insert(c.end(), {0x03, 0xC2});       // add eax, edx
  }
  c.insert(c.end(), {0xC3}); // ret
  if (c.size() > 0x80) {
    note("[splitscreen] pane count: cave too small (%zu)\n", c.size());
    return false;
  }
  if (!write_bytes(cave, c.data(), c.size())) {
    return false;
  }

  uint8_t patch[5] = {0xE9};
  const auto rel =
      static_cast<int32_t>(cave_addr - (b + get_active_count_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));
  uint8_t old_head[5] = {};
  std::memcpy(old_head, fn, sizeof(old_head));
  if (!write_bytes(fn, patch, sizeof(patch))) {
    return false;
  }

  uint32_t applied = 0;
  for (const auto &f : pane_bounds) {
    auto *site = reinterpret_cast<uint8_t *>(b + f.rva + f.offset);
    if (*site != f.from || !write_bytes(site, &f.to, sizeof(f.to))) {
      // roll the whole transaction back
      for (uint32_t i = 0; i < applied; ++i) {
        auto *undo = reinterpret_cast<uint8_t *>(b + pane_bounds[i].rva +
                                                 pane_bounds[i].offset);
        write_bytes(undo, &pane_bounds[i].from, 1);
      }
      write_bytes(fn, old_head, sizeof(old_head));
      return false;
    }
    ++applied;
  }

  pane_counts_installed = true;
  return true;
}

// --- the UNNAMED per-client CG/UI context array: [2] -> [4] ---------
//
// Found via the 2026-08-22 10:23 crash: 0xC0000005 at 0x022BCEE6 reading
// [rcx+0x1c] with rcx = 0x438000003F666666, i.e. the floats 256.0f and
// 0.9f read as a POINTER. The caller loads that pointer from the global
// at 0x04D323F0 - which sits at +0x140 inside SLOT 2 of this array
// (base 0x04D2CB40, stride 0x2BB8). perclient_sweep had flagged it:
// slot 2 carries 47 references, a region dense with live globals, and a
// client-2 write sprays float defaults across them.
//
// Indexing measured at three independent sites (imul reg, reg, 0x2BB8
// from a movsxd'ed localClientNum at 0x010F30AC, 0x010EB4DC, 0x010CD0EF)
// and the per-map reset walker at 0x02DA6200 (`add rbx, 0x2BB8`,
// `mov ebp,1 / dec / jns` = exactly two iterations, memsets 0x824 +
// 0x11B8 + 0x11B8 per element).
//
// NAMING IS OPEN. dwarf_localclient.txt has no [4] global with a
// comparable element size (PS4 struct sizes differ by design) and
// scrPlaceView does not match. Everything this patch relies on is
// measured on the PC: the six references, their element deltas
// (+0x0, +0x830, +0x2BA0, +0x2BA8, +0x2BB0 - all inside element 0),
// and the walker count.
//
// The array is NOT statically zero (2,324 nonzero bytes in the unpacked
// snapshot), so unlike cg_localEntities the two live elements are
// COPIED. At post_unpack nothing has run yet, so no pointer into the
// old buffer can exist and a flat copy is exact.
//
// The walker count (mov ebp,1 at 0x02DA621A, imm32 at +1) is widened to
// 3 ONLY AFTER the relocation, uiroot-style: with the array moved, the
// walker's elements 2 and 3 land inside the new 4-element block.
constexpr uint32_t percg_base_rva = 0x4CADB40;
constexpr uint32_t percg_stride = 0x2BB8;
constexpr uint32_t percg_old_slots = 2;
constexpr uint32_t percg_new_slots = 4;
constexpr uint32_t percg_old_size = percg_old_slots * percg_stride; // 0x5770
constexpr uint32_t percg_new_size = percg_new_slots * percg_stride; // 0xAEE0

constexpr uint32_t percg_walker_count_rva = 0x2D2D0AA;
constexpr uint8_t percg_walker_expected[] = {0xBD, 0x01, 0x00, 0x00, 0x00};

struct percg_ref {
  uint32_t rva;
  uint8_t len;
  uint8_t disp_off;
  uint32_t delta; // byte offset within element 0
  bool rip;       // true = rip-relative, false = ABS32 off the image base
  uint8_t expected[12];
};

constexpr percg_ref percg_refs[] = {
    {0x10EB4F0, 7, 3, 0x0, true, {0x48, 0x8D, 0x15, 0x49, 0x26, 0xBC, 0x03}},
    {0x10F30C2, 7, 3, 0x0, true, {0x48, 0x8D, 0x05, 0x77, 0xAA, 0xBB, 0x03}},
    {0x2D2D0AF, 7, 3, 0x830, true, {0x48, 0x8D, 0x1D, 0xFA, 0x0B, 0xF8, 0x01}},
    {0x10CD118,
     12,
     4,
     0x2BB0,
     false,
     {0x42, 0xC7, 0x84, 0x30, 0xF0, 0x06, 0xCB, 0x04, 0xFF, 0xFF, 0xFF, 0xFF}},
    {0x10CD124,
     8,
     4,
     0x2BA0,
     false,
     {0x4A, 0x89, 0x8C, 0x30, 0xE0, 0x06, 0xCB, 0x04}},
    {0x10CD12C,
     8,
     4,
     0x2BA8,
     false,
     {0x4A, 0x89, 0x8C, 0x30, 0xE8, 0x06, 0xCB, 0x04}},
};

bool percg_relocated = false;
size_t percg_new_base_rva = 0;

bool relocate_percg_context() {
  if (percg_relocated) {
    return true;
  }

  const auto b = base();

  for (const auto &r : percg_refs) {
    const auto *p = reinterpret_cast<const void *>(b + r.rva);
    if (!readable(p, r.len) || std::memcmp(p, r.expected, r.len) != 0) {
      return false;
    }
  }
  if (!readable(reinterpret_cast<const void *>(b + percg_walker_count_rva),
                sizeof(percg_walker_expected)) ||
      std::memcmp(reinterpret_cast<const void *>(b + percg_walker_count_rva),
                  percg_walker_expected, sizeof(percg_walker_expected)) != 0) {
    return false;
  }

  auto *destination = allocate_near_module(percg_new_size);
  if (!destination) {
    return false;
  }
  const auto new_base = reinterpret_cast<size_t>(destination) - b;

  if (!readable(reinterpret_cast<const void *>(b + percg_base_rva),
                percg_old_size)) {
    return false;
  }
  std::memcpy(destination, reinterpret_cast<const void *>(b + percg_base_rva),
              percg_old_size);

  int32_t old_values[std::size(percg_refs)] = {};
  size_t done = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < done; ++i) {
      write_bytes(reinterpret_cast<void *>(b + percg_refs[i].rva +
                                           percg_refs[i].disp_off),
                  &old_values[i], sizeof(old_values[i]));
    }
  };

  // rip-relative counts from the END of the instruction; the ABS32 form
  // is `[reg + disp32]` with the register holding the IMAGE BASE, so
  // its displacement is simply the RVA.
  const auto field_value = [&](const percg_ref &r) {
    const size_t tgt = new_base + r.delta;
    return r.rip ? static_cast<int32_t>(tgt - (r.rva + r.len))
                 : static_cast<int32_t>(tgt);
  };

  for (size_t i = 0; i < std::size(percg_refs); ++i) {
    const auto &r = percg_refs[i];
    const int32_t value = field_value(r);
    auto *field = reinterpret_cast<void *>(b + r.rva + r.disp_off);
    std::memcpy(&old_values[i], field, sizeof(int32_t));
    if (!write_bytes(field, &value, sizeof(value))) {
      rollback();
      return false;
    }
    ++done;
  }

  for (size_t i = 0; i < std::size(percg_refs); ++i) {
    int32_t seen = 0;
    std::memcpy(&seen,
                reinterpret_cast<const void *>(b + percg_refs[i].rva +
                                               percg_refs[i].disp_off),
                sizeof(seen));
    if (seen != field_value(percg_refs[i])) {
      rollback();
      return false;
    }
  }

  // ONLY NOW the walker count: elements 2/3 land in the new block.
  const uint8_t three = 0x03;
  auto *count = reinterpret_cast<void *>(b + percg_walker_count_rva + 1);
  const uint8_t old_count = *reinterpret_cast<const uint8_t *>(count);
  if (!write_bytes(count, &three, sizeof(three))) {
    rollback();
    return false;
  }
  uint8_t count_seen = 0;
  std::memcpy(&count_seen, count, sizeof(count_seen));
  if (count_seen != three) {
    write_bytes(count, &old_count, sizeof(old_count));
    rollback();
    return false;
  }

  percg_new_base_rva = new_base;
  percg_relocated = true;
  // No status slot free; verify from outside with tools/verify_percg.py.
  return true;
}

// --- the per-client LUI ROOT array: [2] -> [4], a FLAT relocation ----
//
// This is the array whose earlier relocation was RETRACTED on 2026-08-06
// for moving two globals as one. The retraction was right, and the reason
// was the BASE.
//
// The initialisation loop carries TWO cursors into the SAME array:
//
//   01F29168  lea rbp, [rip+...]   -> 0x162E2120
//   01F2930E  lea rsi, [rip+...]   -> 0x162E21A0      == rbp + 0x80
//   01F29315  lea rcx, [rbp+0x8C]  ; sprintf(dst, 0x20, "UIRoot%d", ebx)
//   01F293A5  add rbp, 0xb0
//   01F293B0  add rsi, 0xb0        ; lockstep, so ONE array
//   01F293B7  cmp ebx, 2
//   01F293BA  jl  0x01F29315       ; only UIRoot0 and UIRoot1 are built
//
// Both advance by 0xB0 together and differ by exactly 0x80, so element 0
// begins at 0x162E2120 - NOT at 0x162E21A0, which is 0x80 into it. The
// name strings confirm it: elem+0x8C gives 0x162E21AC and 0x162E225C, the
// same "UIRoot0"/"UIRoot1" bytes the old entry read at +0x0C from the
// wrong base. The old range 0x162E21A0..0x162E2300 therefore started
// inside element 0 and ran past element 1 into s_perController.
//
// A BOUND WIDEN IS NOT AVAILABLE, and this is the trap. `cmp ebx,2` is a
// one-byte length-preserving patch that looks free, but with the array
// still in place element 2 lands at 0x162E2280 - exactly
// s_perController's base - and the loop would write over
// s_perController[0] and [1]. s_perController is indexed by CONTROLLER
// index, not local client. (2026-09-28 correction: the "five lea
// references to slot 2's +8 field" from 0x01F2DE87.. address the glyph
// buffer after s_perController, which is only [2] on PC - see
// relocate_per_controller.) So the array MOVES first and the bound is
// widened only afterwards, in that order, inside one transaction.
//
// WHY THIS MAY MATTER FOR THE ROUND - stated as a hypothesis, not a
// result. The round-start failure is a Havok Script panic whose message,
// read out of the live process, is `Unprotected error (processEvent)`
// with an EMPTY CallInfo stack. Bounded at 2, local client 2 never gets a
// LUI root, so a dispatch reaching for it finds nothing and raises
// outside any pcall - which is what an empty stack looks like. That fits,
// but it is NOT demonstrated until a round runs with this applied.
//
// TIMING. CLAUDE.md lists "relocating s_rootData at component startup"
// as a closed dead end with an int3 at 0x01D492DB. That attempt used the
// wrong base and dragged s_perController along, which is a sufficient
// explanation for the assert on its own. post_unpack is also the only
// moment that works here: the init loop above populates the array later,
// so moving it BEFORE the loop runs is what lets element 2 exist at all.
// If this reintroduces the startup int3, the move is wrong and not the
// timing - revert it rather than retrying at menu time.
//
// 20 references from uiroot2_reloc.validated.txt: 12 rip-relative lea and
// 8 ABS32. They land at +0x0, +0x80, +0x8C and +0xAC - one contiguous
// cluster inside element 0, no second cluster after a gap, which is the
// split-by-target-offset check the retraction demands.
constexpr uint32_t uiroot_base_rva = 0x162631B0;
constexpr uint32_t uiroot_stride = 0xB0;
constexpr uint32_t uiroot_old_slots = 2;
constexpr uint32_t uiroot_new_slots = 4;
constexpr uint32_t uiroot_old_size = uiroot_old_slots * uiroot_stride; // 0x160
constexpr uint32_t uiroot_new_size = uiroot_new_slots * uiroot_stride; // 0x2C0

// `cmp ebx,2` = 83 FB 02, immediate at +2. Only the FIRST bound is
// widened here: 0x01F293B7 is the one that indexes this array. The second
// `cmp ebx,2` at 0x01F2945B drives a different loop that only calls
// 0x0283AA50 and 0x01322080 per client and touches none of these arrays,
// so widening it needs its own rule-4 classification of what those two
// functions write. Left alone deliberately.
constexpr uint32_t uiroot_bound_rva = 0x1F1CC37;
constexpr uint8_t uiroot_bound_expected[] = {0x83, 0xFB, 0x02};

// The SECOND `cmp ebx,2` (0x01F2945B), classified 2026-08-25 by the
// 0x0270D553 crash: its loop (head 0x01F29440) gates on IsActive then
// calls the per-client UI-context setup 0x01322080, and touches the
// relocated LUI roots (crash rsi = relocated slot 2). With the bound
// at 2, context 2 is never constructed and every later read of it
// derefs NULL (the crash, and the 6-FPS menu). Safe to widen only
// after the roots are relocated - hence applied in the same function.
constexpr uint32_t uiroot_bound2_rva = 0x1F1CCDB;
constexpr uint8_t uiroot_bound2_expected[] = {0x83, 0xFB, 0x02};

struct uiroot_ref {
  uint32_t rva;
  uint8_t len;
  uint8_t disp_off;
  uint8_t delta; // byte offset within element 0
  bool rip;      // true = rip-relative, false = ABS32 off the image base
  uint8_t expected[9];
};

constexpr uiroot_ref uiroot_refs[] = {
    {0x1F1C1CC, 7, 3, 0x00, true, {0x48, 0x8D, 0x0D, 0x1D, 0x69, 0x34, 0x14}},
    {0x1F1C3D5,
     8,
     3,
     0xAC,
     false,
     {0x80, 0xBC, 0x38, 0x5C, 0x32, 0x26, 0x16, 0x00}},
    {0x1F1C3DF, 7, 3, 0x8C, false, {0x4C, 0x8D, 0x87, 0x3C, 0x32, 0x26, 0x16}},
    {0x1F1C46C,
     8,
     3,
     0xAC,
     false,
     {0x80, 0xBC, 0x38, 0x5C, 0x32, 0x26, 0x16, 0x00}},
    {0x1F1C476, 7, 3, 0x8C, false, {0x4C, 0x8D, 0x87, 0x3C, 0x32, 0x26, 0x16}},
    {0x1F1C5E6,
     8,
     3,
     0xAC,
     false,
     {0x80, 0xBC, 0x28, 0x5C, 0x32, 0x26, 0x16, 0x00}},
    {0x1F1C5F0, 7, 3, 0x8C, false, {0x48, 0x8D, 0x9D, 0x3C, 0x32, 0x26, 0x16}},
    {0x1F1C79E, 7, 3, 0x00, true, {0x48, 0x8D, 0x05, 0x4B, 0x63, 0x34, 0x14}},
    {0x1F1C9E8, 7, 3, 0x00, true, {0x48, 0x8D, 0x2D, 0x01, 0x61, 0x34, 0x14}},
    {0x1F1CB8E, 7, 3, 0x80, true, {0x48, 0x8D, 0x35, 0xDB, 0x5F, 0x34, 0x14}},
    {0x1F1CD35, 7, 3, 0xAC, true, {0x48, 0x8D, 0x1D, 0x60, 0x5E, 0x34, 0x14}},
    {0x1F1CD3C, 7, 3, 0x8C, true, {0x48, 0x8D, 0x35, 0x39, 0x5E, 0x34, 0x14}},
    {0x1F1D832, 7, 3, 0x00, true, {0x48, 0x8D, 0x05, 0xB7, 0x52, 0x34, 0x14}},
    {0x1F21A19, 7, 3, 0x00, true, {0x48, 0x8D, 0x05, 0xD0, 0x10, 0x34, 0x14}},
    {0x1F25915, 7, 3, 0x00, true, {0x48, 0x8D, 0x15, 0xD4, 0xD1, 0x33, 0x14}},
    {0x1F25D13,
     9,
     4,
     0xAC,
     false,
     {0x42, 0x80, 0xBC, 0x39, 0x5C, 0x32, 0x26, 0x16, 0x00}},
    {0x1F25D1E, 7, 3, 0x8C, false, {0x4D, 0x8D, 0x87, 0x3C, 0x32, 0x26, 0x16}},
    {0x1F26428, 7, 3, 0x00, true, {0x4C, 0x8D, 0x2D, 0xC1, 0xC6, 0x33, 0x14}},
    {0x1F26AB4, 7, 3, 0xAC, true, {0x48, 0x8D, 0x3D, 0xE1, 0xC0, 0x33, 0x14}},
    {0x1F26ABB, 7, 3, 0x8C, true, {0x48, 0x8D, 0x35, 0xBA, 0xC0, 0x33, 0x14}},
};

bool lui_roots_relocated = false;
size_t uiroot_new_base_rva = 0;

bool relocate_lui_roots() {
  if (lui_roots_relocated) {
    return true;
  }

  const auto b = base();

  // Every site must match its recorded bytes, or this is a different
  // build and nothing at all is written.
  for (const auto &r : uiroot_refs) {
    const auto *p = reinterpret_cast<const void *>(b + r.rva);
    if (!readable(p, r.len) || std::memcmp(p, r.expected, r.len) != 0) {
      return false;
    }
  }
  if (!readable(reinterpret_cast<const void *>(b + uiroot_bound_rva),
                sizeof(uiroot_bound_expected)) ||
      std::memcmp(reinterpret_cast<const void *>(b + uiroot_bound_rva),
                  uiroot_bound_expected, sizeof(uiroot_bound_expected)) != 0) {
    return false;
  }

  auto *destination = allocate_near_module(uiroot_new_size);
  if (!destination) {
    return false;
  }
  const auto new_base = reinterpret_cast<size_t>(destination) - b;

  // Unlike cg_localEntities this array is NOT all zeros - it carries
  // static content - so the two live elements are copied rather than
  // assumed empty. Elements 2 and 3 stay zero; the init loop fills them,
  // building their names with sprintf(elem+0x8C, 0x20, "UIRoot%d", i).
  if (!readable(reinterpret_cast<const void *>(b + uiroot_base_rva),
                uiroot_old_size)) {
    return false;
  }
  std::memcpy(destination, reinterpret_cast<const void *>(b + uiroot_base_rva),
              uiroot_old_size);

  int32_t old_values[std::size(uiroot_refs)] = {};
  size_t done = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < done; ++i) {
      write_bytes(reinterpret_cast<void *>(b + uiroot_refs[i].rva +
                                           uiroot_refs[i].disp_off),
                  &old_values[i], sizeof(old_values[i]));
    }
  };

  // rip-relative counts from the END of the instruction; the ABS32 form
  // used here is `[reg + disp32]` with the register holding the IMAGE
  // BASE, so its displacement is simply the RVA.
  const auto field_value = [&](const uiroot_ref &r) {
    const size_t tgt = new_base + r.delta;
    return r.rip ? static_cast<int32_t>(tgt - (r.rva + r.len))
                 : static_cast<int32_t>(tgt);
  };

  for (size_t i = 0; i < std::size(uiroot_refs); ++i) {
    const auto &r = uiroot_refs[i];
    const int32_t value = field_value(r);
    auto *field = reinterpret_cast<void *>(b + r.rva + r.disp_off);
    std::memcpy(&old_values[i], field, sizeof(int32_t));
    if (!write_bytes(field, &value, sizeof(value))) {
      rollback();
      return false;
    }
    ++done;
  }

  for (size_t i = 0; i < std::size(uiroot_refs); ++i) {
    int32_t seen = 0;
    std::memcpy(&seen,
                reinterpret_cast<const void *>(b + uiroot_refs[i].rva +
                                               uiroot_refs[i].disp_off),
                sizeof(seen));
    if (seen != field_value(uiroot_refs[i])) {
      rollback();
      return false;
    }
  }

  // ONLY NOW is the bound safe to widen: element 2 no longer lands on
  // s_perController because the array is somewhere else entirely.
  const uint8_t four = 0x04;
  auto *bound = reinterpret_cast<void *>(b + uiroot_bound_rva + 2);
  const uint8_t old_bound = *reinterpret_cast<const uint8_t *>(bound);
  if (!write_bytes(bound, &four, sizeof(four))) {
    rollback();
    return false;
  }
  uint8_t bound_seen = 0;
  std::memcpy(&bound_seen, bound, sizeof(bound_seen));
  if (bound_seen != four) {
    write_bytes(bound, &old_bound, sizeof(old_bound));
    rollback();
    return false;
  }

  // The second bound - the UI-context constructor loop. Same value,
  // same verify; if it is not the expected 0x02 we skip it rather
  // than fail the whole relocation (it is a distinct instruction and
  // a mismatch means only that this one site moved).
  if (readable(reinterpret_cast<const void *>(b + uiroot_bound2_rva),
               sizeof(uiroot_bound2_expected)) &&
      std::memcmp(reinterpret_cast<const void *>(b + uiroot_bound2_rva),
                  uiroot_bound2_expected,
                  sizeof(uiroot_bound2_expected)) == 0) {
    auto *bound2 = reinterpret_cast<void *>(b + uiroot_bound2_rva + 2);
    const uint8_t old_bound2 = *reinterpret_cast<const uint8_t *>(bound2);
    if (write_bytes(bound2, &four, sizeof(four))) {
      uint8_t seen2 = 0;
      std::memcpy(&seen2, bound2, sizeof(seen2));
      if (seen2 != four) {
        write_bytes(bound2, &old_bound2, sizeof(old_bound2));
      }
    }
  }

  // The remaining UI-context sibling loops, same family, same value.
  //
  // (loop #3, the registrar at 0x01F331D2, is NOT widened here: its
  //  prerequisite relocation runs later in try_apply, so it is applied
  //  next to that relocation instead - see widen_ui_registrar_bound.)
  //   0x01F332CD  cmp ebx,2  loop #4, broadcasts "update_safe_area".
  //               Cursors are relocated-roots fields; rbp (0x162E20FC)
  //               is loop-invariant, so nothing steps into a new slot.
  //
  // LOOP #2 (0x01F29514 `cmp edi,2`, the HUD-menu loop) STAYS AT 2.
  // It was widened here until 2026-09-26 and that widen is what kept
  // pane 1 on the loading screen and pane 2 without a HUD. The loop
  // is PS4 UI_CoD_Init 0xD04C0F: for every active controller,
  // UI_AddMenu(UI_CoD_GetRootNameForController(c), "HUD", c). Root 2
  // is held inUse = 0 (splitscreen_player_count_stub: the PC builds
  // no third LUI context), so for controller 2 the root name falls
  // back to "UIRootFull" and HUD(2) was added THERE. UIRootFull is
  // the first root UI_CoD_Init creates, so it is LUI.primaryRoot, and
  // stock codroot.lua offers every event of every other root to the
  // primary root FIRST (PropagateEventToPrimaryRoot, before
  // UIElement.processEvent on the target). HUD(2)'s first_snapshot
  // handler accepts any controller and returns true, so it consumed
  // the first_snapshot the engine sent to UIRoot0 and UIRoot1
  // (component trace, site 0x01321038): HUD(0) never closed its
  // Loading menu (hud.lua creates it for controller 0 only; the text
  // is the stale ls_status EXE_AWAITINGHOST = "Waiting for other
  // players") and HUD(1) never built. Console never has a HUD in
  // UIRootFull during splitscreen - every active controller has its
  // own root there - so the stock bound of 2 is the honest state
  // until the third LUI context exists: no HUD for controller 2,
  // instead of one that steals the other roots' events.
  //
  // 2026-09-27: BACK TO FOUR, together with releasing root 2's in-use
  // flag in the count stub. With root 2 in use,
  // UI_CoD_GetRootNameForController(2) answers "UIRoot2", so HUD(2) is
  // added to player 3's own root - the console state - and no longer
  // to UIRootFull. The loop only adds a HUD for ACTIVE controllers.
  struct ui_bound {
    uint32_t rva;
    uint8_t modrm;
  };
  constexpr ui_bound ui_bounds[] = {
      {0x01F2720D, 0xFB}, // cmp ebx,2
      {0x01F1D454, 0xFF}, // cmp edi,2  - HUD menus (see above)
  };
  for (const auto &ub : ui_bounds) {
    const uint8_t want[] = {0x83, ub.modrm, 0x02};
    auto *at = reinterpret_cast<uint8_t *>(b + ub.rva);
    if (!readable(at, sizeof(want)) ||
        std::memcmp(at, want, sizeof(want)) != 0) {
      continue;
    }
    auto *imm = reinterpret_cast<uint8_t *>(b + ub.rva + 2);
    if (write_bytes(imm, &four, sizeof(four))) {
      uint8_t back = 0;
      std::memcpy(&back, imm, sizeof(back));
      if (back != four) {
        const uint8_t two = 0x02;
        write_bytes(imm, &two, sizeof(two));
      }
    }
  }

  uiroot_new_base_rva = new_base;
  lui_roots_relocated = true;
  // No status slot is free (0..95 are all taken). Verify from outside
  // with tools/verify_uiroot.py, which reads the patched bytes back out
  // of the live process.
  return true;
}

// ============ s_perController (LUI_CoD) [2] -> [4] (2026-09-28, player 4's
// pane goes grey mid-round) ============
//
// Seen in 4-player MP: a few seconds into a round pane 4 turns into a
// blurred grey image with no HUD and stays so across rounds. Live: only
// controller 3 carries BIT_UI_ACTIVE, no LUI menu is open for it, its pad
// cannot close anything. PS4 CL_IsUIActive 0x415330 = (clientUIActives[lc]
// .keyCatchers & 8) || UI_CoD_IsUIActive 0xD08870 = s_perController[ctrl]
// byte +1. PS4 PerController (LUI_CoD.cpp) is 0x14 bytes, [4]. The PC
// clears it with length 0x28 (0x01F1D0CD `lea r8d,[rdx+0x28]` -> memset of
// 0x16263310) = [2]. Slots 2/3 lie in the 0x40-byte button-glyph markup
// buffer at 0x16263340 (the `^...^` parser 0x01F21D97 copies names like
// "XENONButtonshoulderR" there); read live while pane 4 was grey:
// s_perController[3] = "houlderR..", byte +1 = 'o' (odd) -> UI active.
// So whenever a long glyph name passes through that buffer, player 4 is
// "in a menu" and the renderer blurs the pane. (Player 3's +1 byte sits in
// 8 unreferenced bytes before the buffer, which is why only pane 4 shows it;
// its subscriber counter at +8 is the buffer's first dword.)
// The 2026-08 note that slot 2 "has five lea references to its +8 field"
// was wrong: those five (0x01F21DC7..0x01F21F79) address the glyph buffer.
// scratch array_refs.py over [base-0x10, base+0x50]: 17 real references,
// 12 into slots 0/1 (below), 5 into the buffer (NOT rewritten); one raw
// candidate in the Arxan section (0x1D00137F) is data - it decodes as no
// rip-relative instruction and equals the dump live. No stored pointers
// into the array anywhere in live module memory. Every access goes through
// Com_LocalClient_GetControllerIndex; no loop bound over controllers.
// Moved at startup, next to the LUI roots, so the engine's own init clear
// (widened 0x28 -> 0x50) gives slots 2/3 their initial state.
constexpr uint32_t perctrl_base = 0x16263310;
constexpr uint32_t perctrl_stride = 0x14;
constexpr uint32_t perctrl_old_count = 2;
constexpr uint32_t perctrl_new_count = 4;
constexpr entcoll_site perctrl_sites[] = {
    {0x1F14F0D, 3, 7, true, 0x8},    // lea rcx,[+8]    subscribers++
    {0x1F1A550, 3, 7, true, 0x4},    // lea rcx,[+4]    float getter
    {0x1F1C68C, 5, 9, false, 0xC},   // mulss xmm1,[rbp+rbx*4+RVA+0xC]
    {0x1F1C6B8, 5, 9, false, 0x10},  // mulss xmm1,[rbp+rbx*4+RVA+0x10]
    {0x1F1CA11, 3, 7, true, 0x0},    // lea r14,[base]  init clear
    {0x1F1D09E, 3, 8, false, 0x0},   // cmp byte [rcx+rax*4+RVA],0
    {0x1F1D0E0, 3, 7, true, 0x1},    // lea rcx,[+1]    UI_CoD_IsUIActive
    {0x1F1F66E, 3, 7, true, 0x0},    // lea rax,[base]  flag setter
    {0x1F21363, 6, 10, false, 0xC},  // movss [r14+rax*4+RVA+0xC],xmm6
    {0x1F2136D, 6, 10, false, 0x10}, // movss [r14+rax*4+RVA+0x10],xmm7
    {0x1F2197D, 3, 7, true, 0x8},    // lea rcx,[+8]    subscribers--
    {0x1F265E7, 3, 7, true, 0x1},    // lea rax,[+1]
};
constexpr uint32_t perctrl_clear_rva = 0x1F1CA0D; // lea r8d,[rdx+0x28]
constexpr uint8_t perctrl_clear_stock[] = {0x44, 0x8D, 0x42, 0x28};
const char *perctrl_result = "s_perController: not attempted";
size_t perctrl_new = 0;

bool relocate_per_controller() {
  if (perctrl_new) {
    return true;
  }
  const auto b = base();
  auto *clear = reinterpret_cast<uint8_t *>(b + perctrl_clear_rva);
  if (!readable(clear, sizeof(perctrl_clear_stock)) ||
      std::memcmp(clear, perctrl_clear_stock, sizeof(perctrl_clear_stock)) !=
          0) {
    perctrl_result = "s_perController: NOT moved - init clear bytes differ";
    return false;
  }
  auto *fresh = static_cast<uint8_t *>(
      allocate_near_module(perctrl_new_count * perctrl_stride));
  if (!fresh) {
    perctrl_result = "s_perController: NOT moved - allocation failed";
    return false;
  }
  const auto *old = reinterpret_cast<const uint8_t *>(b + perctrl_base);
  std::memcpy(fresh, old,
              perctrl_old_count * perctrl_stride); // slots 2/3 stay zero
  const auto fresh_abs = reinterpret_cast<size_t>(fresh);
  static int32_t saved[std::size(perctrl_sites)]{};
  if (!rewrite_entcoll(perctrl_sites, std::size(perctrl_sites), perctrl_base,
                       fresh_abs, saved)) {
    perctrl_result = "s_perController: NOT moved - a reference did not match";
    return false;
  }
  const uint8_t len =
      static_cast<uint8_t>(perctrl_new_count * perctrl_stride); // 0x50
  if (!write_bytes(clear + 3, &len, 1)) {
    for (size_t j = 0; j < std::size(perctrl_sites); ++j) {
      auto *insn = reinterpret_cast<uint8_t *>(b + perctrl_sites[j].rva);
      write_bytes(insn + perctrl_sites[j].disp_off, &saved[j], sizeof(int32_t));
    }
    perctrl_result =
        "s_perController: NOT moved - init clear write failed (rolled back)";
    return false;
  }
  perctrl_new = fresh_abs;
  perctrl_result =
      "s_perController [2] -> [4] (12 sites, init clear 0x28 -> 0x50)";
  note("[splitscreen] s_perController [2] -> [4] at RVA 0x%08X\n",
       static_cast<uint32_t>(fresh_abs - b));
  return true;
}

// --- cg_localEntities: [2] -> [4], a FLAT relocation ----------------
//
// THIS IS THE ROOT CAUSE OF THE ROUND-START FAILURE, measured 2026-08-20
// 22:44 with tools/watch_clientfields.py:
//
//   22:44:08  all 14 clientfield sets emptied  (map load tears down)
//   22:44:18  all 14 re-registered, world numFields=64
//   22:44:19  world ALONE back to 0, the other thirteen untouched
//   22:44:20  crash
//
// perclient_sweep.py then named the writer in a single line:
//
//   FOREIGN base 0x049C9B10 stride 0x7C00 slots 2/0/95/0 sites 0x0083D632
//
// 0x0083D620 is CG_InitLocalEntities(localClientNum) and it does
//
//   0083D632  lea  rsi, [rip+...]      ; cg_localEntities
//   0083D63E  mov  r8d, 0x7c00
//   0083D644  imul rbx, rbx, 0x7c00    ; localClientNum
//   0083D651  call memset              ; memset(&pool[lc], 0, 0x7C00)
//
// so for localClientNum 2 it zeroes 0x049D9310..0x049E0F10, and the
// CLIENTFIELD SYSTEM global lives at 0x049D9430 - 0x120 bytes inside that
// range. The world set's fields are at 0x049DD440 (numFields) and
// 0x049DD448 (fields[0]); set 1 begins at 0x049E1458, past the end of the
// memset. That is exactly the observation: set 0 wiped, 1..13 alive.
//
// RULE ZERO. PS4 CG_InitLocalEntities(LocalClientNum_t) at 0x0021ED10
// materialises all three arrays and uses the SAME 0x7C00 length as PC:
//
//   0021ED18  lea rax, &cg_freeLocalEntities    -> 0x04201280
//   0021ED1F  lea rcx, &cg_localEntities        -> 0x042012B0
//   0021ED26  lea rdx, &cg_activeLocalEntities  -> 0x04200EA0
//   0021ED32  movabs r8, 0x7c00
//
// An equal memset length means MAX_LOCAL_ENTITIES = 128 and
// sizeof(localEntity_s) = 0xF8 hold on PC too - one of the rare cases
// where a stride transfers. Only the client dimension differs, and the
// console gaps prove it without guessing:
//
//   PS4 active 0x04200EA0 -> free 0x04201280 = 0x3E0 = 4 * 0xF8   [4]
//   PC  active 0x049C9900 -> free 0x049C9AF0 = 0x1F0 = 2 * 0xF8   [2]
//
// WHY IT MOVES INSTEAD OF WIDENING: slot 2 is occupied twice - a plain
// global written by absolute stores at 0x00866B0A..0x00866DE5 sits at
// 0x049D9310, and the clientfield system at 0x049D9430. Neither is a
// per-client array (perclient_sweep lists no base at 0x049D9310), so both
// are simply collateral, and both are fixed by moving the pool.
//
// WHY NO BOUND PATCH IS NEEDED - swept, not assumed. The four functions
// touching these arrays (CG_FreeLocalEntity, CG_AllocLocalEntity,
// CG_InitLocalEntities and the tracer walker at 0x0083B986) carry no
// `cmp reg,1` client limit. The only client bound among them is
//   0083D189  cmp r9d, dword ptr [rip + 0x4b65590]
// whose target computes to 0x053A2720 - cl_maxLocalClients itself. It is
// dynamic, so the existing cl_maxLocalClients machinery already covers it.
//
// THE EASY KIND. The client dimension is OUTSIDE the element in all three
// arrays - the index expressions are lc*0x7C00, lc*0xF8 and lc*8 - so
// widening to [4] changes no field offset and no index expression. This is
// a flat relocation, unlike RadiantExploderData which needed a record
// transformer.
//
// The 17 references come from localentities_reloc.validated.txt: 99 raw
// disp32 candidates in real code, every one decoded AND boundary-swept.
// Six of the first thirteen began ONE BYTE LATE by matching the
// un-prefixed decode and missing the REX.W prefix - exactly the trap
// CLAUDE.md rule 4 names, which is why every entry below carries its full
// expected byte string and nothing is matched by pattern.
constexpr uint32_t le_active_rva = 0x494A900;
constexpr uint32_t le_free_rva = 0x494AAF0;
constexpr uint32_t le_pool_rva = 0x494AB10;
constexpr uint32_t le_entity_size = 0xF8;
constexpr uint32_t le_entities_per_client = 128;
constexpr uint32_t le_clients = 4;

constexpr uint32_t le_old_active_size = 2 * le_entity_size; // 0x1F0
constexpr uint32_t le_old_free_size = 2 * 8;                // 0x10
constexpr uint32_t le_old_pool_size =
    2 * le_entities_per_client * le_entity_size; // 0xF800

constexpr uint32_t le_new_active_size = le_clients * le_entity_size; // 0x3E0
constexpr uint32_t le_new_free_size = le_clients * 8;                // 0x20
constexpr uint32_t le_new_pool_size =
    le_clients * le_entities_per_client * le_entity_size; // 0x1F000

// Offsets inside the single block. The pool sits at 0x400 so it keeps
// 1024-byte alignment; localEntity_s carries SIMD fields and the original
// array is page aligned.
constexpr uint32_t le_off_active = 0;
constexpr uint32_t le_off_free = le_new_active_size; // 0x3E0
constexpr uint32_t le_off_pool = 0x400;
constexpr uint32_t le_block_size = le_off_pool + le_new_pool_size; // 0x1F400

enum le_array_id : uint8_t { le_active = 0, le_free = 1, le_pool = 2 };

struct le_ref {
  uint32_t rva;
  uint8_t len;
  uint8_t disp_off;
  uint8_t arr;
  uint32_t delta; // byte offset within that array
  bool rip;       // true = rip-relative, false = ABS32 off the image base
  uint8_t expected[9];
};

constexpr le_ref le_refs[] = {
    {0x0083B986,
     7,
     3,
     le_active,
     0x0,
     true,
     {0x48, 0x8D, 0x05, 0x73, 0xEF, 0x10, 0x04}},
    {0x0083D20B,
     8,
     4,
     le_free,
     0x0,
     false,
     {0x4B, 0x8B, 0x84, 0xE5, 0xF0, 0xAA, 0x94, 0x04}},
    {0x0083D217,
     8,
     4,
     le_free,
     0x0,
     false,
     {0x4B, 0x89, 0x9C, 0xE5, 0xF0, 0xAA, 0x94, 0x04}},
    {0x0083D519,
     9,
     4,
     le_free,
     0x0,
     false,
     {0x48, 0x83, 0xBC, 0xFE, 0xF0, 0xAA, 0x94, 0x04, 0x00}},
    {0x0083D52E,
     8,
     4,
     le_active,
     0x0,
     false,
     {0x48, 0x8B, 0x94, 0x32, 0x00, 0xA9, 0x94, 0x04}},
    {0x0083D53B,
     8,
     4,
     le_free,
     0x0,
     false,
     {0x48, 0x8B, 0x9C, 0xFE, 0xF0, 0xAA, 0x94, 0x04}},
    {0x0083D552,
     8,
     4,
     le_free,
     0x0,
     false,
     {0x48, 0x89, 0x84, 0xFE, 0xF0, 0xAA, 0x94, 0x04}},
    {0x0083D569,
     8,
     4,
     le_active,
     0x8,
     false,
     {0x48, 0x8B, 0x8C, 0x37, 0x08, 0xA9, 0x94, 0x04}},
    {0x0083D575,
     7,
     3,
     le_active,
     0x0,
     false,
     {0x48, 0x8D, 0x8E, 0x00, 0xA9, 0x94, 0x04}},
    {0x0083D582,
     8,
     4,
     le_active,
     0x8,
     false,
     {0x48, 0x8B, 0x8C, 0x37, 0x08, 0xA9, 0x94, 0x04}},
    {0x0083D58D,
     8,
     4,
     le_active,
     0x8,
     false,
     {0x48, 0x89, 0x9C, 0x37, 0x08, 0xA9, 0x94, 0x04}},
    {0x0083D5F9,
     7,
     3,
     le_free,
     0x0,
     true,
     {0x48, 0x8D, 0x0D, 0xF0, 0xD4, 0x10, 0x04}},
    {0x0083D632,
     7,
     3,
     le_pool,
     0x0,
     true,
     {0x48, 0x8D, 0x35, 0xD7, 0xD4, 0x10, 0x04}},
    {0x0083D667,
     7,
     3,
     le_active,
     0x0,
     false,
     {0x48, 0x8D, 0x82, 0x00, 0xA9, 0x94, 0x04}},
    {0x0083D66E,
     8,
     4,
     le_free,
     0x0,
     false,
     {0x48, 0x89, 0x9C, 0xFA, 0xF0, 0xAA, 0x94, 0x04}},
    {0x0083D67D,
     8,
     4,
     le_active,
     0x8,
     false,
     {0x48, 0x89, 0x84, 0x11, 0x08, 0xA9, 0x94, 0x04}},
    {0x0083D68B,
     7,
     3,
     le_pool,
     0x8,
     true,
     {0x48, 0x8D, 0x0D, 0x86, 0xD4, 0x10, 0x04}},
};

bool local_entities_relocated = false;
size_t le_new_base_rva = 0;

bool relocate_local_entities() {
  if (local_entities_relocated) {
    return true;
  }

  const auto b = base();

  // Every site must match its recorded bytes, or this is a different
  // build and nothing at all is written.
  for (const auto &r : le_refs) {
    const auto *p = reinterpret_cast<const void *>(b + r.rva);
    if (!readable(p, r.len) || std::memcmp(p, r.expected, r.len) != 0) {
      return false;
    }
  }

  // THE GUARD THAT REPLACES A COPY. cg_freeLocalEntities holds POINTERS
  // into cg_localEntities, so copying a populated pair of slots would
  // carry pointers into the abandoned buffer. At post_unpack, before any
  // map, all three arrays are still their .bss zeros - and a zeroed
  // destination then IS the correct transformed result, because
  // VirtualAlloc zero-fills. Refuse rather than assume it.
  const auto all_zero = [&](const uint32_t rva, const uint32_t size) {
    const auto *p = reinterpret_cast<const uint8_t *>(b + rva);
    if (!readable(p, size)) {
      return false;
    }
    for (uint32_t i = 0; i < size; ++i) {
      if (p[i] != 0) {
        return false;
      }
    }
    return true;
  };
  if (!all_zero(le_active_rva, le_old_active_size) ||
      !all_zero(le_free_rva, le_old_free_size) ||
      !all_zero(le_pool_rva, le_old_pool_size)) {
    return false;
  }

  auto *destination = allocate_near_module(le_block_size);
  if (!destination) {
    return false;
  }
  const auto new_base = reinterpret_cast<size_t>(destination) - b;

  const auto target_rva = [&](const le_ref &r) -> size_t {
    const uint32_t arr_off =
        r.arr == le_active ? le_off_active
                           : (r.arr == le_free ? le_off_free : le_off_pool);
    return new_base + arr_off + r.delta;
  };
  // rip-relative counts from the END of the instruction; the ABS32 form
  // used here is `[reg + disp32]` where the register holds the IMAGE
  // BASE, so its displacement is simply the RVA.
  const auto field_value = [&](const le_ref &r) {
    const auto tgt = target_rva(r);
    return r.rip ? static_cast<int32_t>(tgt - (r.rva + r.len))
                 : static_cast<int32_t>(tgt);
  };

  int32_t old_values[std::size(le_refs)] = {};
  size_t done = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < done; ++i) {
      write_bytes(
          reinterpret_cast<void *>(b + le_refs[i].rva + le_refs[i].disp_off),
          &old_values[i], sizeof(old_values[i]));
    }
  };

  for (size_t i = 0; i < std::size(le_refs); ++i) {
    const auto &r = le_refs[i];
    const int32_t value = field_value(r);
    auto *field = reinterpret_cast<void *>(b + r.rva + r.disp_off);
    std::memcpy(&old_values[i], field, sizeof(int32_t));
    if (!write_bytes(field, &value, sizeof(value))) {
      rollback();
      return false;
    }
    ++done;
  }

  // Read every field back before declaring success.
  for (size_t i = 0; i < std::size(le_refs); ++i) {
    const auto &r = le_refs[i];
    int32_t seen = 0;
    std::memcpy(&seen, reinterpret_cast<const void *>(b + r.rva + r.disp_off),
                sizeof(seen));
    if (seen != field_value(r)) {
      rollback();
      return false;
    }
  }

  le_new_base_rva = new_base;
  local_entities_relocated = true;
  // No status slot is free (0..95 are all taken). Verify from outside
  // with tools/verify_localentities.py, which reads the patched bytes
  // back out of the live process - stronger evidence than a flag.
  return true;
}

// --- RadiantExploderData: [2] -> [4], WITH A LAYOUT CHANGE ----------
//
// The 2026-08-20 20:36 round crashed at 0x00200AF7 writing through
// effectCount[localClientNum]. Retail PC embeds only TWO counts and TWO
// 500-pointer rows per record, and the record closes exactly:
//
//   +0x0000 hash   +0x0004 name(0x80)
//   +0x19E8 effectCount[2]      init clears rax..rax+8, i.e. 2 dwords
//   +0x19F0 effects[2][500]     2 * 500 * 8 = 0x1F40
//           0x19F0 + 0x1F40 = 0x3930 == stride, no slack anywhere
//
// so effectCount[2] IS the low dword of effects[0][0]. The dump proved it
// numerically: 0x086701F0 + 2*0x1F4 = 0x086705D8 = rax exactly.
//
// New layout, which closes the same way:
//
//   +0x19E8 effectCount[4]      0x10
//   +0x19F8 effects[4][500]     4 * 500 * 8 = 0x3E80
//           0x19F8 + 0x3E80 = 0x5878 == new stride
//           256 * 0x5878 = 0x587800   (old 0x393000 / 0x3930 = 256 exactly)
//
// NO RECORD-BY-RECORD TRANSFORM IS NEEDED, and that is why this is
// tractable. The array is per-map FX data registered by
// CG_RadiantRegisterFXExploder at map load; this runs at post_unpack,
// before any map, and CG_RadiantExplodersReset memsets the array and
// zeroes the live count. If that count is 0 every byte is zero, so a
// zeroed destination IS the correctly transformed result. The guard below
// refuses when it is not 0 rather than assuming it.
//
// THE TRAP, measured: +0x19F0 and +0x19E8 are NOT unique to this array.
// Boundary-validated enumeration over all real code found 15 instructions
// using +0x19F0 (only ONE ours) and 13 using +0x19E8 (only THREE ours);
// the rest are unrelated structures with a field at the same offset. So
// every site below is an EXPLICIT address with its expected bytes, never
// a pattern.
constexpr uint32_t exploder_base_rva = 0x43016E0;
constexpr uint32_t exploder_count_rva = 0x43016C4;
constexpr uint32_t exploder_new_size = 0x587800;
constexpr uint32_t exploder_new_stride = 0x5878;
constexpr uint32_t exploder_counts_off = 0x19E8; // unchanged
constexpr uint32_t exploder_new_effects_off = 0x19F8;

// Expected ORIGINAL bytes, so a different build fails closed instead of
// corrupting code. Every boundary was confirmed by linear sweep from a
// distant anchor - a naive immediate scan reported four of these one byte
// late by matching the un-prefixed decode and missing the REX.W prefix.
constexpr uint8_t exploder_expect_A49[] = {0x48, 0x69, 0xC9, 0x30,
                                           0x39, 0x00, 0x00};
constexpr uint8_t exploder_expect_A5C[] = {0x48, 0x81, 0xC6, 0x30,
                                           0x39, 0x00, 0x00};
constexpr uint8_t exploder_expect_703[] = {0x49, 0x81, 0xC0, 0x30,
                                           0x39, 0x00, 0x00};
constexpr uint8_t exploder_expect_7A4[] = {0x41, 0xB8, 0x00, 0x30, 0x39, 0x00};
constexpr uint8_t exploder_expect_AF7[] = {0x48, 0x89, 0xAC, 0xC6,
                                           0xF0, 0x19, 0x00, 0x00};
constexpr uint8_t exploder_expect_AC9[] = {0x48, 0x8D, 0x48, 0x08};
constexpr uint8_t exploder_expect_589[] = {0x48, 0x8D, 0x3D, 0x40,
                                           0xDB, 0x0F, 0x04};
constexpr uint8_t exploder_expect_5D7[] = {0x48, 0x8D, 0x3D, 0xF2,
                                           0xDA, 0x0F, 0x04};
constexpr uint8_t exploder_expect_6E3[] = {0x48, 0x8D, 0x3D, 0xE6,
                                           0xD9, 0x0F, 0x04};
constexpr uint8_t exploder_expect_5CF[] = {0x48, 0x63, 0x84, 0x87,
                                           0xC8, 0x30, 0x30, 0x04};
constexpr uint8_t exploder_expect_79B[] = {0x48, 0x8D, 0x0D, 0x3E,
                                           0x3F, 0x10, 0x04};
constexpr uint8_t exploder_expect_A38[] = {0x48, 0x8D, 0x35, 0xA1,
                                           0x0C, 0x10, 0x04};

bool exploders_relocated = false;
size_t exploder_new_base_rva = 0;

bool relocate_radiant_exploders() {
  if (exploders_relocated) {
    return true;
  }

  const auto b = base();

  // Verify every original site first. One mismatch and nothing at all
  // is written.
  const auto expect = [&](const uint32_t rva, const uint8_t *want,
                          const size_t n) {
    const auto *p = reinterpret_cast<const void *>(b + rva);
    return readable(p, n) && std::memcmp(p, want, n) == 0;
  };
  if (!expect(0x00200A49, exploder_expect_A49, sizeof(exploder_expect_A49)) ||
      !expect(0x00200A5C, exploder_expect_A5C, sizeof(exploder_expect_A5C)) ||
      !expect(0x00205703, exploder_expect_703, sizeof(exploder_expect_703)) ||
      !expect(0x001FD7A4, exploder_expect_7A4, sizeof(exploder_expect_7A4)) ||
      !expect(0x00200AF7, exploder_expect_AF7, sizeof(exploder_expect_AF7)) ||
      !expect(0x00200AC9, exploder_expect_AC9, sizeof(exploder_expect_AC9)) ||
      !expect(0x00205589, exploder_expect_589, sizeof(exploder_expect_589)) ||
      !expect(0x002055D7, exploder_expect_5D7, sizeof(exploder_expect_5D7)) ||
      !expect(0x002056E3, exploder_expect_6E3, sizeof(exploder_expect_6E3)) ||
      !expect(0x002055CF, exploder_expect_5CF, sizeof(exploder_expect_5CF)) ||
      !expect(0x001FD79B, exploder_expect_79B, sizeof(exploder_expect_79B)) ||
      !expect(0x00200A38, exploder_expect_A38, sizeof(exploder_expect_A38))) {
    // Byte mismatch: a different build. Refuse and write nothing.
    return false;
  }

  // THE GUARD that replaces a record transformer. A non-zero live count
  // means records already exist, and a flat zeroed destination would
  // silently lose them.
  uint32_t live_records = 0;
  std::memcpy(&live_records,
              reinterpret_cast<const void *>(b + exploder_count_rva),
              sizeof(live_records));
  if (live_records != 0) {
    // Already populated: a flat zeroed destination would lose records.
    return false;
  }

  auto *destination = allocate_near_module(exploder_new_size);
  if (!destination) {
    // No reachable 5.8 MB hole within a 32-bit displacement.
    return false;
  }
  const auto new_base = reinterpret_cast<size_t>(destination) - b;
  // VirtualAlloc zero-fills and the source is provably all zeros, so the
  // destination already IS the transformed array. Nothing to copy.

  struct pending {
    uint32_t rva;
    uint8_t disp_offset;
    int32_t value;
  };

  const auto rip_to = [&](const size_t insn, const size_t len,
                          const size_t target) {
    return static_cast<int32_t>(target - (insn + len));
  };

  const pending writes[] = {
      // base references
      {0x001FD79B, 3, rip_to(0x001FD79B, 7, new_base)},
      {0x00200A38, 3, rip_to(0x00200A38, 7, new_base)},
      // &effects[0][0] moves to the NEW field offset, not the old one
      {0x00205589, 3,
       rip_to(0x00205589, 7, new_base + exploder_new_effects_off)},
      {0x002055D7, 3,
       rip_to(0x002055D7, 7, new_base + exploder_new_effects_off)},
      {0x002056E3, 3,
       rip_to(0x002056E3, 7, new_base + exploder_new_effects_off)},
      // &effectCount[0] is an ABS32 displacement off rdi, and rdi holds
      // the IMAGE BASE (set by `lea rdi,[rip-0x2055cf]` at 0x002055C8),
      // so this displacement is an RVA.
      {0x002055CF, 4, static_cast<int32_t>(new_base + exploder_counts_off)},
      // strides and sizes
      {0x00200A49, 3, static_cast<int32_t>(exploder_new_stride)},
      {0x00200A5C, 3, static_cast<int32_t>(exploder_new_stride)},
      {0x00205703, 3, static_cast<int32_t>(exploder_new_stride)},
      {0x001FD7A4, 2, static_cast<int32_t>(exploder_new_size)},
      // the ONE effects displacement that belongs to this array
      {0x00200AF7, 4, static_cast<int32_t>(exploder_new_effects_off)},
  };

  int32_t old_values[std::size(writes)] = {};
  size_t done = 0;
  const auto rollback = [&] {
    for (size_t i = 0; i < done; ++i) {
      write_bytes(
          reinterpret_cast<void *>(b + writes[i].rva + writes[i].disp_offset),
          &old_values[i], sizeof(old_values[i]));
    }
  };

  for (size_t i = 0; i < std::size(writes); ++i) {
    auto *field =
        reinterpret_cast<void *>(b + writes[i].rva + writes[i].disp_offset);
    std::memcpy(&old_values[i], field, sizeof(int32_t));
    if (!write_bytes(field, &writes[i].value, sizeof(writes[i].value))) {
      rollback();
      return false;
    }
    ++done;
  }

  // `lea rcx,[rax+8]` -> `[rax+0x10]`: the init loop must now clear FOUR
  // count dwords, not two. Single byte, 8-bit displacement.
  const uint8_t clear_len = 0x10;
  auto *clear_field = reinterpret_cast<void *>(b + 0x00200AC9 + 3);
  const uint8_t old_clear = *reinterpret_cast<const uint8_t *>(clear_field);
  if (!write_bytes(clear_field, &clear_len, sizeof(clear_len))) {
    rollback();
    return false;
  }

  // Read every field back before declaring success.
  for (size_t i = 0; i < std::size(writes); ++i) {
    int32_t seen = 0;
    std::memcpy(&seen,
                reinterpret_cast<const void *>(b + writes[i].rva +
                                               writes[i].disp_offset),
                sizeof(seen));
    if (seen != writes[i].value) {
      write_bytes(clear_field, &old_clear, sizeof(old_clear));
      rollback();
      return false;
    }
  }

  exploder_new_base_rva = new_base;
  exploders_relocated = true;
  // No status slot: 0..95 are all taken (71 is ShoutcasterReadResult,
  // 73 is the netchan base) and clobbering a live diagnostic to report
  // this would be a bad trade. Verify from outside instead by reading
  // the patched bytes, which is stronger evidence than a flag anyway:
  //   0x00200A49+3 == 0x5878   stride patched
  //   0x00200AF7+4 == 0x19F8   effects displacement patched
  //   0x00200AC9+3 == 0x10     init clears four counts
  return true;
}

// --- PUT THE NATIVE GUEST INTO THE REAL LOBBY -----------------------
//
// The component used to stop after Live_HandleClientSplitscreenSignin.
// Treyarch's Lua SigninLocalClient continues from there into
// LobbyHost_AddLocalClients, but repeating that plural function late is
// unsafe: LobbySession_AddClientAtPosition returns an existing XUID and
// the outer loop still increments session+0xF0 for it. The supported
// single-client path is LobbyHost_AddLocalClient(actionId, ci, type),
// named by the PS4 symbol at 0xCA5C70 and exposed on PC by
// Engine.LobbyHostAddLocal. Its first argument is explicitly `actionId`;
// zero is the PC binding's valid default and only tags the UI result.
constexpr uint32_t lobby_host_add_local_rva = 0x1ECAAF0;
constexpr uint32_t lobby_get_session_rva = 0x1ED03E0;
constexpr uint32_t lobby_get_client_by_xuid_rva = 0x1EF3920;
constexpr uint32_t live_user_get_xuid_rva = 0x1EBA880;
constexpr uint32_t mutable_client_info_rva = 0x1EBEB00;
constexpr uint32_t lobby_update_client_rva = 0x1EF5590;
constexpr int game_lobby_type = 1;

constexpr uint8_t lobby_host_add_local_bytes[] = {
    0xE9, 0xDB, 0xC7, 0x00, 0x00,
};
constexpr uint8_t lobby_get_session_bytes[] = {
    0x83, 0xF9, 0x01, 0x77, 0x16, 0x48, 0x63, 0xC1,
};
constexpr uint8_t lobby_get_client_bytes[] = {
    0x4C, 0x8D, 0x89, 0xF8, 0x00, 0x00, 0x00, 0x33, 0xC0,
};
constexpr uint8_t live_user_get_xuid_bytes[] = {
    0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x8B, 0xD9,
};
constexpr uint8_t mutable_client_info_bytes[] = {
    0x48, 0x8B, 0xC4, 0x48, 0x89, 0x50, 0x10, 0x55, 0x41, 0x56,
};
constexpr uint8_t lobby_update_client_bytes[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x6C,
    0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57,
};

template <size_t N>
bool engine_bytes_match(const uint32_t rva, const uint8_t (&expected)[N]) {
  const auto *p = reinterpret_cast<const void *>(base() + rva);
  return readable(p, N) && std::memcmp(p, expected, N) == 0;
}

bool lobby_enrollment_api_matches() {
  return engine_bytes_match(lobby_host_add_local_rva,
                            lobby_host_add_local_bytes) &&
         engine_bytes_match(lobby_get_session_rva, lobby_get_session_bytes) &&
         engine_bytes_match(lobby_get_client_by_xuid_rva,
                            lobby_get_client_bytes) &&
         engine_bytes_match(live_user_get_xuid_rva, live_user_get_xuid_bytes);
}

// LobbyBase_GetNetworkMode (PS4 0xCBFB40, 7 B) is PC 0x01EE8160:
//     mov eax, [0x1574D28C] ; ret
// The only store to 0x1574D28C is 0x01EE82F0, which has the exact shape
// of PS4 LobbyBase_SetNetworkMode 0xCBFAE0 (store, LobbyUI update, then
// Com_SessionMode_SetNetworkMode of the converted value). 0x01E0C960
// branches on this value (`cmp eax,1` LAN, `cmp eax,2` LIVE). Names from
// PS4 LobbyTypes_GetLobbyNetworkModeName 0xCC55B0: 0 LOCAL, 1 LAN, 2 LIVE.
// Measured: BOIII main menu 2 (LIVE); Zombies PRIVATE GAME lobby 1 (LAN).
constexpr uint32_t lobby_get_network_mode_rva = 0x1EDB7F0;
constexpr uint8_t lobby_get_network_mode_bytes[] = {
    0x8B, 0x05, 0x66, 0x24, 0x7F, 0x13, 0xC3,
};
constexpr int lobby_network_local = 0;
constexpr int lobby_network_lan = 1;

// JOIN ORDER: the host and the native guest must already be members of
// the GAME lobby before player 3 is seated or enrolled.
//
// Measured 2026-09-26 09:47, activation at the main menu and then
// Zombies: START GAME hung on a black screen with the spinner. The
// launch pump (PC 0x01EDD8B4, inlined PS4 HasAllClientsGotLatestStateMsg
// 0xCAF3B0) waits until every active SessionClient of the game lobby -
// 18 slots of 0x30 at session+0xF8 - has an ack at session+0x2780+4*i
// newer than the state number at 0x15749480. Live: state 14; host ack 16,
// player 2 ack 16, player 3 ack 5 - frozen. Player 3 sat in slot 0 with
// joinOrder (+0x20; PS4 SessionClient 0x28 names it) 0, player 2 had 1,
// the host 2: player 3 was added to the brand-new game lobby BEFORE the
// host, and its entry also lacked the 8-byte value at +0x28 that the host
// and player 2 share. On the working PRIVATE GAME -> ACTIVATE path the
// lobby already holds both when player 3 arrives, so he is always last.
//
// 2026-09-27: player 2 is required only while he is SEATED - on console
// a local player may leave on his own and the others stay, so player 3
// can (re)join a lobby of host + player 3. What must never happen is
// player 3 overtaking anyone already seated.
// `below`: the guest being enrolled - every SEATED controller below it
// (the host always) must already be a game-lobby member (join order).
// Player 4 (2026-09-27): controller 3 waits for 0 and any seated 1/2.
bool host_and_guest_in_game_lobby(void *session, const int below = 2) {
  using get_xuid_fn = uint64_t (*)(int);
  using get_client_fn = void *(*)(void *, uint64_t);
  const auto get_xuid =
      reinterpret_cast<get_xuid_fn>(base() + live_user_get_xuid_rva);
  const auto get_client =
      reinterpret_cast<get_client_fn>(base() + lobby_get_client_by_xuid_rva);
  for (int controller = 0; controller < below; ++controller) {
    if (controller >= 1 && !controller_seated(controller)) {
      continue;
    }
    const auto xuid = get_xuid(controller);
    if (!xuid || !get_client(session, xuid)) {
      return false;
    }
  }
  return true;
}

// Player 3 may be seated only in an offline lobby (LOCAL or LAN - never
// LIVE, nor any value outside the enum) whose GAME lobby is up - session
// +0x40 is the state LobbyHost_AddLocalClient itself insists on (see
// ensure_guest2_game_lobby) - and already holds the host and the native
// guest (join order, above). Unverified bytes answer "no": on a build
// these checks do not recognise, player 3 is never seated.
bool offline_lobby_ready_for_player3() {
  if (!engine_bytes_match(lobby_get_network_mode_rva,
                          lobby_get_network_mode_bytes) ||
      !lobby_enrollment_api_matches()) {
    return false;
  }

  const auto network_mode =
      reinterpret_cast<int (*)()>(base() + lobby_get_network_mode_rva)();
  if (network_mode != lobby_network_local &&
      network_mode != lobby_network_lan) {
    return false;
  }

  auto *session = reinterpret_cast<void *(*)(int)>(
      base() + lobby_get_session_rva)(game_lobby_type);
  return session &&
         *reinterpret_cast<const uint32_t *>(reinterpret_cast<size_t>(session) +
                                             0x40) != 0 &&
         host_and_guest_in_game_lobby(session);
}

bool profile_publish_api_matches() {
  return engine_bytes_match(lobby_get_session_rva, lobby_get_session_bytes) &&
         engine_bytes_match(lobby_get_client_by_xuid_rva,
                            lobby_get_client_bytes) &&
         engine_bytes_match(live_user_get_xuid_rva, live_user_get_xuid_bytes) &&
         engine_bytes_match(mutable_client_info_rva,
                            mutable_client_info_bytes) &&
         engine_bytes_match(lobby_update_client_rva, lobby_update_client_bytes);
}

bool lobby_enrollment_in_progress = false;
bool guest2_lobby_enrolled = false;
uint32_t lobby_enrollment_attempts = 0;
constexpr uint32_t lobby_enrollment_max_attempts = 16;

// Per-guest lobby state. Controller 2 keeps its original globals (the
// proven player-3 path); controller 3 (player 4) gets its own set.
struct guest_lobby_state {
  bool join_done = false;
  bool enrolled = false;
  uint32_t enroll_attempts = 0;
  bool published = false;
  uint32_t publish_attempts = 0;
};
guest_lobby_state guest3_lobby{};

bool ensure_guest_game_lobby(int controller, bool join_done, bool &enrolled,
                             uint32_t &attempts);

bool ensure_guest2_game_lobby() {
  return ensure_guest_game_lobby(2, guest_join_done, guest2_lobby_enrolled,
                                 lobby_enrollment_attempts);
}

bool ensure_guest_game_lobby(const int controller, const bool join_done,
                             bool &enrolled, uint32_t &attempts) {
  if (enrolled) {
    return true;
  }
  if (!join_done || lobby_enrollment_in_progress ||
      attempts >= lobby_enrollment_max_attempts) {
    return false;
  }
  if (!lobby_enrollment_api_matches()) {
    report87(47);
    return false;
  }

  using get_session_fn = void *(*)(int);
  using get_xuid_fn = uint64_t (*)(int);
  using get_client_fn = void *(*)(void *, uint64_t);
  using add_local_fn = void (*)(int, int, int);

  const auto get_session =
      reinterpret_cast<get_session_fn>(base() + lobby_get_session_rva);
  const auto get_xuid =
      reinterpret_cast<get_xuid_fn>(base() + live_user_get_xuid_rva);
  const auto get_client =
      reinterpret_cast<get_client_fn>(base() + lobby_get_client_by_xuid_rva);
  const auto add_local =
      reinterpret_cast<add_local_fn>(base() + lobby_host_add_local_rva);

  const auto xuid = get_xuid(controller);
  auto *session = get_session(game_lobby_type);
  // LobbyHost_AddLocalClient itself rejects session+0x40 == 0. Test
  // the same producer-owned state before entering it and retry later.
  if (!xuid || !session ||
      *reinterpret_cast<const uint32_t *>(reinterpret_cast<size_t>(session) +
                                          0x40) == 0) {
    report87(48);
    return false;
  }

  if (get_client(session, xuid)) {
    enrolled = true;
    report87(50);
    return true;
  }

  // Never ahead of the host and the native guest - a player 3 added to
  // a game lobby that does not hold them yet gets joinOrder 0 and an
  // entry the launch pump never acks (host_and_guest_in_game_lobby).
  // Wait for them without spending an attempt.
  if (!host_and_guest_in_game_lobby(session, controller)) {
    report87(55);
    return false;
  }

  const in_progress_guard guard(lobby_enrollment_in_progress);
  ++attempts;
  report87(49);
  add_local(0, controller, game_lobby_type);

  enrolled = get_client(session, xuid) != nullptr;
  if (enrolled) {
    report87(50);
  }
  return enrolled;
}

bool guest2_profile_published = false;
bool profile_publish_in_progress = false;
uint32_t profile_publish_attempts = 0;

bool refresh_guest_lobby_profile(int controller, bool enrolled, bool &published,
                                 uint32_t &attempts);

bool refresh_guest2_lobby_profile() {
  return refresh_guest_lobby_profile(2, guest2_lobby_enrolled,
                                     guest2_profile_published,
                                     profile_publish_attempts);
}

bool refresh_guest_lobby_profile(const int controller, const bool enrolled,
                                 bool &published, uint32_t &attempts) {
  if (published) {
    return true;
  }
  if (!enrolled || profile_publish_in_progress) {
    return false;
  }
  if (!profile_publish_api_matches()) {
    report87(51);
    return false;
  }

  using mutable_info_fn = void (*)(int, void *);
  using get_session_fn = void *(*)(int);
  using get_xuid_fn = uint64_t (*)(int);
  using get_client_fn = void *(*)(void *, uint64_t);
  using update_client_fn = bool (*)(void *, uint64_t, const void *, bool *);

  const in_progress_guard guard(profile_publish_in_progress);
  ++attempts;
  alignas(16) std::array<uint8_t, 0x410> info{};
  reinterpret_cast<mutable_info_fn>(base() + mutable_client_info_rva)(
      controller, info.data());

  // PC LobbyClient_GetMutableClientInfo writes the five equipped gum IDs
  // at +0x20..+0x24. Empty is the exact state we are repairing, and it
  // also means CAC/storage is not ready yet, so leave the official
  // publisher to be retried rather than copying another player's data.
  bool gums_ready = false;
  for (size_t i = 0x20; i < 0x25; ++i) {
    gums_ready = gums_ready || info[i] != 0;
  }
  if (!gums_ready) {
    report87(52);
    return false;
  }

  const auto get_session =
      reinterpret_cast<get_session_fn>(base() + lobby_get_session_rva);
  const auto get_xuid =
      reinterpret_cast<get_xuid_fn>(base() + live_user_get_xuid_rva);
  const auto get_client =
      reinterpret_cast<get_client_fn>(base() + lobby_get_client_by_xuid_rva);
  const auto update_client =
      reinterpret_cast<update_client_fn>(base() + lobby_update_client_rva);

  const auto xuid = get_xuid(controller);
  if (!xuid) {
    return false;
  }

  bool updated_any = false;
  for (int lobby_type = 0; lobby_type < 2; ++lobby_type) {
    auto *session = get_session(lobby_type);
    if (!session || !get_client(session, xuid)) {
      continue;
    }
    bool changed = false;
    updated_any =
        update_client(session, xuid, info.data(), &changed) || updated_any;
  }

  if (updated_any) {
    published = true;
    report87(53);
  }
  return published;
}

// PLAYER 4 (2026-09-27): controller 3 joins through the stock Lua (A);
// the engine then leaves him out of the game lobby and without a
// published profile, exactly as it did controller 2 - the same two
// steps, every frame from the per-controller update for controller 3.
void guest3_input_frame();

void advance_guest3_join() {
  guest3_input_frame();
  auto &g = guest3_lobby;
  if (g.join_done &&
      ensure_guest_game_lobby(3, g.join_done, g.enrolled, g.enroll_attempts)) {
    refresh_guest_lobby_profile(3, g.enrolled, g.published, g.publish_attempts);
  }
}

// Defined with the Deactivate code further down: player 3's own
// controller - A joins, B or unplugging leaves (console behaviour).
void guest2_input_frame();

// Runs every frame (Live_Frame 0x0135C8A4 -> per_controller_update_stub
// for controller 2). Player 3 is no longer seated automatically: like
// every local player on console he joins by pressing A on his own
// controller in the lobby (guest2_input_frame).
void advance_guest2_join() {
  // Every frame until it has happened: the host alone in an offline
  // lobby is enough (gamepads_may_activate), so player 3's pad is live
  // before anyone else joins - as on console.
  activate_gamepads_in_lobby();
  guest2_input_frame();
  if (guest_join_done && ensure_guest2_game_lobby()) {
    refresh_guest2_lobby_profile();
  }
}

// Hook the plural add only to move the sign-in as early as possible. If
// controller 2 is already poll-ready, the original loop sees and adds it
// in the same pass. If not, the later single-client path above handles it
// without ever repeating the plural function or corrupting +0xF0.
constexpr uint32_t lobby_add_all_rva = 0x1ECAB00;
constexpr uint8_t lobby_add_all_prologue[] = {
    0x48, 0x8B, 0xC4, 0x57, 0x41, 0x54, 0x41, 0x55,
    0x41, 0x56, 0x41, 0x57, 0x48, 0x81, 0xEC, 0xB0,
};
utils::hook::detour lobby_add_all_hook;

bool lobby_add_all_stub(const int lobby_type) {
  // Relocate the gamepad table as soon as player 2 is seated, so
  // player 3's controller gets its slot right away. Player 3 himself
  // is NOT seated here any more - he presses A (guest2_input_frame).
  if (lobby_type == game_lobby_type && (seat_flags(0) & 1) &&
      (seat_flags(1) & 1)) {
    activate_gamepads_in_lobby();
  }

  const auto result = lobby_add_all_hook.invoke<bool>(lobby_type);

  if (lobby_type == game_lobby_type && (seat_flags(0) & 1) &&
      (seat_flags(1) & 1)) {
    activate_gamepads_in_lobby();
  }

  if (guest_join_done && ensure_guest2_game_lobby()) {
    refresh_guest2_lobby_profile();
  }
  return result;
}

// --- DEACTIVATE SPLITSCREEN: player 3 leaves with player 2 -----------
//
// Test report 2026-09-27: DEACTIVATE SPLITSCREEN removes player 2 only;
// player 3 stays seated (seats 1110 -> 1010), the button keeps saying
// Deactivate and does nothing, the lobby is unusable until a restart.
//
// PS4 Lua_CoD_LuaCall_SetLocalClientToInactive 0xCF8070, per controller:
//     if LobbyClient_IsActive(GAME)  LobbyVM_OnLocalClientLeave(ci)
//     elif LobbyHost_IsHost(PARTY)   LobbyHost_RemoveClient(PARTY, xuid,
//                                        "Local Client Left.")
//     Live_HandleClientSplitscreenSignin(ci, false, false)   ALWAYS
// The C side is per controller; WHICH controllers leave is decided in
// Lua. Player 3 never went through the Lua join - the component seats
// him through the engine (0x01E0C960 + LobbyHost_AddLocalClient,
// ensure_guest2_game_lobby) - so the button's Lua never asks for him.
//
// MEASURED 2026-09-27 10:56 and ~11:05 (build A25FFA7D, commit 3221940):
// the first attempt detoured the LobbyVM leave 0x01EF04B0 for
// controllers 2/3, assuming the Lua refused him there. Two DEACTIVATE
// presses, seats 1110 -> 1010 and then unchanged, and NOT ONE call for
// controller 2 - he is never asked about at all.
// LobbyRemoveAllLocalSplitscreenClient 0x01F16D60 would have reached
// him (seat lookup 0x020EF7C0 is relocated to 3 slots at
// 0x1A8A7508..0x1A8A7574, seat 2 in use, game lobby active -> LobbyVM
// branch), so that binding is not what the button runs.
//
// First fix (2e692e0, verified 11:05-11:07): player 3 followed player 2
// out at Live_HandleClientSplitscreenSignin 0x01E0C960. REPLACED the same
// day - the goal is console behaviour, where every local player
// leaves on his own and the others stay. Now: DEACTIVATE removes every
// extra player in Lua (ui_scripts/zz_splitscreen overrides the stock
// LobbySplitscreenToggle, which only ever touches controller 1); for
// controller 2 that ends in SetLocalClientToInactive(2) -> the LobbyVM
// leave below, whose refusal is repaired with the engine remove; and
// player 3's own controller joins (A) and leaves (B, unplugged) through
// guest2_input_frame. 0x01E0C960 stays detoured for its log line: every
// splitscreen sign-in/out with seats and stack.
//
// Sign-out 0x01E0C960(ci, false, false) takes the r14b == 0 branch
// straight to 0x01E0CA44: SetBeingUsed(lc, false) on the relocated seat
// table and userData[ci]+0x28 via 0x01EC6E80 - the same two entries the
// verified sign-in of controller 2 writes (rule 4: nothing new indexed).
// LobbySession_RemoveClient's peer-drop loop (0x01F01AF8, cmp ebx,2)
// runs over controllers 0/1 only, exactly as for player 2's removal.
//
// Every call of 0x01E0C960 is logged with its stack to
// splitscreen_ui_trace.txt, so the button's real path is on record.
constexpr uint32_t lobbyvm_local_leave_rva = 0x1EE3B40;
constexpr uint8_t lobbyvm_local_leave_prologue[] = {
    0x40, 0x57,                               // push rdi
    0x48, 0x81, 0xEC, 0xB0, 0x00, 0x00, 0x00, // sub rsp, 0xB0
    0x48, 0xC7, 0x44, 0x24, 0x38, 0xFE, 0xFF, 0xFF, 0xFF,
};
constexpr uint8_t guest_signin_prologue[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, // mov [rsp+8], rbx
    0x48, 0x89, 0x6C, 0x24, 0x18, // mov [rsp+18h], rbp
    0x48, 0x89, 0x74, 0x24, 0x20, // mov [rsp+20h], rsi
    0x57, 0x41, 0x56, 0x41, 0x57, // push rdi / r14 / r15
};
constexpr uint32_t lobby_host_is_host_rva = 0x1ECC700;
constexpr uint8_t lobby_host_is_host_bytes[] = {
    0x48, 0x83, 0xEC, 0x28, 0xE8, 0xD7, 0x3C, 0x00, 0x00, // call 0x01EDCDD0
};
constexpr uint32_t lobby_host_remove_client_rva = 0x1ECD250;
constexpr uint8_t lobby_host_remove_client_bytes[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
    0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20,
};
constexpr uint32_t local_client_left_reason_rva = 0x2FB1188;
constexpr char local_client_left_reason[] = "Local Client Left.";
utils::hook::detour lobbyvm_local_leave_hook;
utils::hook::detour guest_signin_hook;

// Player 3 is gone: the next ACTIVATE must seat and enrol him afresh.
void reset_guest2_join() {
  guest_join_done = false;
  guest_join_attempts = 0;
  guest2_lobby_enrolled = false;
  lobby_enrollment_attempts = 0;
  guest2_profile_published = false;
  profile_publish_attempts = 0;
}

// Take a component-seated controller out of every lobby we host that
// lists him. True when no lobby still lists him afterwards - only then
// may he be signed out: a seat without a lobby entry, or a lobby entry
// without a seat, is the "Failed to host lobby" state.
bool remove_guest_from_lobbies(const int controller, trace_line &l) {
  const auto *reason =
      reinterpret_cast<const char *>(base() + local_client_left_reason_rva);
  if (!lobby_enrollment_api_matches() ||
      !engine_bytes_match(lobby_get_network_mode_rva,
                          lobby_get_network_mode_bytes) ||
      !engine_bytes_match(lobby_host_is_host_rva, lobby_host_is_host_bytes) ||
      !engine_bytes_match(lobby_host_remove_client_rva,
                          lobby_host_remove_client_bytes) ||
      !readable(reason, sizeof(local_client_left_reason)) ||
      std::memcmp(reason, local_client_left_reason,
                  sizeof(local_client_left_reason)) != 0) {
    l.str(" lobbies=bytes-mismatch");
    return false;
  }

  const auto network_mode =
      reinterpret_cast<int (*)()>(base() + lobby_get_network_mode_rva)();
  const auto xuid = reinterpret_cast<uint64_t (*)(int)>(
      base() + live_user_get_xuid_rva)(controller);
  l.str(" mode=");
  l.dec(static_cast<uint64_t>(network_mode));
  if ((network_mode != lobby_network_local &&
       network_mode != lobby_network_lan) ||
      !xuid) {
    l.str(" lobbies=refused");
    return false;
  }

  using get_session_fn = void *(*)(int);
  using get_client_fn = void *(*)(void *, uint64_t);
  using is_host_fn = bool (*)(int);
  using remove_fn = bool (*)(int, uint64_t, const char *);
  const auto get_session =
      reinterpret_cast<get_session_fn>(base() + lobby_get_session_rva);
  const auto get_client =
      reinterpret_cast<get_client_fn>(base() + lobby_get_client_by_xuid_rva);
  const auto is_host =
      reinterpret_cast<is_host_fn>(base() + lobby_host_is_host_rva);
  const auto remove =
      reinterpret_cast<remove_fn>(base() + lobby_host_remove_client_rva);

  // Game lobby first (where ensure_guest2_game_lobby put him), then the
  // party in case he ever landed there too. A lobby that does not hold
  // him is left alone; one we do not host cannot be changed from here.
  bool failed = false;
  for (const int lobby_type : {game_lobby_type, 0}) {
    auto *session = get_session(lobby_type);
    const bool member = session && get_client(session, xuid);
    l.str(lobby_type == game_lobby_type ? " game=" : " party=");
    if (!member) {
      l.str("absent");
      continue;
    }
    if (!is_host(lobby_type)) {
      l.str("not-host");
      failed = true;
      continue;
    }
    const bool removed = remove(lobby_type, xuid, reason);
    l.str(removed ? "removed" : "remove-failed");
    failed = failed || !removed;
  }
  return !failed;
}

// Kept from the first attempt: if any path ever does ask the LobbyVM
// to let controller 2/3 leave and the Lua refuses, remove him from the
// lobbies and let the caller sign him out.
bool lobbyvm_local_leave_stub(const int controller,
                              const uint64_t client_xuid) {
  const auto lua_result =
      lobbyvm_local_leave_hook.invoke<bool>(controller, client_xuid);
  if (controller < 2 || controller >= 4) {
    return lua_result;
  }

  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" local_leave ci=");
  l.dec(static_cast<uint64_t>(controller));
  l.str(" lua=");
  l.dec(lua_result ? 1 : 0);

  const bool result = lua_result || remove_guest_from_lobbies(controller, l);
  if (result && controller == 2) {
    reset_guest2_join();
  }
  l.str(" signout=");
  l.dec(result ? 1 : 0);
  trace_write(l);
  return result;
}

uint32_t seat_bits() {
  uint32_t bits = 0;
  for (uint32_t lc = 0; lc < signin_new_slots; ++lc) {
    bits |= static_cast<uint32_t>(seat_flags(lc) & 1) << lc;
  }
  return bits;
}

void guest_signin_stub(const int controller, const bool signin,
                       const bool arg3) {
  const auto before = seat_bits();
  guest_signin_hook.invoke<void>(controller, signin, arg3);
  const auto after = seat_bits();

  {
    trace_line l;
    trace_head(l, "splitscreen_signin", call_site_of(_ReturnAddress()));
    l.str(" ci=");
    l.dec(static_cast<uint64_t>(controller));
    l.str(" add=");
    l.dec(signin ? 1 : 0);
    l.str(" a3=");
    l.dec(arg3 ? 1 : 0);
    l.str(" seats=");
    l.hex(before);
    l.str("->");
    l.hex(after);
    trace_stack(l);
    trace_write(l);
  }

  // Controller 2 signed out by any path: the next A press starts afresh.
  // (2026-09-27, console behaviour - so player 3 no longer
  // follows player 2 out; each local player leaves on his own.)
  if (!signin && controller == 2 && !controller_seated(2)) {
    reset_guest2_join();
  }
  // Controller 2 seated by the STOCK join (CoDMenu -> LobbyAddLocalClient
  // -> Engine.SigninLocalClient reaches this same function): the rest of
  // the join - game-lobby entry, profile - follows exactly as after the
  // component's own call, which latches the same flag on the same test.
  if (signin && controller == 2 && controller_seated(2)) {
    guest_join_done = true;
  }
  if (controller == 3) {
    if (signin && controller_seated(3)) {
      guest3_lobby.join_done = true;
    } else if (!signin && !controller_seated(3)) {
      guest3_lobby = {};
    }
  }

  // LOCAL CLIENT 2 IS REAL THE MOMENT ITS SEAT IS - initialise it now.
  //
  // PS4 Com_Init runs CL_Init(lc) for lc 0..3 at boot (0xE49E98 `cmp 4`);
  // the PC boots 0..1 and this component supplies CL_Init(2) - until now
  // only once THREE seats existed. Measured 2026-09-27 16:14 (player 3
  // joined before player 2, seats {0, 2}): at START GAME
  // Com_LocalClients_CompressClients (PS4 0xE35450, PC 0x020EFF70) packs
  // controller 2 into lc 1, and SwapClients (PS4 0xE356F0, PC 0x020F0150)
  // MEMCPYs whole clientUIActives entries - so lc 1 received lc 2's
  // never-initialised state (flags 0x4, bit 1 = CL_Init's receipt
  // missing; lc 2 got 0x2). lc 1 then parked at CA_CONFIRMLOADING (6)
  // exactly like lc 2 in August before CL_Init(2) existed, and the
  // round fell back to the frontend. The earlier working
  // "host + player 3" round had had a 3-player round first, i.e.
  // CL_Init(2) done. The seat of lc 2 (record 2, before any compress)
  // is the precondition run_cl_init_for_local_client2 was designed for
  // ("the INIT runs as soon as the seat is real"); the sign-in is the
  // moment it becomes real, on the game thread - the same context the
  // three-seat trigger already ran it in, in the lobby.
  if (signin && (seat_flags(2) & 1) && raise_local_client_count &&
      signin_relocated && !lc2_fully_done()) {
    run_cl_init_for_local_client2();
  }
  // PLAYER 4: local client 3 is real the moment seat record 3 is in use.
  if (signin && (seat_flags(3) & 1) && raise_local_client_count &&
      signin_relocated) {
    run_cl_init_for_local_client3();
  }
  // Every added controller starts from player 1's classes and stats
  // (see reread_guest_saves) - only on a real seat add.
  if (signin && after != before && controller_seated(controller)) {
    reread_guest_saves(controller);
  }
}

// ---- TRACE: SwapClients (PC 0x020F0150, PS4 SwapClients 0xE356F0) ----
//
// Diagnostic, read-only. SwapClients memcpys clientConnections[a] and [b]
// (base [0x053D8BB8], stride 0x25780) with only a null check; if the
// block is carved for fewer clients than max(a, b) + 1, the swap reads
// and writes past it. Logs both indices, cl_maxLocalClients, whether the
// connection block exists, and both clients' flags before and after.
constexpr uint32_t swap_clients_rva = 0x20E39D0;
constexpr uint8_t swap_clients_prologue[] = {
    0x89, 0x54, 0x24, 0x10,             // mov [rsp+0x10], edx
    0x89, 0x4C, 0x24, 0x08,             // mov [rsp+8], ecx
    0x53, 0x55, 0x56, 0x57, 0x41, 0x54, // push rbx/rbp/rsi/rdi/r12
};
utils::hook::detour swap_clients_hook;

void swap_clients_stub(const int a, const int b) {
  const auto ui_flags = [](const int lc) -> uint32_t {
    if (lc < 0 || lc > 3) // lc 3: the owned head of clientUIActives[3]
    {
      return 0xFFFFFFFF;
    }
    return *reinterpret_cast<const volatile uint32_t *>(
        base() + 0x05359BC0 + static_cast<size_t>(lc) * 0x1078);
  };
  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" SwapClients a=");
  l.dec(static_cast<uint64_t>(a));
  l.str(" b=");
  l.dec(static_cast<uint64_t>(b));
  l.str(" cl_max=");
  l.dec(*reinterpret_cast<const volatile uint32_t *>(base() +
                                                     cl_max_local_clients_rva));
  l.str(" conn=");
  l.dec(*reinterpret_cast<const volatile uint64_t *>(base() + 0x05359BB8) ? 1
                                                                          : 0);
  l.str(" flags ");
  l.hex(ui_flags(a));
  l.str("/");
  l.hex(ui_flags(b));

  // PLAYER 4 - clientUIActives[3] (0x053DBD28..0x053DCDA0) owns only its
  // first 0x3F0 bytes (voice_comm's vacated tail). From +0x3F0 on it lies
  // over live data: fourteen client globals (0x053DC118, eleven of them
  // dvar_t pointers), `clients` (0x053DC188) and the start of cls
  // (0x053DC190: hunkUsersStarted, servername, rendererStarted, realtime,
  // ..., localServers[]). Moving that data out is a CLOSED DEAD END
  // (2026-09-27, LOG): its writers include transient Arxan-decrypted code
  // no scan can see (dvar registration, cls.realtime every frame).
  // PS4 clientUIActive_t: flags, keyCatchers, connectionState,
  // nextScrollTime, migrationState all live below +0x18; +0x18..+0x1070
  // is migrationPers (online host migration only) and +0x1070/+0x1074
  // two voice counters. Of the whole-entry writers, SwapClients (this
  // memcpy) is the one offline play reaches - so for a swap involving
  // slot 3 the foreign tail [+0x3F0, +0x1078) is held in place on BOTH
  // sides. (CL_Frame resets lc 3's voice counters about once a second;
  // they land in cls.localServers[10], a LAN-browser entry offline play
  // never reads - documented, not hidden.)
  constexpr size_t swap_uia_base = 0x5359BC0, swap_uia_stride = 0x1078;
  constexpr size_t window_off = 0x3F0, window_len = 0x1078 - 0x3F0;
  const bool guard = (a == 3 || b == 3) && a >= 0 && b >= 0 && a <= 3 && b <= 3;
  static uint8_t keep_a[window_len], keep_b[window_len];
  auto *win_a = reinterpret_cast<uint8_t *>(base() + swap_uia_base +
                                            a * swap_uia_stride + window_off);
  auto *win_b = reinterpret_cast<uint8_t *>(base() + swap_uia_base +
                                            b * swap_uia_stride + window_off);
  if (guard) {
    std::memcpy(keep_a, win_a, window_len);
    std::memcpy(keep_b, win_b, window_len);
  }
  swap_clients_hook.invoke<void>(a, b);
  if (guard) {
    std::memcpy(win_a, keep_a, window_len);
    std::memcpy(win_b, keep_b, window_len);
    l.str(" window-held");
  }
  l.str(" -> ");
  l.hex(ui_flags(a));
  l.str("/");
  l.hex(ui_flags(b));
  trace_write(l);
}

// --- PLAYER 3'S OWN CONTROLLER: A joins, B / unplugging leaves ---------
//
// Console (stock Lua, identical in the PS4 and PC ship dumps): an unused
// controller pressing a button in the lobby raises "unused_gamepad_button"
// (LUIElement.AcceptGamePadButtonInput) -> LobbyAddLocalClient(menu, c)
// -> Engine.SigninLocalClient(c); B on a non-primary controller ->
// LobbyRemoveLocalClientFromLobby(menu, c) -> SetLocalClientToInactive(c).
// On PC the lobby menu only listens to controllers below
// GetMaxLocalControllers() (2, CoDMenu.lua ButtonBits subscriptions), so
// controller 2's presses never reach that Lua. The component therefore
// reads them itself and runs its proven join (try_join_guest2) and the
// engine leave - frontend only, never in a round.
//
// Button bits: gamepad record +0x08 (0x022F3135 moves it to +0x0C, then
// 0x022F314B ORs the mapped bits in); XInput A -> 0x10000100, B ->
// 0x10000200 (table 0x0305E1E0, 14 pairs {XInput mask, game bits}).
constexpr size_t gamepad_buttons = 0x8;
constexpr uint32_t game_button_a = 0x100;
constexpr uint32_t game_button_b = 0x200;
constexpr uint64_t guest2_join_request_ms = 3000;
constexpr uint32_t guest2_unplug_frames = 30;
uint32_t guest2_prev_buttons = 0;
bool guest2_join_requested = false;
uint64_t guest2_join_request_tick = 0;
uint32_t guest2_unplugged_frames = 0;
bool guest2_leave_in_progress = false;

uint32_t gamepad_buttons_of(const size_t slot) {
  return *reinterpret_cast<const volatile uint32_t *>(
      base() + gamepads_reserved_rva + slot * gamepad_stride + gamepad_buttons);
}

bool guest_listed_in_game_lobby(int controller);

bool guest2_listed_in_game_lobby() { return guest_listed_in_game_lobby(2); }

bool guest_listed_in_game_lobby(const int controller) {
  if (!lobby_enrollment_api_matches()) {
    return false;
  }
  const auto xuid = reinterpret_cast<uint64_t (*)(int)>(
      base() + live_user_get_xuid_rva)(controller);
  auto *session = reinterpret_cast<void *(*)(int)>(
      base() + lobby_get_session_rva)(game_lobby_type);
  return xuid && session &&
         reinterpret_cast<void *(*)(void *, uint64_t)>(
             base() + lobby_get_client_by_xuid_rva)(session, xuid);
}

// Out of every lobby we host, then signed out through the (hooked)
// sign-in function, exactly as the engine does for player 2.
void guest_leave(int controller, const char *why);

void guest2_leave(const char *why) { guest_leave(2, why); }

void guest_leave(const int controller, const char *why) {
  if (guest2_leave_in_progress) {
    return;
  }
  const in_progress_guard guard(guest2_leave_in_progress);
  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(controller == 2 ? " guest2_leave why=" : " guest3_leave why=");
  l.str(why);
  if (remove_guest_from_lobbies(controller, l)) {
    if (controller_seated(controller)) {
      reinterpret_cast<void (*)(int, bool, bool)>(base() + guest_signin_rva)(
          controller, false, false);
    }
    const bool out = !controller_seated(controller);
    if (out) {
      if (controller == 2) {
        reset_guest2_join();
      } else {
        guest3_lobby = {};
      }
    }
    l.str(out ? " signout=1" : " signout=seat-still-set");
  } else {
    l.str(" signout=0");
  }
  l.str(" seats=");
  l.hex(seat_bits());
  trace_write(l);
}

void guest2_input_frame() {
  if (!gamepads_activated) {
    return;
  }
  const bool connected = gamepad_connected(2);
  const uint32_t buttons = connected ? gamepad_buttons_of(2) : 0;
  const uint32_t pressed = buttons & ~guest2_prev_buttons;
  guest2_prev_buttons = buttons;

  if (!game::com::Com_IsRunningUILevel()) {
    guest2_join_requested = false;
    guest2_unplugged_frames = 0;
    return;
  }

  if (!controller_seated(2)) {
    guest2_unplugged_frames = 0;
    // Seat gone by some other path but still listed: finish the leave,
    // or the lobby keeps a member without a seat.
    if (guest2_lobby_enrolled && guest2_listed_in_game_lobby()) {
      guest2_leave("seat-lost");
      return;
    }
    // With controller 2's ButtonBits models created (see
    // widen_gamepad_button_models) the stock Lua joins him on A and
    // removes him on B, as on console - the component only keeps
    // the unplug / seat-lost cleanup below.
    if (gamepad_models_widened) {
      return;
    }
    if (pressed & game_button_a) {
      guest2_join_requested = true;
      guest2_join_request_tick = GetTickCount64();
      guest_join_attempts = 0;
      trace_line l;
      l.str("t=");
      l.dec(guest2_join_request_tick);
      l.str(" guest2_join_request A");
      trace_write(l);
    }
    if (guest2_join_requested) {
      if (guest_join_done || GetTickCount64() - guest2_join_request_tick >
                                 guest2_join_request_ms) {
        guest2_join_requested = false;
      } else {
        try_join_guest2();
      }
    }
    return;
  }

  guest2_join_requested = false;
  if ((pressed & game_button_b) && !gamepad_models_widened) {
    guest2_leave("B");
    return;
  }
  if (!connected) {
    if (++guest2_unplugged_frames >= guest2_unplug_frames) {
      guest2_unplugged_frames = 0;
      guest2_leave("unplugged");
    }
    return;
  }
  guest2_unplugged_frames = 0;
}

// PLAYER 4's controller (2026-09-27): A joins and B leaves through the
// stock Lua; what the engine does not do for a guest controller - leave
// when it is unplugged, or when the seat is gone but the game lobby
// still lists him - is done here, exactly as for player 3.
uint32_t guest3_unplugged_frames = 0;

void guest3_input_frame() {
  if (!gamepads_activated || !game::com::Com_IsRunningUILevel()) {
    guest3_unplugged_frames = 0;
    return;
  }
  if (!controller_seated(3)) {
    guest3_unplugged_frames = 0;
    if (guest3_lobby.enrolled && guest_listed_in_game_lobby(3)) {
      guest_leave(3, "seat-lost");
    }
    return;
  }
  if (!gamepad_connected(3)) {
    if (++guest3_unplugged_frames >= guest2_unplug_frames) {
      guest3_unplugged_frames = 0;
      guest_leave(3, "unplugged");
    }
    return;
  }
  guest3_unplugged_frames = 0;
}

void cl_init_watch() {
  patch_probe_once(); // diagnostic, BO3_PATCH_PROBE=1 only

  // NOT cl_init2_done alone: the widens are deferred until the
  // allocation is real, so this must keep re-entering until BOTH
  // halves are done.
  if (lc2_fully_done()) {
    return;
  }
  // Breadcrumbs, so a run that does nothing says WHY rather than
  // leaving the same silent zero for four different causes - which
  // is exactly what cost the previous cycle.
  if (!signin_relocated) {
    report87(10);
    return;
  }
  if (!raise_local_client_count) {
    report87(11);
    return;
  }

  uint32_t seats = 0;
  for (uint32_t lc = 0; lc < signin_new_slots; ++lc) {
    uint8_t flags = 0;
    std::memcpy(&flags,
                reinterpret_cast<const void *>(base() + signin_new_base +
                                               lc * signin_stride),
                sizeof(flags));
    if (flags & 1) {
      ++seats;
    }
  }

  if (seats >= 3) {
    run_cl_init_for_local_client2();
    return;
  }
  report87(20 + seats);
}

// THE TRIGGER, moved here after measuring that the count detour is too
// early: it is asked by CL_AllocatePerLocalClientMemory at 0x0135D665,
// BEFORE the inner allocator stores cl_maxLocalClients (0x0135D489), so
// our gate read 0 and declined - status 87 came back 30 (= 30 + 0),
// which is exactly the refusal code.
//
// CL_LocalClient_SetActive (0x0283AAB0) is the right seam. The connect
// loop calls it once per client at 0x01359D4C, in index order, so by
// the time it runs for client 0 or 1 the allocation is complete
// (cl_maxLocalClients measured as 3 at that point) and client 2's own
// iteration has not happened yet - which is precisely the window
// CL_Init(2), the frame pump and the netchan widen need.
//
// CL_Init does not call SetActive (PS4 0x414750 calls SetBeingUsed),
// and cl_init2_done is latched before any work, so this cannot recurse.
// MEASURED 2026-08-18 20:11, and it overturns the previous choice.
//
// Hooking CL_LocalClient_SetActive (0x0283AAB0) does NOT work, and the
// reason is now proven rather than guessed:
//   * the hook IS installed (prologue reads E9 ... at the menu AND at
//     the crash - so Arxan does not remove it, that theory is dead too)
//   * the connect loop really does call that address (pe_calls lists
//     0x01359D4C among its 13 callers)
//   * yet our call counter stayed at 5 across the whole launch
// => the connect loop's SetActive call never executes; the crash comes
//    first.
//
// But clientUIActives[2] IS 0x01 at the crash, so something set the
// active bit. That something is **CL_LocalClients_SetAllUsedActive
// (0x0283AB30)**, which writes the bit INLINE (0x0283AB7B) instead of
// calling SetActive - and this file already measured it running during
// the launch (the count cave inside it executed twice, last=3).
//
// So hook the function we have PROOF runs. It is also the right place
// semantically: PS4 CL_SetupClientsForIngame calls SetAllUsedActive
// immediately before CL_AllocatePerLocalClientMemory and the connect
// loop, which is exactly the window CL_Init(2) needs.
constexpr uint32_t set_active_rva = 0x27C19C0;
constexpr uint8_t set_active_prologue[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, // mov [rsp+8], rbx
    0x48, 0x89, 0x74, 0x24, 0x10, // mov [rsp+0x10], rsi
    0x57,                         // push rdi
    0x48, 0x83, 0xEC, 0x20,       // sub rsp, 0x20
};
utils::hook::detour set_active_hook;

void run_cl_init_for_local_client2();

// SetAllUsedActive takes no arguments (PS4 0x1517020: `for i in 0..3
// SetActive(i, IsBeingUsed(i))`), so the stub matches that signature.
// It runs the engine's own pass FIRST, then our step - by which point
// the used clients are active and the allocation has happened.
void set_active_stub() {
  set_active_hook.invoke<void>();

  // Counted BEFORE any gate, so the number means "our hook ran",
  // independent of what the gate then decides. Published even when
  // nothing else is, so a run that applies nothing still says why.
  ++set_active_calls;
  report87(0);

  // THE SEAT CHECK, and it must be here. Dropping the
  // cl_maxLocalClients gate (correct - see run_cl_init...) left this
  // path with NO precondition at all, so the very first
  // SetAllUsedActive at BOOT called CL_Init(2) for a client that does
  // not exist yet, and the client died during startup (measured:
  // [87] = 0x0100, i.e. calls = 1 code = 0, with cl_maxLocalClients 2
  // and clientUIActives[2] still zero).
  //
  // The right precondition is the SEAT, not the allocation: three
  // used seats mean local client 2 is real, which is exactly what
  // CL_Init needs and all it needs. Read straight from the relocated
  // seat table (bit 0 of each 0x24 record) - the same plain read the
  // count detour uses, never the engine predicates.
  if (lc2_fully_done() || !raise_local_client_count || !signin_relocated) {
    return;
  }

  uint32_t seats = 0;
  for (uint32_t lc = 0; lc < signin_new_slots; ++lc) {
    uint8_t flags = 0;
    std::memcpy(&flags,
                reinterpret_cast<const void *>(base() + signin_new_base +
                                               lc * signin_stride),
                sizeof(flags));
    if (flags & 1) {
      ++seats;
    }
  }

  if (seats >= 3) {
    run_cl_init_for_local_client2();
  }
}

// ---- PLAYER 4: the per-frame loops that need FOUR allocated clients ------
//
// (2026-09-27, static, untested.) The client-2 path widens these to 3 in
// run_cl_init_for_local_client2 once cl_maxLocalClients is 3; each one
// indexes per-client HEAP memory the allocator carves for cl_maxLocalClients
// clients, so each may only reach index 3 once the engine has sized for
// four - never in the frontend, never in a 1-3 player round:
//   frame pump 0x020F95DE, netchan poll 0x020F7BA4   clientConnections
//   cg_frame_imms (5, BO3_CG_FRAME)                  gated by the IsActive
//                                                    cave (clamps cl_max),
//                                                    viewport count reads +8
//   LUI render contexts 0x026FC3F5 (cmp r15d,3)      root 3 released by the
//                                                    count stub at 4 seats
// Each site must read the value the 3-player widen left (03); anything
// else is left alone and reported. One-way, like the 3-player widens.
bool round4_widened = false;

void widen_round_for_four() {
  if (round4_widened || !lc2_fully_done()) {
    return;
  }
  const auto b = base();
  const auto max_local = *reinterpret_cast<const volatile uint32_t *>(
      b + cl_max_local_clients_rva);
  if (max_local < 4) {
    return;
  }
  round4_widened = true;
  const uint8_t four = 0x04;
  uint32_t done = 0, skipped = 0;
  const auto bump = [&](const uint32_t rva) {
    auto *at = reinterpret_cast<uint8_t *>(b + rva);
    if (readable(at, 1) && *at == 0x3 && write_bytes(at, &four, 1)) {
      ++done;
    } else {
      ++skipped;
    }
  };
  bump(cl_frame_pump_imm_rva);
  bump(netchan_poll_imm_rva);
  for (const auto rva : cg_frame_imms) {
    bump(rva);
  }
  // LUI context bound: `41 83 FF 03 90 90 90` (hold_lui_context_count)
  bump(lui_ctx_bound_rva + 3);
  trace_line l;
  l.str("player 4 round: per-frame loops 3 -> 4, written ");
  l.dec(done);
  l.str(", not at 3 (left) ");
  l.dec(skipped);
  l.str(", cl_max ");
  l.dec(max_local);
  trace_write(l);
}

int splitscreen_player_count_stub() {
  if (signin_relocated) {
    // CONSOLE SEMANTICS. PS4 CL_SplitscreenPlayerCount is nothing
    // but a read of the splitscreen_playerCount dvar - the lobby-
    // committed count - and Com_InitClientGameStates runs ONCE at
    // boot (single caller: Com_Init_Try_Block_Function), so console
    // never sees seats flicker at allocation time. The PC re-seats
    // guests during map load, and the reallocation asks INSIDE that
    // window: two record-local bridges in a row lost to it, because
    // the re-seat's intermediate state (flags=0, localClientNum=i)
    // is byte-identical to a genuine sign-out.
    //
    // So do what the console does: treat the dvar as the committed
    // count. The stub is engine-called at exactly the allocation
    // moments, so it can also COMMIT seats upward into the dvar
    // here - the component's scheduler (where the old hold lived)
    // freezes after splitscreen sign-in ([83]=2/[84]=1/[34]=0) and
    // never saw the third seat.
    //
    // A guest leaving in the lobby lowers the seat count at once;
    // the dvar then over-allocates one client until the engine's own
    // updater lowers it. That is memory, not behaviour: panes are
    // gated by IsActive, which reads the live seats.
    const uint32_t seats = bridged_seat_count();
    uint32_t n = seats;

    // KEEP THE PER-CLIENT HUD GATE CLEAR FOR CLIENTS 2/3.
    //
    // The crash stack (0x0270D553, reproduced on three builds) runs
    // CG_SetView -> 0x010F6830 -> ScrPlace GetViewWritable
    // (0x013E55E9) -> 0x01F320DA. At 0x01F32095 that caller loads the
    // LUI roots and gates the per-client HUD draw on
    // `cmp byte [lc*0xB0 + roots + 0xAC], 0; je skip`. Loop #1, whose
    // bound relocate_lui_roots widens to 4, sets that flag for client 2
    // - so the HUD draw is ENABLED for a client whose LUI element tree
    // the PC never builds, and the element resolver falls through to the
    // NULL sentinel.
    //
    // Holding the flag at 0 is the engine's own way of saying "this
    // client has no root to draw" - the same value a client that never
    // constructed one would carry. The WORLD view is unaffected: it is
    // drawn by CG_SetView itself, not by this gate. So player 3 gets a
    // rendered viewport and no HUD, which is the honest state until the
    // third LUI context is genuinely built.
    //
    // This rides the count stub because the component's async scheduler
    // FREEZES after a splitscreen sign-in (measured: [25] async ticks
    // stuck at 0), so a scheduler::loop would never run in-round.
    // HOT PATH DISCIPLINE. This stub answers >1.2 MILLION queries per
    // session, so nothing here may call readable() (VirtualQuery) or
    // write_bytes() (two VirtualProtects) unconditionally. Doing so is
    // what collapsed the menu to single-digit FPS the moment a third
    // seat armed these branches - measured 2026-08-25, and the reason
    // the drop tracked SEATS rather than cl_maxLocalClients.
    // Validate once; afterwards read with a plain deref and write only
    // on an actual change. The roots block is component-owned RW
    // memory, so it needs no VirtualProtect at all.
    if (lui_roots_relocated && uiroot_new_base_rva) {
      static bool hud_flags_checked = false;
      static bool hud_flags_ok = false;
      if (!hud_flags_checked) {
        hud_flags_checked = true;
        hud_flags_ok =
            readable(reinterpret_cast<const uint8_t *>(
                         base() + uiroot_new_base_rva + 2 * 0xB0 + 0xAC),
                     0xB0 + 1);
      }
      if (hud_flags_ok) {
        // 2026-09-27: root 2 is RELEASED for pane 3's HUD - its
        // handle problem was the [2] uiElemHandles array and its
        // stale reader (fixed 2026-09-26), and root 2's in-use flag
        // is what makes UI_CoD_GetRootNameForController(2) answer
        // "UIRoot2" instead of UIRootFull when UI_CoD_Init adds
        // HUD(2). Holding it here (this stub runs inside UI_CoD_Init)
        // is exactly what sent HUD(2) to UIRootFull. Root 3 stays
        // held: there is no fourth local client.
        // PLAYER 4 (2026-09-27): root 3 is released too once four seats
        // are in use - otherwise UI_CoD_GetRootNameForController(3)
        // answers "UIRootFull" and HUD(3) lands in the primary root,
        // where it swallows every other root's first_snapshot (the
        // pane-1 loading-screen bug of 2026-09-26). The HUD loop only
        // adds a HUD for ACTIVE controllers.
        const uint32_t held_from = seat_count() >= 4 ? 4u : 3u;
        for (uint32_t lc = held_from; lc < 4; ++lc) {
          auto *flag = reinterpret_cast<uint8_t *>(
              base() + uiroot_new_base_rva + lc * 0xB0 + 0xAC);
          if (*flag != 0) {
            *flag = 0; // component-owned RW, no VirtualProtect
          }
        }
      }
    }

    // COMMIT THE ALLOCATION FLOOR, the engine's own mechanism.
    // max(count, FLOOR) at 0x0135D66C forwards on registers into
    // every per-client allocation and the cl_maxLocalClients store
    // (see local_client_count_patches). Once three seats have
    // GENUINELY seated (flags==1, not bridged - seat_count()), the
    // floor becomes 3 for the rest of the session, so the map-load
    // reallocation can no longer shrink the engine below the party
    // no matter what any count source reads inside the re-seat
    // window - the 17:33 and 17:43 rounds proved every source
    // (seat bits, records, the dvar) flickers there.
    //
    // Race-free by construction: our caller IS the allocator,
    // parked at 0x0135D665; the immediate is read right after we
    // return, on this same thread. Only 0x02 is ever overwritten,
    // so the BO3_SS_FLOOR=4 experiment byte is left alone.
    if (raise_local_client_count) {
      const uint32_t constituted = seat_count();
      if (constituted > committed_seats) {
        committed_seats = constituted;
      }
      if (committed_seats >= 3) {
        // plain read - the immediate lives in the mapped image, and
        // the VirtualProtect only happens on the single 02->03 edge
        auto *floor_imm = reinterpret_cast<uint8_t *>(base() + alloc_floor_rva);
        // PLAYER 4 (2026-09-27): four once four seats have seated -
        // the allocator then sizes every per-client heap block
        // (clients, clientConnections, cg, ...) for local client 3.
        // Like 3, one-way for the session and only upward.
        const uint8_t target = committed_seats >= 4 ? 0x04 : 0x03;
        if (*floor_imm == 0x02 || (*floor_imm == 0x03 && target == 0x04)) {
          write_bytes(floor_imm, &target, sizeof(target));
          note("[splitscreen] allocation floor committed to %u\n", target);
        }
        if (n < committed_seats) {
          // the same commitment, answered directly
          n = committed_seats;
        }
      }
    }
    uint64_t dvar = 0;
    std::memcpy(&dvar,
                reinterpret_cast<const void *>(
                    base() + splitscreen_player_count_dvar_rva),
                sizeof(dvar));
    if (dvar) {
      auto *current = reinterpret_cast<uint32_t *>(dvar + dvar_current_offset);
      static bool dvar_checked = false;
      static bool dvar_ok = false;
      static uint32_t dvar_pushes = 0;
      if (!dvar_checked) {
        dvar_checked = true;
        dvar_ok = readable(current, sizeof(uint32_t));
      }
      if (dvar_ok) {
        // BOUNDED. The engine's own updater recounts the seats during
        // the map-load re-seat window and pushes this back down, so an
        // unconditional push means a VirtualProtect pair on EVERY one
        // of the >1.2M queries - the measured cause of the splitscreen
        // frame-rate collapse. Losing this tug-of-war is harmless: the
        // allocation FLOOR above is what actually guarantees the
        // engine sizes for three, and it is a code immediate the
        // engine never rewrites. It is a plain dvar field, so the
        // write needs no VirtualProtect either.
        if (seats >= 2 && seats > *current && dvar_pushes < 64) {
          *current = seats;
          ++dvar_pushes;
        }
        if (*current > n) {
          n = *current;
        }
      }
    }
    if (n > 0) {
      // No set_status here: this is a hot query and set_status
      // calls VirtualProtect. The async publisher reports it.
      ++player_count_queries;
      player_count_last = n;

      // THE TRIGGER THAT ACTUALLY FIRES. Measured 2026-08-18
      // evening, in a HEALTHY three-player lobby (menu drawn,
      // 60 FPS, [3/1/3], seats 1/1/1 read directly): every one
      // of the component's scheduler counters was FROZEN across
      // repeated samples - [21] and [34] both stuck at 33851,
      // [25] async at 47, and slot 87 stuck at "two seats".
      // So the scheduler loops stop after a splitscreen sign-in
      // - not just per_controller_update_stub (which STATE.md
      // already recorded) but async AND renderer together. Any
      // one-shot hung on a scheduler loop is therefore dead on
      // arrival in exactly the state we need it.
      //
      // This detour is engine-called and, crucially, the
      // ALLOCATOR calls it at map load (0x0135D665, inside
      // CL_AllocatePerLocalClientMemory) - i.e. when START GAME
      // is pressed, on the game thread, BEFORE the connect loop
      // runs. That is the right moment anyway: CL_Init only has
      // to precede the connect loop, and here it does.
      //
      // cl_init2_done is set first inside the callee, so this
      // is recursion-proof if CL_Init queries the count again.
      if (n >= 3 && !lc2_fully_done() && raise_local_client_count) {
        run_cl_init_for_local_client2();
      }
      if (n >= 4 && raise_local_client_count) {
        widen_round_for_four();
      }
      return static_cast<int>(n);
    }
  }
  return splitscreen_player_count_hook.invoke<int>();
}

utils::hook::detour start_op_hook;
uint32_t start_op_counts[4] = {};

void start_op_stub(const int controller, const int operation, void *files) {
  // clientGameStates is NOT relocated. BOTH timings fail:
  //   post_unpack     76/76 refs rewritten, destination verified empty,
  //                   and then NOTHING signs in - StartOp 0 calls for
  //                   every controller, all four storage slots empty
  //   first StartOp   crashes at startup
  // chain4 does this move at MENU time and it works there. So the move
  // is sound and the moment is not, exactly like the s_rootData dead
  // end - but menu time is far too late for the boot storage read,
  // which is the whole reason it was wanted. See LOG.md.

  if (controller >= 0 && controller < 4) {
    set_status(37 + controller, ++start_op_counts[controller]);
  } else {
    set_status(41, static_cast<uint32_t>(controller));
  }
  start_op_hook.invoke<void>(controller, operation, files);
}

constexpr uint32_t clear_storage_rva = 0x2218F80;
constexpr uint8_t clear_storage_prologue[] = {0x48, 0x89, 0x6C,
                                              0x24, 0x20, 0x56};
bool clear_storage_ok = false;
bool guests_cleared = false;

void clear_guest_storage() {
  if (guests_cleared || !clear_storage_ok || !guests_filled) {
    return;
  }
  guests_cleared = true;
  const auto fn = reinterpret_cast<void (*)(int)>(base() + clear_storage_rva);
  fn(2);
  fn(3);
  set_status(35, 1);
}

constexpr uint32_t per_controller_update_rva = 0x1E19AE0;
constexpr uint8_t per_controller_update_prologue[] = {0x48, 0x8B, 0xC4,
                                                      0x55, 0x41, 0x54};
utils::hook::detour per_controller_update_hook;
bool per_controller_update_hooked = false;

// The task list, read out of TaskIsInProgress (PC 0x022B0D20) and the two
// gamer-profile handlers (0x02274A20 / 0x022749F0):
//
//   head at RVA 0x17A91930
//   +0x00 next   +0x08 definition   +0x10 state   +0x48 opData   +0x51 flag
//   "in progress" = state in {2,4,5} && flag == 0
//
// and opData points into s_localFileOpData, so (opData - base) / 0x1820
// IS the controller the task belongs to.
constexpr uint32_t task_head_rva = 0x17A12A30;
constexpr uint32_t gamerprofile_def_rva = 0x2FD3838;

// Which guest controller has a wedged gamer-profile task, or -1.
//
// Calling ProcessTasks blindly for controllers 2 and 3 is what the first
// version did, and it killed startup intermittently (RVA 0x020ECCB0
// dereferencing -1, rsi inside the task-node region). The game never
// calls ProcessTasks for a controller with nothing to process, so
// neither should we - look first, and only then act, on the one
// controller that actually has a stuck task.
int wedged_guest_controller() {
  if (!storage_base_rva) {
    return -1;
  }
  const auto module_base = base();
  const auto def = module_base + gamerprofile_def_rva;
  if (!localfileop_new_rva) {
    return -1;
  }
  const auto lfo = module_base + localfileop_new_rva;

  size_t node = 0;
  std::memcpy(&node,
              reinterpret_cast<const void *>(module_base + task_head_rva),
              sizeof(node));

  for (int guard = 0; node && guard < 64; ++guard) {
    size_t next = 0;
    size_t definition = 0;
    int32_t state = 0;
    size_t opdata = 0;
    uint8_t flag = 0;
    std::memcpy(&next, reinterpret_cast<const void *>(node), sizeof(next));
    std::memcpy(&definition, reinterpret_cast<const void *>(node + 0x08),
                sizeof(definition));
    std::memcpy(&state, reinterpret_cast<const void *>(node + 0x10),
                sizeof(state));
    std::memcpy(&opdata, reinterpret_cast<const void *>(node + 0x48),
                sizeof(opdata));
    std::memcpy(&flag, reinterpret_cast<const void *>(node + 0x51),
                sizeof(flag));

    const bool in_progress =
        (state >= 2 && state <= 5 && state != 3 && flag == 0);
    if (definition == def && in_progress && opdata >= lfo) {
      const auto index = (opdata - lfo) / localfileop_elem;
      if (index == 2 || index == 3) {
        return static_cast<int>(index);
      }
    }
    node = next;
  }
  return -1;
}

void reap_guest_tasks() {
  if (!process_tasks_ok) {
    return;
  }
  const auto who = wedged_guest_controller();
  if (who < 0) {
    return;
  }
  set_status(32, static_cast<uint32_t>(who));
  reinterpret_cast<void (*)(int)>(base() + process_tasks_rva)(who);
}

void storage_pump_stub(const int controller) {
  storage_pump_hook.invoke<void>(controller);

  // Once storage has produced a real CAC root, retry only the official
  // publisher/update half. This does not pump another controller or run a
  // completion handler, so it cannot invalidate the outer storage walk.
  if (guest2_lobby_enrolled && !guest2_profile_published) {
    refresh_guest2_lobby_profile();
  }

  // Re-entry guard: pump_guest_storage calls back through here.
  if (inside_guest_pump || controller != 1) {
    return;
  }
  set_status(21, ++ticks_main); // times the game pumped controller 1

  // REAPER OFF while bisecting, 2026-08-12.
  //
  // The all-four-slots result arrived in the same build as the
  // allocation padding, and padding alone is the better candidate for
  // having fixed it: the -1 read that padding cures is a plausible
  // reason the 'hdd' task got wedged in the first place. Meanwhile the
  // reaper is the strongest suspect for a new intermittent startup
  // death - RVA 0x020ECCB0 dereferencing -1 with rsi = 0x17A8C238,
  // which is inside the task-node region it walks.
  //
  // Calling ProcessTasks for a controller with no valid task records
  // is exactly the kind of thing the game never does itself.
  //
  // NARROWING IT DID NOT HELP. Measured 2026-08-12, four launches each:
  //
  //   padding only        4/4 alive, controllers 0/1/2 have storage
  //   padding + reaper    2/4 alive; when alive, ALL FOUR have storage
  //
  // and that second row is the TARGETED reaper, which only ever calls
  // ProcessTasks for the single controller that genuinely owns a
  // wedged gamer-profile task. So the problem is not which controller
  // - it is calling ProcessTasks from inside the Storage_Pump detour
  // at all. It runs completion handlers, and those re-enter storage
  // while the outer pump is still on the stack.
  //
  // A coin-flip startup is worse than a missing fourth controller.
  // This stays OFF here. The reap now happens one level up, in
  // per_controller_update_stub, where the storage call has fully
  // returned - see there for why that matters and why it is needed.

  // DELIBERATELY NOT PUMPING THE GUESTS HERE ANY MORE.
  //
  // Doing it from this point crashed at RVA 0x02275034 with rbp = 3 and
  // a wild pointer read. The reason is re-entrancy: Storage_Pump(1) has
  // returned by now, but its CALLER (0x01E26570) is still on the stack
  // and still holds pointers into the storage it was working on.
  // Pumping controllers 2 and 3 underneath it mutates that state and
  // the caller reads through stale pointers afterwards.
  //
  // And it should not be needed. Forcing extra pumps was a workaround
  // for the game's own loop running 3 times and stopping - which it did
  // because storage never made progress while s_targets still had
  // two-controller rows. With that fixed the loop runs continuously
  // (112 calls a minute, measured), so the game pumps every controller
  // itself. Let it.
}

// WHY THE REAP IS THE LOADOUT FIX, not just a tidy-up.
//
// Measured at boot with tools/call_counter.py:
//
//   Storage_Read      0x022775A0   53 / 19 / 19 /  0
//   read completion   0x01EA9EC0   10 /  5 /  0 /  0
//
// Controller 2 REQUESTS exactly as many reads as controller 1 and
// completes none of them. The completion path (0x0227712F) is what
// invokes FileProperties.readCallback, and that callback is what sets
// ready[ci] on the per-controller stats records - the flag the lobby
// checks before it will draw a loadout.
//
// Nothing completes because controller 2's gamer-profile task is never
// reaped: s_localFileOpData[2].opStatus sits at DONE instead of
// returning to IDLE, and the 'hdd' busy query (0x02274D30) is
// TaskIsInProgress on ONE GLOBAL task, so that single stuck task also
// blocks every other controller's storage assignment.
//
// So the missing gobblegums and the wedged task are the same bug.
void per_controller_update_stub(const int controller) {
  per_controller_update_hook.invoke<void>(controller);

  // PLAYER 3'S SIGN-IN GOES HERE, and the seam matters.
  //
  // It must run on the GAME's own thread (the renderer pipeline answers
  // engine predicates wrongly - see cl_init_watch's comment) and it must
  // NOT run inside the Storage_Pump detour: doing storage work from there
  // crashed at 0x02275034 because 0x01E26570 is still on the stack holding
  // pointers into the storage it was working on. Here that call has fully
  // returned, which is the same reasoning the reap below is built on.
  //
  // Controller 2, not 3: the driving loop at 0x0135C8AD is patched to
  // `cmp ebx, 3`, so the sweep is 0..2 and controller 3 NEVER arrives -
  // which is why status 34 has always read 0 and everything below this
  // point is currently dead code.
  if (controller == 2) {
    advance_guest2_join();
  } else if (controller == 3) {
    advance_guest3_join();
  }

  // Reap on the LAST controller of the pass, so the whole 0..3 sweep
  // has finished its storage work before any completion handler runs.
  if (controller != 3) {
    return;
  }
  // Live since the loop reaches controller 3 (player 4, 2026-09-27) -
  // once per frame. set_status is a VirtualProtect pair, so publish
  // only every 256th pass.
  if ((++update_calls & 0xFF) == 1) {
    set_status(34, update_calls);
  }

  // Refresh the engine's own active count whenever the number of local
  // clients CHANGES, on the game thread, in the lobby - so
  // splitscreen_playerCount is already true before a map load reads it.
  // Only on change, so this is a handful of calls per session, and only
  // once the active-count fix is actually installed (without it the
  // call can only ever produce 2 and there is no point).
  // Mark the guests' storage dirty ONCE, past boot, and let the game
  // re-read it in its own frames. Deliberately after the same
  // threshold: at boot the login read is already in flight and there is
  // nothing to redo.
  // clear_guest_storage() is NOT called - see its comment. It zeroes the
  // xuid and the re-assign never happens, because the same global 'hdd'
  // task is in progress and StorageTarget_IsBusy is not per controller.
}

uint32_t link_client_objects(const size_t table_slot, const size_t array_slot) {
  if (!new_base_rva[table_slot] || !new_base_rva[array_slot]) {
    return 0;
  }

  const auto table = base() + new_base_rva[table_slot];
  const auto array = base() + new_base_rva[array_slot];
  guest_array_rva = new_base_rva[array_slot];

  uint32_t written = 0;
  for (size_t i = 0; i < 4; ++i) {
    const auto object = array + i * client_ui_stride;
    if (write_bytes(reinterpret_cast<void *>(table + i * sizeof(size_t)),
                    &object, sizeof(object))) {
      ++written;
    }
  }
  return written;
}

// Verify the expected original byte before writing, and skip with a log
// line on mismatch - fail safe, never fail dirty.
template <size_t N>
uint32_t apply_byte_patches(const byte_patch (&patches)[N]) {
  uint32_t done = 0;
  for (const auto &p : patches) {
    auto *site = reinterpret_cast<uint8_t *>(base() + p.rva);
    if (*site != p.expect) {
      note("[splitscreen] %s: 0x%zX holds 0x%02X, expected 0x%02X - skipped\n",
           p.what, p.rva, *site, p.expect);
      continue;
    }
    if (write_bytes(site, &p.value, 1)) {
      ++done;
      note("[splitscreen] %s\n", p.what);
    }
  }
  return done;
}

uint32_t apply_storage_patches() { return apply_byte_patches(storage_patches); }

// Set when BO3_SS_SKIP=floor, to leave the ALLOCATION FLOOR alone while
// everything else in the count group stays on. It is the one patch that
// changes how much memory the game reserves - PC 0x0135D66C is the `mov
// r14d, 2` of `max(CL_SplitscreenPlayerCount(), 2)` inside
// CL_AllocatePerLocalClientMemory, which matches PS4 0x416A32 exactly - so
// with it applied a SOLO round allocates the whole per-local-client family
// (clients, clientConnections, snapshots, parseEntities, the 0x1E940 block)
// for four clients when one exists. Removing the WHOLE count group makes
// the solo round play; this narrows it to one byte or exonerates it.
bool skip_alloc_floor = false;

uint32_t apply_local_client_count_patches() {
  if (!raise_local_client_count) {
    return 0;
  }
  uint32_t done = 0;
  for (const auto &p : local_client_count_patches) {
    if (skip_alloc_floor && p.rva == alloc_floor_rva) {
      note("[splitscreen] allocation floor left at 2 (BO3_SS_SKIP=floor)\n");
      continue;
    }
    const byte_patch one[1] = {p};
    done += apply_byte_patches(one);
  }
  return done;
}

// The clientUIActives WALKER END-BOUNDS. Twelve loops walk the array
// with `lea end, [0x053DACB0]` (four of them at field offset +8)
// instead of an index bound - the inlined CL_AnyLocalClientsRunning
// sentinel among them (0x01359B72, LOG.md). With the activation loop
// widened to three they must walk three elements too, or client 2
// stays invisible to the per-frame upkeep and AnyLocalClientsRunning
// reports "nobody" when only client 2 is in a round.
//
// The new end is old + 0x1078 (two elements -> three); the +8 field
// walkers keep their +8 by the same addition. Every site is verified
// byte-for-byte first, so a misclassified site is skipped and
// reported, never corrupted. The thirteenth reference to that address
// (0x1C056059) is an Arxan code copy and is deliberately left alone -
// July's eviction left it too and was stable.
struct end_bound_fix {
  uint32_t insn_rva;
  uint8_t expect[7];
};

constexpr end_bound_fix client_ui_end_bounds[] = {
    {0x0134B907, {0x48, 0x8D, 0x15, 0xA2, 0x03, 0x01, 0x04}},
    {0x0135950F, {0x48, 0x8D, 0x0D, 0x9A, 0x27, 0x00, 0x04}},
    {0x01359B92, {0x48, 0x8D, 0x0D, 0x17, 0x21, 0x00, 0x04}},
    {0x0135A27B, {0x48, 0x8D, 0x15, 0x2E, 0x1A, 0x00, 0x04}},
    {0x0135D1BB, {0x48, 0x8D, 0x15, 0xEE, 0xEA, 0xFF, 0x03}},
    {0x027C1DDE, {0x4C, 0x8D, 0x0D, 0xCB, 0x9E, 0xB9, 0x02}},
    {0x027C1E49, {0x48, 0x8D, 0x15, 0x60, 0x9E, 0xB9, 0x02}},
    {0x027C1F50, {0x48, 0x8D, 0x0D, 0x59, 0x9D, 0xB9, 0x02}},
    {0x020ED0CF, {0x48, 0x8D, 0x15, 0xE2, 0xEB, 0x26, 0x03}},
    {0x027C1CD7, {0x48, 0x8D, 0x0D, 0xDA, 0x9F, 0xB9, 0x02}},
    {0x027C1D10, {0x48, 0x8D, 0x0D, 0xA1, 0x9F, 0xB9, 0x02}},
    {0x027C1D50, {0x48, 0x8D, 0x0D, 0x61, 0x9F, 0xB9, 0x02}},
};

// CONNECT-GATE TRACER - read-only, answers one question exactly.
//
// CL_ConnectFromLobby's connect loop (0x0134C6A5) is what turns an
// active local client into a CONNECTED one (it sets connectionState at
// 0x0134C6DD and flags bit 2 at 0x0134C6FA). Measured 2026-08-18 with
// activation_watch.py: local client 2 reaches flags 0x01 (ACTIVE, set
// by SetAllUsedActive) and then NEVER reaches 0x05 - while local client
// 1 goes 0x03 -> 0x07 0.4 s later. So the loop either does not iterate
// to index 2, or it sees IsActive(2) == false at that instant. Those
// two have completely different fixes and nothing readable statically
// can tell them apart - Arxan has flattened the surrounding control
// flow, and the async status publisher is dead during a launch.
//
// So: replace the loop's `call CL_LocalClient_IsActive` with a call to
// a cave that computes the SAME answer inline (movsxd/imul/and 1 - the
// whole of 0x0283AA50) and records, per index, that it was asked and
// what it answered. Same value returned, no behaviour change; this is
// the "recording is safe, writing is not" pattern this project already
// relies on at the stride sites.
//
// Read from outside via the call target itself (activation_watch.py
// resolves the cave through this very call site), so it needs no status
// slot and survives the dead async pipeline.
constexpr uint32_t is_active_rva = 0x27C18E0;
constexpr uint32_t client_ui_actives_rva = 0x05359BC0;
constexpr uint8_t is_active_bytes[] = {
    0x48, 0x63, 0xC1, // movsxd rax, ecx
    0x48, 0x8D, 0x0D, // lea rcx, [clientUIActives]
};

// Hooking ONE call site was the mistake that cost a run: the first
// version of this tracer sat on the `call IsActive` at 0x0134C6A7,
// recorded ZERO calls through an entire three-player launch, and the
// tempting conclusion was "that loop never runs". pe_calls.py then
// found 93 direct call sites for IsActive - the loop that really runs
// is simply a different one, and Arxan's flattening means the copy in
// the executed path need not be the one that reads nicely.
//
// So instrument the FUNCTION, not a call site: replicate its whole
// body (it is six instructions and pure) and record, per local client
// index, WHO asked (the return address), how often, and what it was
// told. The caller address then names the loop that matters, and the
// answer for index 2 says whether the connect loop was lied to or
// never got that far.
bool install_is_active_tracer() {
  auto *fn = reinterpret_cast<uint8_t *>(base() + is_active_rva);
  if (std::memcmp(fn, is_active_bytes, sizeof(is_active_bytes)) != 0) {
    note("[splitscreen] IsActive tracer: unexpected prologue\n");
    return false;
  }

  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x100));
  if (!cave) {
    return false;
  }
  // per index i: +0 last caller (qword), +8 calls, +12 last answer
  auto *slots = cave + 0x80;
  std::memset(slots, 0, 0x40);
  const auto cave_addr = reinterpret_cast<size_t>(cave);

  std::vector<uint8_t> c;
  const auto rip32 = [&](const size_t tgt) {
    const auto v = static_cast<int32_t>(tgt - (cave_addr + c.size() + 4));
    const auto *p = reinterpret_cast<const uint8_t *>(&v);
    c.insert(c.end(), p, p + 4);
  };

  c.insert(c.end(), {0x4C, 0x8B, 0x04, 0x24}); // mov r8, [rsp]
  c.insert(c.end(), {0x48, 0x63, 0xC1});       // movsxd rax, ecx
  c.insert(c.end(),
           {0x48, 0x69, 0xC0, 0x78, 0x10, 0x00, 0x00}); // imul rax,0x1078
  c.insert(c.end(), {0x48, 0x8D, 0x15});                // lea rdx, [uiactives]
  // After Phase 1 the array lives elsewhere; the tracer has to read
  // the SAME memory the engine now reads or it reports on a dead copy.
  rip32(base() + (client_ui_actives_relocated ? uia_new_base_rva
                                              : client_ui_actives_rva));
  c.insert(c.end(), {0x8B, 0x04, 0x10});       // mov eax, [rax+rdx]
  c.insert(c.end(), {0x83, 0xE0, 0x01});       // and eax, 1
  c.insert(c.end(), {0x83, 0xF9, 0x04});       // cmp ecx, 4
  c.insert(c.end(), {0x73, 0x1C});             // jae +28 -> ret
  c.insert(c.end(), {0x4C, 0x63, 0xC9});       // movsxd r9, ecx
  c.insert(c.end(), {0x49, 0xC1, 0xE1, 0x04}); // shl r9, 4
  c.insert(c.end(), {0x48, 0x8D, 0x15});       // lea rdx, [slots]
  rip32(reinterpret_cast<size_t>(slots));
  c.insert(c.end(), {0x4C, 0x03, 0xCA});       // add r9, rdx
  c.insert(c.end(), {0x4D, 0x89, 0x01});       // mov [r9], r8
  c.insert(c.end(), {0x41, 0xFF, 0x41, 0x08}); // inc dword [r9+8]
  c.insert(c.end(), {0x41, 0x89, 0x41, 0x0C}); // mov [r9+12], eax
  c.insert(c.end(), {0xC3});                   // ret

  if (!write_bytes(cave, c.data(), c.size())) {
    return false;
  }

  // Jump the function itself into the cave. The cave returns the
  // identical value, so every one of the 93 callers is unaffected.
  uint8_t patch[5] = {0xE9};
  const auto rel =
      static_cast<int32_t>(cave_addr - (base() + is_active_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));
  return write_bytes(fn, patch, sizeof(patch));
}

uint32_t widen_client_ui_walker_bounds() {
  uint32_t done = 0;
  for (const auto &f : client_ui_end_bounds) {
    auto *insn = reinterpret_cast<uint8_t *>(base() + f.insn_rva);
    if (std::memcmp(insn, f.expect, sizeof(f.expect)) != 0) {
      note("[splitscreen] walker end-bound at 0x%X: unexpected "
           "bytes - skipped\n",
           f.insn_rva);
      continue;
    }
    // These twelve leas are loop END bounds: they point AT
    // clientUIActives[2] = 0x053DACB0, which is also the base of the
    // next foreign per-client array. Once the array has been
    // RELOCATED they must address the new one-past-the-end, not the
    // old address shifted - shifting would still leave every loop
    // walking the foreign neighbour.
    int32_t new_disp = 0;
    if (client_ui_actives_relocated) {
      const size_t end = uia_new_base_rva + 4 * uia_stride;
      new_disp = static_cast<int32_t>(end - (f.insn_rva + sizeof(f.expect)));
    } else {
      // PLAYER 4 (2026-09-27): &[4] (was &[3]). Disassembled: every walker
      // reads +0 (flags) or +8 (connectionState) of each element - slot 3's
      // owned head - and compares the end address; it never dereferences it.
      int32_t disp = 0;
      std::memcpy(&disp, f.expect + 3, sizeof(disp));
      new_disp = disp + 2 * 0x1078;
    }
    if (write_bytes(insn + 3, &new_disp, sizeof(new_disp))) {
      ++done;
    }
  }
  return done;
}

// THE TWO END-OF-MATCH CLIENT LOOPS (2026-09-26 17:44).
//
// With the stats cache moved (batch 14) the round reached GAME OVER and
// crashed in the next frame: 0xC0000005 at 0x0104769A, a cg accessor
// (0x01045E30) dereferencing a NULL cg array for local client 2,
// called through SCR_UpdateFrame -> 0x02271410 -> 0x02270780 ->
// 0x02273800, which gates on clientUIActives[lc].flags bit 5 (cgame up).
// firstsnap_watch had already shown the shape at the 17:10 GAME OVER:
// lc 0 and 1 went to state 0 with flags 0x37 -> 0x07, lc 2 stayed at
// state 11 with flags 0x37. The console shuts down every local client:
//
//   PS4 Com_ShutdownInternal (0xE47020):
//       for (lc = 0; lc < 4; lc++) CL_Disconnect(lc, false)
//       (CORRECTED 2026-09-26 20:00: the argument is false, and the PC
//       function that matches it is 0x02149C60, widened further down.
//       The 0x02149110 loop below passes true - a different caller.)
//   PS4 CL_ShutdownAllClientsCGame (0x3EADA0):
//       for (lc = 4 - 1; lc >= first; lc--) CL_ShutdownCGame(lc)
//       -> CG_Shutdown(lc), clear CUI flags 0x10 and 0x20
//
// The PC twins stop at two:
//   0x02149242  CL_Disconnect(edi, true) [0x0135D860]; flags &= ~2;
//   0x02149257  cmp edi, 2                        -> 3
//   0x0132E2FA  mov edi, 1                        -> 2
//   0x0132E2FF  lea rbx, [clientUIActives + 1*0x1078] -> + 2*0x1078
//               (the loop tests flags & 0x10, calls CG_Shutdown
//               0x00926D90 and clears 0x30, walking rbx down)
// THREE, not four, like every other clientUIActives walker here:
// slot 2 is real memory once voice_comm has moved (counts_ok), slot 3
// is still foreign. Both loops only act on a client that is up - PC
// CL_Disconnect returns at once unless flags bit 1 is set, the cgame
// loop skips a client without bit 4 - so a two-player session is
// unaffected. The cgame loop's two instructions are written together
// or not at all: a start index without its pointer would walk the
// wrong slot.
uint32_t widen_client_shutdown_loops() {
  uint32_t done = 0;
  const auto b = base();

  auto *disconnect_bound = reinterpret_cast<uint8_t *>(b + 0x020F0E57);
  constexpr uint8_t disconnect_old[] = {0x83, 0xFF, 0x02};
  if (readable(disconnect_bound, sizeof(disconnect_old)) &&
      std::memcmp(disconnect_bound, disconnect_old, sizeof(disconnect_old)) ==
          0) {
    // PLAYER 4 (2026-09-27): four - CL_Disconnect(3) returns unless flags
    // bit 1 (CL_Init(3)) is set; flags &= ~2 is the owned head of slot 3.
    const uint8_t four = 0x04;
    if (write_bytes(disconnect_bound + 2, &four, 1)) {
      ++done;
    }
  } else {
    note("[splitscreen] shutdown disconnect loop: bytes differ - skipped\n");
  }

  // THE REAL Com_ShutdownInternal (2026-09-26 20:00). The loop above is
  // in 0x02149110 and passes `true`; PS4 Com_ShutdownInternal
  // (0xE47020) passes FALSE - `for (lc = 0; lc < 4; lc++)
  // CL_Disconnect(lc, false)` - and the PC function that matches it
  // call for call is 0x02149C60 (CL_Disconnect loop, SV_Shutdown,
  // Dvar_ResetDvars x2, Com_Restart, ..., CL_FreePerLocalClientMemory),
  // with an inlined copy in 0x0214A1E0 (PS4 calls it from
  // Com_ErrorCleanup and Com_ShutdownAndReinitialize). Both still
  // stopped at two. Measured with firstsnap_watch at the end of a
  // round: lc 0 and 1 went to state 0 before the per-client memory was
  // freed, lc 2 stayed CA_ACTIVE through the free and was only reset a
  // second later by the frontend setup - and the NEXT round's lc 2
  // hung at CA_CONNECTED, never receiving its gamestate. CL_Disconnect
  // (2) is already exercised (SetActive and the loop above call it),
  // it returns at once unless flags bit 1 is set, and clientUIActives
  // slot 2 is real - so the bound follows PS4 up to the clients that
  // exist.
  //
  // The per-client UI close loops in the same two functions
  // (0x02149CD9, 0x0214A39C, `0x0228B0C0(i)` = UI_SetActiveMenu(i, 0),
  // PS4 Com_UnloadFrontEnd UI_CloseAll i < 4) reach uiInfoArray via
  // UI_UIContext_GetInfo(uictx(i)). They follow ONLY when
  // relocate_batch17 moved that array - its old slot 2 is dvar
  // pointers and a static stringstream (CLAUDE.md rule 4).
  struct shutdown_site {
    uint32_t rva;
    uint8_t modrm;
    const char *what;
    bool needs_uiinfo;
  };
  constexpr shutdown_site com_shutdown_sites[] = {
      {0x020F188E, 0xFF, "Com_ShutdownInternal disconnect loop", false},
      {0x020F1F4B, 0xFB, "inlined Com_ShutdownInternal disconnect loop", false},
      {0x020F18D9, 0xFB, "Com_ShutdownInternal UI close loop", true},
      {0x020F1F9C, 0xFB, "inlined Com_ShutdownInternal UI close loop", true},
  };
  for (const auto &s : com_shutdown_sites) {
    if (s.needs_uiinfo && !batch17_new[0]) {
      note("[splitscreen] %s: uiInfoArray not moved - left at 2\n", s.what);
      continue;
    }
    auto *at = reinterpret_cast<uint8_t *>(b + s.rva);
    const uint8_t want[] = {0x83, s.modrm, 0x02};
    if (!readable(at, sizeof(want)) ||
        std::memcmp(at, want, sizeof(want)) != 0) {
      note("[splitscreen] %s: bytes differ - skipped\n", s.what);
      continue;
    }
    const uint8_t four = 0x04; // player 4: uiInfoArray [4], seat record 3
    if (write_bytes(at + 2, &four, 1)) {
      ++done;
    }
  }

  auto *start = reinterpret_cast<uint8_t *>(b + 0x0132E31A);
  constexpr uint8_t start_old[] = {0xBF, 0x01, 0x00, 0x00, 0x00};
  // PLAYER 4 (2026-09-27): start at client 3 - the walk tests flags & 0x10
  // (cgame up) per element, +0 of slot 3 is owned (and zero while unused).
  constexpr uint8_t start_new[] = {0xBF, 0x03, 0x00, 0x00, 0x00};
  auto *cursor = reinterpret_cast<uint8_t *>(b + 0x0132E31F);
  constexpr uint8_t cursor_old[] = {0x48, 0x8D, 0x1D, 0x12, 0xC9, 0x02, 0x04};
  if (!readable(start, sizeof(start_old)) ||
      std::memcmp(start, start_old, sizeof(start_old)) != 0 ||
      !readable(cursor, sizeof(cursor_old)) ||
      std::memcmp(cursor, cursor_old, sizeof(cursor_old)) != 0) {
    note("[splitscreen] cgame shutdown loop: bytes differ - skipped\n");
    return done;
  }
  int32_t disp = 0;
  std::memcpy(&disp, cursor_old + 3, sizeof(disp));
  disp += 2 * 0x1078; // clientUIActives[1] -> clientUIActives[3] (0x053DBD28)
  if (!write_bytes(cursor + 3, &disp, sizeof(disp))) {
    return done;
  }
  if (!write_bytes(start, start_new, sizeof(start_new))) {
    write_bytes(cursor, cursor_old, sizeof(cursor_old));
    return done;
  }
  return done + 1;
}

// SEED cl_maxLocalClients FOR THE FRONTEND.
//
// Measured 2026-08-16 with a counter on CL_AllocatePerLocalClientMemory
// (0x0135D650): it runs **zero times** from boot through the main menu and
// the offline Zombies lobby. It allocates session-sized per-client memory,
// so it runs at MAP LOAD. Until then `cl_maxLocalClients` is simply the
// image's STATIC initialiser, which is 2 (verified in the file at
// 0x053A2720) - not a value anything computed.
//
// That creates a chicken-and-egg: the frontend needs the count to be 4 to
// let a third controller in, but the count is not computed until a map is
// already loading, by which time the players had to be added.
//
// So seed it. This is NOT faking a state: with the floor patch above, the
// allocator WILL compute exactly 4 when it runs, so this declares the value
// the engine is going to reach anyway, early enough for the menus to use.
//
// WHY IT IS SAFE BEFORE THE MEMORY EXISTS. PS4 CG_GetLocalClientGlobals
// (0x1D76850) tests in this order:
//
//     if (cgArray == NULL)                      return NULL   <- FIRST
//     if (localClientNum >= cl_maxLocalClients) return NULL
//     return cgArray + localClientNum * stride
//
// Before map load cgArray is null, so every local client - including 0 and
// 1 - already gets NULL from the first test. Raising the count cannot make
// that worse and cannot produce an out-of-bounds index, because the
// pointer check precedes the index check. Once the map loads, the block is
// sized for 4 and index 2 is genuinely valid.
bool seed_cl_max_local_clients() {
  auto *v = reinterpret_cast<uint32_t *>(base() + cl_max_local_clients_rva);
  if (*v != 2) {
    note(
        "[splitscreen] cl_maxLocalClients holds %u, expected 2 - not seeding\n",
        *v);
    return false;
  }
  return write_bytes(v, &seed_max_local_clients,
                     sizeof(seed_max_local_clients));
}

// The cave's own counters, published so a run can say what the fix DID
// rather than only that it installed. Status 2 has always reported
// "installed" and nothing has ever reported "fired", which is the
// difference between a suspect and a culprit.
//
//   +0x00  substitutions   how often the delivered base was replaced
//   +0x04  executions      how often the site ran at all
//   +0x08  delivered       the last value that arrived in rax
//   +0x10  substituted     the last value written over it
uint8_t *stride_slots = nullptr;

// MAKE THE ACTIVE COUNT TRUE - the fix for "only two screens".
//
// PS4 CL_LocalClients_SetAllUsedActive (caller at 0x151705D) loops the
// local clients, sets each one's active flag to Com_LocalClient_IsBeingUsed,
// then does
//
//     count = CL_LocalClient_GetActiveCount();   // 0x1516A20, i < 4
//     Dvar_SetInt(splitscreen_playerCount, count > 0 ? count : 1);
//
// The PC has the same function at 0x0283AB30 with GetActiveCount INLINED
// AND UNROLLED TO TWO ELEMENTS:
//
//   0283AB7D  40 84 35 ..  test byte [0x053D8BC0], sil   clientUIActives[0]
//   0283AB84  B8 00000000  mov eax, 0
//   0283AB89  0F 45 C6     cmovne eax, esi
//   0283AB8C  40 84 35 ..  test byte [0x053D9C38], sil   clientUIActives[1]
//   0283AB93  74 02        je +2
//   0283AB95  FF C0        inc eax
//   0283AB97  mov rcx,[splitscreen_playerCount] ... call Dvar_SetInt
//
// so the count can never exceed 2, the dvar can never exceed 2, and
// max(CL_SplitscreenPlayerCount(), 2) can never allocate for more than two
// local clients. Measured 2026-08-18: with three players signed in the
// inner allocator still received localClientCount = 2.
//
// WHY NOT WIDEN THE ARRAY. clientUIActives is [2] - proven, not assumed:
// three loops use 0x053DACB0 = 0x053D8BC0 + 2*0x1078 as their END bound
// (0x0283A8A9, 0x0283AA1D, ...). Relocating it would mean rewriting the
// 128 address-takers, the 16 direct accesses AND the 8407 references
// inside the array, against 33/70/28/3 for the seven arrays already
// moved. Not worth it for a number we can compute correctly.
//
// WHAT THIS DOES INSTEAD. Replace those 26 bytes with a jump to a cave
// that counts Com_LocalClient_IsBeingUsed(lc) over lc = 0..3 and returns
// it in eax, then falls back into the engine's own
// `test eax,eax / cmovg edx,eax / Dvar_SetInt`. Nothing is faked: the
// engine's own rule in this very function is active = IsBeingUsed, so
// counting IsBeingUsed over four clients is exactly what its
// GetActiveCount would report if the array had four elements.
//
// It also needs no stack work - the enclosing function already reserves
// shadow space and calls this same function at 0x0283AB52.
//
// The loop bound at 0x0283ABB5 is deliberately LEFT AT 2: widening it
// would make the loop write `or dword [rbx], esi` into clientUIActives[2],
// which does not exist.
constexpr size_t active_count_rva = 0x27C1A0D;
constexpr uint8_t active_count_bytes[] = {
    0x40, 0x84, 0x35, 0xEC, 0x7A, 0xB9, 0x02, // test byte [rip+..], sil
    0xB8, 0x00, 0x00, 0x00, 0x00,             // mov eax, 0
    0x0F, 0x45, 0xC6,                         // cmovne eax, esi
    0x40, 0x84, 0x35, 0x55, 0x8B, 0xB9, 0x02, // test byte [rip+..], sil
    0x74, 0x02,                               // je +2
    0xFF, 0xC0,                               // inc eax
};

bool install_active_count_fix() {
  auto *site = reinterpret_cast<uint8_t *>(base() + active_count_rva);
  // The recorded 26 bytes contain TWO rip displacements. The second
  // (offset 18, `test byte [clientUIActives[1]], sil`) is one of the
  // 250 references Phase 1 rewrites, so after the relocation the
  // literal no longer matches. Rebuild the expectation from the live
  // target instead of refusing - refusing here would silently drop the
  // dvar/allocator count fix the moment the pane fix landed.
  uint8_t expect[sizeof(active_count_bytes)];
  std::memcpy(expect, active_count_bytes, sizeof(expect));
  if (client_ui_actives_relocated) {
    const size_t tgt = base() + uia_new_base_rva + uia_stride;
    const int32_t disp =
        static_cast<int32_t>(tgt - (base() + active_count_rva + 18 + 4 + 1));
    std::memcpy(expect + 18, &disp, sizeof(disp));
  }
  if (std::memcmp(site, expect, sizeof(expect)) != 0) {
    note("[splitscreen] active count: unexpected bytes at 0x%zX\n",
         active_count_rva);
    return false;
  }

  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x100));
  if (!cave) {
    return false;
  }

  auto *slots = cave + 0x80; // +0 index, +4 count, +8 executions, +12 last
  const auto cave_addr = reinterpret_cast<size_t>(cave);

  std::vector<uint8_t> c;
  // `extra` = bytes that follow the displacement inside the SAME
  // instruction (an immediate). Getting this wrong silently aims every
  // rip-relative access a few bytes off - the bug that made
  // call_counter.py read counter[i+1]*256 for a whole session.
  const auto rip32 = [&](const size_t target, const size_t extra = 0) {
    const auto value =
        static_cast<int32_t>(target - (cave_addr + c.size() + 4 + extra));
    const auto *p = reinterpret_cast<const uint8_t *>(&value);
    c.insert(c.end(), p, p + 4);
  };

  const auto slot = [&](const size_t i) {
    return reinterpret_cast<size_t>(slots + i);
  };

  // mov dword [executions], ... -> just increment it
  c.insert(c.end(), {0xFF, 0x05});
  rip32(slot(8));
  // mov dword [index], 0
  c.insert(c.end(), {0xC7, 0x05});
  rip32(slot(0), 4);
  c.insert(c.end(), {0x00, 0x00, 0x00, 0x00});
  // mov dword [count], 0
  c.insert(c.end(), {0xC7, 0x05});
  rip32(slot(4), 4);
  c.insert(c.end(), {0x00, 0x00, 0x00, 0x00});

  const auto loop_start = c.size();
  // mov ecx, dword [index]
  c.insert(c.end(), {0x8B, 0x0D});
  rip32(slot(0));
  // call Com_LocalClient_IsBeingUsed
  c.insert(c.end(), {0xE8});
  rip32(base() + is_being_used_rva);
  // test al, al ; je +6
  c.insert(c.end(), {0x84, 0xC0});
  c.insert(c.end(), {0x74, 0x06});
  // inc dword [count]            (6 bytes - the je above skips exactly this)
  c.insert(c.end(), {0xFF, 0x05});
  rip32(slot(4));
  // inc dword [index]
  c.insert(c.end(), {0xFF, 0x05});
  rip32(slot(0));
  // cmp dword [index], 4
  c.insert(c.end(), {0x83, 0x3D});
  rip32(slot(0), 1);
  c.insert(c.end(), {0x04});
  // jl loop
  c.insert(c.end(), {0x0F, 0x8C});
  {
    const auto target = cave_addr + loop_start;
    const auto value =
        static_cast<int32_t>(target - (cave_addr + c.size() + 4));
    const auto *p = reinterpret_cast<const uint8_t *>(&value);
    c.insert(c.end(), p, p + 4);
  }
  // mov eax, dword [count]   -> the value the engine then uses
  c.insert(c.end(), {0x8B, 0x05});
  rip32(slot(4));
  // mov dword [last], eax    (publishable)
  c.insert(c.end(), {0x89, 0x05});
  rip32(slot(12));
  // jmp back, past the 26 replaced bytes
  c.insert(c.end(), {0xE9});
  rip32(base() + active_count_rva + sizeof(active_count_bytes));

  if (c.size() > 0x80) {
    note("[splitscreen] active count: cave too small (%zu)\n", c.size());
    return false;
  }

  std::memcpy(cave, c.data(), c.size());
  std::memset(slots, 0, 0x30);

  uint8_t patch[sizeof(active_count_bytes)];
  std::memset(patch, 0x90, sizeof(patch)); // nop the remainder
  patch[0] = 0xE9;
  const auto rel =
      static_cast<int32_t>(cave_addr - (base() + active_count_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));

  if (!write_bytes(site, patch, sizeof(patch))) {
    return false;
  }

  active_count_slots = slots;
  note("[splitscreen] active count fix installed at 0x%zX\n", active_count_rva);
  return true;
}

void publish_active_count() {
  if (!active_count_slots) {
    return;
  }
  uint32_t execs = 0;
  uint32_t last = 0;
  std::memcpy(&execs, active_count_slots + 8, sizeof(execs));
  std::memcpy(&last, active_count_slots + 12, sizeof(last));
  set_status(86, execs);
  set_status(87, last);
  set_status(93, player_count_queries);
  set_status(94, player_count_last);
}

bool install_stride_fix() {
  auto *site = reinterpret_cast<uint8_t *>(base() + stride_site_rva);
  if (std::memcmp(site, stride_site_bytes, sizeof(stride_site_bytes)) != 0) {
    note("[splitscreen] stride fix: unexpected bytes at 0x%zX\n",
         stride_site_rva);
    return false;
  }

  auto *cave = static_cast<uint8_t *>(allocate_near_module(0x100));
  if (!cave) {
    return false;
  }

  auto *slots = cave + 0x80; // count, delivered, substituted
  const auto cave_addr = reinterpret_cast<size_t>(cave);

  std::vector<uint8_t> c;
  const auto rip32 = [&](const size_t target) {
    const auto value =
        static_cast<int32_t>(target - (cave_addr + c.size() + 4));
    const auto *p = reinterpret_cast<const uint8_t *>(&value);
    c.insert(c.end(), p, p + 4);
  };

  // inc dword [executions] - FIRST, so it counts every entry including
  // the ones that are left alone. A fix that fires zero times in a run
  // cannot be that run's cause, and until now there was no way to tell.
  // Flags: this clobbers them, but so do the test/inc already below and
  // the displaced imul at the end overwrites them again before the
  // original code sees anything.
  c.insert(c.end(), {0xFF, 0x05});
  rip32(reinterpret_cast<size_t>(slots + 4));
  // mov [delivered], rax
  c.insert(c.end(), {0x48, 0x89, 0x05});
  rip32(reinterpret_cast<size_t>(slots + 8));
  // Only repair a value that is clearly not a pointer: a real user-mode
  // x64 pointer has its top 16 bits clear. Substituting unconditionally
  // killed the process outright when these sites ran with the menu up.
  c.insert(c.end(), {0x50});                   // push rax
  c.insert(c.end(), {0x48, 0xC1, 0xE8, 0x30}); // shr rax, 48
  c.insert(c.end(), {0x85, 0xC0});             // test eax, eax
  c.insert(c.end(), {0x58});     // pop rax  (does not touch flags)
  c.insert(c.end(), {0x74, 20}); // jz -> skip the 20-byte repair
  // mov rax, [base_table]
  c.insert(c.end(), {0x48, 0x8B, 0x05});
  rip32(base() + base_table_rva);
  // inc dword [count]
  c.insert(c.end(), {0xFF, 0x05});
  rip32(reinterpret_cast<size_t>(slots));
  // mov [substituted], rax
  c.insert(c.end(), {0x48, 0x89, 0x05});
  rip32(reinterpret_cast<size_t>(slots + 16));
  // the displaced imul LAST, so the flags the following add/comiss/jbe
  // read are exactly the ones they would have seen
  c.insert(c.end(), std::begin(stride_site_bytes), std::end(stride_site_bytes));
  // jmp back
  c.insert(c.end(), {0xE9});
  rip32(base() + stride_site_rva + sizeof(stride_site_bytes));

  std::memcpy(cave, c.data(), c.size());
  std::memset(slots, 0, 0x30);

  uint8_t patch[7] = {0xE9, 0, 0, 0, 0, 0x90, 0x90};
  const auto rel =
      static_cast<int32_t>(cave_addr - (base() + stride_site_rva + 5));
  std::memcpy(patch + 1, &rel, sizeof(rel));

  if (!write_bytes(site, patch, sizeof(patch))) {
    return false;
  }

  stride_slots = slots;
  note("[splitscreen] stride fix installed at 0x%zX\n", stride_site_rva);
  return true;
}

// Read-only, four dwords and two qwords - it cannot disturb what it
// measures, which this project has broken four times by being clever.
void publish_stride_counters() {
  if (!stride_slots) {
    return;
  }

  uint32_t substitutions = 0;
  uint32_t executions = 0;
  uint64_t delivered = 0;
  uint64_t substituted = 0;
  std::memcpy(&substitutions, stride_slots, sizeof(substitutions));
  std::memcpy(&executions, stride_slots + 4, sizeof(executions));
  std::memcpy(&delivered, stride_slots + 8, sizeof(delivered));
  std::memcpy(&substituted, stride_slots + 16, sizeof(substituted));

  set_status(76, executions);
  set_status(77, substitutions);
  set_status(78, static_cast<uint32_t>(delivered));
  set_status(79, static_cast<uint32_t>(delivered >> 32));
  set_status(80, static_cast<uint32_t>(substituted));
  set_status(81, static_cast<uint32_t>(substituted >> 32));
}

// The injected local client has no message channel, so it can never answer
// the lobby state message and the launch stalls on it
// ("lobby State message failed or timed out.", string at RVA 0x0301F4D8).
//
// Cloning a SNAPSHOT of a finished handshake does not survive: the launch
// sequence advances afterwards and the slot refreezes. Holding it equal to
// a real local guest for the whole launch does - measured, the reservation
// then runs with three clients on its own (cl_maxLocalClients 2 -> 3,
// flags 4 -> 0) with no manual step at any point.
//
// This copies a TRUE state from slot 1, a genuine splitscreen guest that
// does complete. It is not faking a count.
void hold_injected_clients() {
  const auto session =
      base() + lobby_pool_rva + game_lobby_index * lobby_pool_stride;
  const auto sequence =
      *reinterpret_cast<uint32_t *>(base() + launch_sequence_rva);

  uint32_t target_ack = sequence + 1;
  for (size_t i = 0; i < 2; ++i) {
    const auto xuid = *reinterpret_cast<uint64_t *>(
        session + session_clients_offset + i * session_client_stride);
    if (!xuid) {
      continue;
    }
    const auto ack =
        *reinterpret_cast<uint32_t *>(session + acks_offset + i * 4);
    target_ack = std::max(target_ack, ack);
  }

  for (const auto slot : injected_slots) {
    const auto xuid = *reinterpret_cast<uint64_t *>(
        session + session_clients_offset + slot * session_client_stride);
    if (!xuid) {
      continue; // not signed in, nothing to hold
    }

    for (const auto gate : gate_arrays) {
      const auto ref = session + gate + reference_slot * session_client_stride;
      const auto dst = session + gate + slot * session_client_stride;
      for (const auto field : copy_fields) {
        auto *d = reinterpret_cast<uint32_t *>(dst + field);
        const auto v = *reinterpret_cast<uint32_t *>(ref + field);
        if (*d != v) {
          *d = v;
        }
      }
    }

    auto *ack = reinterpret_cast<uint32_t *>(session + acks_offset + slot * 4);
    if (*ack < target_ack) {
      *ack = target_ack;
    }
  }
}

// There is no console in a normal launch, so printf tells us nothing. This
// writes a status block into the free .data window (see the reserved map in
// CLAUDE.md - 0x1A8A7D00 sits between the publish/clear tracer caves and
// trace_null_caller) so an external reader can confirm what actually ran.
// Guessing whether a patch applied has cost this project entire sessions.
constexpr size_t status_rva = 0x1A828D00;
constexpr uint32_t status_magic = 0xB03C0FFE;

void set_status(const size_t index, const uint32_t value) {
  // 0x180, not 0x100: slots 64..95 are in use now (the settings-callback
  // group). They still land inside the reserved window and short of
  // 0x1A8A7E80, which the occupancy map assigns to trace_null_caller.
  auto *s = reinterpret_cast<uint32_t *>(base() + status_rva);
  DWORD old{};
  if (VirtualProtect(s, 0x180, PAGE_READWRITE, &old)) {
    s[index] = value;
    DWORD tmp{};
    VirtualProtect(s, 0x180, old, &tmp);
  }
}

// Does every reference still look exactly as the table says?
//
// The retry exists because the first build patched nothing and it was not
// obvious why. It turned out NOT to be a timing problem - the status block
// showed `attempts 1`, i.e. the image matches these tables immediately at
// post_unpack - the allocator was simply failing. The retry is kept anyway:
// it costs one comparison pass and removes a whole class of assumption.
bool table_matches(const reloc_table &t) {
  for (size_t i = 0; i < t.count; ++i) {
    const auto &r = t.refs[i];
    const auto *field =
        reinterpret_cast<const int32_t *>(base() + r.insn_rva + r.disp_offset);
    const auto expected =
        r.rip_relative
            ? static_cast<int32_t>(r.target_rva - (r.insn_rva + r.length))
            : static_cast<int32_t>(r.target_rva);
    if (!readable(field, sizeof(int32_t)) || *field != expected) {
      return false;
    }
  }
  return true;
}

// Only the stride site is checked here, NOT every table.
//
// This used to require every table to match, and that is an all-or-
// nothing gate that got worse as tables were added: with the storage
// table present, one unmatched storage reference blocked the bit array
// relocation too, so a component that used to apply 1 of 1 applied 0 of
// 2. Measured - status showed attempts climbing with every field zero.
//
// Each table is verified in full inside relocate() anyway: it dry-runs
// every reference and writes nothing unless all of them match. So per
// table is both safer and less coupled than one shared gate.
bool ready() {
  return std::memcmp(reinterpret_cast<void *>(base() + stride_site_rva),
                     stride_site_bytes, sizeof(stride_site_bytes)) == 0;
}

uint32_t attempts = 0;

// KILL SWITCH: set BO3_SPLITSCREEN=off and this component does NOTHING.
//
// Added 2026-08-17 while bisecting the solo round-start crash. Two things
// forced it:
//
// 1. The bisect kept changing two variables at once. Disabling the
//    client_objs/client_ui relocations to test one theory moved the crash
//    to RVA 0x01EC6FCE (the LiveUser accessors) instead of removing it,
//    because the guest fill depends on those tables existing. A single
//    all-or-nothing switch gives a clean control: same binary, component
//    inert, so anything that still happens is not ours.
// 2. The build currently BREAKS ROUND START (see LOG.md). Until that is
//    fixed, the person whose machine this is needs a way to play with the
//    same executable rather than swapping boiii.exe back and forth.
//
// Status slot 3 reports it: 0 = active, 0xFF = disabled by the variable.
//
// It is also a BISECT DIAL, not just an on/off. The round-start crash is
// somewhere in this component and removing one relocation at a time
// changed two things at once (dropping client_objs/client_ui MOVED the
// crash to 0x01EC6FCE instead of clearing it, because the guest fill
// depends on those tables). Levels let each run change exactly one group:
//
//   off      nothing at all                      (verified: solo round plays)
//   reloc    the container relocations only
//   storage  + the storage byte patches and the stride fix
//   signin   + clientGameStates, the seat, the guest fill
//   full     + every detour and the count patches   (default)
//
// Status slot 74 reports the level actually in force, so a run can never
// be attributed to the wrong configuration afterwards.
enum apply_level {
  level_off = 0,
  level_reloc = 1,
  level_storage = 2,
  level_signin = 3,
  level_full = 4,
};

apply_level current_level() {
  char buf[16]{};
  const auto n = GetEnvironmentVariableA("BO3_SPLITSCREEN", buf, sizeof(buf));
  if (n == 0 || n >= sizeof(buf)) {
    return level_full;
  }
  if (_stricmp(buf, "off") == 0 || _stricmp(buf, "0") == 0) {
    return level_off;
  }
  if (_stricmp(buf, "reloc") == 0) {
    return level_reloc;
  }
  if (_stricmp(buf, "storage") == 0) {
    return level_storage;
  }
  if (_stricmp(buf, "signin") == 0) {
    return level_signin;
  }
  return level_full;
}

apply_level level = level_full;

bool at_least(const apply_level want) { return level >= want; }

// LEAVE-ONE-OUT, which is what this actually needs.
//
// The cumulative levels above cannot isolate these groups, and that is
// measured rather than suspected: at level `storage` the client
// BLACK-SCREENS AT BOOT, because the storage byte patches widen the
// controller bounds to four and the guest reads then complete into the
// settings path. The thing that prevents that is the guest read FILTER -
// a detour, so only present at `full`. The later groups are what make the
// earlier ones safe, so removing groups from the bottom up just breaks
// the build in a new way each time.
//
//   BO3_SS_SKIP=counts     everything except the local-client count patches
//   BO3_SS_SKIP=settings   ...except the two settings-callback detours
//   BO3_SS_SKIP=signin     ...except clientGameStates, the seat, guest fill
//   BO3_SS_SKIP=storage    ...except the storage byte patches + stride fix
//   BO3_SS_SKIP=stride     ...except the stride fix ALONE
//   BO3_SS_SKIP=readfilter ...except the guest storage read filter
//
// `stride` was split out of `storage` on 2026-08-17, because the bisect
// that finished that day could not tell the two apart and the conclusion
// hangs on which one it was. Two measurements point straight at the fix
// itself: at level `reloc` - no byte patches and NO stride fix - a solo
// round PLAYS, and at `full` it dies at RVA 0x00F7E91F, which is stride
// site "b", the SECOND consumer of the very value the fix substitutes at
// site "a" (0x01F2FDD1). The fix has never been tested on its own.
//
// Every other group stays ON, so each run removes exactly one thing with
// its dependencies intact. Status 75 reports which group was skipped, so
// a result can never be attributed to the wrong configuration later.
enum skip_group {
  skip_none = 0,
  skip_counts = 1,
  skip_settings = 2,
  skip_signin = 3,
  skip_storage = 4,
  skip_readfilter = 5,
  skip_stride = 6,
  skip_floor = 7,
};

skip_group skipped = skip_none;

skip_group current_skip() {
  char buf[16]{};
  const auto n = GetEnvironmentVariableA("BO3_SS_SKIP", buf, sizeof(buf));
  if (n == 0 || n >= sizeof(buf)) {
    return skip_none;
  }
  if (_stricmp(buf, "counts") == 0) {
    return skip_counts;
  }
  if (_stricmp(buf, "settings") == 0) {
    return skip_settings;
  }
  if (_stricmp(buf, "signin") == 0) {
    return skip_signin;
  }
  if (_stricmp(buf, "storage") == 0) {
    return skip_storage;
  }
  if (_stricmp(buf, "readfilter") == 0) {
    return skip_readfilter;
  }
  if (_stricmp(buf, "stride") == 0) {
    return skip_stride;
  }
  if (_stricmp(buf, "floor") == 0) {
    return skip_floor;
  }
  return skip_none;
}

bool alloc_floor_requested() {
  char buf[8]{};
  const auto n = GetEnvironmentVariableA("BO3_SS_FLOOR", buf, sizeof(buf));
  if (n == 0 || n >= sizeof(buf)) {
    return false;
  }
  return _stricmp(buf, "on") == 0 || _stricmp(buf, "1") == 0;
}

bool group_enabled(const skip_group g) { return skipped != g; }

bool try_apply() {
  set_status(0, status_magic);
  set_status(4, ++attempts);

  level = current_level();
  skipped = current_skip();
  set_status(74, static_cast<uint32_t>(level));
  set_status(75, static_cast<uint32_t>(skipped));
  if (level == level_off) {
    set_status(3, 0xFF);
    return true; // inert on purpose - do not retry, do not patch
  }

  if (!ready()) {
    return false; // image not settled yet - try again
  }

  // Reserve this while near address space is still available, but do not
  // copy or repoint s_gamePads yet. Activating the relocation here crashed
  // pre-menu gamepad init at RVA 0x022E9550. The lobby-time preparation
  // tool uses this exact reserved RVA after slots 0/1 are both signed in.
  const bool gamepads_reserved = reserve_gamepads_region();
  set_status(56, gamepads_reserved ? 1u : 2u);
  set_status(57, static_cast<uint32_t>(gamepads_reserved_rva));
  set_status(58, 0); // lobby tool: references activated (expect 38)
  set_status(59, 0); // lobby tool: bounds activated (expect 6)

  // Did we get in before Storage_Init? Zero means the storage allocator
  // has not run yet, so relocating s_storage and widening the allocator
  // still means something. Non-zero means we are too late and slots 2/3
  // will stay empty no matter what else is done - record it rather than
  // assume it, because assuming this is exactly what hid the problem.
  const auto pool_before =
      *reinterpret_cast<uint64_t *>(base() + storage_pool_rva);
  const bool early = pool_before == 0;
  set_status(9, early ? 1u : 2u);

  // Doing the storage work LATE is worse than not doing it at all,
  // and that is measured, not cautious: relocating s_storage and
  // widening the ten loop bounds after Storage_Init has already run
  // leaves records 2 and 3 uninitialised while making the whole
  // storage API iterate over them. The game died within seconds.
  //
  // So the storage half is strictly conditional on being ahead of
  // Storage_Init. If we are late we skip it entirely and keep the
  // previous known-good behaviour (bit array + stride fix), and
  // status index 9 says which happened. Never trade a working result
  // for a crash.
  uint32_t ok = 0;
  bool storage_moved = false;
  size_t slot = 0;
  for (const auto &t : reloc_tables) {
    const auto this_slot = slot++;
    const bool is_storage = std::strcmp(t.name, "storage") == 0;
    if (is_storage && !early) {
      note("[splitscreen] storage: Storage_Init already ran "
           "(pool 0x%llX) - skipping the relocation and the byte "
           "patches, they are only safe before it\n",
           static_cast<unsigned long long>(pool_before));
      continue;
    }

    if (relocate(t, this_slot)) {
      ++ok;
      if (is_storage) {
        storage_moved = true;
      }
      if (std::strcmp(t.name, "voice_comm") == 0) {
        // The vacated original IS clientUIActives slot 2 (plus
        // the head of slot 3). Zero it so the engine's
        // activation loop writes its active bit into the
        // console's initial state - a zeroed clientUIActive_t -
        // instead of stale voice data. relocate() has already
        // copied the live bytes to the new location.
        std::vector<uint8_t> zeros(t.old_size, 0);
        write_bytes(reinterpret_cast<void *>(base() + t.base_rva), zeros.data(),
                    zeros.size());
      }
    }
  }
  set_status(13, 99); // reached the end of the relocation loop
  set_status(1, ok);
  complete_players_kb();

  // RadiantExploderData is NOT in reloc_tables: the generic path copies
  // old_size and rewrites references at a FIXED offset, and this array
  // changes its internal layout (effects rows move +0x19F0 -> +0x19F8).
  // It carries its own verified transaction with its own rollback.
  // BO3_SKIP_FIX=<csv> disables individual [2]->[4] relocations so a
  // defect can be bisected against them without a rebuild per test.
  // Names: exploder, localentities, uiroot, percg.
  char skip_fix[128] = {};
  GetEnvironmentVariableA("BO3_SKIP_FIX", skip_fix, sizeof(skip_fix));
  const auto fix_enabled = [&](const char *name) {
    return std::strstr(skip_fix, name) == nullptr;
  };

  if (fix_enabled("exploder")) {
    relocate_radiant_exploders();
  }

  // cg_localEntities / cg_activeLocalEntities / cg_freeLocalEntities
  // are [2] where PS4 has [4], and the pool's slot 2 covers the
  // clientfield system, so CG_InitLocalEntities(2) zeroes the
  // world set. Flat relocation, its own verified transaction.
  if (fix_enabled("localentities")) {
    relocate_local_entities();
  }

  // The per-client LUI root array is [2] and its element 2 would land
  // on s_perController, so it MOVES and only then is its bound
  // widened. This is the array the 2026-08-06 retraction got wrong.
  if (fix_enabled("uiroot")) {
    relocate_lui_roots();
  }

  // Per-controller LUI state is [2]; controller 3's "UI active" byte
  // sat in the button-glyph text buffer (pane 4 went grey mid-round).
  if (fix_enabled("perctrl")) {
    relocate_per_controller();
    trace_line pc;
    pc.str(perctrl_result);
    trace_write(pc);
  }

  // The unnamed per-client CG/UI context array (0x04D2CB40, stride
  // 0x2BB8) is [2]; its slot 2 holds 47-ref-dense globals that a
  // client-2 write sprays with float defaults (the 10:23 crash).
  if (fix_enabled("percg")) {
    relocate_percg_context();
  }

  // THE PANE FIX, storage then geometry. The bounds and the pane
  // count are applied later, only if both of these took - see
  // install_pane_counts_and_bounds().
  //
  // BISECTABLE. The first build of Phase 2 blacked out the frontend
  // (a shared index lea converted while one of its two consumers was
  // not), and the second smeared it. Until the geometry is proven,
  // each phase can be switched off from outside without a rebuild:
  //     set BO3_PANES=storage    only the clientUIActives relocation
  //     set BO3_PANES=off        neither
  // anything else (or unset) runs the full fix.
  // OFF BY DEFAULT since 2026-08-22. The clientUIActives relocation
  // BLACK-SCREENS THE FRONTEND AT LAUNCH, and the cause is the
  // reference table, not the mechanism. A bisect (BO3_PANES=storage)
  // pinned it to Phase 1 alone, and a boundary audit found a row whose
  // write destroyed `lea r15,[rip+0x595805]` at 0x02B9D98C. Removing
  // that row did NOT fix it, so more of the 108 ABS32 rows are wrong -
  // which matches gen_reloc_header.py's own count for this array
  // ("128 address-takers, 16 direct accesses"), about 144, not the 249
  // this table carries.
  //
  // Opt IN only, never by default:
  //     set BO3_PANES=storage   the clientUIActives relocation only
  //     set BO3_PANES=full      storage + geometry
  // Anything else leaves the stock two-pane behaviour alone.
  char panes_mode[32] = {};
  GetEnvironmentVariableA("BO3_PANES", panes_mode, sizeof(panes_mode));
  // storage relocation = research only; geometry + cave always run.
  const bool want_storage = std::strcmp(panes_mode, "storage") == 0 ||
                            std::strcmp(panes_mode, "full") == 0;

  if (want_storage) {
    // research path only - see CLAUDE.md closed dead ends
    relocate_client_ui_actives();
  }
  // THE THIRD SCREEN, part 1: geometry only. The IsActive cave needs
  // the clientGameStates relocation to already have happened (it answers
  // lc>=2 from it), and that runs later in this same function - so the
  // cave is installed down there, next to the bounds, NOT here.
  relocate_view_params();
  // The two [2] arrays the pane path itself indexes at 2 - the
  // 2026-08-24 19:10 crash (scrPlaceView[2] overflow, proven by the
  // crash registers) and the sweep's FOREIGN slot 2 (0x04D322C0).
  // The pane bounds gate refuses without them, so a refused byte
  // check here degrades to stock two panes, never to a crash.
  if (fix_enabled("scrplace")) {
    relocate_flat24("scrPlaceView", 0x0577B800, 0x7C, scrplace_sites,
                    std::size(scrplace_sites), scrplace_relocated,
                    scrplace_new_rva);
  }
  if (fix_enabled("perclient54")) {
    relocate_flat24("perclient54", 0x04CB32C0, 0x54, perclient54_sites,
                    std::size(perclient54_sites), perclient54_relocated,
                    perclient54_new_rva);
  }
  // AimAssist globals: CG_SetView(2) reads AND writes slot 2, so this
  // must land before the pane bound is allowed to widen.
  // Complete v2 table (50 sites) - the flat24 table moved only the base
  // leas and left the accessors on the old array (see aaglob_array).
  if (fix_enabled("aaglob") && !aaglob_relocated) {
    const auto fresh = relocate_perclient(aaglob_array);
    if (fresh) {
      aaglob_relocated = true;
      aaglob_new_rva = fresh - base();
    }
    trace_line aa_line;
    aa_line.str(fresh ? "aaGlobArray [2] -> [4] (complete table: 50 sites)"
                      : "aaGlobArray: NOT moved - a site did not match");
    trace_write(aa_line);
  }
  // The UI element-handle word array - prerequisite for widening the
  // registrar loop (bound 0x01F331D2) below.
  if (fix_enabled("uielem")) {
    relocate_flat24("uiElemHandles", 0x1795CED8, 0x2, uielem_sites,
                    std::size(uielem_sites), uielem_relocated, uielem_new_rva);
    retarget_uielem_reader();
  }
  // Only now - the registrar bound needs the array above AND the LUI
  // roots relocation, both of which have run by this point.
  widen_ui_registrar_bound();
  // Keep the LUI renderer at two contexts - see hold_lui_context_count.
  hold_lui_context_count();
  // And make the HUD-refresh reader survive a client with no snapshot.
  install_snapguard_cave();
  // ...and let the per-client buffer clear skip clients it has no
  // buffers for (see install_perclient_buffer_guard).
  // Move the vestigial scene buffer B out of A[2]/A[3] BEFORE anything
  // reads them, then guard the clear until fill_scene_buffers() has
  // put real buffers in those slots.
  relocate_scene_buffer_b();

  // The entity-collision pair, opt-in with the same BO3_CG_FRAME flag as
  // the frame loop that needs it. It is only reachable once the cgame
  // ticks for client 2, and ae57c92 shipped an incomplete version of this
  // ungated and broke the DEFAULT build - so the good build stays
  // untouched until this has proved itself.
  {
    char entcoll_env[16] = {};
    GetEnvironmentVariableA("BO3_CG_FRAME", entcoll_env, sizeof(entcoll_env));
    if (std::strcmp(entcoll_env, "on") == 0) {
      relocate_entity_collision();
      // Kill-switch for bisecting: BO3_CF=off leaves the clientfield
      // pending-callback array stock while the rest of the group stays on.
      // Added 2026-09-14 when the third seat stopped auto-enrolling right
      // after that fix landed; the switch lets one launch settle blame.
      char cf_env[16] = {};
      GetEnvironmentVariableA("BO3_CF", cf_env, sizeof(cf_env));
      if (std::strcmp(cf_env, "off") != 0) {
        relocate_clientfield_callbacks();
      } else {
        note("[splitscreen] clientfield relocation SKIPPED (BO3_CF=off)\n");
      }
      // These five were added one at a time after the kill-switch and each
      // was pasted directly after the previous call - which put ALL of them
      // inside the BO3_CF block, so BO3_CF=off silently disabled them too.
      // Found in the 2026-09-25 audit. They are independent of the
      // clientfield fix and belong to the group, not to the switch.
      relocate_entword_table();
      {
        trace_line ow;
        ow.str(entword_result);
        trace_write(ow);
      }
      relocate_exposure_adaptions();
      {
        trace_line ex;
        ex.str(exposure_result);
        trace_write(ex);
      }
      relocate_sst_ring();
      {
        trace_line sr;
        sr.str(sst_result);
        trace_write(sr);
      }
      // 3 and 4 players in MP: every player's ChooseClass builds ~11.1k
      // model nodes; the stock pool (0x9000) holds two.
      relocate_ui_model_pool();
      {
        trace_line mp;
        mp.str(model_pool_result);
        trace_write(mp);
      }
      // Entering a mode with 3-4 players seated: the party join needs
      // every member's agreement over the lobby message loop.
      relocate_join_clients();
      {
        trace_line jc;
        jc.str(joinclient_result);
        trace_write(jc);
      }
      // MP HUD players 3/4: Engine.GetClientNum answered -1 for them.
      widen_lua_controller_checks();
      {
        trace_line lc;
        lc.str(lua_ctrl_result);
        trace_write(lc);
      }
      // (cl_voiceCommunication is NOT here: reloc_tables' voice_comm
      // moves it at startup, all 12 references - see its comment.)
      relocate_cgdc();
      relocate_playerkeys();
      relocate_notetracklerps();
      relocate_batch1b();
      relocate_batch2();
      relocate_batch3();
      relocate_batch4();
      {
        trace_line ik;
        ik.str(ikstates_new ? "ikStates [3] -> [5] (9 sites + reset end marker)"
                            : "ikStates: NOT moved - reset loop widened one "
                              "slot (3 players only)");
        trace_write(ik);
      }
      relocate_batch5();
    }
  }
  // NOT behind BO3_CG_FRAME: CG_Init(2) runs whenever player 3's cgame
  // initialises at map load, with or without the third pane.
  // The per-local-client [2][18] array the August seat table had split
  // (see relocate_session_members). Ungated: lc 2 needs it too.
  {
    trace_line sm;
    sm.str(
        relocate_session_members()
            ? "session members [2][18] x 0x132 -> [4] (4 sites, clear 0x5610)"
            : "session members: NOT moved (bytes differ)");
    trace_write(sm);
  }
  relocate_batch6();
  // Also ungated: the 190 MB slide happened with the third pane off too.
  relocate_batch7();
  // Before install_perclient_buffer_guard(): its cave bakes C's base.
  relocate_batch8();
  relocate_batch9();
  relocate_batch10();
  relocate_batch11();
  relocate_batch12();
  relocate_batch13();
  relocate_batch14();
  relocate_batch15();
  relocate_batch16();
  relocate_batch17();
  relocate_lightq();
  // Before R_Init allocates the culler object (see grow_umbra_client_arrays).
  grow_umbra_client_arrays();
  install_perclient_buffer_guard();
  install_ui_trace();
  install_guest_copy();
  widen_csc_lc_checks();
  gate_lensflares_for_extra_clients();
  // Before the clamp: it bounds the slot by the slices this creates.
  // OPT-IN (BO3_SUN4=on) until verified: 401d038 and 61b0c41 crashed
  // entering a 4-player round, and the next run (a32ed5d) died later on
  // a stray write over the cgame media table whose writer is not
  // identified yet. Without it players 2-4 share sun-shadow slot 1.
  char sun4_env[8] = {};
  GetEnvironmentVariableA("BO3_SUN4", sun4_env, sizeof(sun4_env));
  if (std::strcmp(sun4_env, "on") == 0) {
    grow_sun_shadow_slices();
    trace_line sg;
    sg.str(sun_grow_result);
    trace_write(sg);
  }
  clamp_sun_shadow_slot();
  skip_lensflare_exit_shutdown();
  // Before Com_Init runs Com_LocalClient_LastInput_Init.
  create_extra_controller_models();
  fix_gamepad_type_selectors();

  // Link the two client tables, by NAME not by index, so adding or
  // reordering a table cannot silently point this at the wrong array.
  size_t table_slot = SIZE_MAX;
  size_t array_slot = SIZE_MAX;
  for (size_t i = 0; i < std::size(reloc_tables); ++i) {
    if (std::strcmp(reloc_tables[i].name, "client_objs") == 0) {
      table_slot = i;
    } else if (std::strcmp(reloc_tables[i].name, "client_ui") == 0) {
      array_slot = i;
    }
  }
  set_status(14, (table_slot != SIZE_MAX && array_slot != SIZE_MAX)
                     ? link_client_objects(table_slot, array_slot)
                     : 0u);

  // Where each table actually landed. The allocator probes for a free
  // region within rip-relative reach, so the answer DIFFERS FROM RUN TO
  // RUN - 0x1FB00000 one launch, unmapped the next. Every external tool
  // that wants to read a relocated array has to be told the address, and
  // hard-coding last run's value silently reads unmapped memory and
  // reports "<unreadable>" as if the array were broken.
  //
  // Slots 16..19 line up with reloc_tables[]: bit_array, storage,
  // client_objs, client_ui.
  //
  // NOTE the collision above: mark() writes 11 + slot, so the stage
  // breadcrumbs for client_objs and client_ui land on 13 and 14, which
  // set_status(13, 99) and set_status(14, ...) then overwrite. Those two
  // slots are the loop/link results, NOT stage marks - do not read them
  // as stages.
  for (size_t i = 0; i < 4; ++i) {
    set_status(16 + i, static_cast<uint32_t>(new_base_rva[i]));
  }

  // By name, for the same reason as above.
  for (size_t i = 0; i < std::size(reloc_tables); ++i) {
    if (std::strcmp(reloc_tables[i].name, "storage") == 0) {
      storage_base_rva = new_base_rva[i];
    }
    // The fifth table cannot use the 16..19 window - slot 20 is
    // already "guest storage pumped". Publish it on its own slot.
    else if (std::strcmp(reloc_tables[i].name, "netchan") == 0) {
      set_status(73, static_cast<uint32_t>(new_base_rva[i]));
    }
  }

  // Local clients 2/3 get their command buffers (MP class choice,
  // every "cmd" a guest sends) - needs the relocated cbuf records above.
  install_cbuf_for_players34();
  note("[splitscreen] %s\n", cbuf34_result);

  // Every one of these widens a loop or a gate to reach s_storage[2]
  // and [3]. If the relocation did NOT happen, those records are not
  // ours - they are whatever the linker put after a [2] array, and
  // widening the loops would write into it. So this is strictly
  // conditional: no relocation, no patches.
  const bool do_storage =
      at_least(level_storage) && group_enabled(skip_storage);
  set_status(10, storage_moved ? 1u : 0u);
  set_status(8, (storage_moved && do_storage) ? apply_storage_patches() : 0u);

  // The stride fix is its OWN group. It shares the `storage` level with
  // the byte patches only because that is where it has always sat; it
  // has nothing to do with s_storage and is not gated on the relocation.
  // Keeping them in one skip group means a run can never say which of
  // the two mattered, and right now that is exactly the open question.
  // Deliberately NOT `do_storage && ...`: the stride fix has nothing to
  // do with s_storage, and chaining it to that group is what made
  // SKIP=storage change two things at once for the whole of the last
  // bisect. It shares only the LEVEL, so `reloc` still stops short of it.
  const bool do_stride = at_least(level_storage) && group_enabled(skip_stride);
  set_status(2, (do_stride && install_stride_fix()) ? 1u : 0u);

  // THE LOCAL-CLIENT COUNT. Strictly after the container relocations, and
  // only if every one of them succeeded: `ok` counts the relocated tables
  // (storage, client_objs and client_ui among them), so requiring the full
  // set is the same all-or-nothing rule the byte patches use. Raising the
  // count over a still-[2] array is the documented crash family.
  //
  // Status 60: how many of the three count patches applied (expect 3).
  // Status 61: the live cl_maxLocalClients (RVA 0x053A2720) - the single
  // number ~2630 bounds checks compare against, so it is worth reading
  // back rather than assumed. It is written later, by
  // CL_AllocatePerLocalClientMemory, so at post_unpack it reads 0.
  const auto counts_ok = (ok == std::size(reloc_tables)) &&
                         at_least(level_full) && group_enabled(skip_counts);

  // The cl_maxLocalClients HOLD belongs to this group too, and until
  // 2026-08-17 it did not know that: it was gated only on the
  // compile-time flag, so BO3_SS_SKIP=counts reported "0 count patches
  // applied" while status 61 still read 4. The engine went on believing
  // there were four local clients - the one number ~2630 bounds checks
  // compare against - and the run proved nothing at all. Measured live:
  // status 60 = 0 and status 61 = 4 in the same process.
  raise_local_client_count = counts_ok;

  // Make the engine's own active count able to reach 3 and 4, so it
  // sets splitscreen_playerCount correctly through its own Dvar_SetInt
  // (0x0283ABA5). Part of the COUNT group so the bisect can remove it.
  // Status 85 = installed, 86 = executions, 87 = the last count it
  // handed the engine.
  set_status(85, (counts_ok && install_active_count_fix()) ? 1u : 2u);

  // THE ALLOCATION FLOOR IS OFF BY DEFAULT from 2026-08-17. That one byte
  // is the whole round-start crash: with everything else on and only this
  // left out, a solo round PLAYS (measured, in game, 118 FPS). It tells
  // CL_AllocatePerLocalClientMemory four while splitscreen_playerCount -
  // and therefore every other caller - still says one.
  // hold_splitscreen_player_count() replaces it by setting the dvar to the
  // real count, which keeps allocator and readers in agreement.
  // BO3_SS_FLOOR=on restores the old byte for comparison runs.
  skip_alloc_floor = !alloc_floor_requested() || !group_enabled(skip_floor);

  set_status(60, counts_ok ? apply_local_client_count_patches() : 0u);
  // The twelve walker end-bounds belong to the same feature as the
  // activation loop bound above: both widen the client-UI walkers to
  // three elements, and both are only safe with voice_comm moved -
  // which counts_ok guarantees (it requires every relocation).
  set_status(95, counts_ok ? widen_client_ui_walker_bounds() : 0u);
  // Same gate: the end-of-match loops walk clientUIActives slot 2.
  if (counts_ok) {
    widen_client_shutdown_loops();
  }

  // The pane count and the four dispatcher bounds. Applied LAST and
  // only when Phase 1 (storage) and Phase 2 (geometry) both took:
  // IsActive has no bounds check of its own, so a widened loop with an
  // unrelocated array reads the foreign global at 0x053DACB0, and an
  // unreshaped geometry table indexes past its two rows.
  // Read-only; safe to install whenever the count group is on. Its
  // results are read straight out of the cave by activation_watch.py,
  // which resolves it through the patched call site.

  // NOTE: cl_maxLocalClients is NOT seeded here. At post_unpack it is not
  // yet 2 - CL init writes that later - so a one-shot seed at this point
  // always refused (measured: status 62 = 0 while status 60 = 3). The
  // async probe seeds it the moment the value first appears as 2, and
  // latches so it runs exactly once. Status 62 is set there.

  // clientGameStates 2 -> 3 slots. This is what makes
  // Com_ControllerIndex_GetLocalClientNum(2) able to return 2 instead of
  // -1, and PS4 proves that one number gates the gumball row
  // (BG_UnlockablesGetLocalCACRoot), guest menu input (CL_GamepadEvent
  // early-exits on IsMappedToLocalClient, which IS the -1 test) and the
  // per-player "LastInput"/"ControllerType" UI models, which are handles
  // stored in this very struct.
  //
  // Status 43: 1 relocated, 2 refused. maintain_signin_seats() then
  // reports whether the seat survives (52 = re-asserts, 53 = the raw
  // controllerIndex), and 51 reports the number that actually matters.
  // Gated for the bisect: 'reloc' and 'storage' levels stop short of
  // the seat work, so a run can isolate it.
  set_status(43, (at_least(level_signin) && group_enabled(skip_signin))
                     ? (relocate_signin_field() ? 1u : 2u)
                     : 0u);

  // THE THIRD SCREEN, part 2 - and the ORDER here is load-bearing.
  // install_isactive_cave() answers IsActive(lc>=2) out of the RELOCATED
  // clientGameStates, so it must run AFTER relocate_signin_field() above.
  // Placing it earlier (with the geometry) silently failed its
  // signin_relocated guard, the bounds then refused, and the old
  // read-only tracer took the same address instead - which looked like a
  // caved IsActive from outside. Verified by reading the cave body: the
  // tracer starts 4C 8B 04 24, this cave starts 83 F9 02.
  install_isactive_cave();
  install_pane_counts_and_bounds();
  // Also after relocate_signin_field(): it reads seat record 2 and
  // checks signin_relocated (placed with the controller models first,
  // it silently refused - measured 2026-09-27 15:27).
  widen_gamepad_button_models();
  widen_lobby_max_local_players();

  // The tracer patches the SAME function as the cave, so it runs AFTER it
  // and only when the cave is absent. Ordered the other way round the
  // tracer wins the address and the third screen silently never installs.
  if (counts_ok && !isactive_caved) {
    install_is_active_tracer();
  }

  // Cheap enough to run continuously - a handful of 4-byte reads. The
  // measuring instrument must not disturb what it measures; this project
  // has broken that rule four times.
  // ON THE RENDERER TOO, and that is the fix for the loading hang.
  //
  // Measured 2026-08-18 while stuck on a black screen with a spinner and
  // no map artwork - the classic "lobby State message failed or timed
  // out" stall:
  //
  //     [25] ticks: pipeline async      48 -> 48      FROZEN
  //     [24] ticks: pipeline renderer 2195 -> 2227    alive
  //     [34] per-controller sweeps   37446 -> 37945   alive
  //
  //     game lobby acks: [23, 23, 15, 0]   <- slot 2 stuck 8 behind
  //
  // So this hold - the ONE thing that carries the injected client
  // through the launch handshake - was registered only on the async
  // pipeline, and the async pipeline STOPS during the launch. The ack
  // never gets forced forward, the launch waits for a client that can
  // never answer, and the load hangs.
  //
  // This also retires STATE.md's "async reliable, renderer ~20 then
  // stops": during a LAUNCH it is exactly the other way round.
  //
  // The hold only ever moves values FORWARD (`if (*ack < target_ack)`)
  // and copies a real guest's state, so running it from two pipelines is
  // idempotent, not a race.
  scheduler::loop(hold_injected_clients, scheduler::pipeline::async, 5ms);
  scheduler::loop(hold_injected_clients, scheduler::pipeline::renderer, 5ms);

  // Fill the guest records the moment element 1 becomes a signed-in
  // profile. Deliberately fast (5ms): the window closes when the boot
  // storage pass runs, and that pass is what gives controller 1 its
  // local-file stats readiness at the first menu.
  scheduler::loop(fill_guests_when_ready, scheduler::pipeline::async, 5ms);

  // Storage_Pump must be entered on the GAME'S MAIN THREAD - it is the
  // thread every other caller uses, and it walks 64 file records per
  // target type. Running it from the async pipeline would race the
  // game's own storage work. Slow tick: it stops itself as soon as both
  // guests hold an xuid, and until the guests exist it does nothing.
  scheduler::loop(pump_on_renderer, scheduler::pipeline::renderer, 250ms);
  // Renderer pipeline on purpose: it is the one that keeps running
  // through a LAUNCH (async freezes there - 2026-08-18), which is
  // exactly when R_InitSceneBuffers has just run and client 2 still
  // has no scene buffers. Early-outs to a bool test once filled.
  scheduler::loop(fill_scene_buffers, scheduler::pipeline::renderer, 100ms);
  scheduler::loop(maintain_sun_trans_views, scheduler::pipeline::renderer,
                  100ms);
  scheduler::loop(cl_init_watch, scheduler::pipeline::renderer, 250ms);
  scheduler::loop(count_async_ticks, scheduler::pipeline::async, 250ms);
  scheduler::loop(publish_active_count, scheduler::pipeline::async, 250ms);
  scheduler::loop(publish_stride_counters, scheduler::pipeline::async, 250ms);
  scheduler::loop(mirror_signin_state, scheduler::pipeline::async, 50ms);
  scheduler::loop(maintain_signin_seats, scheduler::pipeline::async, 50ms);

  // DIAGNOSTIC, opt-in (BO3_IDATA_TRAP=on): the 2026-09-25 20:52 round on
  // 9BF0B68B died because the WHOLE .idata section (0x1AAE6000..0x1AAE9600)
  // had been moved down 8 bytes at runtime - every import slot held its
  // successor's function, so `call [EnterCriticalSection]` at 0x0217B22E ran
  // RaiseException(&lock) (code 0xE8F1C490 = the lock's address). A healthy
  // menu process has .idata identical to the file (tools/idata_live.py), and
  // no memory holds a pointer to its start, so the writer computes it. This
  // makes .idata read-only 30 s after launch - after BOIII's own import
  // hooks, which go through VirtualProtect and keep working - so the next
  // write faults AT the culprit instruction and BOIII's dump records it.
  {
    char trap_env[8] = {};
    GetEnvironmentVariableA("BO3_IDATA_TRAP", trap_env, sizeof(trap_env));
    if (std::strcmp(trap_env, "on") == 0) {
      std::thread([] {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        DWORD old{};
        VirtualProtect(reinterpret_cast<void *>(base() + 0x1AA67000), 0x4000,
                       PAGE_READONLY, &old);
      }).detach();
    }
  }

  // pump_guests_when_quiet is deliberately NOT registered. Measured
  // 2026-08-12: even gated on the game's own storage work having
  // started and then stopped for 3 s, driving Storage_Pump from the
  // async pipeline kills the client during startup - twice, and again
  // after the "never pumped yet" bug in the gate was fixed. Whatever
  // makes storage unsafe from this thread is not a timing window that
  // can be waited out.
  //
  // It stays in the file because the REAPER half is what mattered and
  // that runs from the detour, on the game thread, where it works.

  // s_targets MUST be widened before anything pumps controller 2 or 3;
  // without it every storage call for them runs off the end of a
  // two-controller row. If it fails, the guest pump stays disabled
  // rather than crashing the game.
  const auto targets_ok = widen_storage_targets();
  set_status(28, targets_ok ? 1u : 0u);

  // Must happen before controller 2 does any local-file work, i.e. as
  // early as everything else here - once it has written over the
  // experiments table there is nothing to repair.
  set_status(30, widen_local_file_ops() ? 1u : 0u);

  // Verify the expected prologue before ever calling it, same rule as
  // the byte patches and the detour.
  process_tasks_ok =
      std::memcmp(reinterpret_cast<const void *>(base() + process_tasks_rva),
                  process_tasks_prologue, sizeof(process_tasks_prologue)) == 0;
  set_status(31, process_tasks_ok ? 1u : 2u);

  clear_storage_ok =
      std::memcmp(reinterpret_cast<const void *>(base() + clear_storage_rva),
                  clear_storage_prologue, sizeof(clear_storage_prologue)) == 0;
  set_status(36, clear_storage_ok ? 1u : 2u);

  // NOT relocated here. Measured 2026-08-12: doing it at post_unpack
  // succeeds mechanically - 76 of 76 references rewritten, destination
  // verified empty - and then NOTHING signs in at all: StartOp is
  // called zero times for every controller and all four storage slots
  // stay empty. Same shape as the s_rootData dead end in CLAUDE.md,
  // where the move is fine but the moment is not.
  //
  // It happens on the first StartOp instead: by then the game has
  // initialised the array, and no storage read has COMPLETED yet,
  // which is the deadline that matters (completion is what runs
  // SettingsReadResult with the -1).
  // Every detour below is the 'full' level only - the bisect needs a
  // run with the relocations in place and NO detours hooked.
  if (at_least(level_full)) {
    const auto sread = base() + storage_read_rva;
    if (std::memcmp(reinterpret_cast<const void *>(sread),
                    storage_read_prologue,
                    sizeof(storage_read_prologue)) == 0) {
      if (group_enabled(skip_readfilter)) {
        storage_read_hook.create(reinterpret_cast<void *>(sread),
                                 storage_read_stub);
        set_status(48, 1);
      }
    } else {
      set_status(48, 2);
    }

    // The settings completion, neutered for guests only. Same fail-safe
    // rule as every other detour: verify the prologue or do not hook.
    // Without this hook file 0 must stay OUT of guest_seated_file_types -
    // the two changes are one change and only make sense together.
    const auto srr = base() + settings_read_result_rva;
    if (std::memcmp(reinterpret_cast<const void *>(srr),
                    settings_read_result_prologue,
                    sizeof(settings_read_result_prologue)) == 0) {
      if (group_enabled(skip_settings)) {
        settings_read_result_hook.create(reinterpret_cast<void *>(srr),
                                         settings_read_result_stub);
        settings_result_neutered = true;
        set_status(67, 1);
      }
    } else {
      set_status(67, 2);
    }

    const auto scrr = base() + shoutcaster_read_result_rva;
    if (std::memcmp(reinterpret_cast<const void *>(scrr),
                    shoutcaster_read_result_prologue,
                    sizeof(shoutcaster_read_result_prologue)) == 0) {
      if (group_enabled(skip_settings)) {
        shoutcaster_read_result_hook.create(reinterpret_cast<void *>(scrr),
                                            shoutcaster_read_result_stub);
        shoutcaster_result_neutered = true;
        set_status(71, 1);
      }
    } else {
      set_status(71, 2);
    }

    // Part of the COUNT group so the bisect can remove it.
    if (group_enabled(skip_counts)) {
      const auto spc = base() + splitscreen_player_count_rva;
      if (std::memcmp(reinterpret_cast<const void *>(spc),
                      splitscreen_player_count_prologue,
                      sizeof(splitscreen_player_count_prologue)) == 0) {
        splitscreen_player_count_hook.create(reinterpret_cast<void *>(spc),
                                             splitscreen_player_count_stub);
        set_status(90, 1);
      } else {
        set_status(90, 2);
      }

      // CL_LocalClient_SetActive - the correctly-ordered trigger for
      // the CL_Init(2) step (see set_active_stub). Prologue-verified
      // like every other detour here; if it does not match, nothing
      // is hooked and the count detour remains as the fallback.
      const auto sa = base() + set_active_rva;
      if (std::memcmp(reinterpret_cast<void *>(sa), set_active_prologue,
                      sizeof(set_active_prologue)) == 0) {
        set_active_hook.create(reinterpret_cast<void *>(sa), set_active_stub);
        set_status(89, 1);
      } else {
        set_status(89, 2);
      }
    }

    const auto sop = base() + start_op_rva;
    if (std::memcmp(reinterpret_cast<const void *>(sop), start_op_prologue,
                    sizeof(start_op_prologue)) == 0) {
      start_op_hook.create(reinterpret_cast<void *>(sop), start_op_stub);
      set_status(42, 1);
    } else {
      set_status(42, 2);
    }

    // Enter the engine's own plural lobby add early enough that a
    // controller-2 sign-in which is already ready participates in the
    // original loop. The late fallback uses only the verified singular
    // function and never re-enters this detour.
    const auto laa = base() + lobby_add_all_rva;
    if (std::memcmp(reinterpret_cast<const void *>(laa), lobby_add_all_prologue,
                    sizeof(lobby_add_all_prologue)) == 0) {
      lobby_add_all_hook.create(reinterpret_cast<void *>(laa),
                                lobby_add_all_stub);
    }

    // DEACTIVATE SPLITSCREEN must take player 3 out too (see
    // guest_signin_stub). Prologue-verified; no hook otherwise.
    const auto lll = base() + lobbyvm_local_leave_rva;
    if (std::memcmp(reinterpret_cast<const void *>(lll),
                    lobbyvm_local_leave_prologue,
                    sizeof(lobbyvm_local_leave_prologue)) == 0) {
      lobbyvm_local_leave_hook.create(reinterpret_cast<void *>(lll),
                                      lobbyvm_local_leave_stub);
    }
    const auto gsi = base() + guest_signin_rva;
    if (std::memcmp(reinterpret_cast<const void *>(gsi), guest_signin_prologue,
                    sizeof(guest_signin_prologue)) == 0) {
      guest_signin_hook.create(reinterpret_cast<void *>(gsi),
                               guest_signin_stub);
    }
    const auto swc = base() + swap_clients_rva;
    if (std::memcmp(reinterpret_cast<const void *>(swc), swap_clients_prologue,
                    sizeof(swap_clients_prologue)) == 0) {
      swap_clients_hook.create(reinterpret_cast<void *>(swc),
                               swap_clients_stub);
    }

    // Detour the per-controller update so the guests' finished tasks get
    // reaped once the whole 0..3 sweep has returned. Same fail-safe rule:
    // verify the expected prologue or do not hook at all.
    const auto pcu = base() + per_controller_update_rva;
    if (process_tasks_ok &&
        std::memcmp(reinterpret_cast<const void *>(pcu),
                    per_controller_update_prologue,
                    sizeof(per_controller_update_prologue)) == 0) {
      per_controller_update_hook.create(reinterpret_cast<void *>(pcu),
                                        per_controller_update_stub);
      per_controller_update_hooked = true;
      set_status(33, 1);
    } else {
      set_status(33, 2);
    }

    // Verify the expected prologue before detouring, same rule as the
    // byte patches: fail safe, never fail dirty.
    const auto pump = base() + storage_pump_rva;
    if (targets_ok &&
        std::memcmp(reinterpret_cast<const void *>(pump), storage_pump_prologue,
                    sizeof(storage_pump_prologue)) == 0) {
      storage_pump_hook.create(reinterpret_cast<void *>(pump),
                               storage_pump_stub);
      storage_pump_hooked = true;
      set_status(22, 1);

      // NO ASYNC FALLBACK. It was justified by a measurement that
      // widening s_targets then invalidated: with the two-controller
      // rows in place the game called Storage_Pump 3 times and stopped,
      // so async looked like the only way to get more pumps. Once
      // s_targets is [4] the storage subsystem actually makes progress
      // and the game pumps controller 1 ~112 times a minute instead -
      // and an async thread calling into storage alongside that is a
      // straight data race. Measured cost of getting this wrong:
      // s_storage[0] and [1] lost their xuids entirely.
      //
      // The detour gives more than enough chances on the right thread.
    } else {
      set_status(22, 2); // prologue mismatch - refused to hook
    }
  } // end of the level_full detour block
  set_status(3, 1);
  set_status(5, alloc_regions_seen);
  set_status(6, alloc_free_seen);
  set_status(7, alloc_last_error);
  return true;
}
} // namespace

// ---- THE SILENT LAUNCH DEATH: who calls exit? (2026-09-27) --------------
//
// CLAUDE.md "~21 s silent launch death": exit code 0, no dialog, no dump,
// no event log. The trace history of two such launches today (lines 795,
// 831 of splitscreen_ui_trace.txt) shows the component's whole post_unpack
// completed and UI_CoD_Init never ran - the game exits during its own
// Com_Init, ~13 s into a stretch that healthy launches finish.
// Exit code 0 is an orderly ExitProcess, and BOIII routes the game's
// ExitProcess through exit_hook -> component_loader::pre_destroy() (main.cpp).
// So the caller is on this stack: log it. Both the unwound frames and a raw
// scan of stack words that point into the game image, because the
// Arxan-flattened code does not always unwind. If a death leaves NO exit
// line, the process was terminated directly (not ExitProcess) - that is
// the next thing to hook.
uint64_t component_start_tick = 0;

void trace_process_exit() {
  trace_line l;
  l.str("t=");
  l.dec(GetTickCount64());
  l.str(" PROCESS EXIT after ");
  l.dec(component_start_tick ? (GetTickCount64() - component_start_tick) / 1000
                             : 0);
  l.str(" s");
  trace_stack(l);
  l.str(" raw:");
  const auto b = base();
  const auto *tib = reinterpret_cast<const NT_TIB *>(NtCurrentTeb());
  const auto *word = reinterpret_cast<const uint64_t *>(&l);
  const auto *top = static_cast<const uint64_t *>(tib->StackBase);
  int found = 0;
  for (; word < top && found < 24; ++word) {
    const auto v = *word;
    if (v > b + 0x1000 && v < b + 0x1FAB7000) {
      l.str(" ");
      l.hex(v - b);
      ++found;
    }
  }
  trace_write(l);
}

class component final : public generic_component {
public:
  void pre_destroy() override {
    if (!game::is_server()) {
      trace_process_exit();
    }
  }

  void post_unpack() override {
    if (game::is_server()) {
      return;
    }
    if (game::header_checksum() != 0x06531394) {
      return;
    }
    component_start_tick = GetTickCount64();

    // Apply IMMEDIATELY, not from the scheduler.
    //
    // This used to be scheduler::schedule(..., 500ms), and that is a race
    // this component was losing every single run: the scheduler only turns
    // over once the game's main loop is up, which is long after
    // Storage_Init has already allocated storage buffers for two
    // controllers. Relocating s_storage after that point moves an array
    // whose slots 2 and 3 nothing will ever fill again.
    //
    // The status block already proved the image is patchable here - it
    // reported `attempts 1`, i.e. every table matched on the first pass at
    // post_unpack. So the delay bought nothing and cost the storage fix.
    // The retry is kept only as a fallback if that ever stops being true.
    if (!try_apply()) {
      scheduler::schedule(
          [] {
            return try_apply() ? scheduler::cond_end : scheduler::cond_continue;
          },
          scheduler::pipeline::async, 500ms);
    }
  }
};
} // namespace splitscreen

REGISTER_COMPONENT(splitscreen::component)
