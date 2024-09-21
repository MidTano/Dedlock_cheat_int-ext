#include "sniper.h"

bool SniperAutoAim::aim_active = false;
float SniperAutoAim::search_radius = 3000.0f;
float SniperAutoAim::hp_threshold = 1.0f;
std::chrono::steady_clock::time_point SniperAutoAim::last_shot_time = std::chrono::steady_clock::now();
std::map<int, std::chrono::steady_clock::time_point> SniperAutoAim::target_cooldowns;

constexpr float PI = 3.14159f;

Vector3 CalcAngle1(const Vector3& playerPosition, const Vector3& enemyPosition) {
    Vector3 delta = enemyPosition - playerPosition;
    float yaw = atan2f(delta.Y, delta.X) * (180.0f / PI);
    float pitch = atan2f(delta.Z, sqrt(delta.X * delta.X + delta.Y * delta.Y)) * (180.0f / PI);
    return Vector3(-pitch, yaw, 0.0f);
}

Vector3 NormalizeAngles1(Vector3 angle) {
    if (angle.X > 89.0f) angle.X = 89.0f;
    if (angle.X < -89.0f) angle.X = -89.0f;

    while (angle.Y < -180.0f) angle.Y += 360.0f;
    while (angle.Y > 180.0f) angle.Y -= 360.0f;

    angle.Z = 0.0f;
    return angle;
}

void SniperAutoAim::SniperAutoAimLogic(uintptr_t local_entity, uint8_t local_team, Vector3 local_player_pos, ViewMatrix vm) {
    if (!aim_active) return;

    int target_index = -1;
    Vector3 target_position = get_enemy_in_radius(local_player_pos, local_entity, local_team, target_index);

    if (target_index != -1 && Visuals1::entityCache[target_index].isValid && !is_target_in_cooldown(target_index)) {

        Vector3 screen_pos = WorldToScreen(vm, target_position);
        if (screen_pos.Z <= 0.01f) {
            return;
        }

        uintptr_t cammanager = memory::memRead<uintptr_t>(memory::baseAddress + (offsets::CCitadelCameraManager + 0x28));
        if (cammanager) {
            
            ReleaseMouseButtonsIfHeld();

         
            PressKey('4');
            
            AimTenTimes(cammanager, local_entity, local_team, vm);

            if (HoldRightMouseButtonFor100ms()) {
                Fire();
               
                ReleaseRightMouseButton();
               
                PressKey('4');

                target_cooldowns[target_index] = std::chrono::steady_clock::now();  
            }
        }
    }
}

void SniperAutoAim::ReleaseMouseButtonsIfHeld() {
    SHORT leftButtonState = GetAsyncKeyState(VK_LBUTTON);
    SHORT rightButtonState = GetAsyncKeyState(VK_RBUTTON);


    if (leftButtonState & 0x8000) {
        INPUT mouseInput = { 0 };
        mouseInput.type = INPUT_MOUSE;
        mouseInput.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &mouseInput, sizeof(mouseInput));
    }

    if (rightButtonState & 0x8000) {
        INPUT mouseInput = { 0 };
        mouseInput.type = INPUT_MOUSE;
        mouseInput.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &mouseInput, sizeof(mouseInput));
    }
}

void SniperAutoAim::AimTenTimes(uintptr_t cammanager, uintptr_t local_entity, uint8_t local_team, const ViewMatrix& vm) {
    for (int i = 0; i < 10; ++i) {

        Vector3 local_player_pos = memory::memRead<Vector3>(cammanager + 0x38);
        int target_index = -1;
        Vector3 target_position = get_enemy_in_radius(local_player_pos, local_entity, local_team, target_index);

        if (target_index != -1 && Visuals1::entityCache[target_index].isValid) {
            Vector3 new_view_angles = NormalizeAngles1(CalcAngle1(local_player_pos, target_position));
            memory::memWrite<Vector3>(cammanager + 0x44, new_view_angles);
        }
    }
}

bool SniperAutoAim::HoldRightMouseButtonFor100ms() {
    static std::chrono::steady_clock::time_point hold_start_time = std::chrono::steady_clock::now();
    static bool is_holding = false;

    if (!is_holding) {
 
        INPUT mouseInput = { 0 };
        mouseInput.type = INPUT_MOUSE;
        mouseInput.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &mouseInput, sizeof(mouseInput));


        hold_start_time = std::chrono::steady_clock::now();
        is_holding = true;
    }

   
    auto current_time = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - hold_start_time).count() >= 30) {

        is_holding = false;
        return true;  
    }

    return false;  
}

void SniperAutoAim::ReleaseRightMouseButton() {
    INPUT mouseInput = { 0 };
    mouseInput.type = INPUT_MOUSE;
    mouseInput.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &mouseInput, sizeof(mouseInput));
}

void SniperAutoAim::PressKey(int key) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    SendInput(1, &input, sizeof(INPUT));

    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void SniperAutoAim::Fire() {
  
    INPUT mouseInput = { 0 };
    mouseInput.type = INPUT_MOUSE;
    mouseInput.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &mouseInput, sizeof(mouseInput));

    mouseInput.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &mouseInput, sizeof(mouseInput));
}

Vector3 SniperAutoAim::get_enemy_in_radius(const Vector3& local_player_pos, const uintptr_t& local_entity, const uint8_t local_team, int& target_index) {
    for (int i = 0; i < Visuals1::entityCache.size(); ++i) {
        const auto& entity = Visuals1::entityCache[i];

        if (!entity.isValid || entity.entityAddress == local_entity || entity.team == local_team) continue;

        Vector3 entity_world_pos = memory::memRead<Vector3>(entity.entityAddress + offsets::m_vOldOrigin);
        entity_world_pos.Z += 50.f;
        float distance_to_player = get_distance(local_player_pos, entity_world_pos);

        if (distance_to_player <= search_radius) {
            uint32_t entity_hp = memory::memRead<uint32_t>(entity.entityAddress + 0x34c);
            if (entity_hp > 0 && entity_hp <= (hp_threshold * 100)) {
                target_index = i;
                return entity_world_pos;
            }
        }
    }

    return Vector3();
}

bool SniperAutoAim::is_target_in_cooldown(int target_index) {
    auto current_time = std::chrono::steady_clock::now();

    if (target_cooldowns.find(target_index) != target_cooldowns.end()) {
        auto last_shot_time = target_cooldowns[target_index];
        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_shot_time).count();

        if (elapsed_time < 10) {
            return true;
        }
    }

    return false;
}

void SniperAutoAim::RenderSniperSettingsMenu() {
    if (ImGui::Checkbox("Enable Sniper AutoAim", &aim_active)) {
        ImGui::SetNextItemOpen(false);
    }
    if (aim_active && ImGui::TreeNode("Sniper AutoAim Settings")) {
        ImGui::SliderFloat("Search Radius", &search_radius, 100.0f, 5000.0f, "Radius: %100.0f");
        ImGui::SliderFloat("HP Threshold", &hp_threshold, 1.0f, 5.0f, "HP: %1.0f");
        ImGui::TreePop();
    }
}
