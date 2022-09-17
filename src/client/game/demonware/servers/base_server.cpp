#include <std_include.hpp>
#include "base_server.hpp"

#include <utils/cryptography.hpp>

namespace demonware
{
	base_server::base_server(std::string name): name_(std::move(name))
	{
		this->address_ = utils::cryptography::jenkins_one_at_a_time::compute(this->name_);
	}

	const std::string& base_server::get_name() const
	{
		return this->name_;
	}

	uint32_t base_server::get_address() const
	{
		return this->address_;
	}
}
