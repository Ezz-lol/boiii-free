#pragma once

namespace demonware
{
	class bdStats3 final : public service
	{
	public:
		bdStats3();

	private:
		void deleteCSFileStats(service_server* server, byte_buffer* buffer) const;
		void readStatsByEntityID(service_server* server, byte_buffer* buffer) const;
		void readStatsByRank(service_server* server, byte_buffer* buffer) const;
		void readStatsByPivot(service_server* server, byte_buffer* buffer) const;
		void readStatsByRating(service_server* server, byte_buffer* buffer) const;
		void readStatsByMultipleRanks(service_server* server, byte_buffer* buffer) const;
		void readStatsByLeaderboardIDsAndEntityIDs(service_server* server, byte_buffer* buffer) const;
	};
}