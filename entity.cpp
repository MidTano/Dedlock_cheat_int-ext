#pragma once
#include "entity.h"
#include <unordered_map>
#include "ImGui/imgui.h"

static const std::unordered_map<std::string, std::string> model_to_name_map = {
    {"models/heroes_staging/atlas_detective_v2/atlas_detective.vmdl", "ABRAMS"},
    {"models/heroes_staging/bebop/bebop.vmdl", "BEBOP"},
    {"models/heroes_staging/prof_dynamo/prof_dynamo.vmdl", "DYNAMO"},
    {"models/heroes_staging/archer/archer.vmdl", "GREY TALON"},
    {"models/heroes_staging/haze/haze.vmdl", "HAZE"},
    {"models/heroes_staging/inferno_v4/inferno.vmdl", "INFERNUS"},
    {"models/heroes_staging/tengu/tengu_v2/tengu.vmdl", "IVY"},
    {"models/heroes_staging/kelvin_v2/kelvin.vmdl", "KELVIN"},
    {"models/heroes_staging/ghost/ghost.vmdl", "LADY GEIST"},
    {"models/heroes_staging/lash_v2/lash.vmdl", "LASH"},
    {"models/heroes_staging/engineer/engineer.vmdl", "MCGINNIS"},
    {"models/heroes_staging/digger/digger.vmdl", "MO & KRILL"},
    {"models/heroes_staging/chrono/chrono.vmdl", "PARADOX"},
    {"models/heroes_staging/synth/synth.vmdl", "POCKET"},
    {"models/heroes_staging/gigawatt_prisoner/gigawatt_prisoner.vmdl", "SEVEN"},
    {"models/heroes_staging/shiv/shiv.vmdl", "SHIV"},
    {"models/heroes_staging/hornet_v3/hornet.vmdl", "VINDICTA"},
    {"models/heroes_staging/viscous/viscous.vmdl", "VISCOUS"},
    {"models/heroes_staging/warden/warden.vmdl", "WARDEN"},
    {"models/heroes_staging/wraith_gen_man/wraith_gen_man.vmdl", "WRAITH"},
    {"models/heroes_staging/yamato_v2/yamato.vmdl", "YAMATO"}
};


static const std::unordered_map<std::string, int> model_to_head_bone_map = {
    {"ABRAMS", 7},
    {"BEBOP", 6},
    {"DYNAMO", 13},
    {"GREY TALON", 17},
    {"HAZE", 7},
    {"INFERNUS", 30},
    {"IVY", 13},
    {"KELVIN", 12},
    {"LADY GEIST", 11},
    {"LASH", 12},
    {"MCGINNIS", 7},
    {"MO & KRILL", 10},
    {"PARADOX", 8},
    {"POCKET", 13},
    {"SEVEN", 14},
    {"SHIV", 13},
    {"VINDICTA", 7},
    {"VISCOUS", 7},
    {"WARDEN", 11},
    {"WRAITH", 7},
    {"YAMATO", 35},
    {"models/heroes_staging/gen_man/gen_man.vmdl", 12}
};


int get_bone_head_index(const std::string& model_path) {
    auto it = model_to_head_bone_map.find(model_path);
    if (it != model_to_head_bone_map.end()) {
        return it->second;
    }
    return -1;
}




Vector3 get_bone_position_by_index(uintptr_t entity, int bone_index) {
    uintptr_t gameSceneNode = memory::memRead<uintptr_t>(entity + offsets::m_pGameSceneNode);
    uintptr_t boneArray = memory::memRead<uintptr_t>(gameSceneNode + offsets::m_modelState + 0x80);

    if (!boneArray) return Vector3();

    uintptr_t boneAddress = boneArray + bone_index * 0x20;
    return memory::memRead<Vector3>(boneAddress);
}

std::vector<Vector3> get_bone_positions(uintptr_t entity) {
    std::vector<Vector3> bone_positions;

    uintptr_t gameSceneNode = memory::memRead<uintptr_t>(entity + offsets::m_pGameSceneNode);
    uintptr_t boneArray = memory::memRead<uintptr_t>(gameSceneNode + offsets::m_modelState + 0x80);

    if (!boneArray) return bone_positions;

    static const std::vector<int> bone_indices = { 7, 6, 13, 17 }; 
    bone_positions.reserve(bone_indices.size());  

    for (int bone_index : bone_indices) {
        uintptr_t boneAddress = boneArray + bone_index * 0x20;
        Vector3 bonePos = memory::memRead<Vector3>(boneAddress);
        bone_positions.push_back(bonePos);
    }

    return bone_positions;
}


std::string get_entity_human_name(const std::string& model_path) {
    auto it = model_to_name_map.find(model_path);
    return (it != model_to_name_map.end()) ? it->second : "Unknown";
}



ULONG_PTR get_entity_list() {
    return static_cast<ULONG_PTR>(memory::memRead<uintptr_t>(memory::baseAddress + offsets::dwEntityList));
}

int get_max_entities() { 
    ULONG_PTR entity_list = get_entity_list();
    int max_entities = memory::memRead<int>(entity_list + 0x1520);
    return max_entities;
}

uintptr_t get_base_entity_from_index(int index, ULONG_PTR entity_list) {
    uintptr_t entity_base = entity_list + 8LL * ((index & 0x7FFF) >> 9) + 16;
    uintptr_t entity = memory::memRead<uintptr_t>(entity_base);

    if (entity) {
        entity += 120LL * (index & 0x1FF);
        return memory::memRead<uintptr_t>(entity);
    }

    return 0;
}

std::string get_schema_name(const uintptr_t& entity) {
    const uintptr_t entity_identity = memory::memRead<uintptr_t>(entity + 0x10);
    if (!entity_identity) return "";
    const uintptr_t entity_class_info = memory::memRead<uintptr_t>(entity_identity + 0x8);
    const uintptr_t schema_class_info_data = memory::memRead<uintptr_t>(entity_class_info + 0x28);
    const uintptr_t class_name = memory::memRead<uintptr_t>(schema_class_info_data + 0x8);
    return read_str(class_name);
}

std::string get_designer_name(const uintptr_t& entity) {
    const uintptr_t entity_identity = memory::memRead<uintptr_t>(entity + 0x10);
    if (!entity_identity) return "";
    const uintptr_t designer_name = memory::memRead<uintptr_t>(entity_identity + 0x20);
    return read_str(designer_name);
}

uintptr_t find_local_player(uint8_t& local_team) {
    ULONG_PTR entity_list = get_entity_list();
    int max_entities = get_max_entities();

    for (int i = 1; i <= max_entities; ++i) {
        auto entity = get_base_entity_from_index(i, entity_list);
        if (!entity) continue;

        const bool is_local = memory::memRead<bool>(entity + 0x6d0);
        if (is_local) {
            local_team = memory::memRead<uint8_t>(entity + 0x3eb);
            return entity;
        }
    }

    return 0;
}

uintptr_t find_local_player_pawn() {

    uint8_t local_team = 0;
    uintptr_t local_player = find_local_player(local_team);
    if (!local_player) return 0;


    ULONG_PTR entity_list = get_entity_list();
    int max_entities = get_max_entities();

    uintptr_t closest_pawn = 0;
    uintptr_t local_controller = memory::memRead<uintptr_t>(local_player + 0x60c); 
    uintptr_t local_address = local_player;



    for (int i = 1; i <= max_entities; ++i) {
        uintptr_t entity = get_base_entity_from_index(i, entity_list);
        if (!entity) continue;

        std::string designer_name = get_designer_name(entity);


        if (designer_name != "player") continue;

        uintptr_t pawn_controller = memory::memRead<uintptr_t>(entity); 
        if (pawn_controller != local_controller) continue;


        if (closest_pawn == 0 || std::llabs(static_cast<intptr_t>(entity) - local_address) < std::llabs(static_cast<intptr_t>(closest_pawn) - local_address)) {
            closest_pawn = entity;
        }
    }

    return local_player;
}
