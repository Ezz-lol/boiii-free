#pragma once

namespace demonware
{
	class bdFacebook final : public service
	{
	public:
		bdFacebook();

	private:
		void registerAccount(service_server* server, byte_buffer* buffer) const;
		void post(service_server* server, byte_buffer* buffer) const;
		void unregisterAccount(service_server* server, byte_buffer* buffer) const;
		void isRegistered(service_server* server, byte_buffer* buffer) const;
		void getInfo(service_server* server, byte_buffer* buffer) const;
		void getRegisteredAccounts(service_server* server, byte_buffer* buffer) const;
		void getFriends(service_server* server, byte_buffer* buffer) const;
		void getProfilePictures(service_server* server, byte_buffer* buffer) const;
		void uploadPhoto(service_server* server, byte_buffer* buffer) const;
		void registerToken(service_server* server, byte_buffer* buffer) const;
		void uploadVideo(service_server* server, byte_buffer* buffer) const;
		void getFriendsByID(service_server* server, byte_buffer* buffer) const;
		void setLikeStatus(service_server* server, byte_buffer* buffer) const;
	};
}
