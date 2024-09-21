#include "visuals.h"
#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>


std::string read_str_from_memory(uintptr_t address) {
    uintptr_t pointer_to_string = memory::memRead<uintptr_t>(address); 
    if (!pointer_to_string) {
        return ""; 
    }

    const size_t max_length = 1024; 
    char buffer[max_length] = { 0 };

    SIZE_T bytesRead;
    if (!ReadProcessMemory(memory::processHandle, reinterpret_cast<LPCVOID>(pointer_to_string), buffer, max_length - 1, &bytesRead) || bytesRead == 0) {
        return "";  
    }

  
    buffer[max_length - 1] = '\0';

    return std::string(buffer);
}


enum HealthDisplayMode {
    HEALTH_NONE = 0,
    HEALTH_BAR,
    HEALTH_NUMBER
};

HealthDisplayMode health_display_mode = HEALTH_BAR;


enum RoshanDisplayMode {
    ROSHAN_HP_NUMBER,
    ROSHAN_HP_BAR
};


RoshanDisplayMode roshan_display_mode = ROSHAN_HP_NUMBER;



ImColor LerpColor(const ImColor& col1, const ImColor& col2, float t) {
    float r = col1.Value.x + t * (col2.Value.x - col1.Value.x);
    float g = col1.Value.y + t * (col2.Value.y - col1.Value.y);
    float b = col1.Value.z + t * (col2.Value.z - col1.Value.z);
    return ImColor(r, g, b);
}


const float screen_center_x = GetSystemMetrics(SM_CXSCREEN) / 2.0f;
const float screen_height = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));

bool Visuals1::roshan_debug_mode = false;     
ImVec2 Visuals1::roshan_position = ImVec2(screen_center_x-40, 175.0f);  
float Visuals1::roshan_font_size = 24.0f;      




uintptr_t Visuals1::roshanAddress = 0;      
bool Visuals1::roshan_display_enabled = true;


std::vector<EntityCache> Visuals1::entityCache;
std::chrono::time_point<std::chrono::steady_clock> Visuals1::lastCacheUpdate = std::chrono::steady_clock::now();
const int Visuals1::cacheUpdateInterval = 10000; 


float Visuals1::boxThickness = 1.0f;      
float Visuals1::lineThickness = 1.0f;     
ImVec4 Visuals1::boxColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  
ImVec4 Visuals1::lineColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); 

bool Visuals1::esp_show_head = false;
bool Visuals1::esp_show_circle = true; 
float Visuals1::esp_distance_radius = 2400.0f; 


bool Visuals1::esp_box_enabled = true;   
bool Visuals1::esp_line_enabled = true;  


bool Visuals1::esp_health_enabled = true;  
bool Visuals1::show_health = true;         
bool Visuals1::show_max_health = true;    


bool Visuals1::esp_show_address = false;


bool Visuals1::esp_show_name = false;

void Visuals1::UpdateEntityCache() {
    ULONG_PTR entity_list = get_entity_list();
    int max_ents = get_max_entities();
    entityCache.clear();
    entityCache.reserve(max_ents);  

    roshanAddress = 0;

    for (int i = 1; i <= max_ents; ++i) {
        uintptr_t entity = get_base_entity_from_index(i, entity_list);
        if (!entity) continue;



      
        std::string designer_name = get_designer_name(entity);

        if (designer_name == "npc_super_neutral") {
            roshanAddress = entity;  
        }

        if (designer_name != "player") continue;

 
        EntityCache cache;
        cache.entityAddress = entity;
        cache.team = memory::memRead<uint8_t>(entity + 0x3eb);
        cache.isValid = true;

 
        std::ostringstream ss;
        ss << "0x" << std::hex << std::uppercase << entity;
        cache.entityAddressStr = ss.str();

    
        uintptr_t name_ptr = memory::memRead<uintptr_t>(entity + 0x328);
        if (name_ptr) {
 
            std::string model_path = read_str_from_memory(name_ptr + 0x248);
       
            cache.entityName = get_entity_human_name(model_path);

 
            cache.headBoneIndex = get_bone_head_index(cache.entityName);
        }
        else {
            cache.entityName = "Unknown"; 
            cache.headBoneIndex = -1; 
        }

        entityCache.push_back(cache);
    }

    lastCacheUpdate = std::chrono::steady_clock::now(); 
}


void Visuals1::DrawRoshanHp() {
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    if (GetAsyncKeyState(VK_TAB) & 0x8000) {

        return;
    }


    const ImVec2 text_pos = roshan_position;
    const float font_size = roshan_font_size;

    uint32_t roshan_hp = 0;
    uint32_t roshan_max_hp = 0;

    if (roshan_debug_mode) {
   
        roshan_hp = 500;   
        roshan_max_hp = 1000;
    }
    else {
      
        if (!roshanAddress) {
           
            draw_list->AddText(nullptr, font_size, text_pos, ImColor(255, 0, 0), "Roshan\n DEAD");
            return;
        }


        roshan_hp = memory::memRead<uint32_t>(roshanAddress + 0x34c);
        roshan_max_hp = memory::memRead<uint32_t>(roshanAddress + 0x348);
    }


    if (roshan_hp <= 0) {
        draw_list->AddText(nullptr, font_size, text_pos, ImColor(255, 0, 0), "Roshan\n DEAD");
        return;
    }

    if (roshan_display_mode == ROSHAN_HP_NUMBER) {
   
        std::string roshan_hp_info = std::to_string(roshan_hp) + " / " + std::to_string(roshan_max_hp);


        draw_list->AddText(nullptr, font_size, text_pos, ImColor(255, 255, 255), "Roshan");


        draw_list->AddText(nullptr, font_size, ImVec2(text_pos.x - 40.0f, text_pos.y + font_size + 10.0f), ImColor(255, 255, 255), roshan_hp_info.c_str());
    }
    else if (roshan_display_mode == ROSHAN_HP_BAR) {

        const float bar_width = 150.0f;
        const float bar_height = 20.0f;

 
        float health_percentage = static_cast<float>(roshan_hp) / static_cast<float>(roshan_max_hp);

  
        ImColor health_color = LerpColor(ImColor(255, 0, 0), ImColor(0, 255, 0), health_percentage);


        draw_list->AddText(nullptr, font_size, text_pos, ImColor(255, 255, 255), "Roshan");

 
        const ImVec2 bar_pos_min(text_pos.x - bar_width / 2 + bar_width / 4, text_pos.y + font_size + 10.0f);
        const ImVec2 bar_pos_max(text_pos.x + bar_width / 2 + bar_width / 4, text_pos.y + font_size + 10.0f + bar_height);


        draw_list->AddRect(bar_pos_min, bar_pos_max, ImColor(255, 255, 255));

    
        draw_list->AddRectFilled(
            ImVec2(bar_pos_min.x + 1.0f, bar_pos_min.y + 1.0f), 
            ImVec2(bar_pos_min.x + (bar_width - 2.0f) * health_percentage, bar_pos_max.y - 1.0f),
            health_color);  
    }
}








void Visuals1::PlayerEsp(uint8_t& local_team) {
    uintptr_t local_pawn = memory::memRead<uintptr_t>(memory::baseAddress + 0x1F44280); 
    if (!local_pawn) return;

    Vector3 local_player_pos = memory::memRead<Vector3>(local_pawn + offsets::m_vOldOrigin); 

    ViewMatrix view_matrix = get_view_matrix();  

  
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCacheUpdate).count() > cacheUpdateInterval) {
        UpdateEntityCache();
    }

   
    for (auto& cache : entityCache) {
        if (!cache.isValid || cache.entityAddress == local_pawn) {
            continue; 
        }

    
        if (cache.team == local_team) continue;

  
        uint32_t health = memory::memRead<uint32_t>(cache.entityAddress + 0x34c);
        if (health <= 0) continue;

        uint32_t health_max = memory::memRead<uint32_t>(cache.entityAddress + 0x348);
        if (health > health_max) {
            health_max = health;  
        }

        
        Vector3 entity_world_pos = memory::memRead<Vector3>(cache.entityAddress + offsets::m_vOldOrigin);

        Vector3 entity_head_pos;
        if (esp_show_head && cache.headBoneIndex != -1) {
            entity_head_pos = get_bone_position_by_index(cache.entityAddress, cache.headBoneIndex);
        }
        else {
            entity_head_pos = entity_world_pos;
            entity_head_pos.Z += 92.0f; 
        }

      
        Vector3 screen_pos_foot = WorldToScreen(view_matrix, entity_world_pos);
        Vector3 screen_pos_head = WorldToScreen(view_matrix, entity_head_pos);
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();

        if (screen_pos_foot.Z < 0.01f || screen_pos_head.Z < 0.01f) continue;
        if (esp_line_enabled) {
            draw_list->AddLine(
                ImVec2(screen_center_x, screen_height), 
                ImVec2(screen_pos_foot.X, screen_pos_foot.Y),  
                ImColor(lineColor), lineThickness);  
        }

 
        float distance_to_enemy = sqrt(pow(local_player_pos.X - entity_world_pos.X, 2) +
            pow(local_player_pos.Y - entity_world_pos.Y, 2) +
            pow(local_player_pos.Z - entity_world_pos.Z, 2));

 
        if (esp_show_circle && distance_to_enemy <= esp_distance_radius) {
            float head_circle_y_offset = 20.0f;
            draw_list->AddCircleFilled(
                ImVec2(screen_pos_head.X, screen_pos_head.Y - head_circle_y_offset),
                8.0f, ImColor(0, 255, 0)); 
        }

      
        float box_height = screen_pos_foot.Y - screen_pos_head.Y;
        float box_width = box_height / 2.0f;


        if (esp_box_enabled) {
            draw_list->AddRect(
                ImVec2(screen_pos_head.X - box_width / 2, screen_pos_head.Y),
                ImVec2(screen_pos_head.X + box_width / 2, screen_pos_foot.Y),
                ImColor(boxColor), 0.0f, 0, boxThickness);
        }

 
        if (esp_show_head) {
            float head_radius = 5.0f;
            draw_list->AddCircle(ImVec2(screen_pos_head.X, screen_pos_head.Y), head_radius, ImColor(255, 255, 0), 16, 2.0f); // Желтый цвет
        }

   
        if (esp_health_enabled) {
            if (health_display_mode == HEALTH_BAR) {
    
                float health_percentage = static_cast<float>(health) / static_cast<float>(health_max);

      
                ImColor health_color = LerpColor(ImColor(255, 0, 0), ImColor(0, 255, 0), health_percentage);

               
                float health_bar_x = screen_pos_head.X - box_width / 2 - 8.0f; 
                float health_bar_y = screen_pos_head.Y;
                float health_bar_width = 3.5f;
                float health_bar_height = box_height;

             
                draw_list->AddRect(
                    ImVec2(health_bar_x, screen_pos_head.Y),
                    ImVec2(health_bar_x + health_bar_width, screen_pos_foot.Y),
                    ImColor(255, 255, 255), 0.0f, 0, 1.0f);

       
                float filled_health_height = box_height * health_percentage;
                draw_list->AddRectFilled(
                    ImVec2(health_bar_x + 0.5f, screen_pos_foot.Y - filled_health_height),  
                    ImVec2(health_bar_x + health_bar_width - 0.5f, screen_pos_foot.Y),      
                    health_color);
            }
            else if (health_display_mode == HEALTH_NUMBER) {
         
                std::string health_info = std::to_string(health) + " / " + std::to_string(health_max);
                draw_list->AddText(
                    ImVec2(screen_pos_foot.X - 40.f, screen_pos_foot.Y), ImColor(255, 255, 255), health_info.c_str());
            }
        }



 
        if (esp_show_name && !cache.entityName.empty()) {
            draw_list->AddText(
                ImVec2(screen_pos_head.X, screen_pos_head.Y - 15.0f), ImColor(255, 255, 255), cache.entityName.c_str());
        }

   
        if (esp_show_address) {
            draw_list->AddText(
                ImVec2(screen_pos_foot.X, screen_pos_foot.Y + 20.0f), ImColor(255, 255, 255), cache.entityAddressStr.c_str());
        }
        if (roshan_display_enabled) {
            DrawRoshanHp();
        }
        
    }
}




void Visuals1::RenderSettingsMenu() {

    static bool roshan_settings_visible = true;
    if (ImGui::Checkbox("Show Roshan HP", &roshan_display_enabled)) {
        roshan_settings_visible = roshan_display_enabled;
    }

    if (roshan_settings_visible) {
        ImGui::Indent();

    
        if (ImGui::RadioButton("Show HP as Numbers", roshan_display_mode == ROSHAN_HP_NUMBER)) {
            roshan_display_mode = ROSHAN_HP_NUMBER;
        }
        if (ImGui::RadioButton("Show HP as Bar", roshan_display_mode == ROSHAN_HP_BAR)) {
            roshan_display_mode = ROSHAN_HP_BAR;
        }

    
        ImGui::Checkbox("Enable Debug Mode (500 / 1000 HP)", &roshan_debug_mode);

       
        ImGui::SliderFloat("Roshan X Position", &roshan_position.x, 0.0f, GetSystemMetrics(SM_CXSCREEN), "X: %.0f");
        ImGui::SliderFloat("Roshan Y Position", &roshan_position.y, 0.0f, GetSystemMetrics(SM_CYSCREEN), "Y: %.0f");

     
        ImGui::SliderFloat("Roshan Font Size", &roshan_font_size, 10.0f, 72.0f, "Font Size: %.0f");

        ImGui::Unindent();
    }


    static bool circle_settings_visible = true;
    if (ImGui::Checkbox("Enable ESP Circle", &esp_show_circle)) {
        circle_settings_visible = esp_show_circle;
    }
    if (circle_settings_visible) {
        ImGui::Indent();
        ImGui::SliderFloat("Circle Distance Radius", &esp_distance_radius, 500.0f, 5000.0f, "Radius: %.0f");
        ImGui::Unindent();
    }


    static bool box_settings_visible = true;
    if (ImGui::Checkbox("Enable ESP Box", &esp_box_enabled)) {
        box_settings_visible = esp_box_enabled;
    }
    if (box_settings_visible) {
        ImGui::Indent();
        ImGui::SliderFloat("Box Thickness", &boxThickness, 1.0f, 10.0f, "Thickness: %.1f");
        ImGui::ColorEdit4("Box Color", (float*)&boxColor);
        ImGui::Unindent();
    }


    static bool line_settings_visible = true;
    if (ImGui::Checkbox("Enable ESP Line", &esp_line_enabled)) {
        line_settings_visible = esp_line_enabled;
    }
    if (line_settings_visible) {
        ImGui::Indent();
        ImGui::SliderFloat("Line Thickness", &lineThickness, 1.0f, 10.0f, "Thickness: %.1f");
        ImGui::ColorEdit4("Line Color", (float*)&lineColor);
        ImGui::Unindent();
    }


    static bool health_settings_visible = true;
    if (ImGui::Checkbox("Enable ESP Health", &esp_health_enabled)) {
        health_settings_visible = esp_health_enabled;
    }
    if (health_settings_visible) {
        ImGui::Indent();
        ImGui::Text("Health Display Mode");
        if (ImGui::RadioButton("Health Bar", health_display_mode == HEALTH_BAR)) {
            health_display_mode = HEALTH_BAR;
        }
        if (ImGui::RadioButton("Health Number", health_display_mode == HEALTH_NUMBER)) {
            health_display_mode = HEALTH_NUMBER;
        }
        ImGui::Unindent();
    }


    static bool name_settings_visible = false;
    if (ImGui::Checkbox("Show Entity Name", &esp_show_name)) {
        name_settings_visible = esp_show_name;
    }

 
    static bool address_settings_visible = false;
    if (ImGui::Checkbox("Show Entity Address", &esp_show_address)) {
        address_settings_visible = esp_show_address;
    }

 
    static bool head_settings_visible = false;
    if (ImGui::Checkbox("Show Entity Head", &esp_show_head)) {
        head_settings_visible = esp_show_head;
    }
}
