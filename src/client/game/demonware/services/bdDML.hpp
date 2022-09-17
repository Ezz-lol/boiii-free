#pragma once

namespace demonware
{
	class bdDML final : public service
	{
	public:
		bdDML();

	private:
		void recordIP(service_server* server, byte_buffer* buffer) const;
		void getUserData(service_server* server, byte_buffer* buffer) const;
		void getUserHierarchicalData(service_server* server, byte_buffer* buffer) const;
		void getUsersLastLogonData(service_server* server, byte_buffer* buffer) const;
	};
}
