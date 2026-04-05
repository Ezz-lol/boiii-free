#include <std_include.hpp>

#include "game.hpp"
#include "structs.hpp"
#include "utils.hpp"

namespace game
{
	static_assert(offsetof(dvar_t, debugName) == 8);
	static_assert(offsetof(dvar_t, description) == 16);
	static_assert(offsetof(dvar_t, flags) == 24);
	static_assert(offsetof(dvar_t, type) == 28);
	static_assert(offsetof(dvar_t, modified) == 32);
	static_assert(offsetof(dvar_t, current) == 40);

	namespace
	{
		dvar_t* try_get_sessionmode_specific_dvar(dvar_t* dvar)
		{
			if (!dvar || dvar->type != DVAR_TYPE_SESSIONMODE_BASE_DVAR)
			{
				return dvar;
			}

			if (Com_SessionMode_IsMode(MODE_COUNT))
			{
				return nullptr;
			}

			const auto mode = Com_SessionMode_GetMode();
			return Dvar_GetSessionModeSpecificDvar(dvar, static_cast<eModes>(mode));
		}
	}

	std::string get_dvar_string(const char* dvar_name)
	{
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return Dvar_GetString(dvar);
	}

	int get_dvar_int(const char* dvar_name)
	{
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return dvar->current.value.integer;
	}

	int set_dvar_int(const char* dvar_name, int val, DvarSetSource source) {
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}
		int prev_val = dvar->current.value.integer;
		Dvar_SetIntFromSource(dvar, val, source);
		return prev_val;
	}

	uint32_t get_dvar_uint(const char* dvar_name)
	{
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return dvar->current.value.unsignedInt;
	}

	uint64_t get_dvar_uint64(const char* dvar_name)
	{
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return dvar->current.value.unsignedInt64; 
	}

	uint64_t set_dvar_uint64(const char* dvar_name, uint64_t val, DvarSetSource source) {
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}
		uint64_t prev_val = dvar->current.value.unsignedInt64;
		Dvar_SetUInt64FromSource(dvar, val, source);
		return prev_val;
	}

	int64_t get_dvar_int64(const char* dvar_name)
	{
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return dvar->current.value.integer64; 
	}

	int64_t set_dvar_int64(const char* dvar_name, int64_t val, DvarSetSource source) {
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}
		int64_t prev_val = dvar->current.value.integer64;
		Dvar_SetInt64FromSource(dvar, val, source);
		return prev_val;
	}

	bool get_dvar_bool(const char* dvar_name)
	{
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return dvar->current.value.enabled;
	}

	bool set_dvar_bool(const char* dvar_name, bool val, DvarSetSource source) {
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}
		bool prev_val = dvar->current.value.enabled;
		Dvar_SetBoolFromSource(dvar, val, source);
		return prev_val;
	}

	float get_dvar_float(const char* dvar_name)
	{
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return dvar->current.value.value;
	}

	float set_dvar_float(const char* dvar_name, float val, DvarSetSource source) {
		const auto* dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}
		float prev_val = dvar->current.value.value;
		Dvar_SetFloatFromSource(dvar, val, source);
		return prev_val;
	}

	const dvar_t* register_sessionmode_dvar_bool(const char* dvar_name, const bool value, const unsigned int flags,
	                                             const char* description, const eModes mode)
	{
		const auto hash = Dvar_GenerateHash(dvar_name);
		auto* registered_dvar = Dvar_SessionModeRegisterBool(hash, dvar_name, value, flags, description);

		if (registered_dvar)
		{
			registered_dvar->debugName = dvar_name;

			if (mode == MODE_COUNT)
			{
				for (int i = MODE_FIRST; i < MODE_COUNT; ++i)
				{
					Dvar_SessionModeSetDefaultBool.call_safe(hash, value, static_cast<eModes>(i));
				}
			}
			else
			{
				Dvar_SessionModeSetDefaultBool.call_safe(hash, value, mode);
			}
		}

		return registered_dvar;
	}

	const dvar_t* register_dvar_bool(const char* dvar_name, const bool value, const unsigned int flags,
	                                 const char* description)
	{
		const auto hash = Dvar_GenerateHash(dvar_name);
		auto* registered_dvar = Dvar_RegisterBool(hash, dvar_name, value, flags, description);

		if (registered_dvar)
		{
			registered_dvar->debugName = dvar_name;
		}

		return registered_dvar;
	}

	const dvar_t* register_dvar_int(const char* dvar_name, int value, int min, int max, const unsigned int flags,
	                                const char* description)
	{
		const auto hash = Dvar_GenerateHash(dvar_name);
		auto* registered_dvar = Dvar_RegisterInt(hash, dvar_name, value, min, max, flags, description);

		if (registered_dvar)
		{
			registered_dvar->debugName = dvar_name;
		}

		return registered_dvar;
	}

	const dvar_t* register_dvar_float(const char* dvar_name, float value, float min, float max,
	                                  const unsigned int flags,
	                                  const char* description)
	{
		const auto hash = Dvar_GenerateHash(dvar_name);
		auto* registered_dvar = Dvar_RegisterFloat(hash, dvar_name, value, min, max, flags, description);

		if (registered_dvar)
		{
			registered_dvar->debugName = dvar_name;
		}

		return registered_dvar;
	}

	const dvar_t* register_dvar_string(const char* dvar_name, const char* value, const unsigned int flags,
	                                   const char* description)
	{
		const auto hash = Dvar_GenerateHash(dvar_name);
		auto* registered_dvar = Dvar_RegisterString(hash, dvar_name, value, flags, description);

		if (registered_dvar)
		{
			registered_dvar->debugName = dvar_name;
		}

		return registered_dvar;
	}

	void dvar_add_flags(const char* dvar_name, const unsigned int flags)
	{
		auto* dvar = Dvar_FindVar(dvar_name);

		if (!dvar)
		{
			return;
		}

		auto* dvar_to_change = dvar;
		if (dvar_to_change->type == DVAR_TYPE_SESSIONMODE_BASE_DVAR)
		{
			dvar_to_change = try_get_sessionmode_specific_dvar(dvar_to_change);
			if (!dvar_to_change)
			{
				return;
			}
		}

		dvar_to_change->flags |= flags;
	}

	void dvar_set_flags(const char* dvar_name, const unsigned int flags)
	{
		auto* dvar = Dvar_FindVar(dvar_name);

		if (!dvar)
		{
			return;
		}

		auto* dvar_to_change = dvar;
		if (dvar_to_change->type == DVAR_TYPE_SESSIONMODE_BASE_DVAR)
		{
			dvar_to_change = try_get_sessionmode_specific_dvar(dvar_to_change);
			if (!dvar_to_change)
			{
				return;
			}
		}

		dvar_to_change->flags = flags;
	}

	void dvar_remove_flags(const char* dvar_name, const unsigned int flags)
	{
		auto* dvar = Dvar_FindVar(dvar_name);

		if (!dvar)
		{
			return;
		}

		auto* dvar_to_change = dvar;
		if (dvar_to_change->type == DVAR_TYPE_SESSIONMODE_BASE_DVAR)
		{
			dvar_to_change = try_get_sessionmode_specific_dvar(dvar_to_change);
			if (!dvar_to_change)
			{
				return;
			}
		}

		dvar_to_change->flags &= ~flags;
	}

	bool is_server_running()
	{
		return get_dvar_bool("sv_running");
	}

	size_t get_max_client_count()
	{
		return static_cast<size_t>(get_dvar_int("com_maxclients"));
	}

	template <typename T>
	static void foreach_client(T* client_states, const std::function<void(client_s&, size_t index)>& callback)
	{
		if (!client_states || !callback)
		{
			return;
		}

		for (size_t i = 0; i < get_max_client_count(); ++i)
		{
			callback(client_states[i], i);
		}
	}


	template <typename T>
	static bool access_client(T* client_states, const size_t index, const std::function<void(client_s&)>& callback)
	{
		if (!client_states || !callback)
		{
			return false;
		}

		if (index >= get_max_client_count())
		{
			return false;
		}

		auto& client = client_states[index];
		if (client.state == CS_FREE)
		{
			return false;
		}

		callback(client);
		return true;
	}

	void foreach_client(const std::function<void(client_s&, size_t index)>& callback)
	{
		if (is_server())
		{
			foreach_client(*svs_clients, callback);
		}
		else
		{
			foreach_client(*svs_clients_cl, callback);
		}
	}

	void foreach_client(const std::function<void(client_s&)>& callback)
	{
		foreach_client([&](client_s& client, size_t)
		{
			callback(client);
		});
	}

	void foreach_connected_client(const std::function<void(client_s&, size_t index)>& callback)
	{
		foreach_client([&](client_s& client, const size_t index)
		{
			if (client.state != CS_FREE)
			{
				callback(client, index);
			}
		});
	}

	void foreach_connected_client(const std::function<void(client_s&)>& callback)
	{
		foreach_connected_client([&](client_s& client, size_t)
		{
			callback(client);
		});
	}

	bool access_connected_client(const size_t index, const std::function<void(client_s&)>& callback)
	{
		if (is_server())
		{
			return access_client(*svs_clients, index, callback);
		}

		return access_client(*svs_clients_cl, index, callback);
	}
}