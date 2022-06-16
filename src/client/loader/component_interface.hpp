#pragma once

class component_interface
{
public:
	virtual ~component_interface() = default;

	virtual void pre_start()
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
