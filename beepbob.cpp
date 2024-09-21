#include "beepbob.h"
#include "memory.h"
#include "visuals.h"
#include <chrono>
#include <cmath>
#include <map>
#include <string>


std::string GetKeyName1(int key) {
    static std::map<int, std::string> keyNames = {
        { VK_LBUTTON, "MOUSE1" },
        { VK_RBUTTON, "MOUSE2" },
        { VK_MBUTTON, "MOUSE3" },
        { VK_XBUTTON1, "MOUSE4" },
        { VK_XBUTTON2, "MOUSE5" },
        { VK_BACK, "Backspace" },
        { VK_TAB, "Tab" },
        { VK_RETURN, "Enter" },
        { VK_SHIFT, "Shift" },
        { VK_CONTROL, "Ctrl" },
        { VK_MENU, "Alt" },
        { VK_PAUSE, "Pause" },
        { VK_CAPITAL, "Caps Lock" },
        { VK_ESCAPE, "Escape" },
        { VK_SPACE, "Space" },
        { VK_PRIOR, "Page Up" },
        { VK_NEXT, "Page Down" },
        { VK_END, "End" },
        { VK_HOME, "Home" },
        { VK_LEFT, "Left Arrow" },
        { VK_UP, "Up Arrow" },
        { VK_RIGHT, "Right Arrow" },
        { VK_DOWN, "Down Arrow" },
        { VK_INSERT, "Insert" },
        { VK_DELETE, "Delete" },
        { 'A', "A" },
        { 'B', "B" },
        { 'C', "C" },
        { 'D', "D" },
        { 'E', "E" },
        { 'F', "F" },
        { 'G', "G" },
        { 'H', "H" },
        { 'I', "I" },
        { 'J', "J" },
        { 'K', "K" },
        { 'L', "L" },
        { 'M', "M" },
        { 'N', "N" },
        { 'O', "O" },
        { 'P', "P" },
        { 'Q', "Q" },
        { 'R', "R" },
        { 'S', "S" },
        { 'T', "T" },
        { 'U', "U" },
        { 'V', "V" },
        { 'W', "W" },
        { 'X', "X" },
        { 'Y', "Y" },
        { 'Z', "Z" },
        { VK_F1, "F1" },
        { VK_F2, "F2" },
        { VK_F3, "F3" },
        { VK_F4, "F4" },
        { VK_F5, "F5" },
        { VK_F6, "F6" },
        { VK_F7, "F7" },
        { VK_F8, "F8" },
        { VK_F9, "F9" },
        { VK_F10, "F10" },
        { VK_F11, "F11" },
        { VK_F12, "F12" },
        { VK_NUMPAD0, "Numpad 0" },
        { VK_NUMPAD1, "Numpad 1" },
        { VK_NUMPAD2, "Numpad 2" },
        { VK_NUMPAD3, "Numpad 3" },
        { VK_NUMPAD4, "Numpad 4" },
        { VK_NUMPAD5, "Numpad 5" },
        { VK_NUMPAD6, "Numpad 6" },
        { VK_NUMPAD7, "Numpad 7" },
        { VK_NUMPAD8, "Numpad 8" },
        { VK_NUMPAD9, "Numpad 9" },
        { VK_MULTIPLY, "Numpad *" },
        { VK_ADD, "Numpad +" },
        { VK_SUBTRACT, "Numpad -" },
        { VK_DECIMAL, "Numpad ." },
        { VK_DIVIDE, "Numpad /" },
        { VK_OEM_1, ";" },
        { VK_OEM_PLUS, "=" },
        { VK_OEM_COMMA, "," },
        { VK_OEM_MINUS, "-" },
        { VK_OEM_PERIOD, "." },
        { VK_OEM_2, "/" },
        { VK_OEM_3, "`" },
        { VK_OEM_4, "[" },
        { VK_OEM_5, "\\" },
        { VK_OEM_6, "]" },
        { VK_OEM_7, "'" }
    };

    if (keyNames.find(key) != keyNames.end()) {
        return keyNames[key];
    }

    return "Unknown";
}


bool BebopHook::hook_active = false;
int BebopHook::hook_speed = 2500;
bool BebopHook::target_locked = false;
int BebopHook::locked_target_index = -1;
float BebopHook::aim_smoothness = 1.0f;
bool BebopHook::enable_delay = false;
std::chrono::steady_clock::time_point BebopHook::last_target_time = std::chrono::steady_clock::now();
int hookActivationKey = 'F'; 

constexpr float PI = 3.14159f;


inline Vector3 CalcAngle(const Vector3& playerPosition, const Vector3& enemyPosition) {
    Vector3 delta = enemyPosition - playerPosition;
    float yaw = atan2f(delta.Y, delta.X) * (180.0f / PI);  
    float pitch = atan2f(delta.Z, sqrt(delta.X * delta.X + delta.Y * delta.Y)) * (180.0f / PI);  
    return Vector3(-pitch, yaw, 0.0f); 
}



inline Vector3 NormalizeAngles(Vector3 angle) {
    if (angle.X > 89.0f) angle.X = 89.0f;
    if (angle.X < -89.0f) angle.X = -89.0f;

    while (angle.Y < -180.0f) angle.Y += 360.0f;
    while (angle.Y > 180.0f) angle.Y -= 360.0f;

    angle.Z = 0.0f;  
    return angle;
}


void BebopHook::BebopAutoHookLogic(uintptr_t local_entity, uint8_t local_team, Vector3 local_player_pos, ViewMatrix vm) {
    if (!(GetAsyncKeyState(hookActivationKey) & 0x8000)) {
        target_locked = false;
        locked_target_index = -1;
        return;
    }

    if (!hook_active) return;

    int closest_index = -1;
    Vector3 closest_enemy_position = get_closest_enemy_to_cursor(local_player_pos, vm, local_entity, local_team, closest_index);

    if (!target_locked || locked_target_index == -1) {
        if (closest_index != -1 && Visuals1::entityCache[closest_index].isValid) {
          
            uint32_t entity_hp = memory::memRead<uint32_t>(Visuals1::entityCache[closest_index].entityAddress + 0x34c);
            if (entity_hp > 0) {  
                locked_target_index = closest_index;
                target_locked = true;
                last_target_time = std::chrono::steady_clock::now();
            }
        }
    }
    else {
        auto& target = Visuals1::entityCache[locked_target_index];
        if (!target.isValid) {
            target_locked = false;
            locked_target_index = -1;
        }
        else {
           
            uint32_t entity_hp = memory::memRead<uint32_t>(target.entityAddress + 0x34c);
            if (entity_hp <= 0) { 
                target_locked = false;
                locked_target_index = -1;
                return;
            }

            Vector3 target_position = memory::memRead<Vector3>(target.entityAddress + offsets::m_vOldOrigin);
            target_position.Z += 50.f;

            Vector3 target_velocity = {
                memory::memRead<float>(target.entityAddress + 0x408),
                memory::memRead<float>(target.entityAddress + 0x410),
                memory::memRead<float>(target.entityAddress + 0x428)
            };

            float distance_to_target = get_distance(local_player_pos, target_position);
            float time_to_hit = distance_to_target / hook_speed;

            Vector3 predicted_position = target_position + (target_velocity * time_to_hit);

            uintptr_t cammanager = memory::memRead<uintptr_t>(memory::baseAddress + (offsets::CCitadelCameraManager + 0x28));
            if (cammanager) {
                Vector3 current_view_angles = memory::memRead<Vector3>(cammanager + 0x44);

                Vector3 new_view_angles = NormalizeAngles(CalcAngle(local_player_pos, predicted_position));
                new_view_angles.X = (new_view_angles.X - current_view_angles.X) / aim_smoothness + current_view_angles.X;
                new_view_angles.Y = (new_view_angles.Y - current_view_angles.Y) / aim_smoothness + current_view_angles.Y;

                memory::memWrite<Vector3>(cammanager + 0x44, new_view_angles);

                auto current_time = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_target_time).count() > 500 || !enable_delay) {
                    TriggerHook();
                }
            }
        }
    }
}



void BebopHook::TriggerHook() {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = '3';  
    SendInput(1, &input, sizeof(INPUT));

    input.ki.dwFlags = KEYEVENTF_KEYUP;  
    SendInput(1, &input, sizeof(INPUT));
}


Vector3 BebopHook::get_closest_enemy_to_cursor(const Vector3& local_player_pos, const ViewMatrix& vm, const uintptr_t& local_entity, const uint8_t local_team, int& closest_index) {
    float closest_distance = FLT_MAX;
    Vector3 closest_enemy_position;
    const float screen_center_x = GetSystemMetrics(SM_CXSCREEN) / 2.0f;
    const float screen_center_y = GetSystemMetrics(SM_CYSCREEN) / 2.0f;

    for (int i = 0; i < Visuals1::entityCache.size(); ++i) {
        const auto& entity = Visuals1::entityCache[i];

        if (!entity.isValid || entity.entityAddress == local_entity || entity.team == local_team) continue;

        Vector3 entity_world_pos = memory::memRead<Vector3>(entity.entityAddress + offsets::m_vOldOrigin);
        entity_world_pos.Z += 50.f;
        Vector3 screen_pos = WorldToScreen(vm, entity_world_pos);
        if (screen_pos.Z <= 0.01f) continue;

        float deltaX = screen_pos.X - screen_center_x;
        float deltaY = screen_pos.Y - screen_center_y;
        float distance_to_crosshair = sqrtf(deltaX * deltaX + deltaY * deltaY);

        if (distance_to_crosshair < closest_distance) {
            closest_distance = distance_to_crosshair;
            closest_enemy_position = entity_world_pos;
            closest_index = i;
        }
    }

    return closest_enemy_position;
}


void BebopHook::RenderBebopSettingsMenu() {
    if (ImGui::Checkbox("Enable Bebop AutoHook", &hook_active)) {
        ImGui::SetNextItemOpen(false);
    }
    if (hook_active && ImGui::TreeNode("Bebop Hook Settings")) {
        ImGui::SliderInt("Hook Speed", &hook_speed, 1000, 10000, "Speed: %100");
        ImGui::SliderFloat("Aim Smoothness", &aim_smoothness, 1.0f, 10.0f, "Smoothness: %.1f");
        ImGui::Checkbox("Enable Delay Before Hook", &enable_delay);

     
        ImGui::Text("Hook Activation Key:");
        ImGui::SameLine();
        if (ImGui::Button("Choose Key")) {
            ImGui::OpenPopup("key_popup");
        }

        if (ImGui::BeginPopup("key_popup")) {
            ImGui::Text("Press any key to set it as the activation key");
            for (int i = 0; i < 256; ++i) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    hookActivationKey = i;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }

    
        std::string keyName = GetKeyName1(hookActivationKey);
        ImGui::Text("Current Hook Activation Key: %s", keyName.c_str());

        ImGui::TreePop();
    }
}
