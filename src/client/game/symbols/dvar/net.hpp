#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> net_broadcast_ip{0x156DC9BD0, 0x1498C08F0};

WEAK symbol<EngineDependentDvarMut> net_emu_client{0x0, 0x149F24E30};
WEAK symbol<EngineDependentDvarMut> net_emu_jitter{0x15742E048, 0x149F24E18};
WEAK symbol<EngineDependentDvarMut> net_emu_latency{0x15742E040, 0x149F24E10};
WEAK symbol<EngineDependentDvarMut> net_emu_packet_loss{0x15742E050, 0x149F24E20};
WEAK symbol<EngineDependentDvarMut> net_emu_server{0x15742E058, 0x149F24E28};
WEAK symbol<EngineDependentDvarMut> net_ip{0x157DF63E8, 0x14A640488};
WEAK symbol<EngineDependentDvarMut> net_noudp{0x157DF6408, 0x14A6404A8};
WEAK symbol<EngineDependentDvarMut> net_port{0x157DF63F0, 0x14A640490};
WEAK symbol<EngineDependentDvarMut> net_socksenabled{0x157DF68C0, 0x14A640960};
WEAK symbol<EngineDependentDvarMut> net_sockspassword{0x157DF68E0, 0x14A640980};
WEAK symbol<EngineDependentDvarMut> net_socksport{0x157DF68D0, 0x14A640970};
WEAK symbol<EngineDependentDvarMut> net_socksserver{0x157DF68C8, 0x14A640968};
WEAK symbol<EngineDependentDvarMut> net_socksusername{0x157DF68D8, 0x14A640978};
WEAK symbol<EngineDependentDvarMut> netstats_dummy_tracker_data{0x156DC9BD8, 0x1498C08F8};
WEAK symbol<EngineDependentDvarMut> reliableResendTime{0x0, 0x14A6376A0};
WEAK symbol<EngineDependentDvarMut> reliableTimeoutTime{0x0, 0x14A6376A8};

} // namespace game
