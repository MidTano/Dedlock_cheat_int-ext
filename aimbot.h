#pragma once
#include "math.h"
#include <windows.h>
#include "ImGui/imgui.h"
#include "entity.h"
#include "visuals.h"
#include <string>


struct AimbotSettings {
    bool enabled = true;          
    bool targetLocked = false;      
    int lockedTargetIndex = -1;     
    float fov = 30.0f;             
    float smoothness = 1.6f;        
    bool active = false;            
    int activationKey = VK_RBUTTON; 
    float maxDistance = 2500.0f;   
    bool soulsAim = false;          
};


namespace Aimbot {

    extern AimbotSettings settings;

 
    void AimbotLogic(uintptr_t localEntity, uint8_t localTeam, const Vector3& localPlayerPos, const ViewMatrix& vm);

 
    void RenderAimbotSettingsMenu();
    void DrawFOVCircle(float fov);



    float GetAngleDistance(const Vector3& screenPos);  
    Vector3 GetClosestEnemy(const Vector3& localPlayerPos, const ViewMatrix& vm, uintptr_t localEntity, uint8_t localTeam, int& closestIndex, float fov);
    Vector3 GetClosestSoul(const Vector3& localPlayerPos, const ViewMatrix& vm, int& closestIndex, float fov);  

    std::string GetKeyName(int key);
}

