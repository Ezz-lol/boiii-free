#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace cod {
namespace api {
namespace ui {
namespace element {

WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_AddElementAfterInC{
    0x14268D980, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_AddElementBeforeInC{
    0x14268DAC0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_AddElementToC{0x14268DC00,
                                                                   0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_AnimateToState{0x14268DDA0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_ApplyElementTransform{
    0x142693430, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_BeginAnimation{0x14268E1C0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_ClearEntityMidpoint{
    0x14279C220, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_CloseElementInC{
    0x14268E3F0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_CompleteAnimation{
    0x14268E480, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_DisablePlayerBackgroundIdUpdate{0x14279C280, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_FollowActiveLink{
    0x14279C360, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GC{0x142685160, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetAlignment{0x14268E530,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetAnimationFraction{
    0x14268E5E0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetFirstChild{0x14268E6D0,
                                                                   0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetGlobalTextWidth{
    0x14268E770, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetHeight{0x14268E860,
                                                               0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetImageDimensions{
    0x14268E900, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetImageName{0x14268EA00,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetLastChild{0x14268EAE0,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetLocalLeftRight{
    0x14268ED20, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetLocalRect{0x14268EC50,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetLocalTopBottom{
    0x14268EB80, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetModel{0x14268EDF0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetNextRect{0x14268EF10,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetNextSibling{0x14268EFF0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetNumChildren{0x14268F090,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetParent{0x14268F140,
                                                               0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetPreviousSibling{
    0x14268F1E0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetPriority{0x14268F280,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetRect{0x14268F320, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetScale{0x14268F440, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetShaderVector2{
    0x14268F4F0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetTextFromC{0x14268F5D0,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetTextHeightForWidth{
    0x14268F6A0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetTextWidth{0x14268F7B0,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetTextWidthAndHeight{
    0x14268F8A0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetUseStencil{0x14268FA10,
                                                                   0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetWidth{0x14268FAB0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GotoUrl{0x14279C2E0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_HasHTMLContent{0x14268FB50,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_HideCursor{0x14279C340,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_HomeLink{0x14279C2B0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_IsBiDiText{0x14268FC00,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_IsContextualizedBiDiText{
    0x14268FCB0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_IsInFocus{0x14268FD60,
                                                               0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_IsVisible{0x14268FE10,
                                                               0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_NextLink{0x14279C390, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_PreviousLink{0x14279C3C0,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RegisterAnimationState{
    0x14268FEB0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RemoveAllChildren{
    0x142690040, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RemoveElement{0x1426900C0,
                                                                   0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RemoveSubscription{
    0x1426901B0, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_ReverseAndContextualizeIfBiDiText{0x142690540, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RootPixelsToUnits{
    0x14268D860, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_ScrollPage{0x14279C430,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetAlignment{0x142690680,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetAlpha{0x142690790, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetBlur{0x142690880, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetBottomPt{0x142690990,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetBottomPx{0x142690A50,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetEntityContainerClamp{
    0x14279C480, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerFadeWhenInCombat{0x14279C4E0, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerFadeWhenTargeted{0x14279C540, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerHideOutRange{0x14279C5A0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetEntityContainerScale{
    0x14279C600, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerScaleClamp{0x14279C660, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerStopUpdating{0x14279C6C0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetFocus{0x142690B10, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetFont{0x142690BC0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetGraphMode{0x14279C720,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetHeight{0x142690CA0,
                                                               0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetImage{0x142690DD0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLayoutCached{
    0x142690F60, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLeftPt{0x142691010,
                                                               0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLeftPx{0x1426910D0,
                                                               0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLeftRight{0x142691190,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLetterSpacing{
    0x142691380, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLineSpacing{0x142691480,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetMaterial{0x142691570,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetModelInC{0x142691640,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetPriority{0x142691710,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRFTMaterial{0x142691880,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRGB{0x1426919F0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRightPt{0x142691BD0,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRightPx{0x142691C90,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRoot{0x142691D50, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetScale{0x142691D60, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetShaderVector{
    0x142691E50, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetSpacing{0x1426922E0,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTextInC{0x1426924B0,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTileHorizontally{
    0x14279C820, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTileVertically{
    0x14279C880, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTopBottom{0x142692580,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTopPt{0x142692780, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTopPx{0x142692840, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTTF{0x1426923A0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetUI3DWindow{0x142692900,
                                                                   0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_Setup3dPointer{0x14279C8E0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupADSContainer{
    0x14279C950, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupAltitudeValue{
    0x14279C980, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupAmITalking{
    0x1420C1B10, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupAmmoTicks{0x14279CA20,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupArmBladeTarget{
    0x14200C370, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupBinaryHTML{
    0x14279CAC0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupBitchinFX{0x14279CB90,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupBleedOutWidget{
    0x141FF97B0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupBombTimer{0x14279CCC0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupBowLauncherReticleShader{0x14200C480, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCancelChargeShotHint{
    0x1420C1B20, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupCharacterExtraCamHeadshotRender{0x14279CD60, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupCharacterExtraCamRender{0x14279CE50, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupCharacterExtraCamRenderForHeroIndex{0x14279CF80,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupCharacterExtraCamRenderForLobbyClient{0x14279CFE0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCharacterHeadRender{
    0x14279D030, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCharacterItemRender{
    0x14279D160, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCinematicSubtitles{
    0x14279D310, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupClampedShooterWidget{
    0x141FF9900, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupClientStatusImage{
    0x14279D340, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCompassItems{
    0x14205F950, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCompassMap{
    0x14205F9C0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCompassOverlay{
    0x14205FA20, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCompassUnderlay{
    0x14205FA80, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_setupCredit{0x0,
                                                                 0x14045E330};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCredit{0x14279D3C0,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCrosshairDistance{
    0x14279D510, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCursorHint{
    0x1420C1B50, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCybercomLockon{
    0x141FF9A60, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDamageFeedback{
    0x14200C4B0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDashes{0x14279D550,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDeadQuote{0x1420C1BB0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDemoControls{
    0x14279D630, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDemoTimeline{
    0x14279D660, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupDirectionalHitIndicator{0x1420C1BD0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDistanceIndicator{
    0x14279D690, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDoubleJump{
    0x14279D850, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDrawDecalById{
    0x14279D880, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDrawEmblemLayer{
    0x14279D8F0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEdgePointer{
    0x14279D9C0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEllipseShader{
    0x14279DA30, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEmblem{0x14279DA60,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEmblemByEmblemIndex{
    0x14279DAE0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEmblemCopyWidget{
    0x14279DBC0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEndTimer{0x14279DBF0,
                                                                   0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEntityContainer{
    0x14279DC60, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupEntityObjectiveContainer{0x14279DD50, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEntityScanning{
    0x14279DDD0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupFriendlyArrows{
    0x1420C1C00, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGadget{0x14279DE40,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGameMessages{
    0x1420603D0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGameTimer{0x14279DEB0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGameTimerZombie{
    0x14279DEE0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGeneric3DWidget{
    0x14279DF10, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGlobe{0x14279DFC0,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGroupEmblem{
    0x14279E000, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupGuidedMissileTargetHighlights{0x1420C1C30, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHiddenEmblemLayer{
    0x14279E110, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupHighlightReelInformationDisplay{0x14279E1E0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHoldBreathHint{
    0x1420C1C50, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHoldBreathZoomHint{
    0x1420C1C80, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHorizontalCompass{
    0x14279E210, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHUDShaker{0x14279E060,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupImageViewer{
    0x14279E2A0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupIncomingExplosive{
    0x141FF9B70, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupInteractivePrompt{
    0x14279E440, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupInterPromptHideOutRange{0x14279E380, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupInterPromptObjectiveType{0x14279E3E0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupInvalidCmdHint{
    0x1420C1CB0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupJetpack{0x14279E570,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_setupLeagueEmblem{
    0x0, 0x140072DC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLeagueEmblem{
    0x14279E580, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLoadingBar{
    0x14279E590, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLoadingStatusText{
    0x14279E5C0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLockonHighlights{
    0x1420C1CE0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLowHealthOverlay{
    0x1420C1D10, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMantleHint{
    0x1420C1D40, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMeleeChargeHint{
    0x1420C1D70, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMeleeCounterHint{
    0x1420C1DA0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMoviePlayback{
    0x14279E600, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMovingCrosshair{
    0x14279E7E0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupNineSliceShader{
    0x14279E840, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupObjectiveIndicators3D{
    0x1420C1DD0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupObjectiveProgress{
    0x14279E8E0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupOutfitExtraCamRender{
    0x14279E950, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupPaintjobCustomizationType{0x14279EA80, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPaintjobIndex{
    0x14279EB00, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPaintjobSlot{
    0x14279EB90, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPausableContainer{
    0x14279EC20, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPitchLevel{
    0x14279EC50, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPitchMeter{
    0x14279ECE0, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupPlayerBackgroundIdUpdateByXUID{0x14279ED20, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerEmblemByXUID{
    0x14279ED70, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupPlayerGrappleTargetting{0x1420C1E30, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerHealthEKG{
    0x14279EDC0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerMarkedTarget{
    0x1420C1E60, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerPivotTargetting{
    0x1420C1E80, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerStance{
    0x1420C1EB0, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupPlayerTargetHighlightsLineOfSight_Enemy{
        0x1420C1EE0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPrematchTimer{
    0x14279EE60, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupRenderGraph{
    0x14279EF80, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupReticle{0x14200C4E0,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupRocketLauncherTarget{
    0x14200C570, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupRollLevel{0x14279EFB0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupRTSPOIBar{0x14279EF10,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupSafeAreaBoundary{
    0x14279F030, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupScrollingMessage{
    0x14279F060, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupScrollingMessageCategory{0x14279F090, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupServerTime{
    0x14279F0C0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupShowIfXUIDTalking{
    0x14279F120, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupSpinner{0x14279F170,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupStaffPieces{
    0x14279F1A0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupSunFlare{0x14279F260,
                                                                   0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupTacticalWidget{
    0x141FF9CC0, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupTargetHighlights_Enemy{0x1420C1F10, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupTargetHighlights_Friendly{0x1420C1F40, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupTiles{0x14279F2A0,
                                                                0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupTwitchStreamPlayback{
    0x14279F320, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIElement{0x14279F450,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIHorizontalList{
    0x14279F480, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIImage{0x14279F4B0,
                                                                  0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIStreamedImage{
    0x14279F500, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIText{0x14279F580,
                                                                 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUITextUncached{
    0x14279F5B0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUITightText{
    0x14279F5E0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIVerticalList{
    0x14279F610, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupVisorImage{
    0x14279F640, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupVoiceMeter{
    0x14279F680, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupVoipImage{0x14279F6D0,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupVoipImageByXUID{
    0x14279F750, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupWaypointContainer{
    0x1427A0000, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupWCFileshareIconExtraCamRender{0x14279F7B0, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupWCPaintjobIconExtraCamRender{0x14279FBC0, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupWCVariantIconExtraCamRender{0x14279FDE0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupWeakpointIndicator{
    0x141FF9E90, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupWiiUReticlePreview{
    0x1427A0100, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupXCamMouseControl{
    0x1427A0110, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetUseGameTime{0x142692A10,
                                                                    0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetUserDataFloat{
    0x142692BC0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetUseStencil{0x142692B10,
                                                                   0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetWidth{0x142692C80, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetXRot{0x142692DB0, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetYRot{0x142692E90, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetZoom{0x142693050, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetZRot{0x142692F70, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SubscribeToModel{
    0x142693130, 0x0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_UndoElementTransform{
    0x14268E140, 0x0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_UnsubscribeFromAllModelsInC{0x1426934B0, 0x0};

WEAK symbol<LuaUIElementFunction *> LuaUIElementFunctionListTail{0x159BF7E00,
                                                                 0x0};

} // namespace element
} // namespace ui
} // namespace api
} // namespace cod
} // namespace lua
} // namespace game
