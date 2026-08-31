#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace com {

constexpr int32_t CHANGELIST_NUMBER_SV = 0x343723;
// Also matches changelist number in latest client.
constexpr int32_t CHANGELIST_NUMBER_CL = 0xD3FC12;

constexpr const char BUILD_MACHINE_VAL[] = "CODBUILD8-764";
constexpr const char BUILD_TYPE_VAL[] = "P4";

constexpr const char BUILD_TIME_CL[] = "Fri Feb 24 15:18:05 2023";
constexpr const char BUILD_TIME_SV[] = "Mon Dec 16 10:44:20 2019";
constexpr const char *LATEST_CLIENT_BUILD_TIME = "Wed Dec 10 17:10:46 2025";

#define Com_Error(code, fmt, ...)                                              \
  Com_Error_(__FILE__, __LINE__, code, fmt, ##__VA_ARGS__)

// Com
WEAK symbol<void()> Com_Init_Try_Block_Function{0x1421123B0, 0x140504170};
WEAK symbol<void(consoleChannel_e channel, consoleLabel_e label,
                 const char *fmt, ...)>
    Com_Printf{0x142148F60, 0x140505630};
WEAK symbol<void(const char *file, int line, errorParm code, const char *fmt,
                 ...)>
    Com_Error_{0x1420F8170, 0x140501470};
WEAK symbol<bool(eModes mode)> Com_SessionMode_IsMode{0x1420F7370, 0x140500940};

WEAK symbol<game::eModes()> _Com_SessionMode_GetMode{0x1420F6D30, 0x1405002D0};
// To allow for forward declaration for use in struct methods
inline eModes Com_SessionMode_GetMode() { return _Com_SessionMode_GetMode(); }
WEAK symbol<eGameModes()> Com_SessionMode_GetGameMode{0x1420F68B0, 0x1404FFE50};
WEAK symbol<eNetworkModes()> Com_SessionMode_GetNetworkMode{0x1420F6E00,
                                                            0x1405003A0};
WEAK symbol<void(eNetworkModes networkMode)> Com_SessionMode_SetNetworkMode{
    0x1420F75B0, 0x140500B80};
WEAK symbol<void(eGameModes mode)> Com_SessionMode_SetGameMode{0x1420F7570,
                                                               0x140500B40};
WEAK symbol<bool()> Com_IsRunningUILevel{0x142148350, 0x140504BD0};
WEAK symbol<bool()> Com_IsInGame{0x1421482C0, 0x140504B90};
WEAK symbol<void(int localClientNum, eModes fromMode, eModes toMode,
                 uint32_t flags)>
    Com_SwitchMode{0x14214A4D0};
WEAK symbol<const char *(const char *fullpath)> Com_LoadRawTextFile{
    0x1420F61B0};
WEAK symbol<void(const char *level)> Com_LoadLevelFastFiles{0x1421484E0,
                                                            0x140504C30};
WEAK symbol<void()> Com_UnloadLevelFastFiles{0x14214AAC0, 0x140506E40};
WEAK symbol<void()> Com_TryUnloadLevelFastFiles{0x14214BAC0, 0x140507CF0};

WEAK symbol<bool(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_IsBeingUsed{0x1420EF990};
WEAK symbol<game::ControllerIndex_t(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_GetControllerIndex{0x1420EF930};
WEAK symbol<game::net::netsrc_t(game::LocalClientNum_t localClientNum)>
    Com_LocalClient_GetNetworkID{0x1420EF950};
WEAK symbol<void *(live::LiveAllocTypes type, void *ptr, const size_t size)>
    Com_LiveRealloc{0x14214BE50};
WEAK symbol<void(live::LiveAllocTypes type, void *ptr)> Com_LiveDeallocate{
    0x14214BD80};
WEAK symbol<qboolean> com_errorEntered{0x1568ED8B8};
WEAK symbol<const char *()> Com_SessionMode_GetAbbreviationForCurrentMode{
    0x1420F6790};
WEAK symbol<LocalClientNum_t(ControllerIndex_t controllerIndex)>
    Com_ControllerIndex_GetLocalClientNum{0x1420EF7C0, 0x1404FACB0};
WEAK symbol<void()> Com_Frame_Try_Block_Function{0x1420F8E00, 0x1405020E0};
WEAK symbol<ControllerIndex_t()> Com_ControllerIndexes_GetPrimary{0x1420EF880,
                                                                  0x1404FAD70};
WEAK symbol<void(const char *inString, char *outString, int32_t outStringSize)>
    Com_CleanStringForNetwork{0x1422E8C30, 0x140580A60};
WEAK symbol<void()> Com_ShutdownUILevelAndReinitialize{0x142149D60};
WEAK symbol<void()> Com_ShutdownUILevel{0x142149D10};

// Unsure of exact name. This function does not exist in any prior engine
// versions, nor in the dedicated server. It's return value is compared to
// com_maxfps and the minimum of the two is used as the FPS limit.
WEAK symbol<uint32_t(LocalClientNum_t localClientNum)> Com_FPSLimit{
    0x140F7CFD0};

WEAK symbol<void()> ParseBuildInfo{0x1420EF5F0, 0x1404FAAE0};
WEAK symbol<const char *()> Com_GetBuildConfig{0x1420EF270, 0x1404FA7D0};
WEAK symbol<const char *()> Com_GetBuildDisplayNameR{0x1420EF280, 0x1404FA7E0};
WEAK symbol<const char *()> Com_GetBuildID{0x1420EF290, 0x1404FA7F0};
// Does not exist in server
WEAK symbol<const char *()> Com_GetBuildInfoName{0x1420EF2E0};
WEAK symbol<const char *()> Com_GetBuildInfoString{0x1420EF300, 0x1404FA840};
// Does not exist in server
WEAK symbol<int32_t()> Com_GetBuildInfoVersion{0x1420EF370};
WEAK symbol<const char *()> Com_GetBuildMachine{0x1420EF3D0, 0x1404FA8F0};
WEAK symbol<const char *()> Com_GetBuildName{0x1420EF3E0, 0x1404FA900};
WEAK symbol<const char *()> Com_GetBuildTime{0x1420EF530, 0x1404FAA40};
WEAK symbol<const char *()> Com_GetBuildVersion{0x1420EF550, 0x1404FAA50};

WEAK symbol<int32_t> s_buildInfoVersion{0x1568CE6E0, 0x14946FFA0};
WEAK symbol<bool> s_buildInfoParsed{0x1568CE6E4, 0x14946FFA4};
WEAK symbol<bool> s_buildInfoValid{0x1568CE6E5, 0x14946FFA5};
WEAK symbol<str128_t> s_buildInfoName{0x1568CE6F0, 0x14946FFB0};
WEAK symbol<str64_t> s_buildInfoSource{0x1568CE770, 0x149470030};

WEAK symbol<const char *> BUILD_MACHINE{0x14337B220, 0x14110B3D0};
WEAK symbol<const char *> BUILD_TYPE{0x14337B228, 0x14110B3D8};
WEAK symbol<const char *> BUILD_TIME{0x14337B230, 0x14337B230};

WEAK symbol<int32_t(BuildIntField field)> Com_GetBuildIntField{0x1420EF390,
                                                               0x1404FA8B0};
WEAK symbol<const char *(BuildStringField field)> Com_GetBuildStringField{
    0x1420EF400, 0x1404FA910};

} // namespace com
} // namespace game