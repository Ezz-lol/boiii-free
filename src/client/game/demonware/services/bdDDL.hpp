#pragma once

namespace demonware
{
	class bdDDL final : public service
	{
	public:
		bdDDL();

	private:
		void idk(service_server* server, byte_buffer* buffer) const;
	};
}
