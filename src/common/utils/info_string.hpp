#pragma once

#include <string>
#include <unordered_map>

namespace utils
{
	class info_string
	{
	public:
		info_string() = default;
		info_string(const std::string& buffer);
		info_string(const std::string_view& buffer);

		void set(const std::string& key, const std::string& value);
		std::string get(const std::string& key) const;
		std::string build() const;

	private:
		std::unordered_map<std::string, std::string> key_value_pairs_{};

		void parse(std::string buffer);
	};
}
