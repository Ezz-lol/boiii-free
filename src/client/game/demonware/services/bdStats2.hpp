#pragma once

namespace demonware
{
	class bdStats2 final : public service
	{
	public:
		bdStats2();

	private:
		void startArbitratedSession(service_server* server, byte_buffer* buffer) const;
		void writeArbitratedStats(service_server* server, byte_buffer* buffer) const;
	};
}