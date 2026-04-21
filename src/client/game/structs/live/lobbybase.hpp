#pragma once

#include <cstdint>
namespace game {
namespace live {
enum class AddLocalClientResult : int32_t {
  ADD_LOCAL_CLIENT_RESULT_SUCCESS = 0,
  ADD_LOCAL_CLIENT_RESULT_NO_SIGN_IN = 1,
  ADD_LOCAL_CLIENT_RESULT_NO_FILES_LOCAL = 2,
  ADD_LOCAL_CLIENT_RESULT_NO_FILES_LIVE = 3,
  ADD_LOCAL_CLIENT_RESULT_FAILED = 4

};

} // namespace live
} // namespace game