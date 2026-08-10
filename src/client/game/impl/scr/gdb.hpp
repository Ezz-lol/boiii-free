#pragma once
#include <game/game.hpp>
#include <game/utils.hpp>

namespace game {
namespace scr {
void LoadScriptGDB2_Impl(const scriptInstance_t inst);
void LoadScriptGDB_Impl(const scriptInstance_t inst,
                        objFileInfo_t *const fileInfo);
objFileInfo_t *Scr_FindObjFileInfo_Impl(const scriptInstance_t inst,
                                        void *const addr);
void Scr_GetFileAndLineNum_Impl(const scriptInstance_t inst, uint8_t *const pos,
                                const char **const filename,
                                int32_t *const lineNum,
                                const char **const sourceLine);

} // namespace scr
} // namespace game