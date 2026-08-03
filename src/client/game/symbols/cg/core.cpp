#include <std_include.hpp>
#include "../../game.hpp"
#include "core.hpp"

namespace game {
namespace cg {
LocalClientPool<level::cl::cg_t> cgArray{};
LocalClientPool<level::cl::cgs_t> cgsArray{};
game::LocalClientPool<game::level::cl::centityPool_t> cg_entitiesArray{};
anim::ViewModelInfoPool cg_viewModelArray{};
ClientPlayerAttachmentInfoPool cg_attachmentsArray{};
LocalClientPool<ClientPlayerWeaponInfo> cg_weaponsArray{};
LocalClientPool<phys::Destructible> cg_destructibles{};
LocalClientPool<ik::IkBuf> cg_ikBuf{};
} // namespace cg
} // namespace game