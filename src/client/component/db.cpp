#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>

#include <utils/hook.hpp>

namespace db {
using namespace game;
using namespace game::com;
using namespace game::db;
using namespace game::db::xasset;

utils::hook::detour DB_IsXAssetDefault_hook;
bool DB_IsXAssetDefault_Safe(XAssetType type, const char *name) {
  if ((nonnull(name) || valid_stack_ptr(name)) && name[0]) {
    return DB_IsXAssetDefault_hook.invoke<bool>(type, name);
  }
  return false;
}

utils::hook::detour DDL_Buffer_ResetContext_hook;
bool DDL_Buffer_ResetContext_Safe(void *buff, int32_t len,
                                  const ddl::DDLDef *ddlDef,
                                  ddl::DDLContext *const ddlContext,
                                  ddl::DDLWriteCB writeCB, void *userData) {
  if ((nonnull(buff) || valid_stack_ptr(buff)) &&
      (nonnull(ddlDef) || valid_stack_ptr(ddlDef)) &&
      (nonnull(ddlContext) || valid_stack_ptr(ddlContext))) {
    return DDL_Buffer_ResetContext_hook.invoke<bool>(
        buff, len, ddlDef, ddlContext, writeCB, userData);
  }

  return false;
}

static std::atomic_bool Com_GametypeSettings_Initialised = false;
inline bool Com_GametypeSettings_ShouldInit() {
  return !nonnull(*com::gts::s_gametypeSettingsDDL) ||
         !Com_GametypeSettings_Initialised.load(std::memory_order_seq_cst);
}

utils::hook::detour Com_GametypeSettings_GametypeSetting_f_hook;
void Com_GametypeSettings_GametypeSetting_f_GetOrInitGameTypeSettingsDDL() {

  if (sys::Cmd_Argc() > 1 && Com_GametypeSettings_ShouldInit()) {
    com::gts::Com_GametypeSettings_Init();
  }

  return Com_GametypeSettings_GametypeSetting_f_hook.invoke();
}
utils::hook::detour Com_GametypeSettings_SetGametype_hook;
void Com_GametypeSettings_SetGametype_GetOrInitGameTypeSettingsDDL(
    const char *gametype, bool loadDefaultSettings, bool isModified) {

  if (!com::Com_SessionMode_IsMode(eModes::COUNT) && loadDefaultSettings &&
      Com_GametypeSettings_ShouldInit()) {
    com::gts::Com_GametypeSettings_Init();
  }

  return Com_GametypeSettings_SetGametype_hook.invoke(
      gametype, loadDefaultSettings, isModified);
}

static std::recursive_mutex Com_GametypeSettings_Init_Lock;
utils::hook::detour Com_GametypeSettings_Init_hook;
void Com_GametypeSettings_Init_Once() {
  std::lock_guard<std::recursive_mutex> lock(Com_GametypeSettings_Init_Lock);
  if (Com_GametypeSettings_ShouldInit()) {
    Com_GametypeSettings_Init_hook.invoke();
    Com_GametypeSettings_Initialised.store(true, std::memory_order_seq_cst);
  }
}

static constexpr maptable::MapTable DEFAULT_MAPTABLE = {
    .name = "",
    .mapCount = 0,
    ._padding0C = {0},
    .maps = nullptr,
    .sessionMode = eModes::INVALID,
    .campaignMode = CampaignMode::INVALID,
    .dlcIndex = dlcIndex_t::DLC_INDEX_COUNT,
    ._padding24 = {0}};

utils::hook::detour Com_GetMapTable_hook;
maptable::MapTable *Com_GetMapTable_Safe(const char *mapTableName) {

  maptable::MapTable *mapTable =
      DB_FindXAssetHeader(XAssetType::MAPTABLE, mapTableName, true, -1)
          .mapTable;
  if (!nonnull(mapTable) || !nonnull(mapTable->name) ||
      DB_IsXAssetDefault(XAssetType::MAPTABLE, mapTable->name)) {
    return const_cast<maptable::MapTable *>(&DEFAULT_MAPTABLE);
  }
  return mapTable;
}

utils::hook::detour Image_AssignDefaultTexture_hook;
bool Image_AssignDefaultTexture_SkipMissingVTable(gfx::GfxImage *to,
                                                  gfx::GfxImage *from) {
  if (nonnull(to) && nonnull(from) &&
      (!s_assetPools->typed.image.contains(to) ||
       (nonnull(to->texture.basemap) &&
        nonnull(to->texture.basemap->lpVtbl)))) {
    return Image_AssignDefaultTexture_hook.invoke<bool>(to, from);
  }

  return false;
}

class component final : public generic_component {
public:
  void post_unpack() override {
    DB_IsXAssetDefault_hook.create(DB_IsXAssetDefault.get(),
                                   DB_IsXAssetDefault_Safe);
    DDL_Buffer_ResetContext_hook.create(
        game::ddl::DDL_Buffer_ResetContext.get(), DDL_Buffer_ResetContext_Safe);
    Com_GametypeSettings_GametypeSetting_f_hook.create(
        game::com::gts::Com_GametypeSettings_GametypeSetting_f.get(),
        Com_GametypeSettings_GametypeSetting_f_GetOrInitGameTypeSettingsDDL);
    Com_GametypeSettings_Init_hook.create(
        game::com::gts::Com_GametypeSettings_Init.get(),
        Com_GametypeSettings_Init_Once);
    Com_GetMapTable_hook.create(maptable::Com_GetMapTable.get(),
                                Com_GetMapTable_Safe);
    Com_GametypeSettings_SetGametype_hook.create(
        game::com::gts::Com_GametypeSettings_SetGametype.get(),
        Com_GametypeSettings_SetGametype_GetOrInitGameTypeSettingsDDL);
    Image_AssignDefaultTexture_hook.create(
        game::gfx::Image_AssignDefaultTexture.get(),
        Image_AssignDefaultTexture_SkipMissingVTable);
  }
};
} // namespace db

REGISTER_COMPONENT(db::component)