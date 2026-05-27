#include "xzone.hpp"
#include "../../ugc/ugc.hpp"
#include "../../../../game/game.hpp"

namespace game {
namespace db {
namespace xzone {

bool DB_CheckModXFile_Impl(const char *fastfileName) {
  if (ugc::active_mod->publisherId[0]) {

    ugc::WorkshopData *mod =
        ugc::UGC_GetModByPublisherId(ugc::active_mod->publisherId);
    char file_path[0x100];
    if (mod && mod->absolutePathZoneFiles[0]) {
      snprintf(file_path, sizeof(file_path), "%s/%s/%s%s",
               mod->absolutePathZoneFiles, "", fastfileName, ".ff");
    } else {
      const char *cwd = sys::Sys_Cwd();
      snprintf(file_path, sizeof(file_path), "%s/%s/%s/%s/%s%s", cwd, "mods",
               ugc::active_mod->publisherId, "", fastfileName, ".ff");
    }
    stream::stream_fileid fh = stream::Stream_OpenFile(file_path, 11);
    if (fh != -1) {
      stream::Stream_CloseFile(fh);
      return true;
    }
  }
  return false;
}
} // namespace xzone
} // namespace db
} // namespace game