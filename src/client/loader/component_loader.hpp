#pragma once
#include "component_interface.hpp"

namespace component_loader
{
	using registration_functor = std::function<std::unique_ptr<generic_component>()>;

	void register_component(registration_functor functor, component_type type);

	bool activate(bool server);
	bool post_load();
	void post_unpack();
	void pre_destroy();

	[[noreturn]] void trigger_premature_shutdown();

	class premature_shutdown_trigger final : public std::exception
	{
		[[nodiscard]] const char* what() const noexcept override
		{
			return "Premature shutdown requested";
		}
	};

	template <typename T>
	class installer final
	{
		static_assert(std::is_base_of_v<generic_component, T>, "component has invalid base class");

	public:
		installer()
		{
			register_component([]
			{
				return std::make_unique<T>();
			}, T::type);
		}
	};
};

#define REGISTER_COMPONENT(name)                   \
namespace                                          \
{                                                  \
	component_loader::installer<name> __component; \
}
