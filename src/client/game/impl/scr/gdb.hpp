#pragma once
#include <game/game.hpp>
#include <game/utils.hpp>

namespace game {
namespace scr {
void LoadScriptGDB2_Impl(scriptInstance_t inst);
void LoadScriptGDB_Impl(scriptInstance_t inst, objFileInfo_t *file_info);
objFileInfo_t *Scr_FindObjFileInfo_Impl(scriptInstance_t inst, void *addr);
void Scr_GetFileAndLineNum_Impl(scriptInstance_t inst, uint8_t *pos,
                                const char **filename, int32_t *line_num,
                                const char **source_line);
std::vector<int32_t> Scr_GetImportLineNumbers(scriptInstance_t inst,
                                              const GSC_OBJ *obj,
                                              objFileInfo_t *file_info,
                                              const GSC_IMPORT_ITEM *import);
void ReportObjLinkError_Impl(scriptInstance_t inst, GSC_OBJ *prime_obj,
                             objFileInfo_t *file_info, GSC_IMPORT_ITEM *import,
                             char *errorString, int errorStringLength);

} // namespace scr
} // namespace game
