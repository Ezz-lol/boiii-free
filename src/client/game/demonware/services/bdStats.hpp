#pragma once

namespace demonware
{
	class bdStats final : public service
	{
	public:
		bdStats();

	private:
		void writeStats(service_server* server, byte_buffer* buffer) const;
		void deleteStats(service_server* server, byte_buffer* buffer) const;
		void unk3(service_server* server, byte_buffer* buffer) const;
		void readStatsByRank(service_server* server, byte_buffer* buffer) const;
		void readStatsByPivot(service_server* server, byte_buffer* buffer) const;
		void readStatsByRating(service_server* server, byte_buffer* buffer) const;
		void readStatsByMultipleRanks(service_server* server, byte_buffer* buffer) const;
		void readExternalTitleStats(service_server* server, byte_buffer* buffer) const;
		void readExternalTitleNamedStats(service_server* server, byte_buffer* buffer) const;
		void readStatsByLeaderboardIDsAndEntityIDs(service_server* server, byte_buffer* buffer) const;
		void readStatsByMultipleRatings(service_server* server, byte_buffer* buffer) const;
		void readStatsByEntityID(service_server* server, byte_buffer* buffer) const;
		void writeServerValidatedStats(service_server* server, byte_buffer* buffer) const;
	};
}
