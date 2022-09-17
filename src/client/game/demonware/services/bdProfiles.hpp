#pragma once

namespace demonware
{
	class bdProfiles final : public service
	{
	public:
		bdProfiles();

	private:
		void getPublicInfos(service_server* server, byte_buffer* buffer) const;
		void getPrivateInfo(service_server* server, byte_buffer* buffer) const;
		void setPublicInfo(service_server* server, byte_buffer* buffer) const;
		void setPrivateInfo(service_server* server, byte_buffer* buffer) const;
		void deleteProfile(service_server* server, byte_buffer* buffer) const;
		void setPrivateInfoByUserID(service_server* server, byte_buffer* buffer) const;
		void getPrivateInfoByUserID(service_server* server, byte_buffer* buffer) const;
		void setPublicInfoByUserID(service_server* server, byte_buffer* buffer) const;
	};
}
