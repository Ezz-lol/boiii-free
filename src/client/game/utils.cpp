#include <std_include.hpp>

#include "game.hpp"
#include "utils.hpp"

namespace game
{
	std::string get_dvar_string(const char* dvar_name)
	{
		const auto dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return Dvar_GetString(dvar);
	}

	int get_dvar_int(const char* dvar_name)
	{
		const auto dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return dvar->current.value.integer;
	}

	bool get_dvar_bool(const char* dvar_name)
	{
		const auto dvar = Dvar_FindVar(dvar_name);
		if (!dvar)
		{
			return {};
		}

		return dvar->current.value.enabled;
	}

	dvar_t* register_sessionmode_dvar_bool(const char* dvar_name, const bool value, const dvarFlags_e flags, const char* description)
	{
		const auto hash = Dvar_GenerateHash(dvar_name);
		auto registered_dvar = Dvar_SessionModeRegisterBool(hash, dvar_name, value, flags, description);

		if (registered_dvar)
		{
			registered_dvar->debugName = dvar_name;
		}

		return registered_dvar;
	}

	dvar_t* register_dvar_bool(const char* dvar_name, const bool value, const dvarFlags_e flags, const char* description)
	{
		const auto hash = Dvar_GenerateHash(dvar_name);
		auto registered_dvar = Dvar_RegisterBool(hash, dvar_name, value, flags, description);

		if (registered_dvar)
		{
			registered_dvar->debugName = dvar_name;
		}

		return registered_dvar;
	}

	void dvar_add_flags(const char* dvar_name, const dvarFlags_e flags)
	{
		auto dvar = Dvar_FindVar(dvar_name);

		if (!dvar)
		{
			return;
		}

		auto dvar_to_change = dvar;

		if (dvar_to_change->type == DVAR_TYPE_SESSIONMODE_BASE_DVAR)
		{
			const auto mode = Com_SessionMode_GetMode();
			dvar_to_change = Dvar_GetSessionModeSpecificDvar(dvar_to_change, mode);
		}

		dvar_to_change->flags |= flags;
	}

	void dvar_set_flags(const char* dvar_name, const dvarFlags_e flags)
	{
		auto dvar = Dvar_FindVar(dvar_name);

		if (!dvar)
		{
			return;
		}

		auto dvar_to_change = dvar;

		if (dvar_to_change->type == DVAR_TYPE_SESSIONMODE_BASE_DVAR)
		{
			const auto mode = Com_SessionMode_GetMode();
			dvar_to_change = Dvar_GetSessionModeSpecificDvar(dvar_to_change, mode);
		}

		dvar_to_change->flags = flags;
	}
}
