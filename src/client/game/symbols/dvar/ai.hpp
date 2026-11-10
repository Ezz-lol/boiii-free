#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> ai_accuracy_attackercountdecrease{
    0x14A037B20, 0x14737F200};
WEAK symbol<EngineDependentDvarMut> ai_accuracy_attackercountmax{0x14A037B28,
                                                                 0x14737F208};
WEAK symbol<EngineDependentDvarMut> ai_accuracydistscale{0x14A037B18,
                                                         0x14737F1F8};
WEAK symbol<EngineDependentDvarMut> ai_adjustoffmeshnodes{0x14A0379F0,
                                                          0x1471030D0};
WEAK symbol<EngineDependentDvarMut> ai_angularYawAccelRate{0x14A037880,
                                                           0x147836968};
WEAK symbol<EngineDependentDvarMut> ai_angularYawDecelFactor{0x14A037888,
                                                             0x147836970};
WEAK symbol<EngineDependentDvarMut> ai_angularYawEnabled{0x14A037878,
                                                         0x147836960};
WEAK symbol<EngineDependentDvarMut> ai_arrivalLookaheadDistance{0x14A1D4218,
                                                                0x1478366A8};
WEAK symbol<EngineDependentDvarMut> ai_avoidNavBoundaries{0x14A037830,
                                                          0x147836918};
WEAK symbol<EngineDependentDvarMut> ai_avoidOthersAtCloseDist{0x14A037838,
                                                              0x147836920};
WEAK symbol<EngineDependentDvarMut> ai_awarenessenabled{0x14A1D4478,
                                                        0x1474BAAF0};
WEAK symbol<EngineDependentDvarMut> ai_clamptonavmeshenabled{0x14A037A98,
                                                             0x147103178};
WEAK symbol<EngineDependentDvarMut> ai_codegameskill{0x14A037CC0, 0x14737F3A0};
WEAK symbol<EngineDependentDvarMut> ai_corpsecount{0x14A0379C0, 0x1471030A0};
WEAK symbol<EngineDependentDvarMut> ai_coverflankcheckrad{0x14A037B10,
                                                          0x14737F1F0};
WEAK symbol<EngineDependentDvarMut> ai_coverflankdisttocover{0x14A037B08,
                                                             0x14737F1E8};
WEAK symbol<EngineDependentDvarMut> ai_coverflankmaxangle{0x14A037B00,
                                                          0x14737F1E0};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_covertype{0x14A037AD0,
                                                            0x14737F1B0};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_dangerousnode{0x14A037AB8,
                                                                0x147103198};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_distance{0x14A037AB0,
                                                           0x147103190};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_engagement{0x14A037AC0,
                                                             0x1471031A0};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_flanking{0x14A037AF8,
                                                           0x14737F1D8};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_nodeangle{0x14A037AD8,
                                                            0x14737F1B8};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_playerlos{0x14A037AE8,
                                                            0x14737F1C8};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_priority{0x14A037AF0,
                                                           0x14737F1D0};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_targetdir{0x14A037AE0,
                                                            0x14737F1C0};
WEAK symbol<EngineDependentDvarMut> ai_coverscore_visibility{0x14A037AC8,
                                                             0x1471031A8};
WEAK symbol<EngineDependentDvarMut> ai_coversearchinterval{0x14A037AA8,
                                                           0x147103188};
WEAK symbol<EngineDependentDvarMut> ai_debugaccuracy{0x14A037B78, 0x14737F258};
WEAK symbol<EngineDependentDvarMut> ai_debuganimdeltas{0x14A037A88,
                                                       0x147103168};
WEAK symbol<EngineDependentDvarMut> ai_debuganimscripted{0x14A037B38,
                                                         0x14737F218};
WEAK symbol<EngineDependentDvarMut> ai_debugArrivals{0x14A1D4230, 0x1478366C0};
WEAK symbol<EngineDependentDvarMut> ai_debugcodebehaviortree{0x14A037CA0,
                                                             0x14737F380};
WEAK symbol<EngineDependentDvarMut> ai_debugcoverentitynum{0x14A037B40,
                                                           0x14737F220};
WEAK symbol<EngineDependentDvarMut> ai_debugcoverselection{0x14A037B30,
                                                           0x14737F210};
WEAK symbol<EngineDependentDvarMut> ai_debugcovertool{0x14A037A20, 0x147103100};
WEAK symbol<EngineDependentDvarMut> ai_debugentindex{0x14A037B90, 0x14737F270};
WEAK symbol<EngineDependentDvarMut> ai_debugfindpath{0x14A037A30, 0x147103110};
WEAK symbol<EngineDependentDvarMut> ai_debugfindpathdirect{0x14A037A38,
                                                           0x147103118};
WEAK symbol<EngineDependentDvarMut> ai_debugfindpathlock{0x14A037A48,
                                                         0x147103128};
WEAK symbol<EngineDependentDvarMut> ai_debugfindpathwidth{0x14A037A40,
                                                          0x147103120};
WEAK symbol<EngineDependentDvarMut> ai_debugFlyingPath{0x150BA11F0,
                                                       0x148271010};
WEAK symbol<EngineDependentDvarMut> ai_debugmaymove{0x14A037B60, 0x14737F240};
WEAK symbol<EngineDependentDvarMut> ai_debugmeleeattackspots{0x14A037B70,
                                                             0x14737F250};
WEAK symbol<EngineDependentDvarMut> ai_debugtargets{0x14A037B80, 0x14737F260};
WEAK symbol<EngineDependentDvarMut> ai_debugthreatselection{0x14A037B68,
                                                            0x14737F248};
WEAK symbol<EngineDependentDvarMut> ai_debugturnrate{0x14A037A28, 0x147103108};
WEAK symbol<EngineDependentDvarMut> ai_disablespawn{0x14A037A50, 0x147103130};
WEAK symbol<EngineDependentDvarMut> ai_dogmeleeheight{0x14A037C60, 0x14737F340};
WEAK symbol<EngineDependentDvarMut> ai_dogmeleerange{0x14A037C50, 0x14737F330};
WEAK symbol<EngineDependentDvarMut> ai_dogmeleewidth{0x14A037C58, 0x14737F338};
WEAK symbol<EngineDependentDvarMut> ai_donotdamageteammates{0x14A037C48,
                                                            0x14737F328};
WEAK symbol<EngineDependentDvarMut> ai_eventdistbadplace{0x14A037BA8,
                                                         0x14737F288};
WEAK symbol<EngineDependentDvarMut> ai_exitLookaheadDistance{0x14A1D4220,
                                                             0x1478366B0};
WEAK symbol<EngineDependentDvarMut> ai_foliageseethroughdist{0x14A037BF8,
                                                             0x14737F2D8};
WEAK symbol<EngineDependentDvarMut> ai_friendlyfireblockduration{0x14A037C10,
                                                                 0x14737F2F0};
WEAK symbol<EngineDependentDvarMut> ai_friendlysuppression{0x14A037C00,
                                                           0x14737F2E0};
WEAK symbol<EngineDependentDvarMut> ai_friendlysuppressiondist{0x14A037C08,
                                                               0x14737F2E8};
WEAK symbol<EngineDependentDvarMut> ai_generatenavmeshregions{0x150B9E1E8,
                                                              0x14826DC48};
WEAK symbol<EngineDependentDvarMut> ai_instantNoSolidOnDeath{0x158058440, 0x0};
WEAK symbol<EngineDependentDvarMut> aim_autoaim_enabled{0x1435F61A0,
                                                        0x14137D8A0};
WEAK symbol<EngineDependentDvarMut> ai_maxattackercount{0x14A037C68,
                                                        0x14737F348};
WEAK symbol<EngineDependentDvarMut> ai_meleedamage{0x14A037C40, 0x14737F320};
WEAK symbol<EngineDependentDvarMut> ai_meleeHeight{0x14A037C38, 0x14737F318};
WEAK symbol<EngineDependentDvarMut> ai_meleerange{0x14A037C28, 0x14737F308};
WEAK symbol<EngineDependentDvarMut> ai_meleeWidth{0x14A037C30, 0x14737F310};
WEAK symbol<EngineDependentDvarMut> ai_moveOrientMode{0x14A037A58, 0x147103138};
WEAK symbol<EngineDependentDvarMut> aim_target_closest_first{0x1435F61A8,
                                                             0x14137D8A8};
WEAK symbol<EngineDependentDvarMut> ai_navDisplay_Alpha{0x150BA11A0,
                                                        0x148270FC0};
WEAK symbol<EngineDependentDvarMut> ai_navDisplay_FontSize{0x150BA11A8,
                                                           0x148270FC8};
WEAK symbol<EngineDependentDvarMut> ai_navDisplay_X{0x150BA1190, 0x148270FB0};
WEAK symbol<EngineDependentDvarMut> ai_navDisplay_Y{0x150BA1198, 0x148270FB8};
WEAK symbol<EngineDependentDvarMut> ai_navDraw_DepthTest{0x150BA1180,
                                                         0x148270FA0};
WEAK symbol<EngineDependentDvarMut> ai_navDraw_Height{0x150BA1188, 0x148270FA8};
WEAK symbol<EngineDependentDvarMut> ai_navVolume_Cave{0x150BA11E8, 0x148271008};
WEAK symbol<EngineDependentDvarMut> ai_navVolume_fillCells{0x150BA11C0,
                                                           0x148270FE0};
WEAK symbol<EngineDependentDvarMut> ai_navVolume_labelCells{0x150BA11E0,
                                                            0x148271000};
WEAK symbol<EngineDependentDvarMut> ai_navVolumeLayerIndex{0x150BA1200,
                                                           0x148271020};
WEAK symbol<EngineDependentDvarMut> ai_navVolume_showCellBorders{0x150BA11D0,
                                                                 0x148270FF0};
WEAK symbol<EngineDependentDvarMut> ai_navVolume_showEdgeConnections{
    0x150BA11D8, 0x148270FF8};
WEAK symbol<EngineDependentDvarMut> ai_navVoume_colorCellsRandomly{0x150BA11C8,
                                                                   0x148270FE8};
WEAK symbol<EngineDependentDvarMut> ai_nododge{0x14A037B50, 0x14737F230};
WEAK symbol<EngineDependentDvarMut> ai_pathchokepointcost{0x14A037A70,
                                                          0x147103150};
WEAK symbol<EngineDependentDvarMut> ai_pathmomentum{0x14A037B58, 0x14737F238};
WEAK symbol<EngineDependentDvarMut> ai_pathnegotiationoverlapcost{0x14A037A68,
                                                                  0x147103148};
WEAK symbol<EngineDependentDvarMut> ai_pathrandompercent{0x14A037A78,
                                                         0x147103158};
WEAK symbol<EngineDependentDvarMut> ai_pathsuppressionenabled{0x14A037C20,
                                                              0x14737F300};
WEAK symbol<EngineDependentDvarMut> ai_perfInfoSortType{0x14A037C78,
                                                        0x14737F358};
WEAK symbol<EngineDependentDvarMut> ai_playerfaraccuracy{0x14A037BD0,
                                                         0x14737F2B0};
WEAK symbol<EngineDependentDvarMut> ai_playerfarrange{0x14A037BD8, 0x14737F2B8};
WEAK symbol<EngineDependentDvarMut> ai_playerloshalfwidth{0x14A037BE0,
                                                          0x14737F2C0};
WEAK symbol<EngineDependentDvarMut> ai_playernearaccuracy{0x14A037BC0,
                                                          0x14737F2A0};
WEAK symbol<EngineDependentDvarMut> ai_playernearrange{0x14A037BC8,
                                                       0x14737F2A8};
WEAK symbol<EngineDependentDvarMut> airburstAdjustDistance{0x14A5746A8,
                                                           0x147A5C298};
WEAK symbol<EngineDependentDvarMut> ai_scalesightupdatelatency{0x14A037C80,
                                                               0x14737F360};
WEAK symbol<EngineDependentDvarMut> ai_secondarythreats{0x14A037BF0,
                                                        0x14737F2D0};
WEAK symbol<EngineDependentDvarMut> ai_showcanshootchecks{0x14A037A60,
                                                          0x147103140};
WEAK symbol<EngineDependentDvarMut> ai_showclaimednode{0x14A037BA0,
                                                       0x14737F280};
WEAK symbol<EngineDependentDvarMut> ai_showClearanceCeiling{0x150BA1178,
                                                            0x148270F98};
WEAK symbol<EngineDependentDvarMut> ai_showcount{0x14A0379C8, 0x1471030A8};
WEAK symbol<EngineDependentDvarMut> ai_showcoverscore{0x14A037A18, 0x1471030F8};
WEAK symbol<EngineDependentDvarMut> ai_showdodge{0x14A037B48, 0x14737F228};
WEAK symbol<EngineDependentDvarMut> ai_showNavMemory{0x150BA1168, 0x148270F88};
WEAK symbol<EngineDependentDvarMut> ai_showNavMesh{0x150BA1138, 0x148270F58};
WEAK symbol<EngineDependentDvarMut> ai_showNavMeshBadEdges{0x150BA1150,
                                                           0x148270F70};
WEAK symbol<EngineDependentDvarMut> ai_showNavMeshRegions{0x150BA1140,
                                                          0x148270F60};
WEAK symbol<EngineDependentDvarMut> ai_showNavMeshSilhouettes{0x150BA1148,
                                                              0x148270F68};
WEAK symbol<EngineDependentDvarMut> ai_showNavMeshTriggers{0x150BA1158,
                                                           0x148270F78};
WEAK symbol<EngineDependentDvarMut> ai_showNavPaths{0x150BA1160, 0x148270F80};
WEAK symbol<EngineDependentDvarMut> ai_showNavStats{0x150BA1170, 0x148270F90};
WEAK symbol<EngineDependentDvarMut> ai_showNavVolume{0x150BA11B0, 0x148270FD0};
WEAK symbol<EngineDependentDvarMut> ai_showNavVolumeStats{0x150BA11B8,
                                                          0x148270FD8};
WEAK symbol<EngineDependentDvarMut> ai_shownearestnode{0x14A0379E0,
                                                       0x1471030C0};
WEAK symbol<EngineDependentDvarMut> ai_shownodes{0x14A0379D0, 0x1471030B0};
WEAK symbol<EngineDependentDvarMut> ai_shownodesdist{0x14A0379D8, 0x1471030B8};
WEAK symbol<EngineDependentDvarMut> ai_showoffmeshnodes{0x14A0379E8,
                                                        0x1471030C8};
WEAK symbol<EngineDependentDvarMut> ai_showpathfindnodes{0x14A037A10,
                                                         0x1471030F0};
WEAK symbol<EngineDependentDvarMut> ai_showpaths{0x14A037A08, 0x1471030E8};
WEAK symbol<EngineDependentDvarMut> ai_showperfinfo{0x14A037C70, 0x14737F350};
WEAK symbol<EngineDependentDvarMut> ai_showpotentialthreatdir{0x14A037A80,
                                                              0x147103160};
WEAK symbol<EngineDependentDvarMut> ai_showsuppression{0x14A037C18,
                                                       0x14737F2F8};
WEAK symbol<EngineDependentDvarMut> ai_showvisdata{0x14A0379F8, 0x1471030D8};
WEAK symbol<EngineDependentDvarMut> ai_showvisdatadist{0x14A037A00,
                                                       0x1471030E0};
WEAK symbol<EngineDependentDvarMut> ai_showvolume{0x14A037B98, 0x14737F278};
WEAK symbol<EngineDependentDvarMut> ai_slowdownMaxYawDiff{0x14A037860,
                                                          0x147836948};
WEAK symbol<EngineDependentDvarMut> ai_slowdownMinRate{0x14A037868,
                                                       0x147836950};
WEAK symbol<EngineDependentDvarMut> ai_slowdownMinYawDiff{0x14A037858,
                                                          0x147836940};
WEAK symbol<EngineDependentDvarMut> ai_slowdownRateBlendFactor{0x14A037870,
                                                               0x147836958};
WEAK symbol<EngineDependentDvarMut> ai_stumblesightfov{0x14A037BB8,
                                                       0x14737F298};
WEAK symbol<EngineDependentDvarMut> ai_stumblesightrange{0x14A037BB0,
                                                         0x14737F290};
WEAK symbol<EngineDependentDvarMut> ai_threatupdateinterval{0x14A037BE8,
                                                            0x14737F2C8};
WEAK symbol<EngineDependentDvarMut> ai_trim_path_zigzags{0x14A1D4228,
                                                         0x1478366B8};
WEAK symbol<EngineDependentDvarMut> ai_useBetterLookahead{0x14A037850,
                                                          0x147836938};
WEAK symbol<EngineDependentDvarMut> ai_useFacingTranslation{0x14A037840,
                                                            0x147836928};
WEAK symbol<EngineDependentDvarMut> ai_useLeanRunAnimations{0x14A037848,
                                                            0x147836930};
WEAK symbol<EngineDependentDvarMut> ai_validatebehaviortree{0x14A037CA8,
                                                            0x14737F388};
WEAK symbol<EngineDependentDvarMut> ai_warnMissingNavVolume{0x150BA11F8,
                                                            0x148271018};
WEAK symbol<EngineDependentDvarMut> ai_weaponCamoEnabled{0x14A1D4238,
                                                         0x1478366C8};
} // namespace game
