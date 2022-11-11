#pragma once

enum class component_priority
{
	min = 0,
	// must run after the updater
	steam_proxy,
	updater,
	// must have the highest priority
	arxan,
};

class component_interface
{
public:
	virtual ~component_interface() = default;

	virtual void post_load()
	{
	}

	virtual void pre_destroy()
	{
	}

	virtual void post_unpack()
	{
	}

	virtual component_priority priority() const
	{
		return component_priority::min;
	}
};
