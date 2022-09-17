#pragma once

namespace demonware
{
	class bdRichPresence final : public service
	{
	public:
		bdRichPresence();

	private:
		void setInfo(service_server* server, byte_buffer* buffer) const;
		void getInfo(service_server* server, byte_buffer* buffer) const;
	};
}
