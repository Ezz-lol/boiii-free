#pragma once
#include "../../../game/game.hpp"

namespace game {

extern const dvar_t *sv_wwwBaseURL;
extern const dvar_t *sv_wwwDownload;
extern const dvar_t *sv_wwwDlDisconnected;
extern const dvar_t *workshop_id;

dvarStrHash_t Dvar_GenerateHash_Impl(const char *str);
} // namespace game