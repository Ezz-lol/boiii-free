#include <std_include.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include <loader/component_loader.hpp>

#include <component/game_event.hpp>
#include <utils/hook.hpp>

// Weapon array keys leak a script variable reference on every write, which
// eventually fills the variable pool. Store them as object keys instead.

namespace weapon {
namespace {
constexpr uintptr_t eval_array_ref_weapon_key = 0x14015E112;
constexpr uint8_t eval_array_ref_weapon_key_original[] = {
    0x8B, 0x57, 0x08, 0x8B, 0xCE, 0xE8, 0xE4, 0xD3, 0x00, 0x00, 0xEB, 0x36};

constexpr uintptr_t get_array_keys_type_check = 0x14016521F;
constexpr uint8_t get_array_keys_type_check_original[] = {0x83, 0xF8, 0x04,
                                                          0x74, 0x0E};

game::EngineDependentDvarMut sv_weapon_object_keys;

bool checked = false;
bool supported = false;
bool applied = false;
void *eval_array_ref_stub = nullptr;
void *get_array_keys_stub = nullptr;

void eval_array_ref_weapon_key_stub(utils::hook::assembler &a) {
  const auto found = a.get().new_label();

  a.get().sub(rsp, 0x30);
  a.get().mov(dword_ptr(rsp, 0x20), r8d);
  a.get().mov(edx, dword_ptr(rdi, 8));
  a.get().mov(dword_ptr(rsp, 0x24), edx);
  a.get().mov(ecx, esi);
  a.get().mov(r8d, r8d);
  a.call(0x14016A910_g); // find entry
  a.get().test(eax, eax);
  a.get().jnz(found);

  a.get().mov(ecx, esi);
  a.get().mov(edx, dword_ptr(rsp, 0x24));
  a.get().mov(r8d, dword_ptr(rsp, 0x20));
  a.call(0x14016BE30_g); // create with object key

  a.get().bind(found);
  a.get().mov(dword_ptr(rsp, 0x28), eax);
  a.get().mov(ecx, esi);
  a.get().mov(edx, dword_ptr(rsp, 0x20));
  a.call(0x14016C950_g); // release the stack's key reference

  a.get().mov(eax, dword_ptr(rsp, 0x28));
  a.get().add(rsp, 0x30);
  a.jmp(0x14015E154_g);
}

void get_array_keys_type_check_stub(utils::hook::assembler &a) {
  const auto other_type = a.get().new_label();

  a.get().cmp(eax, 4);
  a.get().jne(other_type);

  a.get().mov(ebx, ecx);
  a.get().mov(edx, ecx);
  a.get().mov(ecx, edi);
  a.call(0x14016F290_g); // push key object
  a.jmp(0x140165278_g);

  a.get().bind(other_type);
  a.jmp(0x140165224_g);
}

bool matches(const uintptr_t address, const uint8_t *bytes,
             const size_t size) {
  return std::memcmp(reinterpret_cast<const void *>(game::relocate(address)),
                     bytes, size) == 0;
}

// Only called from G_InitGame, so the script VM is fresh and no array holds
// keys stored the other way
void set_enabled(const bool enable) {
  if (enable == applied)
    return;

  if (!checked) {
    checked = true;
    supported =
        matches(eval_array_ref_weapon_key, eval_array_ref_weapon_key_original,
                sizeof(eval_array_ref_weapon_key_original)) &&
        matches(get_array_keys_type_check, get_array_keys_type_check_original,
                sizeof(get_array_keys_type_check_original));
    if (!supported) {
      printf("Weapon object keys not applied: the code it replaces has been "
             "changed\n");
    }
  }

  if (!supported)
    return;

  if (enable) {
    if (!eval_array_ref_stub) {
      eval_array_ref_stub =
          utils::hook::assemble(eval_array_ref_weapon_key_stub);
      get_array_keys_stub =
          utils::hook::assemble(get_array_keys_type_check_stub);
    }

    utils::hook::jump(game::relocate(eval_array_ref_weapon_key),
                      eval_array_ref_stub);
    utils::hook::nop(game::relocate(eval_array_ref_weapon_key) + 5,
                     sizeof(eval_array_ref_weapon_key_original) - 5);
    utils::hook::jump(game::relocate(get_array_keys_type_check),
                      get_array_keys_stub);
  } else {
    utils::hook::copy(game::relocate(eval_array_ref_weapon_key),
                      eval_array_ref_weapon_key_original,
                      sizeof(eval_array_ref_weapon_key_original));
    utils::hook::copy(game::relocate(get_array_keys_type_check),
                      get_array_keys_type_check_original,
                      sizeof(get_array_keys_type_check_original));
  }

  applied = enable;
  if (enable)
    printf("Weapon object keys enabled\n");
}
} // namespace

struct component final : server_component {
#ifndef NDEBUG
  std::string name() override { return "weapon"; }
#endif

  void post_unpack() override {
    sv_weapon_object_keys = game::register_dvar_bool(
        "sv_weaponObjectKeys", false, game::DVAR_NONE,
        "Store weapon array keys as objects to work around a stock game bug "
        "that slowly fills the script variable pool. Only needed on servers "
        "that require it. Takes effect on the next map load");

    game_event::on_g_init_game([] {
      const bool wanted =
          game::com::Com_SessionMode_GetMode() == game::eModes::MULTIPLAYER &&
          sv_weapon_object_keys && sv_weapon_object_keys.get_bool();
      set_enabled(wanted);
    });
  }
};
} // namespace weapon

REGISTER_COMPONENT(weapon::component)
