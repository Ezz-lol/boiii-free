#pragma once

namespace console
{
	void set_title(const std::string& title);
	void set_interceptor(std::function<void(const std::string& message)> callback);
	void remove_interceptor();

	struct scoped_interceptor
	{
		scoped_interceptor(std::function<void(const std::string& message)> callback)
		{
			set_interceptor(std::move(callback));
		}

		~scoped_interceptor()
		{
			remove_interceptor();
		}

		scoped_interceptor(scoped_interceptor&&) = delete;
		scoped_interceptor(const scoped_interceptor&) = delete;
		scoped_interceptor& operator=(scoped_interceptor&&) = delete;
		scoped_interceptor& operator=(const scoped_interceptor&) = delete;
	};
}
