#pragma once
#include <cstddef>

namespace offsets
{
    extern std::ptrdiff_t dwEntityList;
    extern std::ptrdiff_t dwLocalPlayer;
    extern std::ptrdiff_t CCitadelCameraManager;
    extern std::ptrdiff_t dwViewMatrix;


    void initializeOffsets();


    constexpr std::ptrdiff_t m_modelState = 0x170;
    constexpr std::ptrdiff_t m_vecVelocity = 0x408;
    constexpr std::ptrdiff_t CSkeletonInstance = 0x8;
    constexpr std::ptrdiff_t m_hModel = 0xD0;
    constexpr std::ptrdiff_t m_ModelName = 0xD8;
    constexpr std::ptrdiff_t m_pGameSceneNode = 0x328;


    constexpr std::ptrdiff_t m_iTeamNum = 0x3EB;
    constexpr std::ptrdiff_t m_fFlags = 0x3f8;
    constexpr std::ptrdiff_t m_iHealth = 0x34C;
    constexpr std::ptrdiff_t m_iszPlayerName = 0x640;
    constexpr std::ptrdiff_t m_hPlayerPawn = 0xa90;

    constexpr std::ptrdiff_t m_vecAbsOrigin = 0xD0;
    constexpr std::ptrdiff_t m_vOldOrigin = 0xEE4;
    constexpr std::ptrdiff_t m_vecViewOffset = 0x7d0; 
}
