#pragma once
#ifdef GAME_HPP

namespace game {

// misc globals
WEAK symbol<clientplatform_t> clientplatform{0x0, 0x14A63D4E8};
WEAK symbol<qboolean(void *ent)> StuckInClient{0x1415A8360, 0x14023BFE0};
// Unnamed
WEAK symbol<const char *(const char *name)> CopyString{0x1422AC220,
                                                       0x14056BD70};
} // namespace game

#endif
