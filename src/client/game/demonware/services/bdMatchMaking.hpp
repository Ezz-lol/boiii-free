#pragma once

namespace demonware
{
	class bdMatchMaking final : public service
	{
	public:
		bdMatchMaking();

	private:
		void createSession(service_server* server, byte_buffer* buffer) const;
		void updateSession(service_server* server, byte_buffer* buffer) const;
		void deleteSession(service_server* server, byte_buffer* buffer) const;
		void findSessionFromID(service_server* server, byte_buffer* buffer) const;
		void findSessions(service_server* server, byte_buffer* buffer) const;
		void inviteToSession(service_server* server, byte_buffer* buffer) const;
		void submitPerformance(service_server* server, byte_buffer* buffer) const;
		void notifyJoin(service_server* server, byte_buffer* buffer) const;
		void getPerformanceValues(service_server* server, byte_buffer* buffer) const;
		void getSessionInvites(service_server* server, byte_buffer* buffer) const;
		void updateSessionPlayers(service_server* server, byte_buffer* buffer) const;
		void findSessionsPaged(service_server* server, byte_buffer* buffer) const;
		void findSessionsByEntityIDs(service_server* server, byte_buffer* buffer) const;
		void findSessionsFromIDs(service_server* server, byte_buffer* buffer) const;
		void findSessionsTwoPass(service_server* server, byte_buffer* buffer) const;
	};
}
