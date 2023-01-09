#pragma once

namespace console_command
{
	using callback = std::function<void(const command::params& params)>;
	void add_console(const std::string& name, const callback& cmd);
}
