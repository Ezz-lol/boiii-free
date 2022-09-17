#pragma once

namespace demonware
{
	class bdUNK63 final : public service
	{
	public:
		bdUNK63();

	private:
		void unk(service_server* server, byte_buffer* buffer) const;
	};
}
