#pragma once
#include <string>
#include "memory.h"
#include "math.h"
#include <vector>


ULONG_PTR get_entity_list();
int get_max_entities();
uintptr_t get_base_entity_from_index(int index, ULONG_PTR entity_list);
std::string get_schema_name(const uintptr_t& entity);
std::string get_designer_name(const uintptr_t& entity);
uintptr_t find_local_player(uint8_t& local_team);


std::vector<Vector3> get_bone_positions(uintptr_t entity);
Vector3 get_bone_position_by_index(uintptr_t entity, int bone_index);
int get_bone_head_index(const std::string& model_path);
std::string get_entity_human_name(const std::string& model_path);
uintptr_t find_local_player_pawn(); 