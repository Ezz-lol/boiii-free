#pragma once

namespace demonware
{
	class bdTitleUtilities final : public service
	{
	public:
		bdTitleUtilities();

	private:
		void get_server_time(service_server* server, byte_buffer* buffer) const;
	};
}
