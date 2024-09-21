#include "aimbot.h"
#include <algorithm>
#include <cmath>
#include "memory.h"
#include "visuals.h"
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include "aimbot.h"
#include <map>
#include <string>

std::string Aimbot::GetKeyName(int key) {
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


AimbotSettings Aimbot::settings;


constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

struct ScreenDimensions {
    float centerX;
    float centerY;

    ScreenDimensions() {
        centerX = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)) / 2.0f;
        centerY = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) / 2.0f;
    }
};

static ScreenDimensions screenDims; 

float Aimbot::GetAngleDistance(const Vector3& screenPos) {
    float deltaX = screenPos.X - screenDims.centerX;
    float deltaY = screenPos.Y - screenDims.centerY;
    return sqrtf(deltaX * deltaX + deltaY * deltaY);
}


void Aimbot::DrawFOVCircle(float fov) {
    if (!settings.enabled) return;

    ImDrawList* drawList = ImGui::GetForegroundDrawList();


    float fovRadians = fov * DEG_TO_RAD;
    float fovRadius = tanf(fovRadians / 2.0f) * screenDims.centerX;

    ImColor circleColor = settings.targetLocked ? ImColor(255, 0, 0, 255) : ImColor(255, 255, 255, 255);


    drawList->AddCircle(ImVec2(screenDims.centerX, screenDims.centerY), fovRadius, circleColor, 100, 2.0f);
}

Vector3 Aimbot::GetClosestSoul(const Vector3& localPlayerPos, const ViewMatrix& vm, int& closestIndex, float fov) {
    float closestDistance = FLT_MAX;
    Vector3 closestSoulPosition;
    float fovRadius = tanf(fov * DEG_TO_RAD / 2.0f) * screenDims.centerX;

    ULONG_PTR entity_list = get_entity_list();
    int max_ents = get_max_entities();

    for (int i = 1; i <= max_ents; ++i) {
        uintptr_t entity = get_base_entity_from_index(i, entity_list);
        if (!entity) continue;

        std::string designer_name = get_designer_name(entity);

        if (designer_name != "item_xp") continue; 


        uint32_t isVisible = memory::memRead<uint32_t>(entity + 0x30);
        if (isVisible != 1) continue;  

        uintptr_t soul_ptr = memory::memRead<uintptr_t>(entity + 0x328);  
        Vector3 soulWorldPos = memory::memRead<Vector3>(soul_ptr + 0x88); 


        Vector3 screenPos = WorldToScreen(vm, soulWorldPos);
        if (screenPos.Z <= 0.01f) continue;  


        float angleDistance = GetAngleDistance(screenPos);
        if (angleDistance < fovRadius && angleDistance < closestDistance) {
            closestDistance = angleDistance;
            closestSoulPosition = soulWorldPos;
            closestIndex = i;
        }
    }

    return closestSoulPosition;
}



bool IsAimbotKeyPressed() {
    return GetAsyncKeyState(Aimbot::settings.activationKey) & 0x8000;
}


void Aimbot::AimbotLogic(uintptr_t localEntity, uint8_t localTeam, const Vector3& localPlayerPos, const ViewMatrix& vm) {
    if (!settings.enabled || !IsAimbotKeyPressed()) return;

    int closestPlayerIndex = -1;
    int closestSoulIndex = -1;
    Vector3 closestTargetPos;
    bool targetIsSoul = false;


    if (settings.soulsAim) {
        Vector3 closestSoulPos = GetClosestSoul(localPlayerPos, vm, closestSoulIndex, settings.fov);
        if (closestSoulIndex != -1) {
            closestTargetPos = closestSoulPos;
            targetIsSoul = true;
        }
    }

 
    if (!targetIsSoul || closestSoulIndex == -1) {
        Vector3 closestEnemyPos = GetClosestEnemy(localPlayerPos, vm, localEntity, localTeam, closestPlayerIndex, settings.fov);
        if (closestPlayerIndex != -1) {
            closestTargetPos = closestEnemyPos;
            targetIsSoul = false;
        }
    }


    if (targetIsSoul && closestSoulIndex != -1) {

        settings.lockedTargetIndex = closestSoulIndex;
        settings.targetLocked = true;


        Vector3 screenPos = WorldToScreen(vm, closestTargetPos);
        if (screenPos.Z > 0.01f) {

            float deltaX = screenPos.X - screenDims.centerX;
            float deltaY = screenPos.Y - screenDims.centerY;


            deltaX /= settings.smoothness;
            deltaY /= settings.smoothness;


            INPUT input = { 0 };
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            input.mi.dx = static_cast<LONG>(deltaX);
            input.mi.dy = static_cast<LONG>(deltaY);
            SendInput(1, &input, sizeof(INPUT));
        }
    }
    else if (!targetIsSoul && closestPlayerIndex != -1) {

        auto& targetEntity = Visuals1::entityCache[closestPlayerIndex];
        uint32_t entity_hp = memory::memRead<uint32_t>(targetEntity.entityAddress + 0x34c); 
        if (targetEntity.isValid && entity_hp > 0) {
            settings.lockedTargetIndex = closestPlayerIndex;
            settings.targetLocked = true;

            Vector3 enemyHeadPos;
            int headBoneIndex = get_bone_head_index(targetEntity.entityName);
            if (headBoneIndex != -1) {
                enemyHeadPos = get_bone_position_by_index(targetEntity.entityAddress, headBoneIndex);
            }
            else {

                enemyHeadPos = memory::memRead<Vector3>(targetEntity.entityAddress + offsets::m_vOldOrigin);
                enemyHeadPos.Z += 92.0f;
            }


            Vector3 screenPos = WorldToScreen(vm, enemyHeadPos);


            if (!settings.targetLocked || screenPos.Z > 0.01f) {

                float deltaX = screenPos.X - screenDims.centerX;
                float deltaY = screenPos.Y - screenDims.centerY;


                deltaX /= settings.smoothness;
                deltaY /= settings.smoothness;


                INPUT input = { 0 };
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_MOVE;
                input.mi.dx = static_cast<LONG>(deltaX);
                input.mi.dy = static_cast<LONG>(deltaY);
                SendInput(1, &input, sizeof(INPUT));
            }
        }
    }
    else {
     
        settings.targetLocked = false;
        settings.lockedTargetIndex = -1;
    }
}






Vector3 Aimbot::GetClosestEnemy(const Vector3& localPlayerPos, const ViewMatrix& vm, uintptr_t localEntity, uint8_t localTeam, int& closestIndex, float fov) {
    float closestDistance = FLT_MAX;
    Vector3 closestEnemyPosition;
    float fovRadius = tanf(fov * DEG_TO_RAD / 2.0f) * screenDims.centerX;

    for (int i = 0; i < Visuals1::entityCache.size(); ++i) {
        const auto& entity = Visuals1::entityCache[i];


        if (!entity.isValid || entity.entityAddress == localEntity || entity.team == localTeam) {
            continue;
        }


        uint32_t entity_hp = memory::memRead<uint32_t>(entity.entityAddress + 0x34c);
        if (entity_hp <= 0) continue; 

        
        if (!settings.targetLocked || settings.lockedTargetIndex != i) {
            uint32_t isVisible = memory::memRead<uint32_t>(entity.entityAddress + 0x30);
            if (isVisible != 1) continue; 
        }

     
        Vector3 entityHeadPos;
        int headBoneIndex = get_bone_head_index(entity.entityName);
        if (headBoneIndex != -1) {
            entityHeadPos = get_bone_position_by_index(entity.entityAddress, headBoneIndex);
        }
        else {

            entityHeadPos = memory::memRead<Vector3>(entity.entityAddress + offsets::m_vOldOrigin);
            entityHeadPos.Z += 92.0f;
        }


        float distanceToEnemy = (entityHeadPos - localPlayerPos).Length();
        if (distanceToEnemy > settings.maxDistance) continue;  


        Vector3 screenPos = WorldToScreen(vm, entityHeadPos);

        if (!settings.targetLocked || screenPos.Z > 0.01f) {
   
            float angleDistance = GetAngleDistance(screenPos);

      
            if (angleDistance < fovRadius && angleDistance < closestDistance) {
                closestDistance = angleDistance;
                closestEnemyPosition = entityHeadPos;
                closestIndex = i;
            }
        }
    }

    return closestEnemyPosition;
}





void Aimbot::RenderAimbotSettingsMenu() {
    if (ImGui::Checkbox("Enable Aimbot", &settings.enabled)) {
        ImGui::SetNextItemOpen(false);
    }

    if (settings.enabled && ImGui::TreeNode("Aimbot Settings")) {
        ImGui::SliderFloat("Aimbot FOV", &settings.fov, 5.0f, 180.0f, "FOV: %.1f");
        ImGui::SliderFloat("Aimbot Smoothness", &settings.smoothness, 1.0f, 10.0f, "Smoothness: %.1f");
        ImGui::SliderFloat("Max Distance", &settings.maxDistance, 100.0f, 5000.0f, "Max Distance: %.1f");

 
        ImGui::Checkbox("Souls Aim", &settings.soulsAim);


        ImGui::Text("Activation Key:");
        ImGui::SameLine();
        if (ImGui::Button("Choose Key")) {
            ImGui::OpenPopup("key_popup");
        }

        if (ImGui::BeginPopup("key_popup")) {
            ImGui::Text("Press any key to set it as the activation key");
            for (int i = 0; i < 256; ++i) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    settings.activationKey = i;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }

        std::string keyName = GetKeyName(settings.activationKey);
        ImGui::Text("Current Activation Key: %s", keyName.c_str());

        ImGui::TreePop();
    }
}

