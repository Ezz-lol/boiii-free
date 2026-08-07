#pragma once

#include "game/ptr.hpp"
#include "game/structs/macros.hpp"
#include "structs/func.hpp"
namespace game {
namespace sys {
enum class CriticalSection : int32_t {
  ALLOC_MARK = 0x0,
  FX_VIS = 0x1,
  OCCLUSION_QUERY = 0x2,
  PHYSICS = 0x3,
  PHYSICS_UPDATE = 0x4,
  PHYSICS_DESTRUCTIBLE_HIT = 0x5,
  STREAM_ALLOC = 0x6,
  STREAM_BACKEND_UPDATE = 0x7,
  STREAM_CANCEL = 0x8,
  STREAM_MODEL_BITS = 0x9,
  FX_CHUNK_ALLOC = 0xA,
  FX_UNIQUE_HANDLE = 0xB,
  SOUND_COMMAND_ALLOC = 0xC,
  SOUND_COMMAND_PUSH = 0xD,
  SOUND_NOTIFY_ALLOC = 0xE,
  SOUND_NOTIFY_PUSH = 0xF,
  SOUND_BANK = 0x10,
  SOUND_LOOKUP_CACHE = 0x11,
  SOUND_SUBMIT = 0x12,
  CAREER_STATS = 0x13,
  CONSOLE = 0x14,
  DEBUG_SOCKET = 0x15,
  COM_ERROR = 0x16,
  STATMON = 0x17,
  MEM_ALLOC0 = 0x18,
  MEM_ALLOC1 = 0x19,
  MEM_ALLOC2 = 0x1A,
  MEM_ALLOC3 = 0x1B,
  MEM_ALLOC4 = 0x1C,
  MEM_ALLOC5 = 0x1D,
  MEM_ALLOC6 = 0x1E,
  MEM_ALLOC7 = 0x1F,
  MEM_ALLOC8 = 0x20,
  MEM_ASYNC_COMMIT = 0x21,
  CLIENT_MESSAGE = 0x22,
  CLIENT_CMD = 0x23,
  DOBJ_ALLOC = 0x24,
  XANIM_ALLOC = 0x25,
  KEY_BINDINGS = 0x26,
  SERVER_MESSAGE = 0x27,
  SERVER_PLAYERINFO = 0x28,
  SERVER_TICK = 0x29,
  SCRIPT_STRING = 0x2E,
  MEMORY_TREE = 0x2F,
  SCRIPT_DEBUGGER = 0x2C,
  VM = 0x2D,
  THREADED_NOTIFY_QUEUE = 0x2A,
  LIVE = 0x2B,
  PCACHE = 0x30,
  AUDIO_PHYSICS = 0x31,
  LUA = 0x32,
  LUI = 0x33,
  LOBBY_LUA = 0x34,
  EXTRACAM = 0x35,
  CINEMATIC = 0x36,
  CINEMATIC_MP4 = 0x37,
  CINEMATIC_UPDATEFRAME = 0x38,
  SAVE_PROFILE = 0x39,
  MEM_CONTAINER_SAVEGAME = 0x3A,
  VIRTUALALLOC = 0x3B,
  RB_TRANSFER = 0x3C,
  STREAM_SYNC_COMMAND = 0x3D,
  STREAM_FIXEDIMAGE_UPDATE = 0x3E,
  STREAM_MODELHINT_UPDATE = 0x3F,
  DWLSG = 0x40,
  DWNET = 0x41,
  IK = 0x42,
  TL_MEMALLOC = 0x43,
  VA_ALLOC = 0x44,
  MEMTRACK = 0x45,
  CBUF = 0x46,
  CURVEALLOC = 0x47,
  NETQUEUE = 0x48,
  ZLIB = 0x49,
  BLACKBOX = 0x4A,
  GDT_COMMAND = 0x4B,
  STRINGED_COMMAND = 0x4C,
  RADIANT_SERVER_COMMAND = 0x4D,
  RADIANT_CLIENT_COMMAND = 0x4E,
  RECORDER = 0x4F,
  SERVERDEMO = 0x50,
  IO_SCHEDULER = 0x51,
  FILE_ID_ARRAY = 0x52,
  MEMFIRSTFIT = 0x53,
  FXBEAM = 0x54,
  GLASS_ACTIONS = 0x55,
  DBHASH = 0x56,
  CLUMP = 0x57,
  SNAPSHOT_PROFILE = 0x58,
  CRITSEC_WEBM_STREAM_ACCESS = 0x59,
  CRITSEC_SV_LEADERBOARDS = 0x5A,
  FONT_CACHE = 0x5B,
  CRITSEC_NETCHAN = 0x5C,
  SWITCHMAP_SV = 0x5D,
  SWITCHMAP_CL = 0x5E,
  DB_THROTTLE_DECOMPRESSION = 0x5F,
  PROFILE_BUF = 0x60,
  SAVE_DEVICE = 0x61,
  WATER_INTERACTION = 0x62,
  WATER_GRID_ALLOC = 0x63,
  WATER_NODE_ALLOC = 0x64,
  MEM_PAGED = 0x65,
  ENTITY_COLL_LINK = 0x66,
  MINIZ = 0x67,
  COMPOSITING = 0x68,
  CREATE_DYNENT = 0x69,
  LENSFLARES_INSTANCE_POOL_ACCESS0 = 0x6A,
  LENSFLARES_INSTANCE_POOL_ACCESS1 = 0x6B,
  LENSFLARES_INSTANCE_POOL_ACCESS2 = 0x6C,
  LENSFLARES_INSTANCE_POOL_ACCESS3 = 0x6D,
  LENSFLARES_INSTANCE_POOL_ACCESS4 = 0x6E,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS0 = 0x6F,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS1 = 0x70,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS2 = 0x71,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS3 = 0x72,
  LENSFLARES_ACTIVE_INSTANCES_ACCESS4 = 0x73,
  LENSFLARES_SOURCE_ACCESS0 = 0x74,
  LENSFLARES_SOURCE_ACCESS1 = 0x75,
  LENSFLARES_SOURCE_ACCESS2 = 0x76,
  LENSFLARES_SOURCE_ACCESS3 = 0x77,
  LENSFLARES_SOURCE_ACCESS4 = 0x78,
  COMSCORE = 0x79,
  WEAPON_DOBJ_INFO_ALLOC = 0x7A,
  LIVE_ALLOC = 0x7B,
  UMBRA_SPOTOMNI_CACHE = 0x7C,
  UMBRA_JOB = 0x7D,
  COUNT = 0x7E,
};
IMPL_ENUM_OPERATORS(CriticalSection);

class ScopedCriticalSection {
public:
  enum ScopedCriticalSectionType : uint32_t {
    NORMAL = 0x0,
    DISABLED = 0x1,
    RELEASE = 0x2,
    TRY = 0x3,
  };

  CriticalSection _s;
  bool _hasOwnership;
  bool _isScopedRelease;
  uint8_t _padding06[2];
  ScopedCriticalSection *_next;

  ScopedCriticalSection(CriticalSection s, ScopedCriticalSectionType type) {
    const thiscallPtr_t<void(ScopedCriticalSection *, CriticalSection,
                             ScopedCriticalSectionType)>
        impl = reinterpret_cast<
            thiscallPtr_t<void(ScopedCriticalSection *, CriticalSection,
                               ScopedCriticalSectionType)>>(
            game::select(0x1400D4030, 0x14004F520));
    impl(this, s, type);
  }

  ~ScopedCriticalSection() {
    const thiscallPtr_t<void(ScopedCriticalSection *)> impl =
        reinterpret_cast<thiscallPtr_t<void(ScopedCriticalSection *)>>(
            game::select(0x1400D40F0, 0x14004F5E0));
    impl(this);
  }
};

} // namespace sys
} // namespace game