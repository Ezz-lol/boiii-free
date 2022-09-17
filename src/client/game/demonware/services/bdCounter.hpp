#pragma once

namespace demonware
{
	class bdCounter final : public service
	{
	public:
		bdCounter();

	private:
		void incrementCounters(service_server* server, byte_buffer* buffer) const;
		void getCounterTotals(service_server* server, byte_buffer* buffer) const;
	};
}
