#include <std_include.hpp>

#include <game/impl/com/com.hpp>

namespace game {
namespace com {
int32_t Com_GetBuildIntField_Impl(BuildIntField field) {

  switch (field) {
  case BuildIntField::CHANGELIST:
    /*
       Note: this also matches the changelist version used in the
       Dec 10, 2025 client version. It seems to have been unchanged in that
       version.
    */
    return 0xD3FC12;
  case BuildIntField::INFO_VERSION:
    ParseBuildInfo();
    return *s_buildInfoVersion;
  case BuildIntField::BUILD_NUMBER:
  default:
    return 0;
  }
}

constexpr const char *LATEST_CLIENT_BUILD_TIME = "Wed Dec 10 17:10:46 2025";

const char *Com_GetBuildVersion_Impl() {
  static str128_t buf = {0};
  snprintf(buf, std::size(buf), "CL(%d) %s %s [%s]", 3421987, *BUILD_MACHINE,
           *BUILD_TYPE,
           // ORIGINAL:
           // *BUILD_TIME
           // PATCH: spoof latest version
           LATEST_CLIENT_BUILD_TIME);
  return buf;
}

const char *Com_GetBuildStringField_Impl(BuildStringField field) {

  switch (field) {
  case BuildStringField::MAJOR_VERSION:
    return "1";
  case BuildStringField::MINOR_VERSION:
    return "0";
  case BuildStringField::BUILD_ID:
    // Resolves to "CODBUILD8-764 CL(3421987)"
    return Com_GetBuildID();
  case BuildStringField::BUILD_VERSION:
    // Resolves to:
    // "CL(3421987) CODBUILD8-764 P4 Mon Dec 16 10:44:20 2019"
    return Com_GetBuildVersion_Impl();
  case BuildStringField::BUILD_MACHINE:
    // "CODBUILD8-764"
    return *BUILD_MACHINE;
  case BuildStringField::BUILD_TYPE:
    // P4
    return *BUILD_TYPE;
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