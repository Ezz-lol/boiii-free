#pragma once

#include "structs.hpp"

namespace game
{
	std::string get_dvar_string(const char* dvar_name);
	int get_dvar_int(const char* dvar_name);
	bool get_dvar_bool(const char* dvar_name);

	dvar_t* register_dvar_bool(const char* dvar_name, bool value, dvarFlags_e flags, const char* description);
	dvar_t* register_sessionmode_dvar_bool(const char* dvar_name, const bool value, const dvarFlags_e flags, const char* description, const eModes mode);
	void dvar_add_flags(const char* dvar, dvarFlags_e flags);
	void dvar_set_flags(const char* dvar_name, dvarFlags_e flags);
}
