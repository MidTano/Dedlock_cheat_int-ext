#pragma once
#include <cstdint>
#include "math.h"

namespace freecam {

    extern uintptr_t freeCamAddress;
    extern uintptr_t heightAddress;
    extern uintptr_t cammanagerAddress;
    extern Vector3 cammanager_pos;
    extern bool isFreeCamEnabled;
    extern uintptr_t currentTarget;
    extern bool isKeyPressed;  

    void EnableFreeCam();
    void DisableFreeCam();
    void NavigateEnemies(uint8_t local_team);
    void UpdateCameraPosition();  
    void RenderFreeCamMenu();
}
