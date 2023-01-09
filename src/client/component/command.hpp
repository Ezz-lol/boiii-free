#pragma once

namespace command
{
	class params
	{
	public:
		params();

		[[nodiscard]] int size() const;
		[[nodiscard]] const char* get(int index) const;
		[[nodiscard]] std::string join(int index) const;

		[[nodiscard]] const char* operator[](const int index) const
		{
			return this->get(index); //
		}

	private:
		int nesting_;
	};

	class params_sv
	{
	public:
		params_sv();

		[[nodiscard]] int size() const;
		[[nodiscard]] const char* get(int index) const;
		[[nodiscard]] std::string join(int index) const;

		[[nodiscard]] const char* operator[](const int index) const
		{
			return this->get(index); //
		}

	private:
		int nesting_;
	};

	using command_function = std::function<void()>;
	using command_param_function = std::function<void(const params&)>;
	using sv_command_param_function = std::function<void(const params_sv&)>;

	void add(const std::string& command, command_function function);
	void add(const std::string& command, command_param_function function);

	void add_sv(const std::string& command, sv_command_param_function function);
}
