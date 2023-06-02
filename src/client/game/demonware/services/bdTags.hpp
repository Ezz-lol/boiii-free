#pragma once

namespace demonware
{
	class bdTags final : public service
	{
	public:
		bdTags();

	private:
		void getTagsForEntityIDs(service_server* server, byte_buffer* buffer) const;
		void setTagsForEntityID(service_server* server, byte_buffer* buffer) const;
		void removeTagsForEntityID(service_server* server, byte_buffer* buffer) const;
		void removeAllTagsForEntityID(service_server* server, byte_buffer* buffer) const;
		void searchByTagsBase(service_server* server, byte_buffer* buffer) const;
	};

	class bdService55 final : public service
	{
	public:
		bdService55();

	private:
		void k9(service_server* server, byte_buffer* buffer) const;
	};
}
