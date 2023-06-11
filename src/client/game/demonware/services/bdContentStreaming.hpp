#pragma once

namespace demonware
{
	class bdContentStreaming final : public service
	{
	public:
		bdContentStreaming();

	private:
		void unk2(service_server* server, byte_buffer* buffer) const;
		void idk(service_server* server, byte_buffer* buffer) const;
		void _preDownloadSummary(service_server* server, byte_buffer*) const;
		void list_all_publisher_files(service_server* server, byte_buffer* buffer) const;
	};
}
