#include <std_include.hpp>
#include <loader/component_loader.hpp>
#include <component/scheduler.hpp>
#include <game/game.hpp>
#include <game/utils.hpp>

#include "../game_event.hpp"

#include <utils/string.hpp>

#include <std_include.hpp>
#include "game/impl/phys/phys.hpp"
#include "loader/component_loader.hpp"

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/nt.hpp>
#include <utils/string.hpp>

namespace vehicle {

static constexpr const uintptr_t NO_CG_ARRAY_SYS_ERROR_BRANCHES[] = {
    0x14002EDF2, 0x14002F08D, 0x14002F5F6, 0x14002FFAD, 0x1400305E7,
    0x140030897, 0x1400318EE, 0x140031D6B, 0x140034DA7, 0x140050554,
    0x140050A40, 0x140050C6E, 0x1400511BD, 0x1400513EE, 0x140051AB6,
    0x1400797DB, 0x140079A26, 0x140079B4A, 0x140079D48, 0x140079DA1,
    0x14007A6E9, 0x14007AA5E, 0x14007AB4B, 0x14007AC06, 0x14007ACBE,
    0x14007FC34, 0x14007FE5E, 0x140080067, 0x14008013E, 0x1400801C4,
    0x1400802AE, 0x140080734, 0x140080938, 0x140082FB6, 0x1400832AC,
    0x140083491, 0x140083DAE, 0x140088B12, 0x140088E4E, 0x140089132,
    0x140089272, 0x14008956E, 0x14008AD6C, 0x14008D808, 0x14008DCFC,
    0x14008ED54, 0x14008F213, 0x14008F333, 0x14008F3BA, 0x14008F423,
    0x14008F63C, 0x14008F68F, 0x14008F75E, 0x14008F893, 0x14008FA3D,
    0x14008FB63, 0x14008FC8E, 0x140090767, 0x140090861, 0x140090931,
    0x140090A01, 0x140090AD1, 0x140090BA1, 0x140090C71, 0x140090EB3,
    0x140090F33, 0x140090FB3, 0x140091033, 0x1400910B3, 0x140091133,
    0x1400911B3, 0x140091233, 0x140092573, 0x140092789, 0x1400927A3,
    0x1400928E4, 0x1400928FE, 0x1400929CF, 0x140093F13, 0x1400947DD,
    0x140095354, 0x14009536E, 0x14009543E, 0x140095514, 0x14009552E,
    0x140095655, 0x140095DBE, 0x140095E0D, 0x14009726A, 0x14009739B,
    0x1400977CE, 0x140097B22, 0x140098682, 0x1400988CD, 0x140098A7B,
    0x1400991A1, 0x14009A414, 0x14009AF14, 0x1400A174E, 0x1400A28B2,
    0x1400A2B1E, 0x1400A2B8E, 0x1400A3426, 0x1400A3A1A, 0x1400A3D34,
    0x1400A4C1E, 0x1400A4D6E, 0x1400A4E98, 0x1400A572E, 0x1400A67F8,
    0x1400A6B4A, 0x1400A70F7, 0x1400A89BA, 0x1400AB2A2, 0x1400AB92C,
    0x1400ACB41, 0x1400AD0EA, 0x1400ADDC4, 0x1400AE27E, 0x1400AEBE3,
    0x1400AEEEC, 0x1400AF7B3, 0x1400B02EB, 0x1400B22AE, 0x1400B2715,
    0x1400B28DA, 0x1400B2A76, 0x1400B2F5F, 0x1400B301F, 0x1400B3129,
    0x1400B33F0, 0x1400B3B6A, 0x1400B6556, 0x1400B9EF3, 0x1400BAB5A,
    0x1400BACEA, 0x1400BBD27, 0x1400BD94A, 0x1400BE267, 0x1400BECC6,
    0x1400BED5A, 0x1400BF105, 0x1400BFE33, 0x1400C0003, 0x1400C14E3,
    0x1400C529C, 0x1400C52ED, 0x1400C58AB, 0x1400C5ADA, 0x1400C5F6E,
    0x1400C7429, 0x1400C85E8, 0x1400CB246, 0x1400CB5DA, 0x1400CB914,
    0x1400D03D0, 0x1400D06B3, 0x1400D0E3E, 0x1400D104C, 0x1400D1465,
    0x1400D157F, 0x1400D15CE, 0x1400D1744, 0x1400D316A, 0x1400D326F,
    0x1400D925A, 0x1400DA3C6, 0x1400DA4D6, 0x1400DA606, 0x1400DA9E3,
    0x1400DB69B, 0x1400DBE36, 0x1400DC284, 0x1400DC7EA, 0x1400DC8DB,
    0x1400DCAA2, 0x1400DCD16, 0x1400DD083, 0x1400DEA71, 0x1400E048B,
    0x1400E1D01, 0x1400E1FD6, 0x1400E2092, 0x1400E213E, 0x1400E222E,
    0x1400E2304, 0x1400E23C4, 0x1400E328E, 0x1400E37D6, 0x1400E3851,
    0x1400E61E3, 0x1400E657E, 0x1400E7CE3, 0x1400E820E, 0x1400E832B,
    0x1400E9A77, 0x1400E9C51, 0x1400EA815, 0x1400EC0EB, 0x1400EDD93,
    0x1400EDEBE, 0x1400EF5E7, 0x1400EF7E6, 0x1400F0D11, 0x1400F525F,
    0x1400F6484, 0x1400F776D, 0x1400F79DC, 0x1400F7AC6, 0x1400F7BEA,
    0x1400F7D8A, 0x1400F7F16, 0x1400F7FDA, 0x1400F8065, 0x1400F82BF,
    0x1400F83F4, 0x1400F8608, 0x1400F8856, 0x1400F8956, 0x1400F8AA6,
    0x1400F8CB6, 0x1400F90A6, 0x1400F9284, 0x1400F9366, 0x1400F9564,
    0x1400F96F9, 0x1400F987B, 0x1400F9BE5, 0x1400F9CFC, 0x1400F9DFE,
    0x1400F9F29, 0x1400FA084, 0x1400FA224, 0x1400FA328, 0x1400FA3D5,
    0x1400FA4A5, 0x1400FA565, 0x1400FA61B, 0x1400FA732, 0x1400FA8A3,
    0x1400FA935, 0x1400FA9A2, 0x1400FACA6, 0x1400FADAB, 0x1400FAF3B,
    0x1400FB0CB, 0x1400FB23C, 0x1400FB45F, 0x1400FB522, 0x1400FB79D,
    0x1400FB90F, 0x1400FBADD, 0x1400FBD4D, 0x1400FC004, 0x1400FC0BC,
    0x1400FC23C, 0x1400FC3F4, 0x1400FC4DD, 0x1400FC616, 0x1400FC6C9,
    0x1400FC7C6, 0x1400FC8AD, 0x1400FCA11, 0x1400FCB5B, 0x1400FCEB8,
    0x1400FCF74, 0x1400FD026, 0x1400FD1DC, 0x1400FD32A, 0x1400FD47A,
    0x1400FD5EA, 0x1400FD72A, 0x1400FD87A, 0x1400FD9BA, 0x1400FDB24,
    0x1400FE52B, 0x1400FE6AB, 0x1400FE7F5, 0x1400FE8C5, 0x1400FF4DF,
    0x1400FF656, 0x1400FF796, 0x1400FF90F, 0x1400FF99F, 0x1400FFA8E,
    0x1401001E7, 0x1401002E5, 0x1401003B7, 0x140100665, 0x140100813,
    0x140100FC1, 0x140101048, 0x1401010DC, 0x1401011CC, 0x14010121F,
    0x140101361, 0x1401013BD, 0x1401016C0, 0x140101886, 0x1401018E6,
    0x140101D36, 0x140101FD6, 0x14010234B, 0x1401024F6, 0x1401026C3,
    0x14010276A, 0x140102ABE, 0x140102D3E, 0x140102E4B, 0x140102EFF,
    0x14010443E, 0x1401046E6, 0x140104E2A, 0x140104FA8, 0x140104FFD,
    0x1401050AC, 0x14010510B, 0x140105943, 0x140105B15, 0x140105C36,
    0x140105F21, 0x140106047, 0x14010616B, 0x1401062EA, 0x140106425,
    0x140106487, 0x14010657E, 0x14010665B, 0x140106B36, 0x140106F3A,
    0x1401070C0, 0x140107413, 0x140107EB6, 0x140107F89, 0x1401084C6,
    0x1401086AC, 0x140108B75, 0x140108DB5, 0x140108E4F, 0x140108F24,
    0x140108FEC, 0x14010918A, 0x1401094A6, 0x1401095AC, 0x1401096C6,
    0x1401097DC, 0x140109900, 0x140109A2C, 0x140109B4C, 0x140109C6C,
    0x140109D8C, 0x140109EAB, 0x140109FDC, 0x14010A0FC, 0x14010A399,
    0x14010A966, 0x14010AEB1, 0x14010B028, 0x14010B2E4, 0x14010C1D8,
    0x14010C400, 0x14010CD1F, 0x14010CDC4, 0x14010CE74, 0x14010CF16,
    0x14010D0B2, 0x14010D70B, 0x14010DD48, 0x14010E1D0, 0x14010E646,
    0x14010E7ED, 0x14010EAA9, 0x14010EBF4, 0x14010EC96, 0x14010EE19,
    0x14010EF3F, 0x14010F1AC, 0x14010F2A4, 0x14010F346, 0x14010F516,
    0x14010F82F, 0x14010F965, 0x14010FA6A, 0x14010FAE4, 0x14010FCE4,
    0x14010FD88, 0x14010FF3A, 0x14011003D, 0x140110461, 0x14011064A,
    0x1401106A4, 0x1401108B4, 0x140110994, 0x14011103C, 0x14011113C,
    0x140111214, 0x140111F0F, 0x140111F9B, 0x1401121B8, 0x1401124B2,
    0x14011266F, 0x140112A2C, 0x140112C2A, 0x14011393B, 0x140114A96,
    0x1401193B1, 0x140119421, 0x1401194AA, 0x140119791, 0x140119852,
    0x140119AA1, 0x140119C82, 0x140119DC3, 0x140119F32, 0x14011A22B,
    0x14011A401, 0x14011A858, 0x14011AE32, 0x14011B169, 0x14011B346,
    0x14011B48E, 0x14011BEDC, 0x14011BFBD, 0x14011C105, 0x14011C491,
    0x14011C5C0, 0x14011CA88, 0x14011DF8E, 0x14011E321, 0x14011E39B,
    0x14011F672, 0x14011FAF1, 0x140122E5F, 0x140122EB8, 0x140122F71,
    0x140124B31, 0x140125152, 0x1401264BA, 0x14012BD1E, 0x14012BDE4,
    0x14012D370, 0x14012EBB4, 0x1401321CC, 0x14013240F, 0x140132DE2,
    0x14013372C, 0x140134376, 0x14013498E, 0x140134B35, 0x140134BE8,
    0x140134D95, 0x14013AD57, 0x14013B024, 0x14013CE89, 0x14013DF02,
    0x14013DF4A, 0x14013FE6D, 0x14013FEDE, 0x1401412BD, 0x14014133D,
    0x140141D67, 0x140141F35, 0x140141FFC, 0x14014303B, 0x1401439FF,
    0x140146F3B, 0x14014735E, 0x140147886, 0x140148697, 0x140148BE9,
    0x14014C3E4, 0x14014C42B, 0x14014D79E, 0x140150039, 0x140150153,
    0x1401506CF, 0x140150D6C, 0x140151F8F, 0x14017DD8E, 0x1401E097B,
    0x1401F208E, 0x1401F3901, 0x1401F3E04, 0x1401F4F0E, 0x14020C92C,
    0x14020CFAF, 0x1402132DD, 0x14021ECB4, 0x140508E19, 0x1405BB3C4,
    0x1405BC60D, 0x1405BCE8D, 0x1405BE038, 0x1405C10CB, 0x1405C5D94,
    0x1405D92A2, 0x1405D9F2D, 0x1405DCA98, 0x1405DCB40, 0x1405DCC3A,
    0x1405DCCE9, 0x1405DEDFA, 0x1405E235F, 0x1405E7AC7, 0x1405EFD45,
    0x1405F2F20, 0x1405F4225, 0x1405F48E0, 0x1405F73E0, 0x1405F74DF,
    0x1405F7F89, 0x1405F93DD, 0x1405FB514, 0x1405FB575, 0x140600DD3,
    0x140601C3B, 0x1406042BC, 0x1406137EF, 0x140616469, 0x14061659E,
    0x140654751, 0x14066B0DD, 0x140670032, 0x140670766, 0x14067F3A2,
    0x14068086F, 0x140681D90, 0x140682C51, 0x140682E1E, 0x14068455A,
    0x14068A72E, 0x1406903F4, 0x140693020, 0x1406AA26E, 0x1406CC172,
    0x1406CC247, 0x1406CC382, 0x1406CC4D4, 0x1406FD63E, 0x1406FE051,
    0x1406FE9A1, 0x1400DF6D5, 0x1405F754B,

};

using namespace game::vehicle;
using namespace game::phys;
using namespace game::cg;
using namespace game::level;
using namespace game::level::cl;
using namespace game;
namespace {

inline void remove_no_cg_array_sys_error_branches() {
  for (const uintptr_t branch : NO_CG_ARRAY_SYS_ERROR_BRANCHES) {
    utils::hook::nop_branch(game::relocate(branch));
  }
}

static std::once_flag assign_pool_pointers_flag;
void assign_pool_pointers() {
  *game::cg::builtin_cgArray =
      reinterpret_cast<game::level::cl::cgPool *>(&cgArray);

  *game::cg::builtin_cgsArray =
      reinterpret_cast<game::level::cl::cgsPool *>(&cgsArray);

  game::cg::builtin_cg_entitiesArray->pools[0] = &cg_entitiesArray[0];

  game::cg::builtin_cg_entitiesArray->pools[1] = &cg_entitiesArray[1];
}

void reset_pools() {
  memset(&cgArray, 0, sizeof(cgArray));
  memset(&cgsArray, 0, sizeof(cgsArray));
  memset(&cg_entitiesArray, 0, sizeof(cg_entitiesArray));
}

inline void use_client_entities() {
  remove_no_cg_array_sys_error_branches();

  game_event::on_g_init_game([]() -> void {
    std::call_once(assign_pool_pointers_flag, assign_pool_pointers);
    reset_pools();
  });
  game_event::on_g_shutdown_game([]() -> void { reset_pools(); });
}

utils::hook::detour path_constraint_update_hook;
utils::hook::detour NitrousVehicle_is_path_moving_hook;
bool NitrousVehicle_is_path_moving_sv(NitrousVehicle *self) {
  if (self) {
    level::gentity_t *ent = self->m_owner;
    return ent && ent->vehicle &&
           (ent->vehicle->moveState != VehicleMoveState::STOP ||
            (ent->vehicle->phys.prevOrigin != ent->vehicle->phys.origin) ||
            (ent->vehicle->phys.prevAngles != ent->vehicle->phys.angles));
  }
  return false;
}

utils::hook::detour NitrousVehicle_unpause_physics_hook;
void NitrousVehicle_unpause_physics_always_collide_wheels(
    NitrousVehicle *self) {
  NitrousVehicle_unpause_physics_hook.invoke(self);
  {
    const sys::ScopedCriticalSection _critsect =
        game::sys::ScopedCriticalSection(
            sys::CriticalSection::PHYSICS,
            sys::ScopedCriticalSection::ScopedCriticalSectionType::NORMAL);
    if (!self->m_vehicle_def->useHeliBoneControllers &&
        self->m_flags.initialized) {
      phys::collide_vehicle_wheels(self);
      for (uint8_t wheelIdx = 0; wheelIdx < ARRAYSIZE(self->m_wheels);
           ++wheelIdx) {
        phys::rigid_body_constraint_wheel *rbc_wheel = self->m_wheels[wheelIdx];
        if (rbc_wheel) {
          rbc_wheel->calc_penetration_depth();
        }
      }
    }
  }
}

utils::hook::detour NitrousVehicle_pause_physics_hook;
void NitrousVehicle_pause_physics_if_not_sentient(NitrousVehicle *self,
                                                  bool shutdown) {
  if (self && self->m_vehicle_def && !self->m_vehicle_def->isSentient) {
    return NitrousVehicle_pause_physics_hook.invoke(self, shutdown);
  }
}
} // namespace

struct component final : server_component {
  void post_unpack() override {
    use_client_entities();
    path_constraint_update_hook.create(path_constraint_update,
                                       path_constraint_update_sv);
    NitrousVehicle_is_path_moving_hook.create(
        NitrousVehicle::syms::is_path_moving, NitrousVehicle_is_path_moving_sv);
    NitrousVehicle_unpause_physics_hook.create(
        NitrousVehicle::syms::unpause_physics,
        NitrousVehicle_unpause_physics_always_collide_wheels);
    NitrousVehicle_pause_physics_hook.create(
        NitrousVehicle::syms::pause_physics,
        NitrousVehicle_pause_physics_if_not_sentient);
  }
};
} // namespace vehicle

REGISTER_COMPONENT(vehicle::component)