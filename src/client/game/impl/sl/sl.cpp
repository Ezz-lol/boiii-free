#include <std_include.hpp>
#include "sl.hpp"


namespace game {
namespace sl {
void SL_RemoveRefToStringFree_ThreadSafe(scr::ScrString_t stringValue) {

  if (game::is_client() || stringValue != *sl_protectedScriptString) {
    const sys::ScopedCriticalSection critsect = sys::ScopedCriticalSection(
        sys::CriticalSection::SCRIPT_STRING,
        sys::ScopedCriticalSection::ScopedCriticalSectionType::NORMAL);

    volatile sl::RefString *refstr = reinterpret_cast<volatile sl::RefString *>(
        &scr::mt::gScrMemTreePub->mt_buffer->nodes[stringValue]);
    if (refstr->dec_ref() == 0) {
      sl::SL_RemoveRefToStringFree(stringValue, refstr);
    }
  }
}
} // namespace sl
} // namespace game