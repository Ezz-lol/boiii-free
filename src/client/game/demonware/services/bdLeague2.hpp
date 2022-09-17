#pragma once

namespace demonware
{
	class bdLeague2 final : public service
	{
	public:
		bdLeague2();

	private:
		void writeStats(service_server* server, byte_buffer* buffer) const;
		void readStatsByTeamID(service_server* server, byte_buffer* buffer) const;
		void readStatsByRank(service_server* server, byte_buffer* buffer) const;
		void readStatsByPivot(service_server* server, byte_buffer* buffer) const;
	};
}