#pragma once

namespace demonware
{
	class bdKeyArchive final : public service
	{
	public:
		bdKeyArchive();

	private:
		void write(service_server* server, byte_buffer* buffer) const;
		void read(service_server* server, byte_buffer* buffer) const;
		void readAll(service_server* server, byte_buffer* buffer) const;
		void readMultipleEntityIDs(service_server* server, byte_buffer* buffer) const;
		void writeMultipleEntityIDs(service_server* server, byte_buffer* buffer) const;
	};
}
