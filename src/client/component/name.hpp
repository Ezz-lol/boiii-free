#pragma once
#include <string>

namespace name {
const char *get_player_name();

void set_name_override(int client_num, const std::string &n);
void set_tag_override(int client_num, const std::string &t);
void clear_name_override(int client_num);
void clear_tag_override(int client_num);
void clear_all_overrides();
bool has_name_override(int client_num);
std::string get_name_override(int client_num);
bool has_tag_override(int client_num);
std::string get_tag_override(int client_num);
void trigger_client_update(int client_num);
}
