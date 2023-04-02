#pragma once

namespace command
{
	class params
	{
	public:
		params();
		params(const std::string& text);
		~params();

		params(params&&) = delete;
		params(const params&) = delete;

		params& operator=(params&&) = delete;
		params& operator=(const params&) = delete;

		[[nodiscard]] int size() const;
		[[nodiscard]] const char* get(int index) const;
		[[nodiscard]] std::string join(int index) const;

		[[nodiscard]] const char* operator[](const int index) const
		{
			return this->get(index); //
		}

	private:
		bool needs_end_{false};
		int nesting_;
	};

	class params_sv
	{
	public:
		params_sv();
		params_sv(const std::string& text);
		~params_sv();

		params_sv(params_sv&&) = delete;
		params_sv(const params_sv&) = delete;

		params_sv& operator=(params_sv&&) = delete;
		params_sv& operator=(const params_sv&) = delete;

		[[nodiscard]] int size() const;
		[[nodiscard]] const char* get(int index) const;
		[[nodiscard]] std::string join(int index) const;

		[[nodiscard]] const char* operator[](const int index) const
		{
			return this->get(index); //
		}

	private:
		bool needs_end_{false};
		int nesting_;
	};

	using command_function = std::function<void()>;
	using command_param_function = std::function<void(const params&)>;
	using sv_command_param_function = std::function<void(const params_sv&)>;

	void add(const std::string& command, command_function function);
	void add(const std::string& command, command_param_function function);

	void add_sv(const std::string& command, sv_command_param_function function);
}
