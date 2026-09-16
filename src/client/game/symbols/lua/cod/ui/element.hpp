#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
namespace lua {
namespace cod {
namespace api {
namespace ui {
namespace element {

WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_AddElementAfterInC{
    0x142706AF0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_AddElementBeforeInC{
    0x142706C30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_AddElementToC{0x142706D70};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_AnimateToState{
    0x142706F10};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_ApplyElementTransform{
    0x1427072B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_BeginAnimation{
    0x142707330};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_ClearEntityMidpoint{
    0x142815390};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_CloseElementInC{
    0x142707560};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_CompleteAnimation{
    0x1427075F0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_DisablePlayerBackgroundIdUpdate{0x1428153F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_FollowActiveLink{
    0x142815420};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GC{0x1426FE2D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetAlignment{0x1427076A0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetAnimationFraction{
    0x142707750};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetFirstChild{0x142707840};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetGlobalTextWidth{
    0x1427078E0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetHeight{0x1427079D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetImageDimensions{
    0x142707A70};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetImageName{0x142707B70};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetLastChild{0x142707C50};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetLocalLeftRight{
    0x142707CF0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetLocalRect{0x142707DC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetLocalTopBottom{
    0x142707E90};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetModel{0x142707F60};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetNextRect{0x142708080};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetNextSibling{
    0x142708160};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetNumChildren{
    0x142708200};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetParent{0x1427082B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetPreviousSibling{
    0x142708350};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetPriority{0x1427083F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetRect{0x142708490};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetScale{0x1427085B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetShaderVector2{
    0x142708660};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetTextFromC{0x142708740};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetTextHeightForWidth{
    0x142708810};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetTextWidth{0x142708920};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetTextWidthAndHeight{
    0x142708A10};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetUseStencil{0x142708B80};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GetWidth{0x142708C20};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_GotoUrl{0x142815450};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_HasHTMLContent{
    0x142708CC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_HideCursor{0x1428154B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_HomeLink{0x1428154D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_IsBiDiText{0x142708D70};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_IsContextualizedBiDiText{
    0x142708E20};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_IsInFocus{0x142708ED0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_IsVisible{0x142708F80};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_NextLink{0x142815500};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_PreviousLink{0x142815530};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RegisterAnimationState{
    0x142709020};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RemoveAllChildren{
    0x1427091B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RemoveElement{0x142709230};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RemoveSubscription{
    0x142709320};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_ReverseAndContextualizeIfBiDiText{0x1427096B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_RootPixelsToUnits{
    0x1427069D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_ScrollPage{0x1428155A0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetAlignment{0x1427097F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetAlpha{0x142709900};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetBlur{0x1427099F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetBottomPt{0x142709B00};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetBottomPx{0x142709BC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetEntityContainerClamp{
    0x1428155F0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerFadeWhenInCombat{0x142815650};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerFadeWhenTargeted{0x1428156B0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerHideOutRange{0x142815710};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetEntityContainerScale{
    0x142815770};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerScaleClamp{0x1428157D0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetEntityContainerStopUpdating{0x142815830};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetFocus{0x142709C80};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetFont{0x142709D30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetGraphMode{0x142815890};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetHeight{0x142709E10};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetImage{0x142709F40};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLayoutCached{
    0x14270A0D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLeftPt{0x14270A180};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLeftPx{0x14270A240};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLeftRight{0x14270A300};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLetterSpacing{
    0x14270A4F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetLineSpacing{
    0x14270A5F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetMaterial{0x14270A6E0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetModelInC{0x14270A7B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetPriority{0x14270A880};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRFTMaterial{
    0x14270A9F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRGB{0x14270AB60};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRightPt{0x14270AD40};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRightPx{0x14270AE00};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetRoot{0x14270AEC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetScale{0x14270AED0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetShaderVector{
    0x14270AFC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetSpacing{0x14270B450};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTextInC{0x14270B620};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTileHorizontally{
    0x142815990};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTileVertically{
    0x1428159F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTopBottom{0x14270B6F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTopPt{0x14270B8F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTopPx{0x14270B9B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetTTF{0x14270B510};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetUI3DWindow{0x14270BA70};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_Setup3dPointer{
    0x142815A50};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupADSContainer{
    0x142815AC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupAltitudeValue{
    0x142815AF0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupAmITalking{
    0x1420CE290};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupAmmoTicks{
    0x142815B90};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupArmBladeTarget{
    0x142018AF0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupBinaryHTML{
    0x142815C30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupBitchinFX{
    0x142815D00};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupBleedOutWidget{
    0x142005F30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupBombTimer{
    0x142815E30};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupBowLauncherReticleShader{0x142018C00};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCancelChargeShotHint{
    0x1420CE2A0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupCharacterExtraCamHeadshotRender{0x142815ED0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupCharacterExtraCamRender{0x142815FC0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupCharacterExtraCamRenderForHeroIndex{0x1428160F0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupCharacterExtraCamRenderForLobbyClient{
        0x142816150};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCharacterHeadRender{
    0x1428161A0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCharacterItemRender{
    0x1428162D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCinematicSubtitles{
    0x142816480};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupClampedShooterWidget{
    0x142006080};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupClientStatusImage{
    0x1428164B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCompassItems{
    0x14206C0D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCompassMap{
    0x14206C140};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCompassOverlay{
    0x14206C1A0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCompassUnderlay{
    0x14206C200};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_setupCredit{0x0,
                                                                 0x14045E330};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCredit{0x142816530};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCrosshairDistance{
    0x142816680};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCursorHint{
    0x1420CE2D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupCybercomLockon{
    0x1420061E0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDamageFeedback{
    0x142018C30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDashes{0x1428166C0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDeadQuote{
    0x1420CE330};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDemoControls{
    0x1428167A0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDemoTimeline{
    0x1428167D0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupDirectionalHitIndicator{0x1420CE350};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDistanceIndicator{
    0x142816800};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDoubleJump{
    0x1428169C0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDrawDecalById{
    0x1428169F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupDrawEmblemLayer{
    0x142816A60};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEdgePointer{
    0x142816B30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEllipseShader{
    0x142816BA0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEmblem{0x142816BD0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEmblemByEmblemIndex{
    0x142816C50};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEmblemCopyWidget{
    0x142816D30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEndTimer{0x142816D60};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEntityContainer{
    0x142816DD0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupEntityObjectiveContainer{0x142816EC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupEntityScanning{
    0x142816F40};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupFriendlyArrows{
    0x1420CE380};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGadget{0x142816FB0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGameMessages{
    0x14206CB50};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGameTimer{
    0x142817020};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGameTimerZombie{
    0x142817050};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGeneric3DWidget{
    0x142817080};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGlobe{0x142817130};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupGroupEmblem{
    0x142817170};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupGuidedMissileTargetHighlights{0x1420CE3B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHiddenEmblemLayer{
    0x142817280};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupHighlightReelInformationDisplay{0x142817350};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHoldBreathHint{
    0x1420CE3D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHoldBreathZoomHint{
    0x1420CE400};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHorizontalCompass{
    0x142817380};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupHUDShaker{
    0x1428171D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupImageViewer{
    0x142817410};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupIncomingExplosive{
    0x1420062F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupInteractivePrompt{
    0x1428175B0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupInterPromptHideOutRange{0x1428174F0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupInterPromptObjectiveType{0x142817550};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupInvalidCmdHint{
    0x1420CE430};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupJetpack{0x1428176E0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_setupLeagueEmblem{
    0x0, 0x140072DC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLeagueEmblem{
    0x1428176F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLoadingBar{
    0x142817700};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLoadingStatusText{
    0x142817730};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLockonHighlights{
    0x1420CE460};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupLowHealthOverlay{
    0x1420CE490};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMantleHint{
    0x1420CE4C0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMeleeChargeHint{
    0x1420CE4F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMeleeCounterHint{
    0x1420CE520};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMoviePlayback{
    0x142817770};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupMovingCrosshair{
    0x142817950};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupNineSliceShader{
    0x1428179B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupObjectiveIndicators3D{
    0x1420CE550};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupObjectiveProgress{
    0x142817A50};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupOutfitExtraCamRender{
    0x142817AC0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupPaintjobCustomizationType{0x142817BF0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPaintjobIndex{
    0x142817C70};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPaintjobSlot{
    0x142817D00};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPausableContainer{
    0x142817D90};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPitchLevel{
    0x142817DC0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPitchMeter{
    0x142817E50};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupPlayerBackgroundIdUpdateByXUID{0x142817E90};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerEmblemByXUID{
    0x142817EE0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupPlayerGrappleTargetting{0x1420CE5B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerHealthEKG{
    0x142817F30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerMarkedTarget{
    0x1420CE5E0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerPivotTargetting{
    0x1420CE600};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPlayerStance{
    0x1420CE630};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupPrematchTimer{
    0x142817FD0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupRenderGraph{
    0x1428180F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupReticle{0x142018C60};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupRocketLauncherTarget{
    0x142018CF0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupRollLevel{
    0x142818120};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupRTSPOIBar{
    0x142818080};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupSafeAreaBoundary{
    0x1428181A0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupScrollingMessage{
    0x1428181D0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupScrollingMessageCategory{0x142818200};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupServerTime{
    0x142818230};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupShowIfXUIDTalking{
    0x142818290};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupSpinner{0x1428182E0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupStaffPieces{
    0x142818310};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupSunFlare{0x1428183D0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupTacticalWidget{
    0x142006440};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupTargetHighlights_Enemy{0x1420CE690};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupTargetHighlights_Friendly{0x1420CE6C0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupTiles{0x142818410};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupTwitchStreamPlayback{
    0x142818490};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIElement{
    0x1428185C0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIHorizontalList{
    0x1428185F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIImage{0x142818620};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIStreamedImage{
    0x142818670};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIText{0x1428186F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUITextUncached{
    0x142818720};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUITightText{
    0x142818750};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupUIVerticalList{
    0x142818780};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupVisorImage{
    0x1428187B0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupVoiceMeter{
    0x1428187F0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupVoipImage{
    0x142818840};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupVoipImageByXUID{
    0x1428188C0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupWaypointContainer{
    0x142819170};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupWCFileshareIconExtraCamRender{0x142818920};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupWCPaintjobIconExtraCamRender{0x142818D30};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_SetupWCVariantIconExtraCamRender{0x142818F50};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupWeakpointIndicator{
    0x142006610};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupWiiUReticlePreview{
    0x142819270};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetupXCamMouseControl{
    0x142819280};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetUseGameTime{
    0x14270BB80};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetUserDataFloat{
    0x14270BD30};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetUseStencil{0x14270BC80};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetWidth{0x14270BDF0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetXRot{0x14270BF20};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetYRot{0x14270C000};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetZoom{0x14270C1C0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SetZRot{0x14270C0E0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_SubscribeToModel{
    0x14270C2A0};
WEAK symbol<hks::lua_CFunction> UI_LuaCall_UIElement_UndoElementTransform{
    0x14270C5A0};
WEAK symbol<hks::lua_CFunction>
    UI_LuaCall_UIElement_UnsubscribeFromAllModelsInC{0x14270C620};

WEAK symbol<LuaUIElementFunction *> LuaUIElementFunctionListTail{0x159C76D80};

} // namespace element
} // namespace ui
} // namespace api
} // namespace cod
} // namespace lua
} // namespace game