#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace cod {
namespace api {
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AbandonDailyChallenge{
    0x141FD6670, 0x1404CE0A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AcceptGroupInvite{0x141F35AF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AcceptingInvite{0x141FC3ED0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AcceptSelectedGroupApplication{
    0x141F35B40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AccountLinkLogin{0x141FC3F10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AccountLinkRegister{
    0x141FC3FD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ActionSlotSelectWeapon{
    0x141F38ED0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ActivatePrimaryLocalClient{
    0x141F13AF0, 0x1404C00D0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_AddFriendToInitialGroupInvitesList{0x141F35BA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AddKeyCatcherSubscriber{
    0x141FE1270};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AddLobbyClientExtracam{
    0x141FB0340};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AddTextMessageRecipient{
    0x141FC4210};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AddThumbnail{0x141FC4220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AddToErrorString{0x141FB0360,
                                                                 0x1404C8A50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AdvertiseErrorShutdown{
    0x141F1C760};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AdvertiseLobby{0x141F1C7B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AllowVoiceChatForAll{
    0x141F17EF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AllowVoiceChatForClient{
    0x141F17F30};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_AnyAttachmentRestrictionStateChanged{0x141FB03E0,
                                                         0x1404C8AD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AnySignedInToLive{0x141FE12C0,
                                                                  0x1404D5B10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreAllChunksInstalledByGameType{
    0x141FC42F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreAllItemsFree{0x141FD6730};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreAllItemsUnlocked{
    0x141FD6770, 0x1404CE160};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_AreAnyCharacterCustomizationItemsNew{0x141FD67B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreAnyNonOpticAttachmentsNew{
    0x141FD6850, 0x1404CE1A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreAnyOpticsNew{0x141FD6920,
                                                                0x1404CE270};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreAttachmentsCompatible{
    0x141FD69F0, 0x1404CE340};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_AreAttachmentsCompatibleByAttachmentID{0x141FD6B00,
                                                           0x1404CE450};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreExtraSlotsPurchased{
    0x141FD6B80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreGametypeSettingsDirty{
    0x141FB0420, 0x1404C8B10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreLocalFilesReady{0x141FE1330,
                                                                   0x1404D5B80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_AreStatsFetched{0x141FE1390,
                                                                0x1404D5BE0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_AttachmentRestrictionStateChangedForAnyWeapon{0x141FB0460,
                                                                  0x1404C8B50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BanGroupMember{0x141F35C20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_bdSecurityIDToString{
    0x0, 0x1404C3C30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BdSecurityIDToString{
    0x141F17E40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BeginEditingPresetClass{
    0x141FB04C0, 0x1404C8BB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BeginSendTextMessage{
    0x141FC4310};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BindCommand{0x141FE1430};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BlurWorld{0x141FE1500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BookmarkRecentGame{0x141FC4320};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BuildObjectivesTable{
    0x141FD57F0, 0x1404CD270};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BurnLootDuplicates{0x141F20450};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_BuyLootCrate{0x141F204E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CalculateSkillRange{
    0x141FC4390};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanDailyChallengeBeAbandoned{
    0x141F13B30, 0x1404C0110};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanHostServer{0x141F13CD0,
                                                              0x1404C02C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanLivestream{0x141FC44F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanMergeSegments{0x141F12A70,
                                                                 0x1404BF270};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanPauseGame{0x141FC4510};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanRenderClip{0x141FB0500,
                                                              0x1404C8BF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanRestartMissions{0x141FC4550};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanSendGroupInvites{
    0x141F35C90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanSwitchToLobby{0x141FB0550};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanViewContent{0x141FC4590};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CanViewContentFromUser{
    0x141FC45D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChangeHistory{0x141FC4620};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_CharacterCustomizationNewItemCount{0x141FD6BE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CharacterLoadoutDrafted{
    0x141F17F80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_ChannelColorGet{
    0x141FC4630};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_ChannelColorSet{
    0x141FC4750};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_ChannelStringGet{
    0x141FC4840};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_EntriesCount{
    0x141FC48B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_FilterChannelGet{
    0x141FC4950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_FilterChannelSet{
    0x141FC49E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_GetEntries{
    0x141FC4A70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_InputChannelGet{
    0x141FC4B50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_InputChannelSet{
    0x141FC4BA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_InputStart{
    0x141FC4C00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_IsAvailable{
    0x141FC4C40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_IsChatting{
    0x141FC4CA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_ResetState{
    0x141FC4D00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_ShowChannelGet{
    0x141FC4D40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_ShowChannelSet{
    0x141FC4DA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_ShowTimestampsGet{
    0x141FC4E10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChatClient_ShowTimestampsSet{
    0x141FC4E70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CheckAccountNameForEliteAccount{
    0x141FC4EE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CheckDoubleXPGroup{0x141F13D70,
                                                                   0x1404C0360};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CheckDownloadStatusByOfferID{
    0x141F35540};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CheckGCCatchUp{0x141FE1560};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CheckNameForEliteAccount{
    0x141FC4F70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CheckNetConnection{0x141FE15F0,
                                                                   0x1404D5C80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CheckPasswordForEliteAccount{
    0x141FC4F90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CheckPSPlus{0x141FD1DC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ChoosePreferredDownloadOrder{
    0x141FD1DF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearBunkCollectible{
    0x141FD58D0, 0x1404CD350};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearError{0x141FE1630,
                                                           0x1404D5CC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearErrorString{0x141FB0570,
                                                                 0x1404C8C40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearLastGroupSearchResults{
    0x141F35D90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearLocalPSCharacter{
    0x141F1D880, 0x1404C6530};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearMapVoteData{0x141F1D220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearObjectiveIcon{0x141F3A7D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearReadyUpData{0x141F1D240};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearTeamData{0x141F1FF10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClearUI3DWindow{0x141FE1640,
                                                                0x1404D5CD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ClientHasCollectible{
    0x141FD5950, 0x1404CD3D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CloseScoreboard{0x141F3C0C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ComError{0x141F13E20,
                                                         0x1404C0410};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CommitProfileChanges{
    0x141FE1680, 0x1404D5D10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ConsumableDecrement{
    0x141FC4FC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ConsumableGet{0x141FC50C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ConsumableIncrement{
    0x141FC5190};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ConsumableSet{0x141FC5290};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CookGobbleGumRecipe{
    0x141F21300};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CopyFromClassSet{0x141FD6CD0,
                                                                 0x1404CE4D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CopyFromClassSetToClassSet{
    0x141FD6D60, 0x1404CE560};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CopyToClassSet{0x141FD6E20,
                                                               0x1404CE620};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CreateGroup{0x141F35DE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CreateModel{0x141FAF920};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CurrentGameTime{0x141F3C120};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CurrentRunningTaskCount{
    0x141FE16E0, 0x1404D5D70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CurrentSessionMode{0x141FE1720,
                                                                   0x1404D5DB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_CurrentTime{0x141FE1760,
                                                            0x1404D5DF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DeactivateAllLocalClients{
    0x141F13EA0, 0x1404C0490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DefaultID64Value{0x141FE17A0,
                                                                 0x1404D5E30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DeleteGroupLeaderboard{
    0x141F35EB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DeleteTextMessage{0x141FC5390};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DemoteGroupAdminToRegularMember{
    0x141F35F00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DestroyScreenshot{0x141FC53A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DevOnlyClearProbation{
    0x141FC53E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DisableSceneFilter{0x141F3D9C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DisplayChatRestrictionForUser{
    0x141FD1E50};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_DisplayContentRestrictionForUser{0x141FD1E60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DisplayNpAvailabilityErrors{
    0x141FD1E70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DisplayNpError{0x141FD1E90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DisplayUpsellAndContentWarnings{
    0x141FD1EB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DoesKillstreakRewardExist{
    0x141F3F270};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DoesPlayerHaveInventoryWeapon{
    0x141F40BF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DoesPlayerHaveKillstreakAmmo{
    0x141F42530};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DoesPollExist{0x141FC53F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DownloadGameSettings{
    0x141FC54B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DownloadScreenshot{0x141FC5530};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_Durango_CheckPrimaryStolenPopupAfterLoading{0x141FE17C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Durango_ContinueAs{0x141FE17D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Durango_TryToSetXUIDToGPad{
    0x141FE17E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DvarBool{0x141FE17F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DvarFloat{0x141FE18C0,
                                                          0x1404D5F20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DvarInt{0x141FE1980};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_DvarString{0x141FE1A40,
                                                           0x1404D60A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ECACImport_PlayerDecision{
    0x141FC55E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ECACImport_ShouldShow{
    0x141FC55F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ECACImport_ValidateDecision{
    0x141FC5610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ELaunchAppExec{0x141FC5620};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ELaunchAppSearch{0x141FC5630};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ELaunchAppStore{0x141FC5640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EmblemBackgroundIsLocked{
    0x141FB0580};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EmblemFilterCount{0x141FB0610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EmblemFilterIconID{0x141FB06C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EmblemIconIsLocked{0x141FB07A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EnablePresenceCacheAutoupdate{
    0x141FD3100};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EnableQuickJoinCacheAutoupdate{
    0x141FD3170};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EnableSceneFilter{0x141F43E50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EncodePlaylistNumber{
    0x141F1E0F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EndSendTextMessage{0x141FC56B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EntitlementExists{0x141F35560};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EquipBubbleGumPack{0x141FD6EB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EquipCustomClassToSession{
    0x141FC56C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EquipCybercore{0x141FD6F40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EReg_PlayerRefusedOrError{
    0x141FC5680};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ERegPopup_ShouldShow{
    0x141FC5650};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ERegPopup_SubmitRegistration{
    0x141FC5670};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_EWelcomePopup_ShouldShow{
    0x141FC5690};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Exec{0x141FE1B10, 0x1404D6160};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ExecAutoEventRuleSet{
    0x141FC56D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ExecNow{0x141FE1BB0,
                                                        0x1404D6200};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ExperimentsGetVariant{
    0x141F205A0, 0x1404C81E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FacebookPost{0x141FC5710};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FavoriteFileInGroupsShowcase{
    0x141F35F70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FeaturedCardMarkRead{
    0x141FB0870};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FeatureFileInGroupsShowcase{
    0x141F35FE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FetchGroupInviteProfiles{
    0x141F36050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FetchGroupInvites{0x141F36090};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_FetchGroupJoinApprovalTypeByGroupId{0x141F360D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FetchGroups{0x141F361A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FetchingGroupPublicProfiles{
    0x141F36290};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FetchOutgoingGroupInvites{
    0x141F361E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FetchSelectedGroupAdmins{
    0x141F36230};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_FetchSelectedGroupJoinApprovalType{0x141F36250};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FetchStoreProductsByCategory{
    0x141FC5730};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileShareCopy{0x141FC57D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileShareDelete{0x141FC5890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareDownload{0x141FC5950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareDownloadSummary{
    0x141FC5A20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareFetchCommunityContent{
    0x141FC5AE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareFetchForUser{
    0x141FC5BE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareFetchGroupContent{
    0x141FC5C30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareGetCommunitySlotData{
    0x141FC5D20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareGetQuota{0x141FC5E10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareGetSlotCount{
    0x141FC5E80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareGetSlotData{
    0x141FC5F60};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_FileshareGetTotalUsedCommunitySlotCount{0x141FC6050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareIsReady{0x141FC6130};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_FileshareIsUIIndexBeyondThreshold{0x141FC6190};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareProcessUIIndex{
    0x141FC61F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareReportView{
    0x141FC6370};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareReportVote{
    0x141FC6410};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FileshareStartup{0x141FC6570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FilterFriendsInGroup{
    0x141F362D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FlushKVS{0x141F13EE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ForceHUDRefresh{0x141F45740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ForceLobbyUIScreen{0x141F13F50,
                                                                   0x1404C04C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ForceNotifyModelSubscriptions{
    0x141FAFA10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FormatNumberWithCommas{
    0x141FE1C40, 0x1404D6290};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FreeTheaterMemoryIfAllocated{
    0x141FC65C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_FriendsIsTaskInProgress{
    0x141FE1CD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GadgetHasTakeTwoAttachment{
    0x141FD6FE0, 0x1404CE6B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameHost{0x141FB08C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameLobbyChooseNextPlaylist{
    0x141F1E140};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameLobbyChoosePreviousPlaylist{
    0x141F1E1B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameLobbyChooseRandomPlaylist{
    0x141F1E220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameLobbyGetGameType{
    0x141F13F90, 0x1404C0500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameLobbyGetMap{0x141F13FF0,
                                                                0x1404C0560};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameLobbyGetMapVote{
    0x141F1D280};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameLobbySetGameType{
    0x141F14050, 0x1404C05C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameLobbySetMap{0x141F14090,
                                                                0x1404C0600};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameModeIsMode{0x141FC65E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameModeResetModes{0x141FC6640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameModeSetMode{0x141FC6660};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedController{
    0x141FE1D90, 0x1404D63A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedCount{
    0x141FE1DE0, 0x1404D63F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedIsActive{
    0x141FE1E20, 0x1404D6430};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedMap{
    0x141FE1E80, 0x1404D6490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedMapAny{
    0x141FE1EE0, 0x1404D64F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedPort{
    0x141FE1F30, 0x1404D6540};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedPortMapping{
    0x141FE1F80, 0x1404D6590};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedUnMap{
    0x141FE1FB0, 0x1404D65C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadsConnectedValidPort{
    0x141FE1FF0, 0x1404D6600};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamepadType{0x141FE1D10,
                                                            0x1404D6320};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GamertagsToXuids{0x141F140D0,
                                                                 0x1404C0640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GameTimeRemaining{0x141F47030};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GametypeDescription{
    0x141FB08E0, 0x1404C8C50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAARLevelAlias{0x141FB2190,
                                                                 0x1404D6B30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAARScoreboard{0x141FB21B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAbbreviationForMode{
    0x141FE2050, 0x1404D6660};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetActiveLocalClientsCount{
    0x141FB2350, 0x1404C8CC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAdjustedWinPercentage{
    0x141FC6670};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAllResolutions{0x141FD1ED0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAmmoForGadget{0x141F4A1D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAnticheatReputation{
    0x141FC6770};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetArenaInfo{0x141FC6860};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetArenaPoints{0x141FC68F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetArenaSeasonStartTime{
    0x141FC6980};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAspectRatio{0x141FE20C0,
                                                               0x1404D66D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAssetInfo{0x141FE20F0,
                                                             0x1404D6700};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAssetInfoFromListIndex{
    0x141FE2180, 0x1404D6790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAssetList{0x141FE2250,
                                                             0x1404D6860};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentAllocationCost{
    0x141FD7080, 0x1404CE750};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentAttachPoint{
    0x141FD7180, 0x1404CE850};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentCosmeticVariant{
    0x141FD7260};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetAttachmentCosmeticVariantCountForAttachment{0x141FD7380};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetAttachmentCosmeticVariantImage{0x141F20630, 0x1404C8270};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetAttachmentCosmeticVariantName{0x141F20720, 0x1404C8360};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentDesc{0x141FD74A0,
                                                                  0x1404CE930};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetAttachmentDescByAttachmentIndex{0x141FD7580,
                                                       0x1404CEA10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentGroupVote{
    0x141F18020};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentImage{0x141FD75F0,
                                                                   0x1404CEAF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentImageByIndex{
    0x141FD76D0, 0x1404CEA80};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetAttachmentImageByWeaponAndAttachIndex{0x141FD7740};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetAttachmentIndexByAttachmentTableIndex{0x141FD7820,
                                                             0x1404CEBD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentIndexByRef{
    0x0, 0x1404CECA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentName{0x141FD7940,
                                                                  0x1404CECF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentNameByIndex{
    0x141FD7A20, 0x1404CEDD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentRef{0x141FD7AA0,
                                                                 0x1404CEE50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentRefByIndex{
    0x141FD7B80, 0x1404CEF30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentRestrictionState{
    0x141FB2390, 0x1404C8D00};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetAttachmentUniqueImageByAttachmentIndex{0x141FD7C00,
                                                              0x1404CEFB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentUniqueImageByIndex{
    0x141FD7CC0, 0x1404CF070};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentUnlockString{
    0x141FB2440};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAttachmentVote{0x141F180B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAvailableRefreshRates{
    0x141FD1F90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetAvailableResolutions{
    0x141FD20B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBackgroundsForCategoryName{
    0x141FD7EF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetbdServerTimestampStr{
    0x141FCC050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBGBTokensGained{0x141FD7DA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBGBTokensRemaining{
    0x141FD7E00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBGBTokensUsed{0x141FD7E90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBindingButtonString{
    0x141FE2340, 0x1404D6950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBubbleGumBuff{0x141FD8080};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBubbleGumPackName{
    0x141FD8150};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBuildIntField{0x141FC69D0,
                                                                 0x1404CB540};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBuildStringField{
    0x141FC6A20, 0x1404CB590};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBurnDupeProgress{
    0x141F207F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBurnDupeState{0x141F20850};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetBusyFriendsOfAllLocalPlayers{
    0x141FC6A90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCACRoot{0x141FC6AA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCalloutPlayerData{
    0x141F4BAE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCamoUnlockString{
    0x141FB2460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallengeComplete{
    0x141FB2480};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetChallengeCompleteForChallengeIndex{0x141FD8210};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallengeForBackground{
    0x141FC6B20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallengeForImage{
    0x141FC6C60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallengeForItemOption{
    0x141FD8310, 0x1404CF150};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallengeHasBeenSeen{
    0x141FD5BC0, 0x1404CD640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallengeInfoByBackingId{
    0x141FD8450};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetChallengeInfoByEmblemOrBackingId{0x141FD84C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallengeInfoForImages{
    0x141FD85A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallenges{0x141FC6E40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChallengeUnlockEmblemInfo{
    0x141FC6DA0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetChallengeValueForChallengeIndex{0x141FD8670};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetCharacterCustomizationItemUnlockInfo{0x141FD8760};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChunkStatusForMainMode{
    0x141FD2180};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetChunkStatusForMode{
    0x141FD21E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCinematicTimeRemaining{
    0x141F4D550};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClanName{0x141FB24A0,
                                                            0x1404C8DB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClanTag{0x141F4D5A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClanTagFeatureCount{
    0x141FD8880, 0x1404CF290};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClanTagFeatureName{
    0x141FB2510, 0x1404C8E20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClanTagForClientNum{
    0x141F4EE90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClassIndexFromName{
    0x0, 0x1404CF330};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClassItem{0x141FD8970,
                                                             0x1404CF380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClassSetItem{0x141FD8A80,
                                                                0x1404CF490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClassSetName{0x141FD8B80,
                                                                0x1404CF590};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClientDemoFileCount{
    0x141FB2580};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClientDemoFileInfo{
    0x141FB25A0, 0x1404C8880};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClientNum{0x141F4EF50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetClientPregameState{
    0x141F18100};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCoDPoints{0x141F20890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCodtvRoot{0x141FC6F70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCollectibleInBunkSlot{
    0x141FD5D40, 0x1404CD7C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCollectibleLayout{
    0x141FD5E30, 0x1404CD8B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCombatRecordBackgroundId{
    0x141FC6F90};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetCombatRecordChallengeBackings{0x141FC7050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCombatRecordScreenshotInfo{
    0x141FC7170};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCombatRecordSortedItemInfo{
    0x141FC7240};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCombatRecordSummaryInfo{
    0x141FC7290};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetConnectionPort{0x141FC7340};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetConnectionType{0x141FC7380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetContracts{0x141FC73C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetControllerForXuid{
    0x141FE24D0, 0x1404D6AE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetControllerType{0x141FE2520};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCountLanguagesInSKU{
    0x141FE2540, 0x1404D6B50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCountryNames{0x141FC74D0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetCountUsedAndSignedInLocalClients{0x141FC73E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCryptoKeyCount{0x141F20910};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCryptoKeyProgress{
    0x141F209A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCryptokeysDue{0x141F20A40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCryptoScale{0x141FC7510};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentArenaSeason{
    0x141FC7560};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetCurrentArenaSeasonTimeRemaining{0x141FC75A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentArenaSlot{
    0x141FC75E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentClassSetIndex{
    0x141FD8C50, 0x1404CF660};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentExe{0x141FE2580,
                                                              0x1404D6B90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentGameType{0x141FB25C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentGametypeName{
    0x141FC7620};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetCurrentGobbleGumRecipeScheduleIndex{0x141FD8CF0,
                                                           0x1404CF700};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentMap{0x141FE25A0,
                                                              0x1404D6BB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentMapName{0x141FB2630};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentMode{0x141FE25D0,
                                                               0x1404D6BE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentModeDownloadProgress{
    0x141FC9070};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentModeDownloadString{
    0x141FC90B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentPlatform{0x141FE2630,
                                                                   0x1404D6C40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentRotatingEvent{
    0x141FC9170};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentTeamCount{
    0x141FB26A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentTokens{0x141FD8D30,
                                                                 0x1404CF740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentUTCTimeStr{
    0x141FC9310};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCurrentYearForElite{
    0x141FC9380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCustomClass{0x141FD8E10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCustomClassCount{
    0x141FD8F10, 0x1404CF820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCustomClassModel{
    0x141FD8FB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCustomElementScrollSpeed{
    0x141FB26F0, 0x1404C8E90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCustomGameData{0x141FC93A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetCustomGamesCount{
    0x141FC9480};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDailyChallengeInfo{
    0x141FD91D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDecalDesc{0x141FB2750};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDecimalDelimiter{
    0x141FE2C90, 0x1404D72A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDediQosResultsByType{
    0x141F1C940};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDefaultClassName{
    0x141FD9270, 0x1404CF990};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDefaultClassSlot{
    0x141FB27C0, 0x1404C8EF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDemoSaveScreenDescription{
    0x0, 0x1404BF350};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDemoSaveScreenName{
    0x0, 0x1404BF150};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDemoSegmentCount{
    0x141F12B90, 0x1404BF1C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDemoSegmentInformation{
    0x141F12BD0, 0x1404BF230};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDemoStreamedDownloadProgress{
    0x141F12C80, 0x1404BF110};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDisplayLevelByXUID{
    0x141FB28A0, 0x1404C8FD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDLCBitForDLCIndex{
    0x141F142D0, 0x1404C0840};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDLCBitForMapName{
    0x141F14320, 0x1404C0890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDLCBits{0x141F143B0,
                                                           0x1404C0920};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDLCBitsForLobby{0x141F143F0,
                                                                   0x1404C0960};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDLCNameForItem{0x141FD9100,
                                                                  0x1404CF8C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDollyCamMarkerCount{
    0x141F12CB0, 0x1404BF3E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetDollyCamMarkerInformationForTime{0x141FB2920,
                                                        0x1404C9050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDoubleXPGroups{0x141FC95D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDoubleXPTimeRemaining{
    0x141FC9660};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDStat{0x141FE2650,
                                                         0x1404D6C60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDStatForMode{0x141FE2960,
                                                                0x1404D6F70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetDvarType{0x0, 0x1404D72F0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetEmblemBackgroundEntitlementString{0x141FD92E0,
                                                         0x1404CFA00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemBackgroundId{
    0x141FB46E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetEmblemBackgroundIndexInCategory{0x141FB4720};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemFilterBGCount{
    0x141FB4820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemFilterBGId{
    0x141FB48E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemIconIndexInCategory{
    0x141FB49E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemMaterialDesc{
    0x141FB4AF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemMaterialFilterCount{
    0x141FB4B60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemMaterialIDByIndex{
    0x141FB4BF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemMaterialName{
    0x141FB4CA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemMaterialRef{
    0x141FB4D10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemScaleMode{0x141FB4D80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemSummaryString{
    0x141FB4DF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemSwatchColor{
    0x141FB4EB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEmblemUploadLimits{
    0x141FC96C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEquippedBubbleGumPack{
    0x141FD9370};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEquippedCybercore{
    0x141FD93F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEquippedGender{0x141FD9480,
                                                                  0x1404CFA90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEquippedHead{0x141FD9510,
                                                                0x1404CFB20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEquippedHero{0x141FD95A0,
                                                                0x1404CFBB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEquippedHeroForCACType{
    0x141FD9630, 0x1404CFC40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEquippedHeroInfo{
    0x141FD96F0, 0x1404CFD00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetEquippedInfoForHero{
    0x141FD9780};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetErrorString{0x141FB4F70,
                                                               0x1404C93C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFactionColor{0x141FB4FC0,
                                                                0x1404C9410};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFactionForClient{
    0x141FB50A0, 0x1404C94F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFactionForTeam{0x141FB5100,
                                                                  0x1404C9550};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFeaturedCard{0x141FB5160};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFeaturedCardsData{
    0x141FB51B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFFOTDVersion{0x141FC9740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFileshareFileCategory{
    0x141FC9780};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFileshareNextSlot{
    0x141FC97F0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetFilesharePaintjobSummaryFileIndex{0x141FC9880};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFileshareWeaponIndex{
    0x141FC98F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFileshareWeaponVariantData{
    0x141FC9960};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFirstSelectedHeroLoadout{
    0x141FD9820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFriendPartry{0x141FD31C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFriends{0x141FD3230};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFriendsCount{0x141FD3340};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetFullGamertagForScoreboardIndex{0x141FB51D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetFullPlayerName{0x141F50810};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGadgetSlotForOffhand{
    0x141FA56F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGameLobbyStatusInfo{
    0x141F14440, 0x1404C09B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGamemodesZombie{0x141FC99B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGamertagForClient{
    0x141FE2EE0, 0x1404D74F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGamertagForController{
    0x141FE2F90, 0x1404D75A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetGamertagWhoStolePrimaryController{0x141FE3000};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGametypeName{0x141FC99C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGametypesBase{0x141FC9A30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGametypeSetting{0x141FB52D0,
                                                                   0x1404C95B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGametypeSettings{
    0x141FB53C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGDTCampaignMapInfo{
    0x141FE2D50, 0x1404D7360};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGDTMapsTable{0x141FE2EA0,
                                                                0x1404D74B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGeographicalRegion{
    0x141FB53E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGeoLocation{0x141F144B0,
                                                               0x1404C0A20};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetGestureBindingLocalizedString{0x141FE3020};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGetRecentPlayerParty{
    0x141FD33E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGlobalModel{0x141FAFA70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGPUCount{0x141FD2230};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupAdminDefaultPrivileges{
    0x141F36320};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupAdminPrivileges{
    0x141F36360};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupData{0x141F36420};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupIDList{0x141F364E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupInviteData{0x141F365B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupInvitesList{
    0x141F36650};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetGroupJoinApprovalTypeByGroupId{0x141F36900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupLeaderboardInfo{
    0x141F369D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupList{0x141F36B50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupPlayerSearchCount{
    0x141F36C90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGroupPlayerSearchList{
    0x141F36D30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGunCurrentRank{0x141FD98E0,
                                                                  0x1404CFD90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGunCurrentRankXP{
    0x141FD99B0, 0x1404CFE60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGunNextRank{0x141FD9A80,
                                                               0x1404CFF30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGunPrevRankXP{0x141FD9B50,
                                                                 0x1404D0000};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGunStableRank{0x141FD9C20,
                                                                 0x1404D00D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetGunXPScale{0x141FC9B60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeadList{0x141FD9CF0,
                                                            0x1404D01A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetHeldGadgetSlotForLoadoutGadget{0x141FA7040};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroBundleInfo{0x141FD9D70,
                                                                  0x1404D0220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroCustomizationTable{
    0x141FD9E00, 0x1404D02B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroIndexForClientNum{
    0x141F18150};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroInfo{0x141FD9E80,
                                                            0x1404D0330};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroItemInfo{0x141FD9F00,
                                                                0x1404D03B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroList{0x141FD9FD0,
                                                            0x1404D0480};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetHeroLoadoutItemIndexLobbyData{0x141FB5400, 0x1404C96A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroName{0x141FDA020,
                                                            0x1404D04D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroSelectSoundAlias{
    0x141FDA0D0, 0x1404D0580};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHeroShowcaseWeaponAttribute{
    0x141FDA240};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetHeroShowcaseWeaponVariantName{0x141FDA360};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHighlightedCameraMarker{
    0x141FB54B0, 0x1404C9750};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetHudIconForWeapon{
    0x1427156E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetIndexForGameType{
    0x0, 0x1404C0B30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetIndexForMap{0x0,
                                                               0x1404C0B80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetInGamePlayerList{
    0x141FB5500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetInventoryItem{0x141FAEE60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetInventoryItemQuantity{
    0x141FAEFC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetIsEliteLockedForItemOption{
    0x141FC9BB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetIsSuperUser{0x141FBA0A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetIString{0x141F52100};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemAllocationCost{
    0x141FDA420, 0x1404D06F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemAttachment{0x141FDA4C0,
                                                                  0x1404D0790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemAttachmentLocked{
    0x141FDA590, 0x1404D0860};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemAttachmentRank{
    0x141FDA690, 0x1404D0960};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemAttachmentReward{
    0x141FDA760, 0x1404D0A30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemAttachmentVote{
    0x141F1B300};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemAttachmentXP{
    0x141FDA830, 0x1404D0B00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemCost{0x141FDA900,
                                                            0x1404D0BD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemCount{0x141FDA9A0,
                                                             0x1404D0C70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemDesc{0x141FDAA40,
                                                            0x1404D0D10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemGroup{0x141FDAB30,
                                                             0x1404D0E00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemGroupByIndex{
    0x141FDABF0, 0x1404D0EC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemGroupEnumByName{
    0x0, 0x1404D0F30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemImage{0x141FDACB0,
                                                             0x1404D0F80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemIndex{0x141FDAD90,
                                                             0x1404D1060};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemIndexFromReference{
    0x141FDAE30, 0x1404D1100};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemMomentumCost{
    0x141FDAED0, 0x1404D11A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemName{0x141FDAF70,
                                                            0x1404D1240};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemOptionChallengeValue{
    0x141FDB060, 0x1404D1330};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemOptionLocked{
    0x141FDB150, 0x1404D1420};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemRarityOccurrences{
    0x141F20A90, 0x1404C8430};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemRef{0x141FDB250,
                                                           0x1404D1520};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemRestrictionState{
    0x141FBA100, 0x1404C97A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemSortKey{0x141FDB340,
                                                               0x1404D1610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemUnlockLevel{0x141FDB3F0,
                                                                   0x1404D16C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetItemVote{0x141F1B390};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetJoinableFriendsOfAllLocalPlayers{0x141FC9C10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetKeyBindingLocalizedString{
    0x141FE3040, 0x1404D7610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetKillstreakVariants{
    0x141FDB490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLangAbbr{0x141FE31F0,
                                                            0x1404D77C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLanguage{0x141FE3250,
                                                            0x1404D7820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLastSessionStat{0x141FBA1A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLayerMaterialScaleMode{
    0x141FBA1B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLeaderboardData{0x141FC9C20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLeaderboardHeaders{
    0x141FC9CD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLeaderboardRow{0x141FC9D50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLeaderboardToolTipText{
    0x141FC9E80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLeagueTeamIDForClient{
    0x141F521D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLevelBestStat{0x141FE32B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLimitValue{0x141F14660,
                                                              0x1404C0BD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLinkedLayerCount{
    0x141FBA220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLoadoutInfoForHero{
    0x141FDB520, 0x1404D1760};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLoadoutItemIndexForHero{
    0x141FDB5C0, 0x1404D1800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLoadoutSlotForItem{
    0x141FDB670, 0x1404D18B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLoadoutSlotName{0x141FDB750,
                                                                   0x1404D1990};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLoadoutTypeForHero{
    0x141FDB7C0, 0x1404D1A00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyClientCount{
    0x141F14700, 0x1404C0C70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyClientNumByXUID{
    0x141F14830, 0x1404C0DA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyHostControllerIndex{
    0x141F148E0, 0x1404C0E50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyHostGamertag{
    0x141F14960, 0x1404C0ED0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyHostInfo{0x141F14A10,
                                                                 0x1404C0F80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyHostXuid{0x141F14BD0,
                                                                 0x1404C1140};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyLobbyID{0x141F1D160};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyLocalClientCount{
    0x141F14C40, 0x1404C11B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyMainMode{0x141F14CA0,
                                                                 0x1404C1210};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyMainModeName{
    0x141F14CE0, 0x1404C1250};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyMaxClients{0x141F14D50,
                                                                   0x1404C12C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyMemberPartyInfo{
    0x141FD3450};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyMode{0x141F14E20,
                                                             0x1404C1390};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyModeName{0x141F14E70,
                                                                 0x1404C13E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyModuleName{0x141F14EE0,
                                                                   0x1404C1450};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyNetworkMode{
    0x141F14F50, 0x1404C14C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyNetworkModeName{
    0x141F14F90, 0x1404C1500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyOnlineState{
    0x141F15000, 0x1404C1570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyPregameState{
    0x141F1B3E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbySession{0x141F15040,
                                                                0x1404C15B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbySessionGameData{
    0x141F150E0, 0x1404C1650};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyTypeName{0x141F15140,
                                                                 0x1404C16B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyUgcInfo{0x141F151F0,
                                                                0x1404C1760};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyUIScreen{0x141F151B0,
                                                                 0x1404C1720};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLobbyXUIDForClientNum{
    0x141F152C0, 0x1404C1830};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLocalClientNum{0x141FE32C0,
                                                                  0x1404D7880};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLocalizedTimeStr{
    0x141FC9EE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLootDuplicateCount{
    0x141F20B90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLootItemCount{0x141F20C30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLootItemQuantity{
    0x141F20CA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLootItems{0x141F20D70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLootResults{0x141F20E40,
                                                               0x1404C8780};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLPCCompletionPercentage{
    0x141FE31B0, 0x1404D7780};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetLuaCodeVersion{0x141FD22E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMarketingMessage{
    0x141FC9F80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMarketplaceItems{
    0x141FAF050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMarketplaceSize{0x141FAF070};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMasteryChallengesList{
    0x141FCA0A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMatchID{0x141FE3310,
                                                           0x1404D78D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMatchRecordStats{
    0x141FCA300};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMatchScoreboardClientCount{
    0x141FBA330};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMatchScoreboardClientXuid{
    0x141FBA390};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetMatchScoreboardIndexAndClientNumForTeam{0x141FBA400};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxAllocation{0x141FBA4F0,
                                                                 0x1404C9840};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxAmmoForItem{0x141FDB850,
                                                                  0x1404D1A90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxArenaPoints{0x141FCA3C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxControllerCount{
    0x141FE3370, 0x1404D7930};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxFetchedGroupsCount{
    0x141F36E40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxGroupAdminsCount{
    0x141F36E60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxLocalControllers{
    0x141FE3390};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxOITQuality{0x141FD2300};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxSceneResolutionMultiplier{
    0x141FD2340};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxShadowQuality{
    0x141FD23D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxTextureQuality{
    0x141FD2410};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMaxUserPlayerCount{
    0x141FCA400};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMedalInfo{0x141FE33B0,
                                                             0x1404D7950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetModel{0x141FAFAB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetModelForController{
    0x141FAFB50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetModelPath{0x141FAFC10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetModelUsage{0x141FAFC20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetModelValue{0x141FAFC40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetModeName{0x141FE3510,
                                                            0x1404D7AB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMonthStringForTimestamp{
    0x141FCA440};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMostRecentPlayedMode{
    0x141FE3570, 0x1404D7B10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMOTD{0x141FBA2D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMOTDImageFileID{0x141FBA2F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetMutedStatus{0x141FBA580,
                                                               0x1404C98D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNatType{0x141FBA640,
                                                           0x1404C9990};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNextUnlockable{0x141FDB930};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNonUsedControllerCount{
    0x141FE35C0, 0x1404D7B60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumAttachments{0x141FDB950,
                                                                  0x1404D1B70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumberOfChallengesForUnlock{
    0x141FBA850};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumberOfClassSetsOwned{
    0x141FDBC90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumberOfHighlightReelMoments{
    0x141F12CF0, 0x1404BF700};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumberOfPlayersAlive{
    0x141F53B00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumBulletWeapons{
    0x141FBA680, 0x1404C99D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumFeatureUnlocks{
    0x141FBA6A0, 0x1404C99F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumGadgetAttachments{
    0x141FDB9F0, 0x1404D1C10};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetNumItemAttachmentsWithAttachPoint{0x141FBA780,
                                                         0x1404C9AD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumLives{0x141F52210};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumOptics{0x141FDBAD0,
                                                             0x1404D1CF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumRigs{0x141FDBB80,
                                                           0x1404D1DA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumTextLines{0x141FE3630};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetNumWeaponOptionsForGroup{
    0x141FDBC40, 0x1404D1E60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveEntity{0x141F53BB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveGamemodeFlags{
    0x141F53C60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveIndexFromName{
    0x141F53D00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveInfo{0x141FD5E80,
                                                                 0x1404CD900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveName{0x141F53DD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveOwner{0x141F53E80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectivePosition{
    0x141F53F20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveProgress{
    0x141F53FE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveState{0x141F54090};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveTeam{0x141F54120};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetObjectiveVisibleTeams{
    0x141F541C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetOnlineFriendsList{
    0x141FD3680};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetOnlinePlayerCount{
    0x141FCA4B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetOptionIDByName{0x141FCA5A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetOptionNameByID{0x141FCA630};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetOtherPlayerLevelBestStat{
    0x141FBA870, 0x1404C9BA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetOtherPlayerStatsBuffer{
    0x141FCA6E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPaintjobSlotInfo{
    0x141FCA740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPaintshopSummaryString{
    0x141FBA890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetParagonIconById{0x141FBA950,
                                                                   0x1404C9BC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetParagonIconTable{
    0x141FDBD30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetParagonRankDisplayLevel{
    0x141FCA790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetParagonXPCap{0x141FDBD80,
                                                                0x1404D1EB0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetParkingPlaylistForRealPlaylist{0x141F1E290};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPartyPrivacy{0x141F15380,
                                                                0x1404C18F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPartyPrivacyName{
    0x141F153C0, 0x1404C1930};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPermanentUnlockCount{
    0x141FDBDF0, 0x1404D1F20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPingForScoreboardIndex{
    0x141FBAA20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPingForServerLocation{
    0x141F1CA50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPingImageCount{0x141FCA850};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPingStepMS{0x141FCA870};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerBalanceByCurrencyId{
    0x141F20EA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerBalanceItems{
    0x141FAF090};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerCommonGamerProfile{
    0x141FE3770};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerCount{0x141FCA890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerGroupCount{
    0x141FCA9D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerGroupCountInt{
    0x141FCAA90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerIndexByXuid{
    0x141FCAB10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerInfo{0x141FD3750};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerInventoryItems{
    0x141FAF170};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerInventorySize{
    0x141FAF190};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerJoinState{0x141FCAC50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerListData{0x141F54260};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerModel{0x141F55C40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerName{0x141F57530};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerNameForClientNum{
    0x141F58E20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerObjectives{
    0x141F5A7B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerQueueInfo{0x141FE3830,
                                                                   0x1404D7BD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerStats{0x141FCAD00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlayerStatsNightmare{
    0x141FCAEE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistCategories{
    0x141F1E2E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistCategoryFilter{
    0x141F1E350};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistCategoryName{
    0x141F1E460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistDesc{0x141F1E540};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistGameType{
    0x141F1E5E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistIconName{
    0x141F1E6C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistID{0x141F1E680};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistInfoByID{
    0x141F1E760};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistMaxPartySize{
    0x141F1E7B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistMinorVersionNumber{
    0x141F1E850};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistMinPartySize{
    0x141F1E800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistName{0x141F1E8C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistPrevCount{
    0x141F1E960};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylists{0x141F1EAD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistSuperCategories{
    0x141F1E9F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistSuperCategoryID{
    0x141F1EA20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPlaylistVersionNumber{
    0x141F1EA90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPollNameByID{0x141FCB050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPollValue{0x141FCB0C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPredictedClientNum{
    0x141F5C380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPregameItemVotes{
    0x141F1B430};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPreReqChallengeValue{
    0x141FDBED0, 0x1404D2000};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPrestigeByXUID{0x141FBAA80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPrestigeCap{0x141FDBFE0,
                                                               0x1404D2110};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPrestigeForScoreboardIndex{
    0x141FBAAA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPrimaryController{
    0x141FE3900, 0x1404D7BF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPrimaryGroup{0x141F36E80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPrimaryGroupName{
    0x141F36EC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetProbationTime{0x141FCB2D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetProfileVarInt{0x141FE3940,
                                                                 0x1404D9E90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetProgressByFileName{
    0x141FCB380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetProtocolVersion{0x141F15430,
                                                                   0x1404C19A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPS3MTXProductIndex{
    0x141F355C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetPurchaseDWSKUResult{
    0x141FAF1F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetQuickJoinPlayers{
    0x141FD38A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetQuickJoinPlayersCount{
    0x141FD3980};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetQuickJoinPlayersPartyInfo{
    0x141FD39D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRank{0x141FBAB00,
                                                        0x1404C9C90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRankByXUID{0x141FBABA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRankDisplayLevel{
    0x141FCB3B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRankForScoreboardIndex{
    0x141FBABC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRankIcon{0x141FBAC80,
                                                            0x1404C9D30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRankIconForClient{
    0x141FE39D0, 0x1404D7C30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRankIconForScoreboardIndex{
    0x141FBAD90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRankName{0x141FCB470};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRankStringForClient{
    0x141FE3AD0, 0x1404D7D30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetReadyUp{0x141F1D430,
                                                           0x1404C6160};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRecentChallenges{
    0x141FCB530};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRecentlyUnlockedItems{
    0x141FCB610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRecentMedals{0x141FCB590};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRecentPlayers{0x141FD3A40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRecentPlayersCount{
    0x141FD3B50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRecentPlayersList{
    0x141FD3BF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRecentUnlocks{0x141FBADF0,
                                                                 0x1404C9E40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRegistrationStatus{
    0x141FCB6A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRegulations{0x141FCB740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRenderTooltip{0x141FBAE50,
                                                                 0x1404C9EA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetRoundsPlayed{0x141F5DC40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSavedMap{0x141FD60E0,
                                                            0x1404CDB50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSavedMapQueuedMap{
    0x141FD6140, 0x1404CDBB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSavedMapSkipto{0x141FD61A0,
                                                                  0x1404CDC10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetScoreboardClientOrderIndex{
    0x141FBB090};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetScoreboardColumnForClient{
    0x141FBC9A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetScoreboardColumnForScoreboardIndex{0x141FBCA60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetScoreboardColumnHeader{
    0x141FBCAF0, 0x1404C9FF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetScoreBoardColumnName{
    0x141FBAFA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetScoreboardData{0x141FBCBA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetScoreboardPlayerData{
    0x141FBCD90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetScoreboardTeamClientCount{
    0x141FBCE70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSearchQuery{0x141F1D080};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSearchResults{0x141F1D0A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSecondsRemaining{
    0x141FCB790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSecondsRemainingServer{
    0x141FCB830};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetSelectedEmblemLayerProperties{0x141FBCEE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedGestureForHero{
    0x141FDC050, 0x1404D2180};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedGroupAdminsCount{
    0x141F36F30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedGroupAdminsList{
    0x141F36FD0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetSelectedGroupApplicationsCount{0x141F370E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetSelectedGroupApplicationsList{0x141F37180};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetSelectedGroupBannedMembersCount{0x141F37290};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetSelectedGroupBannedMembersList{0x141F37330};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetSelectedGroupFriendsFilteredCount{0x141F37440};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetSelectedGroupFriendsFilteredList{0x141F374E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetSelectedGroupJoinApprovalType{0x141F375F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedGroupMembersCount{
    0x141F376F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedGroupMembersList{
    0x141F37790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedGroupMembersParty{
    0x141FD3CC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedGroupMemberStatus{
    0x141F37640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedLayerColor{
    0x141FBD140};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedLayerColor1{
    0x141FBD080};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedLayerData{
    0x141FBD200};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedLayerIconID{
    0x141FBD360};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedLayerMaterialID{
    0x141FBD410};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedLayerMaterialIndex{
    0x141FBD4C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelectedTauntForHero{
    0x141FDC130, 0x1404D2260};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSelfGamertag{0x141FE3C50,
                                                                0x1404D7EB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetServerPingsReady{
    0x141F1CAC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetServerUTCOffset{0x141FCB8D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetServerUTCTimeStr{
    0x141FCB910};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSessionModeFromLobbyMainMode{
    0x141FE3CE0, 0x1404D7F40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSessionPartyPrivacy{
    0x141F15450, 0x1404C19C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSessionStatus{0x141F154F0,
                                                                 0x1404C1A60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetShadowQuality{0x141FD2450};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetShotCountForItem{
    0x141FDC210, 0x1404D2340};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSizeString{0x141FCB980};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSkuRegion{0x141FD2490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSocialCapabilities{
    0x141FCBA50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSpectatingClanTag{
    0x141F5F500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSpectatingClientInfo{
    0x141F60E00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSpectatingPlayerName{
    0x141F62770};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSpectatingTeamID{
    0x141F64060};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetStartLocsZombie{0x141FCBA80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetStatByName{0x141FBD5C0,
                                                              0x1404CA0A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetStatScoreMultiplier{
    0x141FBD6B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetStatusIconForClient{
    0x141FBD6D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSteamExchangeProcessStatus{
    0x141FCBA90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetStoreCategories{0x141FCBAD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetStoreProductsByCategory{
    0x141FCBAF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetStoreSessionAndTransactionID{
    0x141FCBB60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSystemInfo{0x141FBEFA0,
                                                              0x1404CA190};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetSystemInfoValueAndString{
    0x141FBF130, 0x1404CA320};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTableRowCount{0x141FE3D80,
                                                                 0x1404D7FE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTacticalMenuItems{
    0x141F65940};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTeamForXUID{0x141F1FF60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTeamID{0x141F659E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTeamIDByXUID{0x141F672D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTeamName{0x141FBF2A0,
                                                            0x1404CA490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTeamPositions{0x141FBF300};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTeams{0x141F20050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTermsOfService{0x141FCBBE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTextureQuality{0x141FD24B0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetTheaterFilmNotSelectedMessage{0x141FBF500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTimeRemainingString{
    0x141FE3E10, 0x1404D8070};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTitleData{0x141FD3D30};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetTitleFriendsOfAllLocalPlayers{0x141FCBC10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTotalGroupInvitesCount{
    0x141F378A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTotalGroupsCount{
    0x141F37900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetTwitchAccountInfo{
    0x141FCBC20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUI3DWindowAspectRatio{
    0x141FE3FA0, 0x1404D8200};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockableInfoByIndex{
    0x141FDC430, 0x1404D2560};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockableInfoModelByIndex{
    0x141FDC4C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockablesByGroupName{
    0x141FDC5B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockablesBySlotName{
    0x141FDC650};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockedFeatureItemIndex{
    0x141FDC6F0, 0x1404D25F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockedItemsForLevel{
    0x141FCBC80};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetUnlockedParagonIconsForParagonRank{0x141FDC810};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockIndexFromGroupName{
    0x141FDC2C0, 0x1404D23F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockLocString{0x141FDC370,
                                                                   0x1404D24A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnlockString{0x141FBF520};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUnredactTextDimensions{
    0x141FE3FF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUsedAllocation{0x141FDC8A0,
                                                                  0x1404D2710};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUsedControllerCount{
    0x141FE4260, 0x1404D8250};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUsedGroupCount{0x141FBF540};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUsedLayerCount{0x141FBF670};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUserPartyInfo{0x141FD3E50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUserRegulationPreference{
    0x141FCBD40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUserSafeArea{0x141FE42D0,
                                                                0x1404D82B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUserSafeAreaForController{
    0x141FE4350, 0x1404D82F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUserTier{0x141FBF7A0,
                                                            0x1404CA4F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetUTCOffset{0x141FCBC40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetVolumetricQuality{
    0x141FD24F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetVote{0x141FCBD90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetVotingData{0x141FCBEA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetVotingMenuViewCount{
    0x141FCBEC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponGroup{0x141FCBF20};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetWeaponOptionEntitlementString{0x141FDC990, 0x1404D2800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponOptionGroupIndex{
    0x141FDCA00, 0x1404D2870};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponOptionGroupName{
    0x141FDCA90, 0x1404D2900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponOptionImage{
    0x141FDCB00, 0x1404D2970};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponOptionLocked{
    0x141FDCB70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponOptionName{
    0x141FDCB90, 0x1404D29E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponOptionsCount{
    0x141FDCC40, 0x1404D2A90};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_GetWeaponOptionsInfoModelByIndex{0x141FDCC80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponOptionUnlockPLevel{
    0x141FDCBF0, 0x1404D2A40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetWeaponString{0x141FE4440};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetXPCap{0x141FDCD90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetXPScale{0x141FCBFB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetXUID{0x141FBF800,
                                                        0x1404CA550};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetXUID64{0x141FE6010,
                                                          0x1404D8370};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetXuidFromFriendId{
    0x141F15540, 0x1404C1AB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetZMVials{0x141F213C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GetZMVialScale{0x141FCC000};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GiveLeadership{0x141F15650,
                                                               0x1404C1BC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_GroupAdminHasPrivilege{
    0x141F37990};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_HasCompletedCheckingRestrictions{0x141FD2530};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasCurrentLevelBeenBeaten{
    0x141FBF8E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasDLCContent{0x141F355E0,
                                                              0x1404C8480};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasDLCForItem{0x141FDCE00,
                                                              0x1404D2B40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasEntitlement{0x141F35640,
                                                               0x1404C84E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasEntitlementByOwnership{
    0x141F356D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasGroupInviteBeenSent{
    0x141F37A60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasMPPrivileges{0x141FBF900,
                                                                0x1404CA630};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasMTX{0x141F35760};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasMultiCoreTacticalRig{
    0x141F673B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasPerk{0x141F67430};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_HasRecentItemAttachmentsUnlocked{0x141FBF960, 0x1404CA690};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HasSeasonPass{0x141F35780,
                                                              0x1404C8570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HideMouseCursor{0x141FD2550};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HidePsStoreIcon{0x141FCC0C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_HostMigrationWaitingForPlayers{
    0x141FBF9E0, 0x1404CA710};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IncrementBGBTokensGained{
    0x141FDCEF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IncrementBGBTokensUsed{
    0x141FDCF70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IncrementCounter{0x141FCC0D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IncrementCurrency{0x141F20F50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IncrementStoreTransaction{
    0x141FCC140};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_InKillcam{0x141F67500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_InLobby{0x141FBFA40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_InLobbyParty{0x141F156D0,
                                                             0x1404C1C40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_InPrivateParty{0x141FBFA60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Int64AsString{0x141FE6070,
                                                              0x1404D83D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_InvalidateCharacterItemRender{
    0x141FDCFF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_InvalidateWCItemRender{
    0x141F1D8C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Is4K{0x141FE60E0, 0x1404D8440};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsACVItemNew{0x141FDD0E0,
                                                             0x1404D2C30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAdvertisedLobby{0x141F1C850};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAmmoBasedGadget{0x141F68DB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAmmoClipLow{0x141F6A6D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAnyACVItemNew{0x141FDD1E0,
                                                                0x1404D2D30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAnyControllerMPRestricted{
    0x141FBFA80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAnyEmblemBackgroundNew{
    0x141FBFAA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAnyEmblemIconNew{0x141FBFB50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAnyLocalClientLeader{
    0x141F15790, 0x1404C1D00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAnythingInCACNew{0x141FDD2C0,
                                                                   0x1404D2E10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsArenaMaster{0x141FCC180};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAttachmentNew{0x141FDD360,
                                                                0x1404D2EB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAttachmentRestricted{
    0x141FBFBB0, 0x1404CA770};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsAttachmentRestrictedForAllWeapons{0x141FBFCA0,
                                                        0x1404CA860};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsAttachmentRestrictedForAnyWeapon{0x141FBFD00,
                                                       0x1404CA8C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsAttachmentSlotLocked{
    0x141FDD460, 0x1404D2FB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsBetaBuild{0x141FE6120};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsButtonPressed{0x141F15830,
                                                                0x1404C1DA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCampaignGame{0x141FE6140,
                                                               0x1404D8480};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCampaignModeUnlocked{
    0x141FE6180, 0x1404D84C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCampaignModeZombies{
    0x141FE61E0, 0x1404D8520};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsCharacterCustomizationItemLocked{0x141FDD560};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCharacterCustomizationItemNew{
    0x141FDD690};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsChatRestricted{0x141FCC210};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsChunkInstalledByFileName{
    0x141FCC250};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCinematicMp4{0x141FE6230};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCinematicPlaying{0x141F6BFC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCinematicPreloading{
    0x141F6C000};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCinematicStarted{0x141FE6290,
                                                                   0x1404D8570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCinematicWebm{0x141FE62D0,
                                                                0x1404D85B0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsClassSetsAvailableForCurrentGameMode{0x141FDD7B0,
                                                           0x1404D30B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsClientLoaded{0x141FCC270};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsClipboardEmblemGrouped{
    0x141FBFD60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsClipModified{0x141F12D40,
                                                               0x1404BF5C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCodtvContentLoaded{
    0x141FCC2E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCommandBound{0x141FE6330,
                                                               0x1404D8610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCommonFastFileLoaded{
    0x141FE6400, 0x1404D86E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsContentAvailableByPakName{
    0x141F35820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsContentRatingAllowed{
    0x141FE6440};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsControllerBeingUsed{
    0x141FE6460, 0x1404D8720};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsControllerUsed{0x141FE64C0,
                                                                 0x1404D8780};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCPInProgress{0x141FD6200,
                                                               0x1404CDC70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCpOwned{0x141FD2570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCpStillDownloading{
    0x141FD25B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCurrentLanguageReversed{
    0x141FE6560, 0x1404D8820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCurrentLanguageSkuReversed{
    0x141FE65A0, 0x1404D8860};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsCurrentlyCheckingContentRestrictions{0x141FD25F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCurrentWeaponName{
    0x141F6C060};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsCurrentWeaponType{
    0x141F6D9A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsCustomElementScrollLanguageOverrideActive{0x141FBFDA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDedicatedServer{0x141F158C0,
                                                                  0x1404C2050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDedicatedServerMods{
    0x141F158E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoCameraEditMode{
    0x141F12D80, 0x1404BF600};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoClipPlaying{0x141F12DC0,
                                                                  0x1404BFA20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoClipPreviewRunning{
    0x141F12E20, 0x1404BF460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoClipRecording{
    0x141F12E60, 0x1404BF4C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoDollyCamera{0x141F12EA0,
                                                                  0x1404BF420};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoHighlightReelMode{
    0x141F12EE0, 0x1404BF500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoMovieRendering{
    0x141F12F20, 0x1404BF580};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemonwareFetchingDone{
    0x141FE6640, 0x1404D8900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoPlaying{0x141F12F60,
                                                              0x1404BF680};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoShoutcaster{0x141F70BE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDemoStreamingFinished{
    0x141F12FA0, 0x1404BF320};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDevelopmentBuild{0x141F15900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDeviceRestrictingContent{
    0x141FD2610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDOAGame{0x141FE65E0,
                                                          0x1404D88A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDoubleXPWeekend{0x141FCC300};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsDWAmmoClipLow{0x141F6F2F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsEliteAvailable{0x141FCC360};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsEliteButtonAvailable{
    0x141FCC380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsEmblemBackgroundLocked{
    0x141FDD810};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsEmblemBackgroundNew{
    0x141FBFDC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsEmblemEmpty{0x141FBFE50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsEmblemIconNew{0x141FBFF80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsEntityNumberInUse{
    0x141F70C20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsFacebookDuplicate{
    0x141FCC3A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsFacebookLinked{0x141FCC400};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsFeatureBanned{0x141FCC460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsFeaturePermaBanned{
    0x141FCC590};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsFFOTDFetched{0x141FC0010,
                                                               0x1404CA920};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsFirstTimeComplete{
    0x141FCC6C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsFriendFromXUID{0x141FE66D0,
                                                                 0x1404D8980};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsFriendInInitialGroupInvitesList{0x141F37B00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGadgetActive{0x141FA8990};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGadgetAvailable{0x141F756B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGadgetCharging{0x141FAA2D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGadgetContextualWeaponActive{
    0x141F76FA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGadgetCurrentWeapon{
    0x141F788C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGadgetEnergyLow{0x141F7A1C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGadgetFlickering{0x141F7BB20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGadgetReady{0x141F7D470};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGametypeSettingDefault{
    0x141FC0070, 0x1404CA980};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGameTypeValid{0x141F35840,
                                                                0x1404C8610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGestureNew{0x141FDD8D0,
                                                             0x1404D3110};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGroupEmblemEmpty{0x141F37B50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsGuestByXuid{0x141FE6760,
                                                              0x1404D8A10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsHeroVersion_2_0{0x141FABC10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsHiDef{0x141FE67C0,
                                                        0x1404D8A70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsInComError{0x141F15920,
                                                             0x1404C1E30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsInGame{0x141FCC750};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsInputLocked{0x141FE6800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsInRange{0x141FCC7B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsInventoryBusy{0x141FAF240};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemAttachmentLocked{
    0x141FDDA00, 0x1404D3240};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsItemAttachmentRecentlyUnlocked{0x141FDDB00, 0x1404D3340};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemIndexRestricted{
    0x141FC00D0, 0x1404CA9E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemLocked{0x141FDDC00,
                                                             0x1404D3440};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemLockedForAll{0x141FDDCD0,
                                                                   0x1404D3510};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemNew{0x141FDDDB0,
                                                          0x1404D35F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemOptionLocked{0x141FDDE80,
                                                                   0x1404D36C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemPassive{0x141FDDF60,
                                                              0x1404D37A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemPermanentlyUnlocked{
    0x141FDE010, 0x1404D3850};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsItemPurchased{0x141FDE110,
                                                                0x1404D3950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLanguageReversed{0x141FE68B0,
                                                                   0x1404D8B00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLanguageSupportedInSKU{
    0x141FE6910, 0x1404D8B60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLeader{0x141F15950,
                                                         0x1404C1E60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLevelPreloaded{0x141F7EDB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLivestreamEnabled{
    0x141FCC930};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLoadoutSlotNew{0x141FDE1E0,
                                                                 0x1404D3A20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLobbyActive{0x141F159D0,
                                                              0x1404C1EE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLobbyClientActive{
    0x141F15A30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLobbyHost{0x141F15A90,
                                                            0x1404C1F40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLobbyHostActive{0x141F15AF0,
                                                                  0x1404C1FA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLobbyInRecovery{0x141F1D5B0,
                                                                  0x1404C62E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLobbyMigratActive{
    0x141F1D610, 0x1404C6340};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLobbyMigrateActive{
    0x141F1D670};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLocalClient{0x141F15B50,
                                                              0x1404C2000};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsLocalClientInPlatformPartyChat{0x141F15BA0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsLootItemUnlockedByPreRequisites{0x141F21000};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLootReady{0x141F211E0,
                                                            0x1404C8460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsLPCBusy{0x141FE6860,
                                                          0x1404D8AB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMainThread{0x141FE6970,
                                                             0x1404D8BC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMainThreadOrProxy{
    0x141FE69B0, 0x1404D8C00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMapValid{0x141F35910,
                                                           0x1404C86C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMedalUnlocked{0x141FD6240,
                                                                0x1404CDCB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMenuLevel{0x141FE69F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMigrating{0x141F7EDF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMouseInsideElement{
    0x141FE6A10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMpInitialStillDownloading{
    0x141FD2670};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMpOwned{0x141FD26B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMpStillDownloading{
    0x141FD26F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMTXAvailable{0x141F358F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsMultiplayerGame{0x141FE6B00,
                                                                  0x1404D8C40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsOfferPurchased{0x141F359D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsOnboarding{0x141FCC950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsOptic{0x141FDE2B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsOpticByAttachmentIndex{
    0x141FDE390, 0x1404D3BD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsOverheatWeapon{0x141FDE3F0,
                                                                 0x1404D3C30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerDead{0x141F7EE60};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsPlayerEliteEnrollNotificationAgeOK{0x141FCC9F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerEliteFounder{
    0x141FCCA10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerEliteRegistered{
    0x141FCCA20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerEliteRegistrationAgeOK{
    0x141FCCA30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerEnergyLow{0x141F80740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerInCurrentSession{
    0x141FCCA50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerInLastStand{
    0x141F82020};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerInTitle{0x141FE6B40,
                                                                0x1404D8C80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerInVehicle{0x141F83900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerInvitable{0x141FE6C30,
                                                                  0x1404D8D70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerJoinable{0x141FCCAB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerMuted{0x141F20220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerMutedByClientNum{
    0x141F202C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerMuteToggled{
    0x141FCCB60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerQueued{0x141FE6CF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerRemoteControlling{
    0x141F851D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerUnderage{0x141FCCB70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayerWeaponViewOnlyLinked{
    0x141F86AA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlayGoEnabled{0x141FCC9D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlaylistLocked{0x141F1EB40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPlusAuthorized{0x141FD2730};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPowerBarGadget{0x141F88370};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPowerBasedGadget{0x141F89C90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPresentInFileshare{
    0x141FCCB90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPresetClassDefault{
    0x141FC01A0, 0x1404CAAB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPrestigeTokenSpent{
    0x141FDE470, 0x1404D3CB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPrestigeTokenSpentByType{
    0x141FDE560, 0x1404D3DA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPrimaryLocalClient{
    0x141FE6D90, 0x1404D8E30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsProcessingJoin{0x141F1BE30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsProfileBuild{0x141F15C30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsProfileLoggedOnAndOnline{
    0x141FCCC30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsProfileSignedIn{0x141FE6DF0,
                                                                  0x1404D8E90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsPropHuntProp{0x141FC0230,
                                                               0x1404CAB40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsQoSProbeListenerEnabled{
    0x141F1CB00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsRecentGameBookmarked{
    0x141FCCC80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsReleaseBuild{0x141F15C50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsRepositioningCameraMarker{
    0x141F12FE0, 0x1404BF390};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsRequestingJoin{0x141F1BE70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsRig{0x141FDE670};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsRunningUILevel{0x141FCCD10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsScreenshotReady{0x141FCCD50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsScrSystemActive{0x141FE6E50,
                                                                  0x1404D8EF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsServerRunning{0x141FCCDD0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_IsSharedLoadoutAvailableForClient{0x141FDE750};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsShipBuild{0x141F15C70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsShoutcaster{0x141F8B5B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSignedIn{0x141FE6F00,
                                                           0x1404D8FA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSignedInToDemonware{
    0x141FCCE10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSignedInToLive{0x141FE6F70,
                                                                 0x1404D9010};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSignedIntoPSN{0x141FD2750};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSpecialistTransmissionLocked{
    0x141FDE800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSpectatingActiveClient{
    0x141F8B610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSplitscreen{0x141FE6FF0,
                                                              0x1404D9090};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSplitscreenClient{
    0x141F15C90, 0x1404C2070};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsStableStatsBufferInitialized{
    0x141FE7030, 0x1404D90D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsStarterPack{0x141FE70A0,
                                                              0x1404C6740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSteamWorkshopEnabled{
    0x141FE70E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSubUser{0x141FE7160};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsSuperUser{0x141FC1B00,
                                                            0x1404CABE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsTakeTwoGadgetAttachment{
    0x141FDE970, 0x1404D3F90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsTauntNew{0x141FDEA40,
                                                           0x1404D4060};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsTwitchAccountChecked{
    0x141FCCEA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsTwitchAccountRegistered{
    0x141FCCEC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsTwitterAccountChecked{
    0x141FCCEE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsTwitterAccountRegistered{
    0x141FCCF00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUnlimitedAmmoWeapon{
    0x141FDEB70, 0x1404D4190};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUpdatingLeaderboard{
    0x141FCCF20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUserActive{0x141FCCF60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUserChatRestricted{
    0x141FD2770};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUserContentRestricted{
    0x141FD27D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUserGuest{0x141FCCFD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUsingCursor{0x141FE7180};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUsingDemonwareAuthentication{
    0x141F15D50, 0x1404C6860};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUsingMods{0x141FD2820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsUsingUsermap{0x141FD2860};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsValidGadgetSlot{0x141FAD570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsVisibilityBitSet{0x141FE71A0,
                                                                   0x1404D9140};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsWaitingForDatabase{
    0x141FE7230, 0x1404D91D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsWeaponOptionGroupNew{
    0x141FDEBF0, 0x1404D4210};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsWeaponOptionLockedEntitlement{
    0x141FDECF0, 0x1404D4310};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsWeaponOptionNew{0x141FDEDD0,
                                                                  0x1404D43F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsWeaponType{0x141FDEF50,
                                                             0x1404D4570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsWeaponUsedOnVehicle{
    0x141FE7270, 0x1404D9210};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsYouTubeAccountChecked{
    0x141FCD030};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsYouTubeAccountRegistered{
    0x141FCD050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsZmInitialStillDownloading{
    0x141FD28A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsZmOwned{0x141FD28E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsZmStillDownloading{
    0x141FD2920};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_IsZombiesGame{0x141FE8B40,
                                                              0x1404D92A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ItemIndexHasIncentive{
    0x141FDF060, 0x1404D4680};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ItemIndexIsBlackMarket{
    0x141FDF110, 0x1404D4730};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ItemIndexValid{0x141FDF1B0,
                                                               0x1404D47D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_JBMatchmakingEvent{0x141F15D70,
                                                                   0x1404C2130};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_JoinGroup{0x141F37C00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_JoinHost{0x141F1BEB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_JoinPlayerSessionByXuid{
    0x141FCD070};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_JoinXUID{0x141F1C040};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_JoinXUIDExt{0x141F1C120};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_KeyBinding{0x141FE8B80,
                                                           0x1404D92E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_KickClient{0x141F15DB0,
                                                           0x1404C2150};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_KickGroupMember{0x141F37C70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LastInput_Gamepad{0x141FE8DC0,
                                                                  0x1404D9520};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LastInput_Mouse{0x141FE8E50,
                                                                0x1404D95B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LastInput_Vita{0x141FE8EE0,
                                                               0x1404D9640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Leaderboard_GetCached{
    0x141F13770, 0x1404BFE10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Leaderboard_GetCachedByColName{
    0x141F13800, 0x1404BFEA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Leaderboard_GetDeltaStat{
    0x141F13890, 0x1404BFF30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Leaderboard_GetScoreboardStat{
    0x141F13A30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LeaderboardPopulateCustomList{
    0x141FCD090};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LeaveGroup{0x141F37CE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LeaveQueue{0x141FE8F40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LeaveServerImmediately{
    0x141F15F00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LeaveWithParty{0x141F15F60,
                                                               0x1404C22E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiquefyGobbleGum{0x141F21450};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveConnectDisableDemonwareConnect{0x141FE8F80,
                                                       0x1404D96E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveConnectDisconnectFromDemonware{0x141FE8FD0,
                                                       0x1404D96A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveConnectEnableDemonwareConnect{0x141FE9010};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveIsControllerSignedIn{
    0x141FCD1E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveMetPlayerUpdateMetPlayerList{0x141FE9060, 0x1404D9730};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LivePlatformSessionCanProcessLaunchData{0x141F1DA30};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LivePlatformSessionIsInviteJoinInfoAvailable{0x141F1DA90};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LivePlatformSessionIsPlayTogetherInfoAvailable{0x141F1DAB0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamLobby_AddRequestLobbyListNumericalFilter{
        0x141F1EDC0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamLobby_AddRequestLobbyListStringFilter{0x141F1EE50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveSteamLobby_GetLobbyData{
    0x141F1EEF0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamLobby_GetLobbyDataForIndex{0x141F1EFA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveSteamLobby_SetLobbyData{
    0x141F1F0B0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamServer_AddKvpToGameTags{0x141F1F120};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamServer_GetGameDescription{0x141F1F190};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveSteamServer_GetServerName{
    0x141F1F1F0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamServer_GetServerPassword{0x141F1F250};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveSteamServer_SetBotCount{
    0x141F1F2B0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamServer_SetGameDescription{0x141F1F2F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveSteamServer_SetGameTags{
    0x141F1F330};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveSteamServer_SetKeyValue{
    0x141F1F370};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveSteamServer_SetMapName{
    0x141F1F3E0, 0x1404C7BA0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamServer_SetMaxPlayerCount{0x141F1F420};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LiveSteamServer_SetServerName{
    0x141F1F460};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveSteamServer_SetServerPassword{0x141F1F4A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LiveStorageSetAllStatsNotFetched{0x141FE90C0, 0x1404D9790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LivestreamDisableAsync{
    0x141FCD240};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LivestreamEnable{0x141FCD260};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LivestreamStartCapture{
    0x141FCD270};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LivestreamStopCaptureAsync{
    0x141FCD280};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LoadFileshareCustomGame{
    0x141FCD2B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LoadLeaderboard{0x141FCD310};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LoadSaveGame{0x141FE9100,
                                                             0x1404D97D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyClearDemoInformation{
    0x141F13020};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyClientEnd{0x141F16000,
                                                               0x1404C2380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyClientErrorShutdown{
    0x141F16060, 0x1404C23E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyClient_GetProbationTime{
    0x141FCD3A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyClient_IsInProbation{
    0x141FCD450};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyClientIsStateAvailable{
    0x141F160A0, 0x1404C2420};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyClientStart{0x141F16100,
                                                                 0x1404C2480};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyClientTaskErrorShutdown{
    0x0, 0x1404C2510};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyConnectedToDedicatedHost{
    0x141F161A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyDisconnectClient{
    0x141F16200, 0x1404C2520};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyGetDemoInformation{
    0x141F13060, 0x1404BF830};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyGetReservationData{
    0x141F162B0, 0x1404C25D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyGetSessionClients{
    0x141F162D0, 0x1404C25F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostAddLocal{0x141F16370,
                                                                  0x1404C2690};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostAddPrimary{
    0x141F16400, 0x1404C2720};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyHost_AnyClientInProbationForGameMode{0x141FCD500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostAssignMapVoteToClient{
    0x141F1D490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostAssignTeamToClient{
    0x141F20170};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyHost_ClearCharacterDraftSelectionData{0x141F1B5D0,
                                                               0x14048BAE0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyHost_ClearItemRestrictionVoteData{0x141F1B5F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHost_ClearPregameState{
    0x141F1B610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHost_DisconnectClient{
    0x141F16900, 0x1404C2C20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostEnd{0x141F164E0,
                                                             0x1404C2800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostErrorShutdown{
    0x141F16580, 0x1404C28A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyHost_ForceClientCharacterSelection{0x141F1B630};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyHost_ForceClientEditComplete{0x141F1B6C0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyHost_ForceClientRoundComplete{0x141F1B700};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostIsPrivateSession{
    0x141F165C0, 0x1404C28E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostLaunchGetServerStatus{
    0x141F16600, 0x1404C2920};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostLaunchTaskInProgress{
    0x141F16640, 0x1404C2960};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostMigrateSetIndexBits{
    0x141F1D6D0, 0x1404C63A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostSessionSetDirty{
    0x141F16680, 0x1404C29A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyHost_SetAllClientsPregameState{0x141F1B740};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyHost_SetClientPregamePosition{0x141F1B780};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHost_SetClientPregameState{
    0x141F1B7D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHost_SetCustomGameName{
    0x141F1B820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHost_SetOfficialCustomGame{
    0x141F1B860};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHost_SetPregameState{
    0x141F1B8B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostSetPrivateSession{
    0x141F166E0, 0x1404C2A00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyHostStart{0x141F16750,
                                                               0x1404C2A70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyInfoProbe{0x141F1C3C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyInRecovery{0x141F1D740,
                                                                0x1404C6410};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyIsPlayerInLobby{
    0x141F169C0, 0x1404C2CE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyIsPlayerInLobbyByGamertag{
    0x141F16A40, 0x1404C2D60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyIsPlayerInLobbyByXuid{
    0x141F16AD0, 0x1404C2DF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyJoinAdd{0x141F1C430};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyJoinBegin{0x141F1C5B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyJoinCount{0x141F1C690};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyJoinFinalize{0x141F1C6E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyLaunchClear{0x0,
                                                                 0x1404C2E70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyLaunchGetServerStatus{
    0x141F16B60, 0x1404C2E80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyLocalClientLeave{
    0x141F16BD0, 0x1404C2EF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyOnlineCancel{0x141F16D10,
                                                                  0x1404C3030};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyRemoveAllLocalSplitscreenClient{0x141F16D60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyServerListGetClientList{
    0x141F133C0, 0x1404BFA60};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyServerListGetMainModeFilter{0x141F13510, 0x1404BFBB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyServerListGetServers{
    0x141F13550, 0x1404BFBF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyServerListJoinServer{
    0x141F13630, 0x1404BFCD0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_LobbyServerListSetMainModeFilter{0x141F136B0, 0x1404BFD50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyServerRefreshLocalServers{
    0x141F136F0, 0x1404BFD90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyUpdateDemoInformation{
    0x141F13190, 0x1404BF6C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LobbyVM_CallFunc{0x141F16E20,
                                                                 0x1404C3120};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Localize{0x141FE9110,
                                                         0x1404D97E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LocalizeRefExists{0x141FE9120,
                                                                  0x1404D97F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LocalizeWithoutLocsMarkers{
    0x141FE91C0, 0x1404D9890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LockInput{0x141FE91D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LootFailureReason{0x141F21240};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LootResultsReady{0x141F21260};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LUICoordsFromUI3DCoords{
    0x141FE8C90, 0x1404D93F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_LuiVM_Event{0x141F17090};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_MakeStableStatsBufferForController{0x141FD6380,
                                                       0x1404CDDF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_MapVoteSelect{0x141F1D4E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_MarkPSDataDirty{0x141F1D930};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_MediaManagerGetQuota{
    0x141FCD5D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_MigrateToBestGameHost{
    0x141F1D780};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Milliseconds{0x141FEBAC0,
                                                             0x1404DB990};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_MillisecondsUInt64{0x141FEBB00,
                                                                   0x1404DB9D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_InstalledUGC{0x141FE9300};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_InstallProgressUGC{
    0x0, 0x1404D98A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_IsSubscribedItem{
    0x141FE9360};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_Lists_GetInfoEntries{
    0x141FE93C0, 0x1404D9950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_Lists_GetInfoEntriesCount{
    0x141FE9480, 0x1404D9A10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_Lists_IsInstalled{
    0x141FE94F0, 0x1404D9A80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_Lists_Update{0x141FE95C0,
                                                                  0x1404D9B50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_Lists_UpdateMods{
    0x141FE95D0, 0x1404D9B60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_Lists_UpdateUsermaps{
    0x141FE95E0, 0x1404D9B70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_Load_Updating{0x141FE95F0,
                                                                   0x1404D9B80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_SetMod{0x141FE9630,
                                                            0x1404D9BC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Mods_SubscribeUGC{0x141FE9660,
                                                                  0x1404D98F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_MutePlayer{0x141F20370};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_MutePlayerByXuid{0x141FC1B60,
                                                                 0x1404CAC40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_NotifyPsPlus{0x141FD2960};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_NotifyPsPlusAsyncMultiplay{
    0x141FD2970};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_NumberToUInt64{0x141F17330,
                                                               0x1404C3390};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ObjectiveGetTeamUsingCount{
    0x141F8B670};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ObjectiveIsAnyOtherTeamUsing{
    0x141F8CFA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ObjectiveIsPlayerUsing{
    0x141F8E8D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ObjectiveIsTeamUsing{
    0x141F8E9A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ObjectivesGetPlayerUsing{
    0x141F902D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_OnlineSetInternalEvent{
    0x141FCD640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_OpenAnticheatPolicyInBrowser{
    0x141FCD6B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_OpenRegistrationBrowser{
    0x141FCD730};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_OpenURL{0x141FD2980};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_OptInToEmailMarketing{
    0x141FCDA20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_OwnSeasonPass{0x141F359F0,
                                                              0x1404C87A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Pause3DCinematic{0x141F91C60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PCacheInvalidatePlayerEmblem{
    0x141F1D9E0, 0x1404C64F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PermanentlyUnlockItem{
    0x141FDF250, 0x1404D4870};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionClearInviteJoinInfo{0x141F1DAE0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionClearPlayTogetherInfo{0x141F1DAF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionGetDebugInfo{
    0x141F1DB00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionIsTaskProgress{
    0x141F1DBF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPCClearData{
    0x141F1DC30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPCGetData{
    0x141F1DC40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPCSetData{
    0x141F1DC50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4ClearData{
    0x141F1DC60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4Create{
    0x141F1DC70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4GetData{
    0x141F1DC90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4GetInviteInfo{
    0x141F1DCA0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionPS4GetSessionInfo{0x141F1DCC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4Invite{
    0x141F1DCE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4Join{
    0x141F1DD00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4Leave{
    0x141F1DD20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4Search{
    0x141F1DD40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4SetData{
    0x141F1DD60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionPS4Update{
    0x141F1DD70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionSetDebugInfo{
    0x141F1DD90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionXboxClearData{
    0x141F1DE70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionXboxCreateJoin{
    0x141F1DE80};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxDisableRTASubscriptions{0x141F1DEA0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxEnableRTASubscriptions{0x141F1DEC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionXboxGetData{
    0x141F1DEE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionXboxInvite{
    0x141F1DEF0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxIsLiveContextForController{0x141F1DF10};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxIsLiveContextForControllerReadyRaw{
        0x141F1DF20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionXboxLeave{
    0x141F1DF30};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxMultiplayerRoundEnd{0x141F1DF50};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxMultiplayerRoundStart{0x141F1DF70};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxSafeEncodeString{0x141F1DF90};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxSecIdToSessionId{0x141F1DFA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionXboxSetActivity{
    0x141F1DFB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSessionXboxSetData{
    0x141F1DFD0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxSetJoinRestriction{0x141F1DFE0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxSetMaxMembers{0x141F1E000};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxSetSessionClosed{0x141F1E020};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxSetSessionVisibility{0x141F1E040};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxSubscribeToRTA{0x141F1E060};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxSubscriptionsEnabled{0x141F1E0A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxUnsubscribeFromRTA{0x141F1E0C0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PlatformSessionXboxXBLiveSessionClear{0x141F1E0E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlatformSupportsAutoFill{
    0x141FCDA60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlayerHasAnyWeapons{
    0x141F91CA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlayerRequestedAutoFill{
    0x141FCDA80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlayMenuMusic{0x141FE96C0,
                                                              0x1404D9BF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlayRumble{0x141FE9710,
                                                           0x1404D9C40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PlaySound{0x141FE9810,
                                                          0x1404DA790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PopAnticheatMessage{
    0x141FCDA90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrefetchByFileName{0x141FCDB90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PreserveThumbnails{0x141FCDBA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrintError{0x141FE98A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrintInfo{0x141FE98E0,
                                                          0x1404D9D40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrintWarning{0x141FE9920};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrivilegeForceCheck{
    0x141FD29C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrivilegeGet{0x141FD29D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrivilegeIsChecked{0x141FD29F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrivilegeIsCheckInProgress{
    0x141FD29E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PrivilegeIsInDisplayInfoCache{
    0x141FD2A00};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_ProbationCheckForDashboardWarning{0x141FCDBF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProbationCheckForProbation{
    0x141FCDCD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProbationCheckForQuitWarning{
    0x141FCDDB0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_ProbationCheckIfPenalizedForQuit{0x141FCDE90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProbationCheckInProbation{
    0x141FCDF30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProcessingAutoFill{0x141FCE000};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProcessingJoin{0x141F1C720};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProfileBool{0x141FE9960,
                                                            0x1404D9D80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProfileFloat{0x141FE99F0,
                                                             0x1404D9E10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProfileInt{0x141FE9A70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProfileString{0x141FE9B00,
                                                              0x1404D9F20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ProfileValueAsString{
    0x141FE9BB0, 0x1404D9FD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PromoteGroupAdminToOwner{
    0x141F37D30};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_PromoteSelectedGroupMemberToAdmin{0x141F37DD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PromoteToHostDone{0x141F1D7C0,
                                                                  0x1404C6460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PublishClip{0x141FCE010};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PublishCustomGame{0x141FCE0F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PublishEmblem{0x141FCE240};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PublishFileToGroupsShowcase{
    0x141F37E80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PublishPaintjob{0x141FCE330};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PublishPaintjobToSteamWorkshop{
    0x141FCE480};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PublishScreenshot{0x141FCE6A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PublishVariant{0x141FCE790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PurchaseDWSKU{0x141FAF2D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PurchaseDWSKUMultiple{
    0x141FAF420};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PurchaseItem{0x141FDF300,
                                                             0x1404D4920};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PurchaseMTX{0x141F35A90,
                                                            0x1404C8840};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PurchaseProductByID{
    0x141FCE900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PurchaseWeaponContract{
    0x141FAF720};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_PushAnticheatMessageToUI{
    0x141FCE9E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_QoSErrorShutdown{0x141F1CB70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_QoSProbeInitiate{0x141F1CBB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_QoSProbeListenerEnable{
    0x141F1CFD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_QoSProbeListenerUpdate{
    0x141F1D040};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ReadMessages{0x141FE9C60,
                                                             0x1404DA080};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ReadOtherPlayerStats{
    0x141FCEB10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RebootGame{0x141FE9CD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RecordComScoreEvent{
    0x141FCEB90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RecordContentAvailable{
    0x141FCEF30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RecordDailyChallengeAbandoned{
    0x141FCEF40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RecordDailyChallengeCompleted{
    0x141FCF0D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RecordKVS{0x141F17390};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RecordOffensiveEmblem{
    0x141FCF230};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RecordStoreSessionEntry{
    0x141FCF3A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RefetchRegulation{0x141FCF430};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RefetchUserRegulationPreference{
    0x141FCF470};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RegisterClientUIModelField{
    0x141FAFE00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RejectGroupInvite{0x141F37ED0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RejectSelectedGroupApplication{
    0x141F37F20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RemoveFileFromGroupsShowcase{
    0x141F37F80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RemoveItemFromInventory{
    0x141FAF790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RemoveKeyCatcherSubscriber{
    0x141FE9CE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RemoveLobbyClientExtracam{
    0x141FC1C20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ReportItemUsageTime{
    0x141FAF820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ReportMarketingMessageViewed{
    0x141FCF4B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ReportUser{0x141FCF520};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RequestLeaderboardData{
    0x141FCF5E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RequestMultiLeaderboardData{
    0x141FCF6B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ResetDvar{0x141FE9D20,
                                                          0x1404DA0F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ResetHardwareProfileSettings{
    0x141FD2A10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ResetThumbnail{0x141FCF990};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RestrictVoiceChatForClient{
    0x141F1B8F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RetryCryptokeysDue{0x141F212C0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_RollbackArenaPregameMatchHistory{0x141FCF9B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RunPlaylistRules{0x141F1EC10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_RunPlaylistSettings{
    0x141F1EC50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SaveAndUploadClip{0x0,
                                                                  0x1404BF9A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SaveGame{0x141FE9D60,
                                                         0x1404DA130};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SaveScreenshot{0x141F13300,
                                                               0x1404BF640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SearchForLobby{0x141F1D100};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SearchGroups{0x141F37FF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SearchPlayerByGamertag{
    0x141F38050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Seconds{0x141FEBB30,
                                                        0x1404DBA00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SecondsAsTime{0x141FE9D80,
                                                              0x1404DA150};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SelectGesture{0x141FDF3B0,
                                                              0x1404D49D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SelectTaunt{0x141FDF490,
                                                            0x1404D4AB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SelectWeapon{0x141F91CC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendApplicationToSelectedGroup{
    0x141F380C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendAttachmentVote{0x141F1B940};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendCharacterDraftSelection{
    0x141F1BA00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendClientHeartbeat{
    0x141F17430, 0x1404C33F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendClientScriptEntityNotify{
    0x141F91D40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendClientScriptNotify{
    0x141F936E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendDLCMenusViewedRecordEvent{
    0x141FCFA00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendDSPPromotionRecordEvent{
    0x141FCFB70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendGroupBatchInvites{
    0x141F38100};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendGroupInvite{0x141F38150};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendInviteByXuid{0x141FCFCA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendItemAttachmentVote{
    0x141F1BA90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendItemGroupAttachmentVote{
    0x141F1BB70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendItemRestrictionVote{
    0x141F1BC50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendItemVote{0x141F1BC60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendMenuResponse{0x141F93860};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendPregameEditComplete{
    0x141F1BD20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendRecordEvent{0x141FCFD40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendSPReminderRecordEvent{
    0x141FCFE00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SendVotingRecordEvent{
    0x141FCFF60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SerializedAdrToLua{0x141FD2A80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionModeIsMode{0x141FD0100};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionMode_IsOnlineGame{
    0x141FE9E10, 0x1404DA1E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionMode_IsPublicOnlineGame{
    0x141FE9E50, 0x1404DA220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionMode_IsSystemlinkGame{
    0x141FE9E90, 0x1404DA260};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionModeSetOffline{
    0x141FD0170};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionModeSetOnlineGame{
    0x141FD01A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionModeSetPrivate{
    0x141FD01E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionModeSetSystemLink{
    0x141FD0220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SessionModeSetTheater{
    0x141FD0260};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetAccountLinkViewed{
    0x141FD02A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetActiveMenu{0x141FE9ED0,
                                                              0x1404DA2A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetACVItemOld{0x141FDF570,
                                                              0x1404D4B90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetArenaPregameMatchHistory{
    0x141FD0320};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetAttachmentAsOld{0x141FDF650,
                                                                   0x1404D4C70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetAttachmentRestrictionState{
    0x141FC1C40, 0x1404CAD00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetBGBTokensGained{0x141FDF730};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetBGBTokensUsed{0x141FDF7A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetBubbleGumBuff{0x141FDF810};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetBubbleGumPackName{
    0x141FDF8F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetBunkCollectible{0x141FD63E0,
                                                                   0x1404CDE50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetChallengeHasBeenSeen{
    0x141FD64E0, 0x1404CDF50};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SetCharacterCustomizationItemAsOld{0x141FDF9A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetClassItem{0x141FDFAA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetClassSetItem{0x141FDFC50,
                                                                0x1404D4D50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetClassSetName{0x141FDFD60,
                                                                0x1404D4E60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetCombatRecordBackgroundId{
    0x141FD0370};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetCombatRecordScreenshotInfo{
    0x141FD0400};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetCurrentClassSetIndex{
    0x141FDFE20, 0x1404D4F20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetCurrentlyOpenedGroup{
    0x141F381E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetCustomElementScrollSpeed{
    0x141FC1CD0, 0x1404CAD90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetDebugTimerInfo{0x141F93950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetDvar{0x141FE9F40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetEmblemBackgroundAsOld{
    0x141FC1D30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetEmblemIconAsOld{0x141FC1DA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetEmblemIconOld{0x141FC1E10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetExtraCamRenderPriority{
    0x141FE9FB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetFakeDLCBits{0x141F1ECB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetFirstSelectedHeroLoadout{
    0x141FDFEB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetFirstTimeComplete{
    0x141FD04A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetForceMouseRootFull{
    0x141FD2D10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGameLobbyStatusInfo{
    0x141F17490, 0x1404C3450};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGametype{0x141FD0530,
                                                            0x1404CC8E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGametypeSetting{0x141FC1E80,
                                                                   0x1404CADF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGender{0x141FDFF90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGestureAsOld{0x141FE0000,
                                                                0x1404D4FB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGroupEmblem{0x141F38200};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGroupEmblemForGroupCreation{
    0x141F38260};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGroupLeaderboard{
    0x141F382A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetGroupLeaderboardInfo{
    0x141F38370};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetHardwareProfileValue{
    0x141FD2D50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetHead{0x141FE0110};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetHero{0x141FE01A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetHeroForCACType{0x141FE0230};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetHeroItem{0x141FE0310};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetHeroLoadoutItem{0x141FE0470};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetHeroLoadoutItemForCACType{
    0x141FE0530};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetHeroShowcaseWeaponAttribute{
    0x141FE0630};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SetHeroShowcaseWeaponVariantName{0x141FE0760};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetItemAsOld{0x141FE0820,
                                                             0x1404D50C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetItemRestrictionState{
    0x141FC1F80, 0x1404CAEA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLanguage{0x141FE9FD0,
                                                            0x1404DA380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLeaderActivity{0x141F174F0,
                                                                  0x1404C34B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLoadingMovieSkippedState{
    0x141F17550};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLobbyMaxClients{0x141F175A0,
                                                                   0x1404C3510};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLobbyMode{0x141F17610,
                                                             0x1404C3580};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLobbyNetworkMode{
    0x141F17690, 0x1404C3600};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLobbyUIScreen{0x141F176D0,
                                                                 0x1404C3640};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLocalClientToInactive{
    0x141F17710, 0x1404C3680};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetLuiKeyCatcher{0x141FEA0A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetModelValue{0x141FAFF10};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetMostRecentPlayedMode{
    0x141FEA100, 0x1404DA450};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetMouseCursor{0x141FD2DC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetObjectiveClipToMap{
    0x141F93960};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetObjectiveColorMaterialInCode{
    0x141F952A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetObjectiveIcon{0x141F96BE0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetObjectiveIconPulse{
    0x141F98630};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetObjectiveIgnoreEntity{
    0x141F99F70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetObjectiveRotateWithEntity{
    0x141F9B8B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetObjectiveStencilToPartialMap{
    0x141F9D1F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetPartyPrivacy{0x141F17790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetPlaylistID{0x141F1ECC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetPlaylistPrevCount{
    0x141F1ED00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetPrimaryGroup{0x141F384B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetPrimaryGroupName{
    0x141F38500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetProfileVar{0x141FEA160,
                                                              0x1404DA4A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetReadyUp{0x141F1D540};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetSceneFilterConstant{
    0x141F9EB30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetSelectedGroup{0x141F38570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetSelectedGroupDescription{
    0x141F385F0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SetSelectedGroupJoinApprovalType{0x141F38660};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetSelectedGroupMessage{
    0x141F386C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetSelectedGroupName{
    0x141F38730};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetSelectedGroupType{
    0x141F387A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetSessionPartyPrivacy{
    0x141F177D0, 0x1404C3740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetSessionStatus{0x141F17830,
                                                                 0x1404C37A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetShadowQuality{0x141FD2E00};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SetStoreCategoryFetchParamaeters{0x141FD0570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetTauntAsOld{0x141FE08D0,
                                                              0x1404D5170};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetTextureQuality{0x141FD2E40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetUI3DWindows{0x141FEA220,
                                                               0x1404DA560};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetUIActive{0x141FEA2B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupAmITalking{0x1420CE290};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupArmBladeTarget{
    0x142018AF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupBleedOutWidget{
    0x142005F30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupBowLauncherReticleShader{
    0x142018C00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupCancelChargeShotHint{
    0x1420CE2A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupClampedShooterWidget{
    0x142006080};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupCompassColors{0x141FEA320};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupCompassItems{0x14206C0D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupCompassMap{0x14206C140};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupCompassOverlay{
    0x14206C1A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupCompassUnderlay{
    0x14206C200};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupCursorHint{0x1420CE2D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupCybercomLockon{
    0x1420061E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupDamageFeedback{
    0x142018C30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupDeadQuote{0x1420CE330};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupDirectionalHitIndicator{
    0x1420CE350};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupEmblemData{0x141FD0700};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupEmblemEditBuffer{
    0x141FD0790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupFieldOpsKitLoadouts{
    0x141FE0AA0, 0x1404D5340};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupFilesharePaintjobData{
    0x141FD0820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupFriendlyArrows{
    0x1420CE380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupGameMessages{0x14206CB50};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SetupGuidedMissileTargetHighlights{0x1420CE3B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupHoldBreathHint{
    0x1420CE3D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupHoldBreathZoomHint{
    0x1420CE400};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupIncomingExplosive{
    0x1420062F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupInvalidCmdHint{
    0x1420CE430};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupLockonHighlights{
    0x1420CE460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupLowHealthOverlay{
    0x1420CE490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupMantleHint{0x1420CE4C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupMeleeChargeHint{
    0x1420CE4F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupMeleeCounterHint{
    0x1420CE520};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupObjectiveIndicators3D{
    0x1420CE550};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupPaintjobData{0x141FD0870};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupPlayerGrappleTargetting{
    0x1420CE5B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupPlayerMarkedTarget{
    0x1420CE5E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupPlayerPivotTargetting{
    0x1420CE600};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupPlayerStance{0x1420CE630};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_setupPlayerTargetHighlightsLineOfSight_Enemy{0x1420CE660};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupReticle{0x142018C60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupRocketLauncherTarget{
    0x142018CF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupTacticalWidget{
    0x142006440};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupTargetHighlights_Enemy{
    0x1420CE690};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupTargetHighlights_Friendly{
    0x1420CE6C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupTransferData{0x141FD0920};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupUI3DWindow{0x141FEA360,
                                                                0x1404DA5F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetupWeakpointIndicator{
    0x142006610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetUserRegulationPreference{
    0x141FD0610};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetViewport{0x141FA0460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetVolumetricQuality{
    0x141FD2E80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SetWeaponOptionAsOld{
    0x141FE09E0, 0x1404D5280};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShouldPresentRegistration{
    0x141FD09F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShouldShowMOTD{0x141FC1FE0,
                                                               0x1404CAF00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShouldShowSPReminder{
    0x141FC2040, 0x1404CAF60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShouldShowVoting{0x141FC20A0,
                                                                 0x1404CAFC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShouldSwapCircleAndCross{
    0x141FEA470};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShowAccountPicker{0x141FD0AA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShowGamerCard{0x141FD0AB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShowGamerCardForXUID{
    0x141FD0AC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShowPlatformFriendRequest{
    0x141F17890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShowPlatformProfile{
    0x141F17930};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShowProductDetailsByID{
    0x141FD0AD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShowPsStoreIcon{0x141FD0BB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ShowRedeemCode{0x141FD0BC0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SigninLocalClient{0x141F17AD0,
                                                                  0x1404C38C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SignIntoPlatformLiveSystem{
    0x141FD0C00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SignOutAllUsers{0x141F179D0,
                                                                0x1404C3800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SignUserIntoLive{0x141F17A10,
                                                                 0x1404C3820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SignUserOutOfLive{0x141F17A90,
                                                                  0x1404C3880};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SkipMTXItem{0x141F35AD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SnapshotDeltaTime{0x141FA0600};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SortFriends{0x141FD3ED0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SortItemsForCombatRecord{
    0x141FD0C20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SpendZMVials{0x141F21530};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SplitscreenNum{0x141FEA490,
                                                               0x1404DA700};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SplitscreenShouldBeOnline{
    0x141F17CA0, 0x1404C3A90};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Start3DCinematic{0x141FA1EA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StartLoadingCinematic{
    0x141FA1FD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StartPrivateLobbyMigrateTo{
    0x141F1D1C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StartServer{0x141FEA4D0,
                                                            0x1404DA740};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SteamIsFamilySharing{
    0x141FD0D10};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_AddFavoriteServer{0x141F1F500};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SteamServerBrowser_AddFilter{
    0x141F1F570};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SteamServerBrowser_ClearFilter{
    0x141F1F600};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_GetCurrentServerRequestType{0x141F1F640};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_GetPlayerCount{0x141F1F680};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_GetPlayerInfo{0x141F1F6C0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_GetServerCount{0x141F1F750};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_GetServerInfo{0x141F1F790};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SteamServerBrowser_GetServers{
    0x141F1F810};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_IsAnyAttributeFilterActive{0x141F1F8E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_IsAnyFilterActive{0x141F1F920};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_IsAttributeFilterActive{0x141F1F980};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_IsFilterActive{0x141F1F9E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_IsRequestingServers{0x141F1FA70};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_RefreshServer{0x141F1FAC0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_RemoveFavoriteServer{0x141F1FB00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SteamServerBrowser_RemoveFilter{
    0x141F1FB70};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_RequestPlayersInfo{0x141F1FBE0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_RequestServers{0x141F1FC20};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_SteamServerBrowser_SetAttributeFilter{0x141F1FC60};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SteamServerBrowser_Sort{
    0x141F1FCD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SteamStore{0x141FD0D50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SteamVacBanned{0x141FD0DD0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Stop3DCinematic{0x141FA20B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StopEditingPresetClass{
    0x0, 0x1404CB020};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StopSound{0x141FEA520};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StorageCreateScratch{
    0x141FD0E20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StorageGetBuffer{0x141FD0ED0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StorageGetScratchBuffer{
    0x141FD0F80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StorageIsFileReady{0x141FD0FF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StorageReset{0x141FD10C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StorageWrite{0x141FD1170};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StorePlaylistNextPrev{
    0x141F1ED40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StreamForceImage{0x141FA2110};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StreamSetCustomClass{
    0x141FA2180};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StreamStopForcingImage{
    0x141FA21F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StreamTouchWeaponTextures{
    0x141FA2230};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StringIntegerAddition{
    0x141FEA5B0, 0x1404DA820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StringIntegerDivision{
    0x141FEA660, 0x1404DA8D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StringIntegerMultiplication{
    0x141FEA700, 0x1404DA970};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StringIntegerSubtraction{
    0x141FEA7C0, 0x1404DAA30};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StringToUInt64{0x141F17D00,
                                                               0x1404C3AF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StringToXUIDDecimal{
    0x141FEA880, 0x1404DAAF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StructTableLookupNumber{
    0x141FEA8D0, 0x1404DAB40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StructTableLookupString{
    0x141FEAA40, 0x1404DACB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StructTableRowCount{
    0x0, 0x1404DAE40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_StructuredTableHasColumn{
    0x141FEAC30, 0x1404DAEA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SubmitPollVote{0x141FD1220};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SwitchCampaignMode{0x141FEADA0,
                                                                   0x1404DB010};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SwitchDemoCameraMode{
    0x141F13340, 0x1404BF540};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SwitchDemoFreeCameraMode{
    0x141F13380, 0x1404BF9E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SwitchMode{0x141FEADE0,
                                                           0x1404DB050};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SwitchPlaylists{0x141F1EDB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SwitchToInventoryWeapon{
    0x141FA2270};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SystemNeedsUpdate{0x141FEAEA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SystemTime{0x141FEAF30,
                                                           0x1404DB0F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_SystemTimeUInt64{0x141FEAFC0,
                                                                 0x1404DB180};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TableFindRows{0x141FEAFF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TableGetColumnValueForRow{
    0x141FEB1A0, 0x1404DB1B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TableLookup{0x141FEB290,
                                                            0x1404DB2A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TableLookupColumnNumForValue{
    0x141FEB590, 0x1404DB5A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TableLookupGetColumnValueForRow{
    0x141FEB670, 0x1404DB680};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Team{0x141FC2110, 0x1404CB030};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TeamSelection{0x141F201C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TEMPGameModeSetMode{
    0x141F17D50, 0x1404C3B40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ToggleGroupAdminPrivilege{
    0x141F38800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ToggleMuteAllButParty{
    0x141F1C8B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TogglePlayerMute{0x141FD13F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_ToUpper{0x0, 0x1404DB770};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Twitch_Get{0x141FD1460};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Twitch_Set{0x141FD1480};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_TwitterPost{0x141FD1490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UCDLoginAttempt{0x141FD14A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UCDSupportsAutoFill{
    0x141FD14B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UI_CollectGarbage{0x141F33300};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UInt64ToString{0x141F17D90,
                                                               0x1404C3B80};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UnBanGroupMember{0x141F38890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UndefinedDvarIndex{0x141F108F0,
                                                                   0x1404BD240};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_UnfavoriteFileFromGroupsShowcase{0x141F38900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UnfeatureFileFromGroupsShowcase{
    0x141F38970};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UnMutePlayer{0x141F203E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UnpackRgba{0x141FEB7D0,
                                                           0x1404DB7E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UnsubscribeAndFreeModel{
    0x141FB0120};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateCalloutPlayerData{
    0x141FA22D0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateColorSet{0x141FEB890};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateFeaturedCardsCRMMessage{
    0x141FC3AB0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateFriends{0x141FEB960,
                                                              0x1404DB8A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateKillcamUIModels{
    0x141FA3D70};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateMessages{0x141FEB9C0,
                                                               0x1404DB900};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_updateModPreviewImage{
    0x141FD2F00};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateSelectedGroupProfile{
    0x141F38A20};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateStatsForQuit{0x141FD14D0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_UpdateStatsForQuitUnbalancedTeam{0x141FD1590};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UpdateTopPlayerUIModels{
    0x141FA5690};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UploadEmblemData{0x141FD1650};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UploadGameSettings{0x141FD16B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UploadPaintjobData{0x141FD1710};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Url_Load_Destroy{0x141FD17A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Url_Load_Init{0x141FD17B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Url_Load_IsDone{0x141FD17C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_Url_Load_Jpeg{0x141FD17E0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_Url_Load_MeasureDownloadBandwidth{0x141FD1800};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UserData{0x141FEBA00,
                                                         0x1404DB940};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UserTestBuildPauseEvent{
    0x141FEBA50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UsingModsInternalName{
    0x141FD2F40};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UsingModsUgcName{0x141FD2FA0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UsingModsVersion{0x141FD3000};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_UsingUsermapTitle{0x141FD3040};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_usingUsermapUgcName{
    0x141FD30A0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WeaponGroupHasNewItem{
    0x141FE0AE0, 0x1404D5380};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WeaponGroupNewItemCount{
    0x141FE0BF0, 0x1404D5490};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WeaponOptionNewItemCount{
    0x141FE0D00, 0x1404D55A0};
WEAK symbol<hks::lua_CFunction>
    Lua_CoD_LuaCall_WeaponOptionNewModeAgnosticItemCount{0x141FE0DE0,
                                                         0x1404D5680};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_camera_Disable{
    0x141FD18E0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_camera_Enable{0x141FD18F0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_camera_IsAvailable{
    0x141FD1910};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_camera_IsEnabled{
    0x141FD1930};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_Clear{0x141FD1820};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_Close{0x141FD1830};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_CurrentFrameNum{
    0x141FD1840};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_IsBuffered{0x141FD1860};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_Open{0x141FD1880};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_Play{0x141FD18B0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_WebM_Time{0x141FD18C0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_XuidIsValid{0x141F17E00,
                                                            0x1404C3BF0};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_XUIDToGamertag{0x141FD3F50};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_YouTube_Get{0x141FD1950};
WEAK symbol<hks::lua_CFunction> Lua_CoD_LuaCall_YouTube_Set{0x141FD1970};
WEAK symbol<EngineDependentLuaEngineFunction> LuaEngineFunctionListTail{
    0x159C76D80, 0x149114E60};
} // namespace api
} // namespace cod
} // namespace lua
} // namespace game