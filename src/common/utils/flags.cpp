#include "flags.hpp"
#include "string.hpp"
#include "nt.hpp"

#include <shellapi.h>

namespace utils::flags
{
	void parse_flags(std::vector<std::string>& flags)
	{
		int num_args;
		auto* const argv = CommandLineToArgvW(GetCommandLineW(), &num_args);

		flags.clear();

		if (argv)
		{
			for (auto i = 0; i < num_args; ++i)
			{
				std::wstring wide_flag(argv[i]);
				if (wide_flag[0] == L'-')
				{
					wide_flag.erase(wide_flag.begin());
					flags.emplace_back(string::convert(wide_flag));
				}
			}

			LocalFree(argv);
		}
	}

	bool has_flag(const std::string& flag)
	{
		static auto parsed = false;
		static std::vector<std::string> enabled_flags;

		if (!parsed)
		{
			parse_flags(enabled_flags);
			parsed = true;
		}

		for (const auto& entry : enabled_flags)
		{
			if (string::to_lower(entry) == string::to_lower(flag))
			{
				return true;
			}
		}

		return false;
	}
}
