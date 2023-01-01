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

enum class component_type
{
	client,
	server,
	any,
};

struct component_interface
{
	static constexpr component_type type = component_type::any;

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

struct client_component_interface : component_interface
{
	static constexpr component_type type = component_type::client;
};

struct server_component_interface : component_interface
{
	static constexpr component_type type = component_type::server;
};
