#pragma once
#include "math.h"
#include <cstdint>
#include "ImGui/imgui.h"
#include "entity.h"
#include "memory.h"
#include "offsets.h"
#include <windows.h>
#include <string>
#include <vector>
#include <chrono>


struct EntityCache {
    uintptr_t entityAddress;   
    uint8_t team;            
    bool isValid;             
    std::string entityName;  
    std::string entityAddressStr; 
    int headBoneIndex = -1;   
};




namespace Visuals1 {
    extern std::vector<EntityCache> entityCache;  
    extern std::chrono::time_point<std::chrono::steady_clock> lastCacheUpdate;
    extern const int cacheUpdateInterval; 

    extern bool esp_show_head;  

    
    extern float boxThickness;
    extern float lineThickness;
    extern ImVec4 boxColor;
    extern ImVec4 lineColor;

 
    extern bool esp_box_enabled;
    extern bool esp_line_enabled;

  
    extern bool esp_health_enabled;
    extern bool show_health;
    extern bool show_max_health;

    
    extern bool esp_show_address;

    
    extern bool esp_show_name;


    extern bool esp_show_circle;         
    extern float esp_distance_radius;     


    extern uintptr_t roshanAddress;      
    extern bool roshan_display_enabled;   
    extern bool roshan_debug_mode;    
    extern ImVec2 roshan_position;      
    extern float roshan_font_size;      

 
    void UpdateEntityCache();


    void PlayerEsp(uint8_t& local_team);


    void RenderSettingsMenu();


    void DrawRoshanHp();
}
