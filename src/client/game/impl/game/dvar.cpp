#include "../../../std_include.hpp"
#include "dvar.hpp"
#include "game.hpp"

namespace game {
const dvar_t *sv_wwwBaseURL;
const dvar_t *sv_wwwDownload;
const dvar_t *sv_wwwDlDisconnected;
const dvar_t *workshop_id;

dvarStrHash_t Dvar_GenerateHash_Impl(const char *str) {
  if (str == nullptr) {
    return 0;
  }

  // Hexadecimal literal
  if (str[0] == '0' && str[1] == 'x') {
    return std::strtoul(&str[2], nullptr, 16);
  }

  return static_cast<dvarStrHash_t>(CanonHash(str));
}
} // namespace game