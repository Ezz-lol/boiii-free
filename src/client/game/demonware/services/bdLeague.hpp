#pragma once

namespace demonware
{
	class bdLeague final : public service
	{
	public:
		bdLeague();

	private:
		void getTeamID(service_server* server, byte_buffer* buffer) const;
		void getTeamIDsForUser(service_server* server, byte_buffer* buffer) const;
		void getTeamSubdivisions(service_server* server, byte_buffer* buffer) const;
		void setTeamName(service_server* server, byte_buffer* buffer) const;
		void setTeamIcon(service_server* server, byte_buffer* buffer) const;
		void getTeamInfos(service_server* server, byte_buffer* buffer) const;
		void getTeamLeaguesAndSubdivisions(service_server* server, byte_buffer* buffer) const;
		void getTeamMemberInfos(service_server* server, byte_buffer* buffer) const;
		void incrementGamesPlayedCount(service_server* server, byte_buffer* buffer) const;
		void getSubdivisionInfos(service_server* server, byte_buffer* buffer) const;
		void getTeamSubdivisionHistory(service_server* server, byte_buffer* buffer) const;
	};
}