#include "freecam.h"
#include "memory.h"
#include "math.h"
#include <cstdint>  
#include <windows.h>
#include <vector>
#include "ImGui/imgui.h"
#include "visuals.h"
#include <map>
#include <chrono>

namespace freecam {

    uintptr_t freeCamAddress = 0;
    uintptr_t heightAddress = 0;
    uintptr_t cammanagerAddress = 0;
    Vector3 cammanager_pos = { 0.0f, 0.0f, 0.0f };
    bool isFreeCamEnabled = false;
    uintptr_t currentTarget = 0;  
    bool isKeyPressed = false;    
    bool isMasterEnabled = false; 

    BYTE originalBytes[] = { 0xF2, 0x0F, 0x11, 0x7F, 0x38 };
    BYTE nopBytes[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

    BYTE originalHeightBytes[] = { 0x89, 0x5F, 0x40 };
    BYTE nopHeightBytes[] = { 0x90, 0x90, 0x90 };

    void EnableFreeCam() {
        if (freeCamAddress == 0) {
            std::vector<BYTE> patternBytes;
            std::string mask;
            memory::parsePattern("F2 0F 11 7F 38", patternBytes, mask);
            freeCamAddress = memory::patternScan(patternBytes, mask);
        }

        if (heightAddress == 0) {
            std::vector<BYTE> patternBytes;
            std::string mask;
            memory::parsePattern("89 5F ? 48 81 C4 A8 01 00 00", patternBytes, mask);
            heightAddress = memory::patternScan(patternBytes, mask);
        }

        if (cammanagerAddress == 0) {
            cammanagerAddress = memory::memRead<uintptr_t>(memory::baseAddress + offsets::CCitadelCameraManager + 0x28);
        }

        if (cammanagerAddress != 0) {
            cammanager_pos = memory::memRead<Vector3>(cammanagerAddress + 0x38);
        }

        if (freeCamAddress != 0) {
            memory::memSafeWrite(freeCamAddress, nopBytes, sizeof(nopBytes));
            isFreeCamEnabled = true;
        }

        if (heightAddress != 0) {
            memory::memSafeWrite(heightAddress, nopHeightBytes, sizeof(nopHeightBytes));
        }
    }

    void DisableFreeCam() {
        if (freeCamAddress != 0) {
            memory::memSafeWrite(freeCamAddress, originalBytes, sizeof(originalBytes));
            isFreeCamEnabled = false;
        }

        if (heightAddress != 0) {
            memory::memSafeWrite(heightAddress, originalHeightBytes, sizeof(originalHeightBytes));
        }

        currentTarget = 0;
    }

    
    void UpdateCameraPosition() {
        if (isMasterEnabled && currentTarget != 0 && cammanagerAddress != 0) {
            Vector3 enemy_pos = memory::memRead<Vector3>(currentTarget + offsets::m_vOldOrigin);
            enemy_pos.Z += 220.f;
            enemy_pos.X += 100.f;
            memory::memWrite(cammanagerAddress + 0x38, enemy_pos);
        }
    }

    
    void NavigateEnemies(uint8_t local_team) {
        if (!isMasterEnabled) {
            return;  
        }

        int enemyIndex = -1;

        if (!isKeyPressed) {
            if (GetAsyncKeyState('5') & 0x8000) {
                enemyIndex = 0;
                isKeyPressed = true;
            }
            else if (GetAsyncKeyState('6') & 0x8000) {
                enemyIndex = 1;
                isKeyPressed = true;
            }
            else if (GetAsyncKeyState('7') & 0x8000) {
                enemyIndex = 2;
                isKeyPressed = true;
            }
            else if (GetAsyncKeyState('8') & 0x8000) {
                enemyIndex = 3;
                isKeyPressed = true;
            }
            else if (GetAsyncKeyState('9') & 0x8000) {
                enemyIndex = 4;
                isKeyPressed = true;
            }
            else if (GetAsyncKeyState('0') & 0x8000) {
                enemyIndex = 5;
                isKeyPressed = true;
            }
        }
        else {
          
            if (!(GetAsyncKeyState('5') & 0x8000) &&
                !(GetAsyncKeyState('6') & 0x8000) &&
                !(GetAsyncKeyState('7') & 0x8000) &&
                !(GetAsyncKeyState('8') & 0x8000) &&
                !(GetAsyncKeyState('9') & 0x8000) &&
                !(GetAsyncKeyState('0') & 0x8000)) {
                isKeyPressed = false;
            }
        }

        if (enemyIndex >= 0 && enemyIndex < Visuals1::entityCache.size()) {
            int validEnemyIndex = 0;
            for (const auto& entity : Visuals1::entityCache) {
                if (entity.team != local_team && entity.isValid) {
                    if (validEnemyIndex == enemyIndex) {
                        if (currentTarget == entity.entityAddress) {
                            DisableFreeCam();  
                            return;
                        }

                        currentTarget = entity.entityAddress;
                        Vector3 enemy_pos = memory::memRead<Vector3>(entity.entityAddress + offsets::m_vOldOrigin);
                        enemy_pos.Z += 220.f;
                        enemy_pos.X += 100.f;
                        if (cammanagerAddress != 0) {
                            memory::memWrite(cammanagerAddress + 0x38, enemy_pos);
                        }

                        if (!isFreeCamEnabled) {
                            EnableFreeCam();
                        }

                        break;
                    }
                    validEnemyIndex++;
                }
            }
        }
    }


    void MasterSwitch(bool enable) {
        isMasterEnabled = enable;
        if (!isMasterEnabled) {
    
            DisableFreeCam();
        }
    }

    void RenderFreeCamMenu() {
      
        if (ImGui::Checkbox("Enable FreeCam", &isMasterEnabled)) {
            MasterSwitch(isMasterEnabled);
            
        }
        ImGui::Text("Press 5,6,7,8,9,0 to swap enemy");
        ImGui::Text("Doble tap return to main hero");
    }
}

