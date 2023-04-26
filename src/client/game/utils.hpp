#pragma once

#include "structs.hpp"

namespace game
{
	[[nodiscard]] std::string get_dvar_string(const char* dvar_name);
	[[nodiscard]] int get_dvar_int(const char* dvar_name);
	[[nodiscard]] bool get_dvar_bool(const char* dvar_name);

	[[nodiscard]] const dvar_t* register_dvar_bool(const char* dvar_name, bool value, unsigned int flags, const char* description);
	[[nodiscard]] const dvar_t* register_dvar_int(const char* dvar_name, int value, int min, int max, unsigned int flags, const char* description);
	[[nodiscard]] const dvar_t* register_dvar_float(const char* dvar_name, float value, float min, float max, unsigned int flags, const char* description);
	[[nodiscard]] const dvar_t* register_sessionmode_dvar_bool(const char* dvar_name, bool value, unsigned int flags, const char* description, eModes mode = MODE_COUNT);
	[[nodiscard]] const dvar_t* register_dvar_string(const char* dvar_name, const char* value, unsigned int flags, const char* description);

	void dvar_add_flags(const char* dvar, unsigned int flags);
	void dvar_set_flags(const char* dvar_name, unsigned int flags);

	[[nodiscard]] bool is_server_running();
	[[nodiscard]] size_t get_max_client_count();

	void foreach_client(const std::function<void(client_s&, size_t index)>& callback);
	void foreach_client(const std::function<void(client_s&)>& callback);

	void foreach_connected_client(const std::function<void(client_s&, size_t index)>& callback);
	void foreach_connected_client(const std::function<void(client_s&)>& callback);

	bool access_connected_client(size_t index, const std::function<void(client_s&)>& callback);
}
