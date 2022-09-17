#pragma once

namespace demonware
{
	class bdGroup final : public service
	{
	public:
		bdGroup();

	private:
		void setGroups(service_server* server, byte_buffer* buffer) const;
		void setGroupsForEntity(service_server* server, byte_buffer* buffer) const;
		void getEntityGroups(service_server* server, byte_buffer* buffer) const;
		void getGroupCounts(service_server* server, byte_buffer* buffer) const;
	};
}
