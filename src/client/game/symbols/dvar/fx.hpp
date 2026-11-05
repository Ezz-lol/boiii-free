#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> fx_attempt_trail_reconstruction{
    0x149E97758, 0x149F16758, 0x146F62E58};
WEAK symbol<EngineDependentDvarMut> fx_attempt_trail_reconstruction_debug{
    0x149E97760, 0x149F16760, 0x146F62E60};
WEAK symbol<EngineDependentDvarMut> fx_attempt_trail_reconstruction_lerp_ratio{
    0x149E97768, 0x149F16768, 0x146F62E68};
WEAK symbol<EngineDependentDvarMut>
    fx_attempt_trail_reconstruction_lerp_time_ms{0x149E97770, 0x149F16770,
                                                 0x146F62E70};
WEAK symbol<EngineDependentDvarMut> fx_compute_sprites_debug_draw{
    0x149E97738, 0x149F16738, 0x146F62E38};
WEAK symbol<EngineDependentDvarMut> fx_compute_sprites_enable{
    0x149E97728, 0x149F16728, 0x146F62E28};
WEAK symbol<EngineDependentDvarMut> fx_compute_sprites_enable_all{
    0x149E97730, 0x149F16730, 0x146F62E30};
WEAK symbol<EngineDependentDvarMut> fx_compute_sprites_only{
    0x149E97750, 0x149F16750, 0x146F62E50};
WEAK symbol<EngineDependentDvarMut> fx_compute_sprites_shadow{
    0x149E97740, 0x149F16740, 0x146F62E40};
WEAK symbol<EngineDependentDvarMut> fx_compute_sprites_show_lmap{
    0x149E97748, 0x149F16748, 0x146F62E48};
WEAK symbol<EngineDependentDvarMut> fx_count{0x149E976A0, 0x149F166A0,
                                             0x146F62DA0};
WEAK symbol<EngineDependentDvarMut> fx_debugBolt{0x149E97690, 0x149F16690,
                                                 0x146F62D90};
WEAK symbol<EngineDependentDvarMut> fx_draw{0x149E97680, 0x149F16680,
                                            0x146F62D80};
WEAK symbol<EngineDependentDvarMut> fx_drawClouds{0x149E976C0, 0x149F166C0,
                                                  0x146F62DC0};
WEAK symbol<EngineDependentDvarMut> fx_draw_weapon_tags{
    0x149E976D8, 0x149F166D8, 0x146F62DD8};
WEAK symbol<EngineDependentDvarMut> fx_enable{0x149E97678, 0x149F16678,
                                              0x146F62D78};
WEAK symbol<EngineDependentDvarMut> fx_freeze{0x149E97698, 0x149F16698,
                                              0x146F62D98};
WEAK symbol<EngineDependentDvarMut> fxfrustumCutoff{0x0, 0x0, 0x14A1797F8};
WEAK symbol<EngineDependentDvarMut> fx_gpufx_debug_enable{
    0x149E97798, 0x149F16798, 0x146F62E98};
WEAK symbol<EngineDependentDvarMut> fx_gpufx_draw_enable{
    0x149E97788, 0x149F16788, 0x146F62E88};
WEAK symbol<EngineDependentDvarMut> fx_gpufx_elem_limit{
    0x149E977A8, 0x149F167A8, 0x146F62EA8};
WEAK symbol<EngineDependentDvarMut> fx_gpufx_library_enable{
    0x149E97778, 0x149F16778, 0x146F62E78};
WEAK symbol<EngineDependentDvarMut> fx_gpufx_no_water{0x149E977A0, 0x149F167A0,
                                                      0x146F62EA0};
WEAK symbol<EngineDependentDvarMut> fx_gpufx_sort_enable{
    0x149E97790, 0x149F16790, 0x146F62E90};
WEAK symbol<EngineDependentDvarMut> fx_gpufx_sprite_limit{
    0x149E977B0, 0x149F167B0, 0x146F62EB0};
WEAK symbol<EngineDependentDvarMut> fx_gpufx_update_enable{
    0x149E97780, 0x149F16780, 0x146F62E80};
WEAK symbol<EngineDependentDvarMut> fx_limit_spawn_workers{
    0x149E97718, 0x149F16718, 0x146F62E18};
WEAK symbol<EngineDependentDvarMut> fx_mark_profile{0x149E976B8, 0x149F166B8,
                                                    0x146F62DB8};
WEAK symbol<EngineDependentDvarMut> fx_marks_draw{0x149E97688, 0x149F16688,
                                                  0x146F62D88};
WEAK symbol<EngineDependentDvarMut> fx_marks_forward_vol_marks_enable{
    0x149E97720, 0x149F16720, 0x146F62E20};
WEAK symbol<EngineDependentDvarMut> fx_marks_range{0x149E976F8, 0x149F166F8,
                                                   0x146F62DF8};
WEAK symbol<EngineDependentDvarMut> fx_max_per_frame_distance{
    0x149E97710, 0x149F16710, 0x146F62E10};
WEAK symbol<EngineDependentDvarMut> fx_occlusion_max_player_count{
    0x149E976F0, 0x149F166F0, 0x146F62DF0};
WEAK symbol<EngineDependentDvarMut> fx_occlusion_query_profile{
    0x149E976E8, 0x149F166E8, 0x146F62DE8};
WEAK symbol<EngineDependentDvarMut> fx_priority_debug{0x149E976C8, 0x149F166C8,
                                                      0x146F62DC8};
WEAK symbol<EngineDependentDvarMut> fx_priority_enable{0x149E976D0, 0x149F166D0,
                                                       0x146F62DD0};
WEAK symbol<EngineDependentDvarMut> fx_process_clouds{0x0, 0x149F167B8,
                                                      0x146F62EB8};
WEAK symbol<EngineDependentDvarMut> fx_profile{0x149E976B0, 0x149F166B0,
                                               0x146F62DB0};
WEAK symbol<EngineDependentDvarMut> fx_show_distances{0x149E97708, 0x149F16708,
                                                      0x146F62E08};
WEAK symbol<EngineDependentDvarMut> fx_show_names_range{
    0x149E97700, 0x149F16700, 0x146F62E00};
WEAK symbol<EngineDependentDvarMut> fx_use_ground_entity{
    0x149E976E0, 0x149F166E0, 0x146F62DE0};
WEAK symbol<EngineDependentDvarMut> fx_visMinTraceDist{0x149E976A8, 0x149F166A8,
                                                       0x146F62DA8};
} // namespace game
