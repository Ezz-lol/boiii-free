#pragma once

#include <game/symbols/sym_include.hpp>

namespace game {
WEAK symbol<EngineDependentDvarMut> spawnsystem_allow_non_team_spawns{
    0x14A5A6A50, 0x147A8E630};
WEAK symbol<EngineDependentDvarMut> spawnsystem_badspawn_aggression_delay{
    0x14A5A6A10, 0x147A8E5F0};
WEAK symbol<EngineDependentDvarMut> spawnsystem_badspawn_damage_delay{
    0x14A5A6A08, 0x147A8E5E8};
WEAK symbol<EngineDependentDvarMut> spawnsystem_badspawn_force_record{
    0x14A5A6A18, 0x147A8E5F8};
WEAK symbol<EngineDependentDvarMut> spawnsystem_convert_spawns_to_structs{
    0x0, 0x147A8E4E8};
WEAK symbol<EngineDependentDvarMut> spawnsystem_danger_time{0x14A5A6A28,
                                                            0x147A8E608};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug{0x14A5A6918, 0x147A8E4F8};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_archive{0x14A5A69A0,
                                                              0x147A8E580};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_best_points{0x14A5A6960,
                                                                  0x147A8E540};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_influencer_pulse{
    0x14A5A6900, 0x147A8E4E0};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_influencers{0x14A5A6950,
                                                                  0x147A8E530};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_influencer_types{
    0x14A5A6958, 0x147A8E538};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_liveedit{0x14A5A69B0,
                                                               0x147A8E590};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_player{0x14A5A6978,
                                                             0x147A8E558};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_points{0x14A5A6968,
                                                             0x147A8E548};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_point_weights{
    0x14A5A6970, 0x147A8E550};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_reduced_logging{
    0x14A5A69B8, 0x147A8E598};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_showclients{0x14A5A69A8,
                                                                  0x147A8E588};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_sideswitched{0x14A5A6988,
                                                                   0x147A8E568};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_team{0x14A5A6980,
                                                           0x147A8E560};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_visibility{0x14A5A6990,
                                                                 0x147A8E570};
WEAK symbol<EngineDependentDvarMut> spawnsystem_debug_visibility_time{
    0x14A5A6998, 0x147A8E578};
WEAK symbol<EngineDependentDvarMut> spawnsystem_demo_enable_parsing{
    0x14A5A6920, 0x147A8E500};
WEAK symbol<EngineDependentDvarMut> spawnsystem_demo_max_written{0x14A5A6A58,
                                                                 0x147A8E638};
WEAK symbol<EngineDependentDvarMut> spawnsystem_influencer_sight_line{
    0x14A5A69C0, 0x147A8E5A0};
WEAK symbol<EngineDependentDvarMut>
    spawnsystem_influencer_sight_line_min_length{0x14A5A69D0, 0x147A8E5B0};
WEAK symbol<EngineDependentDvarMut>
    spawnsystem_influencer_sight_line_push_through{0x14A5A69C8, 0x147A8E5A8};
WEAK symbol<EngineDependentDvarMut>
    spawnsystem_influencer_sight_line_update_interval{0x14A5A69D8, 0x147A8E5B8};
WEAK symbol<EngineDependentDvarMut> spawnsystem_old_vis_mask{0x14A5A6928,
                                                             0x147A8E508};
WEAK symbol<EngineDependentDvarMut> spawnsystem_score_cull_min_points{
    0x14A5A6A48, 0x147A8E628};
WEAK symbol<EngineDependentDvarMut> spawnsystem_score_cull_percent{0x14A5A6A40,
                                                                   0x147A8E620};
WEAK symbol<EngineDependentDvarMut> spawnsystem_score_cull_time_max{
    0x14A5A6A30, 0x147A8E610};
WEAK symbol<EngineDependentDvarMut> spawnsystem_score_cull_time_min{
    0x14A5A6A38, 0x147A8E618};
WEAK symbol<EngineDependentDvarMut> spawnsystem_sight_check_dual_point{
    0x14A5A69F0, 0x147A8E5D0};
WEAK symbol<EngineDependentDvarMut> spawnsystem_sight_check_dual_point_radius{
    0x14A5A6A00, 0x147A8E5E0};
WEAK symbol<EngineDependentDvarMut> spawnsystem_sight_check_max_distance{
    0x14A5A69E0, 0x147A8E5C0};
WEAK symbol<EngineDependentDvarMut> spawnsystem_sight_height_offset{
    0x14A5A69E8, 0x147A8E5C8};
WEAK symbol<EngineDependentDvarMut>
    spawnsystem_sight_use_player_current_view_height{0x14A5A69F8, 0x147A8E5D8};
} // namespace game
