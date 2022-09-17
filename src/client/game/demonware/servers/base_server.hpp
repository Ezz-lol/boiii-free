#pragma once

namespace demonware
{
	class base_server
	{
	public:
		using stream_queue = std::queue<char>;
		using data_queue = std::queue<std::string>;

		base_server(std::string name);

		base_server(base_server&&) = delete;
		base_server(const base_server&) = delete;
		base_server& operator=(base_server&&) = delete;
		base_server& operator=(const base_server&) = delete;

		virtual ~base_server() = default;

		const std::string& get_name() const;

		uint32_t get_address() const;

		virtual void frame() = 0;

	private:
		std::string name_;
		std::uint32_t address_ = 0;
	};
}
