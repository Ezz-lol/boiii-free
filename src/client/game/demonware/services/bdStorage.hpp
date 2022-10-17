#pragma once

namespace demonware
{
	class bdStorage final : public service
	{
	public:
		bdStorage();

	private:
		using callback = std::function<std::string()>;
		using resource_variant = std::variant<std::string, callback>;
		std::vector<std::pair<std::regex, resource_variant>> publisher_resources_;

		void map_publisher_resource(const std::string& expression, INT id);
		void map_publisher_resource_variant(const std::string& expression, resource_variant resource);
		bool load_publisher_resource(const std::string& name, std::string& buffer);

		void list_publisher_files(service_server* server, byte_buffer* buffer);
		void get_publisher_file(service_server* server, byte_buffer* buffer);
		void set_user_file(service_server* server, byte_buffer* buffer) const;
		void upload_files(service_server* server, byte_buffer* buffer) const;
		void upload_files_new(service_server* server, byte_buffer* buffer) const;
		void get_files(service_server* server, byte_buffer* buffer) const;
		void unk12(service_server* server, byte_buffer* buffer) const;

		static std::string get_user_file_path(const std::string& name);
	};
}
