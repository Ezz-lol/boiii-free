#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdTeams::bdTeams() : service(3, "bdTeams")
	{
		this->register_task(30, &bdTeams::createTeam);
		this->register_task(31, &bdTeams::updateTeamName);
		this->register_task(32, &bdTeams::promoteMember);
		this->register_task(33, &bdTeams::kickMember);
		this->register_task(34, &bdTeams::leaveTeam);
		this->register_task(35, &bdTeams::proposeMembership);
		this->register_task(36, &bdTeams::rejectMembership);
		this->register_task(37, &bdTeams::acceptMembership);
		this->register_task(38, &bdTeams::getPublicProfiles);
		this->register_task(39, &bdTeams::getPrivateProfile);
		this->register_task(40, &bdTeams::getPublicMemberProfiles);
		this->register_task(41, &bdTeams::getPrivateMemberProfiles);
		this->register_task(42, &bdTeams::setPublicProfile);
		this->register_task(43, &bdTeams::setPrivateProfile);
		this->register_task(44, &bdTeams::setPublicMemberProfile);
		this->register_task(45, &bdTeams::setPrivateMemberProfile);
		this->register_task(46, &bdTeams::getMemberships);
		this->register_task(47, &bdTeams::getMembers);
		this->register_task(48, &bdTeams::getOutgoingProposals);
		this->register_task(49, &bdTeams::withdrawProposal);
		this->register_task(50, &bdTeams::demoteMember);
		this->register_task(51, &bdTeams::promoteMemberToOwner);
		this->register_task(52, &bdTeams::getTeamInfo);
		this->register_task(53, &bdTeams::getIncomingProposals);
		this->register_task(54, &bdTeams::sendInstantMessage);
		this->register_task(56, &bdTeams::getMembershipsUser);
		this->register_task(57, &bdTeams::sendInstantMessageToTeam);
		this->register_task(58, &bdTeams::searchPublicTeamProfiles);
		this->register_task(63, &bdTeams::addApplication);
		this->register_task(64, &bdTeams::getApplicationsByTeam);
		this->register_task(65, &bdTeams::acceptApplication);
		this->register_task(66, &bdTeams::rejectApplication);
		this->register_task(68, &bdTeams::autoJoinTeam);
		this->register_task(70, &bdTeams::createTeamWithProfiles);
		this->register_task(73, &bdTeams::banMember);
		this->register_task(74, &bdTeams::unbanMember);
		this->register_task(76, &bdTeams::blockApplication);
		this->register_task(78, &bdTeams::unblockApplication);
		this->register_task(80, &bdTeams::updateTeamType);
		this->register_task(82, &bdTeams::setOnline);
		this->register_task(83, &bdTeams::getMembershipsWithCounts);
		this->register_task(84, &bdTeams::getMembershipsWithCountsUser);
		this->register_task(85, &bdTeams::searchTeams);
		this->register_task(86, &bdTeams::createTeamWithProfilesAndTeamType);
		this->register_task(87, &bdTeams::getMembershipsWithCountsAndTeamTypeUser);
		this->register_task(88, &bdTeams::getMembershipsWithCountsAndTeamType);
		this->register_task(89, &bdTeams::getTeamInfoWithTeamType);
		this->register_task(91, &bdTeams::setTeamAutoJoin);
		this->register_task(92, &bdTeams::getTeamAutoJoin);
		this->register_task(94, &bdTeams::getMembersAndPrivileges);
	}

	void bdTeams::createTeam(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::updateTeamName(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::promoteMember(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::kickMember(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::leaveTeam(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::proposeMembership(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::rejectMembership(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::acceptMembership(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getPublicProfiles(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getPrivateProfile(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getPublicMemberProfiles(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getPrivateMemberProfiles(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::setPublicProfile(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::setPrivateProfile(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::setPublicMemberProfile(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::setPrivateMemberProfile(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getMemberships(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getMembers(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getOutgoingProposals(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::withdrawProposal(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::demoteMember(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::promoteMemberToOwner(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getTeamInfo(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getIncomingProposals(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::sendInstantMessage(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getMembershipsUser(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::sendInstantMessageToTeam(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::searchPublicTeamProfiles(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::addApplication(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getApplicationsByTeam(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::acceptApplication(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::rejectApplication(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::autoJoinTeam(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::createTeamWithProfiles(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::banMember(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::unbanMember(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::blockApplication(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::unblockApplication(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::updateTeamType(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::setOnline(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getMembershipsWithCounts(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getMembershipsWithCountsUser(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::searchTeams(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::createTeamWithProfilesAndTeamType(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getMembershipsWithCountsAndTeamTypeUser(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getMembershipsWithCountsAndTeamType(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getTeamInfoWithTeamType(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::setTeamAutoJoin(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getTeamAutoJoin(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdTeams::getMembersAndPrivileges(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
