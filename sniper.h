#pragma once
#include "memory.h"
#include "visuals.h"
#include <chrono>
#include <map>

class SniperAutoAim {
public:
    static bool aim_active;
    static float search_radius;
    static float hp_threshold;
    static std::chrono::steady_clock::time_point last_shot_time;
    static std::map<int, std::chrono::steady_clock::time_point> target_cooldowns;

    static void SniperAutoAimLogic(uintptr_t local_entity, uint8_t local_team, Vector3 local_player_pos, ViewMatrix vm);
    static void RenderSniperSettingsMenu();
    static void Fire();
    static void AimTenTimes(uintptr_t cammanager, uintptr_t local_entity, uint8_t local_team, const ViewMatrix& vm);
    static void PressKey(int key);
    static bool HoldRightMouseButtonFor100ms();
    static void ReleaseRightMouseButton();
    static void ReleaseMouseButtonsIfHeld();
    static Vector3 get_enemy_in_radius(const Vector3& local_player_pos, const uintptr_t& local_entity, const uint8_t local_team, int& target_index);
    static bool is_target_in_cooldown(int target_index);
};


Vector3 CalcAngle1(const Vector3& playerPosition, const Vector3& enemyPosition);
Vector3 NormalizeAngles1(Vector3 angle);
