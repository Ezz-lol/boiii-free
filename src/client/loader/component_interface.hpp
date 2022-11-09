#pragma once

class component_interface
{
public:
	virtual ~component_interface() = default;

	virtual void pre_load()
	{
	}

	virtual void post_load()
	{
	}

	virtual void pre_destroy()
	{
	}

	virtual void post_unpack()
	{
	}

	virtual bool is_supported()
	{
		return true;
	}

	virtual int priority()
	{
		return 0;
	}
};
