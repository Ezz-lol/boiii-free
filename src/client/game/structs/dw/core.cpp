#include <std_include.hpp>

namespace game {
namespace dw {

template <typename T> std::string _TaskRecord<T>::serialize() const noexcept {
  return std::format(
      "{{ \"next\": {}, \"definition\": {}, \"state\": {}, "
      "\"controllerIndex\": {}, "
      "\"lastPoll\": {:08X}, \"lastPollMS\": {}, \"id\": {:08X}, \"group\": "
      "{}, "
      "\"startMS\": {}, \"timeOut\": {}, \"timeIn\": {}, \"remoteTask\": {}, "
      "\"eventInfo\": {}, \"nestedTask\": {}, \"payload\": {}, "
      "\"isChildTask\": {}, "
      "\"cancelImmediately\": {}, \"skipAllCallbacksAfterComplete\": {} }}",
      ::serialize(next), ::serialize(definition), game::dw::serialize(state),
      game::serialize(controllerIndex), lastPoll, lastPollMS, id,
      game::dw::serialize(group), startMS, timeOut, timeIn,
      remoteTask.serialize(), eventInfo.serialize(), ::serialize(nestedTask),
      ::serialize(payload), ::serialize(isChildTask),
      ::serialize(cancelImmediately),
      ::serialize(skipAllCallbacksAfterComplete));
}

} // namespace dw
} // namespace game
