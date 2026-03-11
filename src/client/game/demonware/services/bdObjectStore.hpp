#pragma once

namespace demonware
{
	class bdObjectStore final : public service
	{
	public:
		bdObjectStore();

	private:
		void getUserObjects(service_server* server, byte_buffer* buffer) const;
	};
}
