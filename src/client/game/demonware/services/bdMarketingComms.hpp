#pragma once

namespace demonware
{
	class bdMarketingComms final : public service
	{
	public:
		bdMarketingComms();

	private:
		void getMessages(service_server* server, byte_buffer* buffer) const;
		void reportFullMessagesViewed(service_server* server, byte_buffer* buffer) const;
	};
}
