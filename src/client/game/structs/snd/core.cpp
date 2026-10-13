#include <std_include.hpp>

#include "core.hpp"

namespace game {
namespace snd {

/*
  Likely because of some kind of recursive `serialize` dependency, inlining
  this method definition in the header causes an error in its alias list
  serialization - `::serialize(alias, aliasCount)` - where `Serializable` is not
  satisfied because `serialize` does not exist on an incomplete `SndAliasList`
  type definition. `SndAliasList` is defined in the same header, before SndBank
  is defined, so this must be caused by a `struct SndAliasList;` declaration in
  a dependency included by either this object's header or one of the headers it
  includes.

  Regardless, this issue is resolved by defining `SndBank`s serialize method in
  an object instead of in a header.
*/
std::string SndBank::serialize() const noexcept {
  return std::format(
      "{{ \"name\": {}, \"zone\": {}, \"gameLanguage\": {}, "
      "\"soundLanguage\": {}, \"aliasCount\": {}, "
      "\"alias\": {}, \"aliasIndex\": {}, "
      "\"reverbCount\": {}, \"reverbs\": {}, "
      "\"duckCount\": {}, \"ducks\": {}, \"ambientCount\": "
      "{}, \"ambients\": {}, "
      "\"scriptIdLookupCount\": {}, \"scriptIdLookups\": "
      "{}, \"ambientBsp\": {}, "
      "\"musicSetCount\": {}, \"musicSets\": {} }}",
      ::serialize(name), ::serialize(zone), ::serialize(gameLanguage),
      ::serialize(soundLanguage), aliasCount, ::serialize(alias, aliasCount),
      ::serialize(aliasIndex, aliasCount), reverbCount,
      ::serialize(reverbs, reverbCount), duckCount,
      ::serialize(ducks, duckCount), ambientCount,
      ::serialize(ambients, ambientCount), scriptIdLookupCount,
      ::serialize(scriptIdLookups, scriptIdLookupCount), ambientBsp.serialize(),
      musicSetCount, ::serialize(musicSets, musicSetCount));
}

std::string SndDuckActive::serialize() const noexcept {
  return std::format(
      "{{ \"duck\": {}, \"voice\": {}, \"category\": {}, \"position\": "
      "{}, "
      "\"inUse\": {}, \"startTime\": {}, \"endTime\": {}, \"pingTime\": {}, "
      "\"id\": 0x{:08X}, \"amount\": {}, \"effect\": {}, \"amplitude\": {} "
      "}}",
      ::serialize(duck), ::serialize(voice), game::snd::serialize(category),
      position.serialize(), inUse.serialize(), startTime, endTime, pingTime, id,
      amount, effect, amplitude);
}
} // namespace snd
} // namespace game
