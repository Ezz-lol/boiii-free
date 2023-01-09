#pragma once

namespace client_command
{
	using callback = std::function<void(game::gentity_s* ent, const command::params_sv& params)>;
	void add(const std::string& name, const callback& cmd);
}
