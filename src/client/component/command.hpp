#pragma once

namespace command
{
	class params
	{
	public:
		params();

		int size() const;
		const char* get(int index) const;
		std::string join(int index) const;

		const char* operator[](const int index) const
		{
			return this->get(index); //
		}

	private:
		int nesting_;
	};

	using command_function = std::function<void()>;
	using command_param_function = std::function<void(const params&)>;

	void add(const std::string& command, command_function function);
	void add(const std::string& command, command_param_function function);
}
