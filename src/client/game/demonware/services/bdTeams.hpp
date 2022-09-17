#pragma once

namespace demonware
{
	class bdTeams final : public service
	{
	public:
		bdTeams();

	private:
		void createTeam(service_server* server, byte_buffer* buffer) const;
		void updateTeamName(service_server* server, byte_buffer* buffer) const;
		void promoteMember(service_server* server, byte_buffer* buffer) const;
		void kickMember(service_server* server, byte_buffer* buffer) const;
		void leaveTeam(service_server* server, byte_buffer* buffer) const;
		void proposeMembership(service_server* server, byte_buffer* buffer) const;
		void rejectMembership(service_server* server, byte_buffer* buffer) const;
		void acceptMembership(service_server* server, byte_buffer* buffer) const;
		void getPublicProfiles(service_server* server, byte_buffer* buffer) const;
		void getPrivateProfile(service_server* server, byte_buffer* buffer) const;
		void getPublicMemberProfiles(service_server* server, byte_buffer* buffer) const;
		void getPrivateMemberProfiles(service_server* server, byte_buffer* buffer) const;
		void setPublicProfile(service_server* server, byte_buffer* buffer) const;
		void setPrivateProfile(service_server* server, byte_buffer* buffer) const;
		void setPublicMemberProfile(service_server* server, byte_buffer* buffer) const;
		void setPrivateMemberProfile(service_server* server, byte_buffer* buffer) const;
		void getMemberships(service_server* server, byte_buffer* buffer) const;
		void getMembers(service_server* server, byte_buffer* buffer) const;
		void getOutgoingProposals(service_server* server, byte_buffer* buffer) const;
		void withdrawProposal(service_server* server, byte_buffer* buffer) const;
		void demoteMember(service_server* server, byte_buffer* buffer) const;
		void promoteMemberToOwner(service_server* server, byte_buffer* buffer) const;
		void getTeamInfo(service_server* server, byte_buffer* buffer) const;
		void getIncomingProposals(service_server* server, byte_buffer* buffer) const;
		void sendInstantMessage(service_server* server, byte_buffer* buffer) const;
		void getMembershipsUser(service_server* server, byte_buffer* buffer) const;
		void sendInstantMessageToTeam(service_server* server, byte_buffer* buffer) const;
		void searchPublicTeamProfiles(service_server* server, byte_buffer* buffer) const;
		void addApplication(service_server* server, byte_buffer* buffer) const;
		void getApplicationsByTeam(service_server* server, byte_buffer* buffer) const;
		void acceptApplication(service_server* server, byte_buffer* buffer) const;
		void rejectApplication(service_server* server, byte_buffer* buffer) const;
		void autoJoinTeam(service_server* server, byte_buffer* buffer) const;
		void createTeamWithProfiles(service_server* server, byte_buffer* buffer) const;
		void banMember(service_server* server, byte_buffer* buffer) const;
		void unbanMember(service_server* server, byte_buffer* buffer) const;
		void blockApplication(service_server* server, byte_buffer* buffer) const;
		void unblockApplication(service_server* server, byte_buffer* buffer) const;
		void updateTeamType(service_server* server, byte_buffer* buffer) const;
		void setOnline(service_server* server, byte_buffer* buffer) const;
		void getMembershipsWithCounts(service_server* server, byte_buffer* buffer) const;
		void getMembershipsWithCountsUser(service_server* server, byte_buffer* buffer) const;
		void searchTeams(service_server* server, byte_buffer* buffer) const;
		void createTeamWithProfilesAndTeamType(service_server* server, byte_buffer* buffer) const;
		void getMembershipsWithCountsAndTeamTypeUser(service_server* server, byte_buffer* buffer) const;
		void getMembershipsWithCountsAndTeamType(service_server* server, byte_buffer* buffer) const;
		void getTeamInfoWithTeamType(service_server* server, byte_buffer* buffer) const;
		void setTeamAutoJoin(service_server* server, byte_buffer* buffer) const;
		void getTeamAutoJoin(service_server* server, byte_buffer* buffer) const;
		void getMembersAndPrivileges(service_server* server, byte_buffer* buffer) const;
	};
}
