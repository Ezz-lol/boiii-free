#include <std_include.hpp>
#include "game.hpp"
#include "../snd/snd.hpp"

#include "../../../../common/utils/string.hpp"

namespace game {

constexpr int32_t TEMP_ENTITY_SOUND_EVENT = 0b100;
constexpr int32_t TEMP_ENTITY_SOUND_NOTIFY_EVENT =
    TEMP_ENTITY_SOUND_EVENT | 0b001;
void G_RegisterSoundWait_Impl(level::gentity_s *ent, snd::SndAliasId id,
                              scr::ScrString_t notifyString) {

  scr::ScrString_t tempNotifyString;
  scr::Scr_SetString(&tempNotifyString, notifyString);

  scr::ScrString_t currentPlaybackNotifyString = ent->snd_wait.notifyString;
  if (currentPlaybackNotifyString) {
    scr::Scr_Notify_ScrString(ent, currentPlaybackNotifyString, 0);
    if (!ent->snd_wait.stoppable) {
      scr::Scr_SetString(&tempNotifyString, 0);
      scr::ScrString_t targetname = ent->targetname;

      const char *targetDisplayStr;
      if (targetname) {
        targetDisplayStr = sl::SL_ConvertToString(targetname);
      } else {
        targetDisplayStr = "<undefined>";
      }
      const char *tempNotifyDisplayStr =
          sl::SL_ConvertToString(tempNotifyString);
      const char *notifyDisplayStr =
          sl::SL_ConvertToString(ent->snd_wait.notifyString);
      double z = ent->r.currentOrigin.z;
      double y = ent->r.currentOrigin.y;
      double x = ent->r.currentOrigin.x;
      const char *targetClassNameStr = sl::SL_ConvertToString(ent->classname);
      const char *errorStr = utils::string::va(
          "issued a second playsound with notification string before the "
          "first finished on entity %i classname %s tar"
          "getname %s location %g %g %g old string %s at time %i new "
          "string %s at time %i\n",
          ent->s.number, targetClassNameStr, targetDisplayStr, x, y, z,
          notifyDisplayStr, ent->snd_wait.basetime, tempNotifyDisplayStr,
          *level::level_time);
      scr::Scr_Error(scr::SCRIPTINSTANCE_SERVER, errorStr, 0);
    }
  }
  scr::Scr_SetString(&ent->snd_wait.notifyString, tempNotifyString);
  scr::Scr_SetString(&tempNotifyString, 0);
  ent->snd_wait.index = static_cast<uint32_t>(id);
  ent->snd_wait.basetime = sv::svs->time;

  // ORIGINAL:
  //  ent->snd_wait.duration = -1;
  // PATCH to use actual duration:
  ent->snd_wait.duration = snd::SND_GetPlaybackTimeById(id);

  ent->snd_wait.stoppable = 1;
}

level::gentity_t *G_PlaySoundAliasAtPoint_Impl(const vec3_t *origin,
                                               snd::SndAliasId alias) {
  level::gentity_t *tmp = 0;
  if (alias) {
    tmp = G_TempEntity(origin, TEMP_ENTITY_SOUND_EVENT);
    tmp->r.svFlags |= 8u;
    tmp->s.loopSound.id = alias;
    tmp->s.otherEntityNum = 1022;
  }
  return tmp;
}

level::gentity_t *G_PlaySoundAliasWithNotify_Impl(level::gentity_t *ent,
                                                  snd::SndAliasId alias,
                                                  scr::ScrString_t notifyString,
                                                  uint32_t tag) {

  level::gentity_t *tmp = nullptr;
  if (alias) {
    int32_t event =
        notifyString ? TEMP_ENTITY_SOUND_NOTIFY_EVENT : TEMP_ENTITY_SOUND_EVENT;
    tmp = G_TempEntity(&ent->r.currentOrigin, event);
    tmp->s.loopSound.id = alias;
    tmp->s.un3.hintString = tag;
    tmp->s.otherEntityNum = ent->s.number;
  }
  if (notifyString) {
    G_RegisterSoundWait_Impl(ent, alias, notifyString);
  }
  return tmp;
}

level::gentity_t *G_PlaySoundAlias_Impl(level::gentity_t *ent,
                                        snd::SndAliasId alias,
                                        scr::ScrString_t notifyString,
                                        BoneIndex bone) {
  level::gentity_t *tmp = nullptr;

  if (alias) {
    tmp = G_TempEntity(&ent->r.currentOrigin, 4);
    tmp->r.svFlags |= 8u;
    if (ent->s.number < 0x400)
      ent->r.svFlags &= ~1u;
    tmp->s.loopSound.id = alias;
    tmp->s.otherEntityNum = ent->s.number;
    tmp->s.un3.hintString = bone;
  }
  return tmp;
}
} // namespace game