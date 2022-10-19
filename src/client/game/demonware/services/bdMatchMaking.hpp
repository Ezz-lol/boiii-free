#pragma once

namespace demonware
{
	class bdMatchMaking final : public service
	{
	public:
		bdMatchMaking();

	private:
		void create_session(service_server* server, byte_buffer* buffer) const;
		void update_session(service_server* server, byte_buffer* buffer) const;
		void delete_session(service_server* server, byte_buffer* buffer) const;
		void get_performance(service_server* server, byte_buffer* buffer) const;
		void find_sessions(service_server* server, byte_buffer* buffer) const;
	};
}
