#include <std_include.hpp>
#include "component_loader.hpp"

#include <utils/nt.hpp>

namespace component_loader
{
	namespace
	{
		std::vector<std::unique_ptr<generic_component>>& get_components()
		{
			using component_vector = std::vector<std::unique_ptr<generic_component>>;
			using component_vector_container = std::unique_ptr<component_vector, std::function<void(component_vector*)>>
				;

			static component_vector_container components(new component_vector,
			                                             [](const component_vector* component_vector)
			                                             {
				                                             pre_destroy();
				                                             delete component_vector;
			                                             });

			return *components;
		}

		std::vector<std::pair<registration_functor, component_type>>& get_registration_functors()
		{
			static std::vector<std::pair<registration_functor, component_type>> functors;
			return functors;
		}

		void activate_component(std::unique_ptr<generic_component> component)
		{
			auto& components = get_components();
			components.push_back(std::move(component));

			std::ranges::stable_sort(components, [](const std::unique_ptr<generic_component>& a,
			                                        const std::unique_ptr<generic_component>& b)
			{
				return a->priority() > b->priority();
			});
		}
	}

	void register_component(registration_functor functor, component_type type)
	{
		if (!get_components().empty())
		{
			throw std::runtime_error("Registration is too late");
		}

		get_registration_functors().emplace_back(std::move(functor), type);
	}

	bool activate(bool server)
	{
		static auto res = [server]
		{
			try
			{
				for (auto& functor : get_registration_functors())
				{
					if (functor.second == component_type::any || server == (functor.second == component_type::server))
					{
						activate_component(functor.first());
					}
				}
			}
			catch (premature_shutdown_trigger&)
			{
				return false;
			}
			catch (const std::exception& e)
			{
				MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
				return false;
			}

			return true;
		}();

		return res;
	}

	bool post_load()
	{
		static auto res = []
		{
			try
			{
				for (const auto& component : get_components())
				{
					component->post_load();
				}
			}
			catch (premature_shutdown_trigger&)
			{
				return false;
			}
			catch (const std::exception& e)
			{
				MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
				return false;
			}

			return true;
		}();

		return res;
	}

	void post_unpack()
	{
		static auto res = []
		{
			try
			{
				for (const auto& component : get_components())
				{
					component->post_unpack();
				}
			}
			catch (const std::exception& e)
			{
				MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
				return false;
			}

			return true;
		}();

		if (!res)
		{
			TerminateProcess(GetCurrentProcess(), 1);
		}
	}

	void pre_destroy()
	{
		static auto res = []
		{
			try
			{
				for (const auto& component : get_components())
				{
					component->pre_destroy();
				}
			}
			catch (const std::exception& e)
			{
				MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
				return false;
			}

			return true;
		}();

		if (!res)
		{
			TerminateProcess(GetCurrentProcess(), 1);
		}
	}

	void trigger_premature_shutdown()
	{
		throw premature_shutdown_trigger();
	}
}
