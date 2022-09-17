#pragma once

namespace demonware
{
	class bdEventLog final : public service
	{
	public:
		bdEventLog();

	private:
		void recordEvent(service_server* server, byte_buffer* buffer) const;
		void recordEventBin(service_server* server, byte_buffer* buffer) const;
		void recordEvents(service_server* server, byte_buffer* buffer) const;
		void recordEventsBin(service_server* server, byte_buffer* buffer) const;
		void initializeFiltering(service_server* server, byte_buffer* buffer) const;
	};
}
