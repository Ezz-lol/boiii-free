#pragma once

#include <game/symbols/sym_include.hpp>
namespace game {
namespace scr {
namespace builtin {
namespace table {
WEAK symbol<ScrActorInterfaceMethodTable> actor_interface_methods{0x143339470,
                                                                  0x1410C96D0};
WEAK symbol<ScrActorMethodTable> actor_methods{0x14347CD60, 0x141187760};
WEAK symbol<ScrBotMethodTable> bot_methods{0x1433396D0, 0x1410C9930};
WEAK symbol<ScrCommonFunctionTable> common_functions{0x1433457A0, 0x1410D59D0};
WEAK symbol<ScrCommonMethodsTable> common_methods{0x14333BCB0, 0x1410CBF10};
WEAK symbol<ScrHelicopterMethodTable> helicopter_methods{0x14333BB80,
                                                         0x1410CBDE0};
WEAK symbol<ScrHudElemMethodTable> hudElem_methods{0x1433446A0, 0x1410D48D0};
WEAK symbol<ScrMathFunctionTable> math_functions{0x14333D810, 0x1410CDA70};
WEAK symbol<ScrPlayerMethodTable> player_methods{0x143340C90, 0x1410D0EC0};
WEAK symbol<ScrScriptEntMethodTable> scriptEnt_methods{0x14334D140,
                                                       0x1410DD370};
WEAK symbol<ScrScriptVehicleMethodTable> scriptVehicle_methods{0x14334DCB0};
;
WEAK symbol<ScrSentientMethodTable> sentient_methods{0x14333DF40, 0x1410CE1A0};
WEAK symbol<ScrUtilFunctionTable> util_functions{0x14333D8A0, 0x1410CDB00};

} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game