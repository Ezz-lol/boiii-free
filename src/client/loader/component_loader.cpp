#include <std_include.hpp>
#include "component_loader.hpp"

#include <utils/nt.hpp>

void component_loader::register_component(std::unique_ptr<component_interface>&& component_)
{
	auto& components = get_components();
	components.push_back(std::move(component_));

	std::ranges::stable_sort(components, [](const std::unique_ptr<component_interface>& a,
	                                        const std::unique_ptr<component_interface>& b)
	{
		return a->priority() > b->priority();
	});
}

bool component_loader::pre_start()
{
	static auto res = []
	{
		clean();

		try
		{
			for (const auto& component_ : get_components())
			{
				component_->pre_start();
			}
		}
		catch (premature_shutdown_trigger&)
		{
			return false;
		}
		catch (const std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
			return false;
		}

		return true;
	}();

	return res;
}

void component_loader::post_unpack()
{
	static auto res = []
	{
		clean();

		try
		{
			for (const auto& component_ : get_components())
			{
				component_->post_unpack();
			}
		}
		catch (const std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
			return false;
		}

		return true;
	}();

	if (!res)
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}
}

void component_loader::pre_destroy()
{
	static auto res = []
	{
		clean();

		try
		{
			for (const auto& component_ : get_components())
			{
				component_->pre_destroy();
			}
		}
		catch (const std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
			return false;
		}

		return true;
	}();

	if (!res)
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}
}

void component_loader::clean()
{
	auto& components = get_components();
	for (auto i = components.begin(); i != components.end();)
	{
		if (!(*i)->is_supported())
		{
			(*i)->pre_destroy();
			i = components.erase(i);
		}
		else
		{
			++i;
		}
	}
}

void component_loader::trigger_premature_shutdown()
{
	throw premature_shutdown_trigger();
}

std::vector<std::unique_ptr<component_interface>>& component_loader::get_components()
{
	using component_vector = std::vector<std::unique_ptr<component_interface>>;
	using component_vector_container = std::unique_ptr<component_vector, std::function<void(component_vector*)>>;

	static component_vector_container components(new component_vector, [](const component_vector* component_vector)
	{
		pre_destroy();
		delete component_vector;
	});

	return *components;
}

size_t get_base()
{
	static auto base = []
	{
		const utils::nt::library host{};
		if(!host || host == utils::nt::library::get_by_address(get_base))
		{
			throw std::runtime_error("Invalid host application");
		}

		return size_t(host.get_ptr());
	}();
	return base;
}

size_t get_game_address(const size_t val)
{
	static auto base = get_base();
	return base + (val - 0x140000000);
}

size_t operator"" _g(const size_t val)
{
	return get_game_address(val);
}

size_t reverse_g(const size_t val)
{
	static auto base = get_base();
	return (val - base) + 0x140000000;
}

size_t reverse_g(const void* val)
{
	return reverse_g(reinterpret_cast<size_t>(val));
}
