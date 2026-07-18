#pragma once
#include <game/game.hpp>

namespace game {

extern EngineDependentDvar sv_wwwBaseURL;
extern EngineDependentDvar sv_wwwDownload;
extern EngineDependentDvar sv_wwwDlDisconnected;
extern EngineDependentDvar workshop_id;
extern EngineDependentDvar com_pauseSupported;

dvarStrHash_t Dvar_GenerateHash_Impl(const char *str);
} // namespace game