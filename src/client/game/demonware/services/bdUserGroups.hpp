#pragma once

namespace demonware
{
	class bdUserGroups final : public service
	{
	public:
		bdUserGroups();

	private:
		void createGroup(service_server* server, byte_buffer* buffer) const;
		void deleteGroup(service_server* server, byte_buffer* buffer) const;
		void joinGroup(service_server* server, byte_buffer* buffer) const;
		void leaveGroup(service_server* server, byte_buffer* buffer) const;
		void getMembershipInfo(service_server* server, byte_buffer* buffer) const;
		void changeMemberType(service_server* server, byte_buffer* buffer) const;
		void getNumMembers(service_server* server, byte_buffer* buffer) const;
		void getMembers(service_server* server, byte_buffer* buffer) const;
		void getMemberships(service_server* server, byte_buffer* buffer) const;
		void readStatsByRank(service_server* server, byte_buffer* buffer) const;
		void getGroupLists(service_server* server, byte_buffer* buffer) const;
	};
}