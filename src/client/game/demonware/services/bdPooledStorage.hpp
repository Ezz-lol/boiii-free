#pragma once

namespace demonware
{
	class bdPooledStorage final : public service
	{
	public:
		bdPooledStorage();

	private:
		void getPooledMetaDataByID(service_server* server, byte_buffer* buffer) const;
		void _preUpload(service_server* server, byte_buffer* buffer) const;
		void _postUploadFile(service_server* server, byte_buffer* buffer) const;
		void remove(service_server* server, byte_buffer* buffer) const;
		void _preDownload(service_server* server, byte_buffer* buffer) const;
		void _preUploadSummary(service_server* server, byte_buffer* buffer) const;
		void _postUploadSummary(service_server* server, byte_buffer* buffer) const;
		void _preDownloadSummary(service_server* server, byte_buffer* buffer) const;
		void _preUploadMultiPart(service_server* server, byte_buffer* buffer) const;
		void _postUploadMultiPart(service_server* server, byte_buffer* buffer) const;
		void _preDownloadMultiPart(service_server* server, byte_buffer* buffer) const;
	};
	const static std::string demo_folder = "demos\\";

	std::vector<std::string> scan_folder_demo();
}
