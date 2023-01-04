#pragma once

namespace demonware
{
	class bdDDL final : public service
	{
	public:
		bdDDL();

	private:
		void verifyDDLFiles(service_server* server, byte_buffer* buffer) const;
	};
}
