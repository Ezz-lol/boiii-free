#include <std_include.hpp>
#include <cstdint>
#include "../../../../../common/utils/string.hpp"
#include "../../../utils.hpp"

#include "../../game/game.hpp"
#include "../scr.hpp"

#include "scrcmd.hpp"

namespace game {
namespace scr {
namespace cmd {

void ScrCmd_PlaySound_Impl(scriptInstance_t inst, scr_entref_t *entref) {
  snd::SndAliasId alias_id;
  if (Scr_GetType(SCRIPTINSTANCE_SERVER, 0) == ScrVarType::STRING) {
    const char *alias = Scr_GetString(SCRIPTINSTANCE_SERVER, 0);
    alias_id = snd::SND_FindAliasId(alias);
  } else {
    alias_id = Scr_GetInt(SCRIPTINSTANCE_SERVER, 0);
  }

  level::gentity_t *entity = GetEntity_Impl(*entref);
  G_PlaySoundAlias_Impl(entity, alias_id, 0, 0);
}

void ScrCmd_PlaySoundOnTag_Impl(scriptInstance_t inst, scr_entref_t *entref) {

  int32_t tag_idx = 0;
  const char *alias;
  snd::SndAliasId alias_id;

  level::gentity_t *ent = GetEntity_Impl(*entref);
  ScrVarType first_param_type = Scr_GetType(SCRIPTINSTANCE_SERVER, 0);
  if (first_param_type == ScrVarType::STRING) {
    alias = Scr_GetString(SCRIPTINSTANCE_SERVER, 0);
    alias_id = snd::SND_FindAliasId(alias);
  } else {
    if (first_param_type != ScrVarType::INT) {
      Scr_ParamError(SCRIPTINSTANCE_SERVER, 0,
                     "Parameter is not a string or integer");
      return;
    }
    alias_id = Scr_GetInt(SCRIPTINSTANCE_SERVER, 0);
  }
  if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) >= 2) {
    ScrString_t tag_name =
        Scr_GetConstLowercaseString(SCRIPTINSTANCE_SERVER, 1u);
    if (tag_name == scr_const->tag_origin ||
        sv::SV_DObjGetBoneIndex(ent, tag_name) >= 0) {
      const char *tagName = Scr_GetString(SCRIPTINSTANCE_SERVER, 1u);
      tag_idx = G_TagIndex(tagName);
    } else {
      sv::SV_DObjDumpInfo(ent);
      const char *model_name = game::bg::BG_Cache_GetModelNameForIndex(
          game::bg::bgCacheInstance::SERVER, ent->model);
      const char *tag_name_display_str = sl::SL_ConvertToString(tag_name);
      const char *error_str =
          utils::string::va("tag '%s' does not exist on entity with model '%s'",
                            tag_name_display_str, model_name);
      Scr_ParamError(SCRIPTINSTANCE_SERVER, 1u, error_str);
    }
  }
  if (ent) {
    if (alias_id) {
      level::gentity_t *sound_ent =
          G_PlaySoundAlias_Impl(ent, alias_id, 0, tag_idx);
      if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) >= 3) {
        sound_ent->s.clientMask[0] = -1;
        // Should be ScrVarType::LOCALIZED_STRING - a conststring matched
        // internally in Scr_GetTeam to return the corresponding team_t
        // enumeration
        if (Scr_GetType(SCRIPTINSTANCE_SERVER, 2u) != ScrVarType::UNDEFINED) {
          game::team_t team = Scr_GetTeam(SCRIPTINSTANCE_SERVER, 2u);
          if (team != team_t::TEAM_FREE) {
            level::gentity_pool *ent_pool = gentity_pool();
            const uint32_t max_clients =
                static_cast<uint32_t>(Dvar_GetInt(*com_maxclients));
            for (uint32_t clientEntIdx = 0; clientEntIdx < max_clients;
                 clientEntIdx++) {
              level::gentity_t *mask_ent = &ent_pool->pool[clientEntIdx];
              if (mask_ent->r.inuse) {
                if (mask_ent->client->sess.cs.team == team) {
                  sound_ent->s.clientMask[mask_ent->s.number >> 5] &=
                      ~(1 << (mask_ent->s.number & 0x1F));
                }
              }
            }
          } else {
            sound_ent->s.clientMask[ent->s.number >> 5] &=
                ~(1 << (ent->s.number & 0x1F));
          }
        }
        if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) >= 4) {
          level::gentity_t *extraEnt = Scr_GetEntity_Impl(3u);
          sound_ent->s.clientMask[extraEnt->s.number >> 5] &=
              ~(1 << (extraEnt->s.number & 0x1F));
        }
      }
    }
  }
}
void ScrCmd_PlaySoundToAllButPlayer_Impl(scriptInstance_t inst,
                                         scr_entref_t *entref) {
  level::gentity_t *ent = Scr_GetEntity_Impl(1u);
  if (!ent->client) {
    const char *error_str =
        utils::string::va("entity %i is not a player", ent->s.number);
    Scr_ObjectError(SCRIPTINSTANCE_SERVER, error_str);
  }
  level::gentity_t *play_ent = GetEntity_Impl(*entref);
  const char *alias = Scr_GetString(SCRIPTINSTANCE_SERVER, 0);
  snd::SndAliasId alias_id = snd::SND_FindAliasId(alias);
  level::gentity_t *temp_ent = G_PlaySoundAlias_Impl(play_ent, alias_id, 0, 0);
  if (temp_ent) {
    temp_ent->s.clientMask[0] = 0;
    temp_ent->s.clientMask[ent->s.number >> 5] |= 1 << (ent->s.number & 0x1F);
  }
}
void ScrCmd_PlaySoundToPlayer_Impl(scriptInstance_t inst,
                                   scr_entref_t *entref) {
  level::gentity_t *client_ent = Scr_GetEntity_Impl(1u);
  if (!client_ent->client) {
    const char *error_str = utils::string::va(
        "client_entity %i is not a player", client_ent->s.number);
    Scr_ObjectError(SCRIPTINSTANCE_SERVER, error_str);
    return;
  }
  const char *alias;
  snd::SndAliasId alias_id;
  if (Scr_GetType(SCRIPTINSTANCE_SERVER, 0) == ScrVarType::STRING) {
    alias = Scr_GetString(SCRIPTINSTANCE_SERVER, 0);
    alias_id = snd::SND_FindAliasId(alias);
  } else {

    alias_id = Scr_GetInt(SCRIPTINSTANCE_SERVER, 0);
  }
  level::gentity_t *ent = GetEntity_Impl(*entref);
  level::gentity_t *temp_ent = G_PlaySoundAlias_Impl(ent, alias_id, 0, 0);
  if (temp_ent) {
    temp_ent->s.clientMask[0] = -1;
    temp_ent->s.clientMask[client_ent->s.number >> 5] &=
        ~(1 << (client_ent->s.number & 0x1F));
  }
}
void ScrCmd_PlaySoundToTeam_Impl(scriptInstance_t inst, scr_entref_t *entref) {
  if (game::is_server()) {
    pix::PIXBeginNamedEvent(-1, "ScrCmd_PlaySoundToTeam");
  }
  team_t team = Scr_GetTeam(SCRIPTINSTANCE_SERVER, 1u);
  level::gentity_t *exclude_ent;
  if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) >= 3) {
    level::gentity_t *ent = Scr_GetEntity_Impl(2u);
    exclude_ent = ent;
    if (!ent->client) {
      const char *error_str =
          utils::string::va("entity %i is not a player", ent->s.number);
      Scr_ObjectError(SCRIPTINSTANCE_SERVER, error_str);
    }
  } else {
    exclude_ent = nullptr;
  }
  const char *alias = Scr_GetString(SCRIPTINSTANCE_SERVER, 0);
  snd::SndAliasId alias_id = snd::SND_FindAliasId(alias);
  level::gentity_t *play_ent = GetEntity_Impl(*entref);
  level::gentity_t *temp_ent = G_PlaySoundAlias_Impl(play_ent, alias_id, 0, 0);
  if (temp_ent) {
    temp_ent->s.clientMask[0] = -1;
    level::gentity_pool *ent_pool = gentity_pool();
    const uint32_t max_clients =
        static_cast<uint32_t>(Dvar_GetInt(*com_maxclients));

    for (uint32_t clientEntIdx = 0; clientEntIdx < max_clients;
         clientEntIdx++) {
      level::gentity_t *mask_ent = &ent_pool->pool[clientEntIdx];
      if (mask_ent->r.inuse) {
        if (mask_ent != exclude_ent && mask_ent->client->sess.cs.team == team) {
          temp_ent->s.clientMask[mask_ent->s.number >> 5] &=
              ~(1 << (mask_ent->s.number & 0x1F));
        }
      }
    }
  }
  if (game::is_server()) {
    pix::PIXEndNamedEvent();
  }
}
void ScrCmd_PlaySoundWithNotify_Impl(scriptInstance_t inst,
                                     scr_entref_t *entref) {
  const char *alias;
  snd::SndAliasId alias_id = 0;
  ScrString_t notify_str = 0;
  level::gentity_t *ent = GetEntity_Impl(*entref);
  int32_t tag_idx = 0;
  if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER)) {
    alias = Scr_GetString(SCRIPTINSTANCE_SERVER, 0);
    alias_id = snd::SND_FindAliasId(alias);
  }
  if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) >= 2) {
    notify_str = Scr_GetConstString(SCRIPTINSTANCE_SERVER, 1u);
  }
  if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) >= 3) {
    ScrString_t tag_name_hash =
        Scr_GetConstLowercaseString(SCRIPTINSTANCE_SERVER, 2u);
    if (tag_name_hash == scr_const->tag_origin ||
        sv::SV_DObjGetBoneIndex(ent, tag_name_hash) >= 0) {
      const char *tag_name = Scr_GetString(SCRIPTINSTANCE_SERVER, 2u);
      tag_idx = G_TagIndex(tag_name);
    } else {
      sv::SV_DObjDumpInfo(ent);
      const char *model_name = game::bg::BG_Cache_GetModelNameForIndex(
          game::bg::bgCacheInstance::SERVER, ent->model);
      const char *tag_name = sl::SL_ConvertToString(tag_name_hash);
      const char *error_str =
          utils::string::va("tag '%s' does not exist on entity with model '%s'",
                            tag_name, model_name);
      Scr_ParamError(SCRIPTINSTANCE_SERVER, 2u, error_str);
    }
  }
  if (ent) {
    G_PlaySoundAliasWithNotify_Impl(ent, alias_id, notify_str, tag_idx);
  }
}
} // namespace cmd
} // namespace scr
} // namespace game