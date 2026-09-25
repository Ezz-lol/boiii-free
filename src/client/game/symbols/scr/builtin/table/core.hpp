#pragma once

#include <game/symbols/sym_include.hpp>
namespace game {
namespace scr {
namespace builtin {
namespace table {
WEAK symbol<ActorInterfaceMethodTable> actor_interface_methods{
    0x1432BA470, 0x143339470, 0x1410C96D0};
WEAK symbol<ActorMethodTable> actor_methods{0x1433FDD80, 0x14347CD60,
                                            0x141187760};
WEAK symbol<BotMethodTable> bot_methods{0x1432BA6D0, 0x1433396D0, 0x1410C9930};
WEAK symbol<CommonFunctionTable> common_functions{0x1432C67A0, 0x1433457A0,
                                                  0x1410D59D0};
WEAK symbol<CommonMethodTable> common_methods{0x1432BCCB0, 0x14333BCB0,
                                              0x1410CBF10};
WEAK symbol<HelicopterMethodTable> helicopter_methods{0x1432BCB80, 0x14333BB80,
                                                      0x1410CBDE0};
WEAK symbol<HudElemMethodTable> hudElem_methods{0x1432C56A0, 0x1433446A0,
                                                0x1410D48D0};
WEAK symbol<MathFunctionTable> math_functions{0x1432BE810, 0x14333D810,
                                              0x1410CDA70};
WEAK symbol<PlayerMethodTable> player_methods{0x1432C1C90, 0x143340C90,
                                              0x1410D0EC0};
WEAK symbol<ScriptEntMethodTable> scriptEnt_methods{0x1432CE140, 0x14334D140,
                                                    0x1410DD370};
WEAK symbol<ScriptVehicleMethodTable> scriptVehicle_methods{0x1432CECC0,
                                                            0x14334DCB0, 0x0};
WEAK symbol<SentientFunctionTable> sentient_functions{0x1432BF320, 0x14333E320,
                                                      0x1410CE580};
WEAK symbol<SentientMethodTable> sentient_methods{0x1432BEF40, 0x14333DF40,
                                                  0x1410CE1A0};
WEAK symbol<UtilFunctionTable> util_functions{0x1432BE8A0, 0x14333D8A0,
                                              0x1410CDB00};

} // namespace table
} // namespace builtin
} // namespace scr
} // namespace game
