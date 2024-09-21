#pragma once
#include "math.h"
#include <windows.h>
#include "ImGui/imgui.h"
#include "entity.h"
#include "visuals.h"
#include <chrono>

namespace BebopHook {

    extern bool hook_active;         
    extern int hook_speed;          
    extern bool target_locked;         
    extern int locked_target_index;    
    extern float aim_smoothness;       
    extern bool enable_delay;          
    extern std::chrono::steady_clock::time_point last_target_time; 


    void RenderBebopSettingsMenu();


    void BebopAutoHookLogic(uintptr_t local_entity, uint8_t local_team, Vector3 local_player_pos, ViewMatrix vm);


    Vector3 get_closest_enemy_to_cursor(const Vector3& local_player_pos, const ViewMatrix& vm, const uintptr_t& local_entity, const uint8_t local_team, int& closest_index);


    void TriggerHook();
}
