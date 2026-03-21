#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace gsc_compiler
{
	struct compile_error
	{
		std::string message;
		std::string file;
		int line;
		int column;
	};

	struct hash_name_pair
	{
		uint32_t hash;
		std::string name;
		int line;
		uint8_t params;
	};

	struct compile_result
	{
		bool success;
		std::vector<uint8_t> bytecode;
		std::vector<compile_error> errors;
		std::vector<hash_name_pair> hash_names;
	};

	// Compile raw GSC source code to T7 PC bytecode.
	compile_result compile(const std::string& source, const std::string& source_name);
}
