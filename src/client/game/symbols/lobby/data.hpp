#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lobby {
namespace data {
WEAK symbol<VoteType(uint32_t itemIndex)> LobbyData_GetItemVoteType{0x141EF2E30, 0x1404ACC80};

WEAK symbol<VoteType(weapon::eAttachment attachment)>
    LobbyData_GetAttachmentVoteType{0x141EF2C20, 0x1404ACB40};
WEAK symbol<VoteType(uint32_t itemIndex, weapon::eAttachment attachment)>
    LobbyData_GetItemAttachmentVoteType{0x141EF2CC0, 0x1404ACBD0};
WEAK symbol<VoteType(itemGroup_t itemGroup, weapon::eAttachment attachment)>
    LobbyData_GetAttachmentGroupVoteType{0x141EF2B60, 0x1404ACA90};
} // namespace data
} // namespace lobby
} // namespace game
