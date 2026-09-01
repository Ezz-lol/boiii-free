#include <std_include.hpp>

/*
  When compiling with MSBuild + clang-cl,
  there is a very unexpected bug where all objects with base name "com.cpp"
  have their temporary files cleaned up when one requires removal.

  This results in erroneous build failure due to a failure to move a temporary
  object generated from one of these "com.cpp" objects to its final ".o" output,
  due to its prior, erroneous removal.

  As such, we have to rename this header to some other name than "com.cpp" to
  circumvent this bug. "g_com.cpp" was selected to specify that this header
  pertains to the game's `COM_`-prefixed functions - not Windows COM.

  Note: this bug does not occur when compiling with gmake + clang.
*/

#include "com.hpp"

namespace game {
namespace com {

int32_t Com_GetBuildIntField_Impl(BuildIntField field) {

  switch (field) {
  case BuildIntField::CHANGELIST:
    return CHANGELIST_NUMBER_CL;
  case BuildIntField::INFO_VERSION:
    ParseBuildInfo();
    return *s_buildInfoVersion;
  case BuildIntField::BUILD_NUMBER:
  default:
    return 0;
  }
}

static str128_t build_version = {0};
static std::once_flag set_build_version_flag;
void set_build_version() {
  snprintf(build_version, std::size(build_version), "CL(%d) %s %s [%s]",
           CHANGELIST_NUMBER_CL, BUILD_MACHINE_VAL, BUILD_TYPE_VAL,
           // ORIGINAL:
           // *BUILD_TIME
           // PATCH: spoof latest version
           LATEST_CLIENT_BUILD_TIME);
}

const char *Com_GetBuildVersion_Impl() {
  std::call_once(set_build_version_flag, set_build_version);
  return build_version;
}

static str128_t build_id = {0};
static std::once_flag set_build_id_flag;
void set_build_id() {
  snprintf(build_id, std::size(build_id), "%s CL(%d)", BUILD_MACHINE_VAL,
           CHANGELIST_NUMBER_CL);
}

const char *Com_GetBuildID_Impl() {
  std::call_once(set_build_id_flag, set_build_id);
  return build_id;
}

const char *Com_GetBuildStringField_Impl(BuildStringField field) {

  switch (field) {
  case BuildStringField::MAJOR_VERSION:
    return "1";
  case BuildStringField::MINOR_VERSION:
    return "0";
  case BuildStringField::BUILD_ID:
    return Com_GetBuildID_Impl();
  case BuildStringField::BUILD_VERSION:
    return Com_GetBuildVersion_Impl();
  case BuildStringField::BUILD_MACHINE:
    return BUILD_MACHINE_VAL;
  case BuildStringField::BUILD_TYPE:
    return BUILD_TYPE_VAL;
  case BuildStringField::BUILD_TIME:
    // ORIGINAL - "Mon Dec 16 10:44:20 2019":
    // return *BUILD_TIME;
    // PATCH: spoof latest version
    return LATEST_CLIENT_BUILD_TIME;
  case BuildStringField::BUILD_DISPLAY_NAME:
    return "Call of Duty  - ship";
  case BuildStringField::BUILD_NAME:
    return "COD_T7_S ";
  case BuildStringField::BUILD_BASE_NAME:
    return "COD_T7";
  case BuildStringField::BUILD_CONFIG:
    return "ship";
  case BuildStringField::BUILD_PROJECT_NAME:
    return "t7";
  case BuildStringField::BUILD_INFO_NAME:
    ParseBuildInfo();
    return *s_buildInfoName;
  case BuildStringField::BUILD_INFO_SOURCE:
    ParseBuildInfo();
    return *s_buildInfoSource;
  case BuildStringField::BUILD_INFO_STRING:
    return Com_GetBuildInfoString();
  default:
    return "";
  }
}
} // namespace com
} // namespace game