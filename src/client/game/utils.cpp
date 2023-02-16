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

		return dvar->current.integer;
	}
}
