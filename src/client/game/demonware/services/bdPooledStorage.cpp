#include <std_include.hpp>
#include "../services.hpp"
#include <codecvt>

namespace demonware
{
	static std::string mem;
	bdPooledStorage::bdPooledStorage() : service(58, "bdPooledStorage")
	{
		this->register_task(1, &bdPooledStorage::getPooledMetaDataByID);
		this->register_task(5, &bdPooledStorage::_preUpload);
		this->register_task(6, &bdPooledStorage::_postUploadFile);
		this->register_task(8, &bdPooledStorage::remove);
		this->register_task(9, &bdPooledStorage::_preDownload);
		this->register_task(17, &bdPooledStorage::_preUploadSummary);
		this->register_task(18, &bdPooledStorage::_postUploadSummary);
		this->register_task(19, &bdPooledStorage::_preDownloadSummary);
		this->register_task(20, &bdPooledStorage::_preUploadMultiPart);
		this->register_task(21, &bdPooledStorage::_postUploadMultiPart);
		this->register_task(22, &bdPooledStorage::_preDownloadMultiPart);
	}

	std::vector<std::string> scan_folder_demo() {
		std::vector<std::string> result;
		result.push_back("");
		for (auto& entry : std::filesystem::directory_iterator(demo_folder)) {
			if (entry.path().extension() == ".summary") {
				result.push_back(entry.path().filename().string().substr(0, entry.path().filename().string().size() - 8));
			}
		}
		return result;
	}

	std::unique_ptr<bdFileMetaData> getMetaDataByID(uint64_t id)
	{

		auto demos = scan_folder_demo();

		auto result = std::make_unique<bdFileMetaData>();
		result->m_fileID = id;
		std::string folderPath = demo_folder;
		std::string demoFileName = demos[id];
		std::string demoFilePath = folderPath + demoFileName;
		if (std::filesystem::exists(demoFilePath)) {
			// Получение размера файла .demo
			result->m_fileSize = (uint32_t)std::filesystem::file_size(demoFilePath);

			auto lastWriteTime = std::filesystem::last_write_time(demoFilePath);
			auto modifedTime = std::chrono::time_point_cast<std::chrono::seconds>(lastWriteTime);
			auto createTime = std::filesystem::last_write_time(demoFilePath);
			auto createdTime = std::chrono::time_point_cast<std::chrono::seconds>(createTime);

			result->m_createTime = static_cast<uint32_t>(createdTime.time_since_epoch().count());
			result->m_modifedTime = static_cast<uint32_t>(modifedTime.time_since_epoch().count());

			// Запись в url
			result->m_url = std::format("{}/ex/demos/{}", "http://127.0.0.1:8132", demoFileName).c_str();
		}
		std::ifstream demoFileSummary(demoFilePath + ".summary");
		if (demoFileSummary.is_open()) {
			result->m_summaryFileSize = (uint32_t)std::filesystem::file_size(demoFilePath + ".summary");
			demoFileSummary.close();
		}
		result->m_fileSlot = 0;
		result->m_numCopiesMade = 0;
		result->m_originID = 0;
		result->m_ownerID = 1;
		//	result->m_ownerName[0] = '\0';

		std::ifstream demoFileThumbnail(demoFilePath + ".thumbnail");
		if (demoFileThumbnail.is_open()) {
			// Запись в m_metaData
			std::string metaData((std::istreambuf_iterator<char>(demoFileThumbnail)), std::istreambuf_iterator<char>());
			result->m_metaData = metaData;
			demoFileThumbnail.close();
		}

		result->m_ownerName = "redacted";
		result->m_fileName = demoFileName;


		std::ifstream demoFileTags(demoFilePath + ".tags");
		if (demoFileTags.is_open()) {
			// Чтение типа
			char type = 8;
			demoFileTags.read(&type, sizeof(type));
			// Чтение первых 4 байт
			uint32_t numTags = 0;
			demoFileTags.read(reinterpret_cast<char*>(&numTags), sizeof(numTags));
			char newline = '\n';
			demoFileTags.read(&newline, sizeof(newline));
			// bdtag
			for (uint32_t k = 0; k < numTags; k++) {
				bdTag tag;
				demoFileTags.read(reinterpret_cast<char*>(&tag.m_priTag), sizeof(tag.m_priTag));
				demoFileTags.read(&newline, sizeof(newline));
				demoFileTags.read(reinterpret_cast<char*>(&tag.m_secTag), sizeof(tag.m_secTag));
				result->m_tags.push_back(tag);
				demoFileTags.read(&newline, sizeof(newline));
			}	
			demoFileTags.close();

		}
		result->m_category = 10; // 0A zombie / 1 mp
		return result;
	}

	void bdPooledStorage::getPooledMetaDataByID(service_server* server, byte_buffer* buffer) const
	{
		uint32_t numIDs;
		unsigned int element_size = 8;
		buffer->read_array_header(10, &numIDs, &element_size);
		buffer->set_use_data_types(false);

		auto reply = server->create_reply(this->task_id());
		for (uint32_t i = 0; i < numIDs; i++)
		{
			uint64_t id;
			buffer->set_use_data_types(false);
			buffer->read_uint64(&id);
			buffer->set_use_data_types(true);
			printf("id: %llu \n", id);
			auto result = getMetaDataByID(id);
			reply.add(result);
		}

		reply.send();



	}

	void bdPooledStorage::_preUpload(service_server* server, byte_buffer* buffer) const
	{
		std::string filename;
		buffer->read_string(&filename);

		uint16_t category;
		buffer->read_uint16(&category);

		uint32_t tagCount;
		buffer->read_array_header(10, &tagCount, 0);

		auto result = std::make_unique<bdURL>();
		mem = filename;
		result->url = std::format("{}/ex/demos/{}", "http://127.0.0.1:8132", filename);
		auto demos = scan_folder_demo();

		result->serverID = 0;
		result->serverNPID = 1;
		result->serverFilename = filename;

		auto reply = server->create_reply(this->task_id());
		reply.add(result);
		reply.send();
	}

	void bdPooledStorage::_postUploadFile(service_server* server, byte_buffer* buffer) const
	{
		uint64_t fileid;
		buffer->read_uint64(&fileid);

		auto result = std::make_unique<bdFileID>();

		result->m_fileID = fileid;
		auto reply = server->create_reply(this->task_id());
		reply.add(result);
		reply.send();
	}

	void bdPooledStorage::remove(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdPooledStorage::_preDownload(service_server* server, byte_buffer* buffer) const
	{
		uint64_t fileid;
		buffer->read_uint64(&fileid);
		auto result = getMetaDataByID(fileid);
		auto reply = server->create_reply(this->task_id());
		reply.add(result);
		reply.send();
	}

	void bdPooledStorage::_preUploadSummary(service_server* server, byte_buffer* buffer) const
	{
		uint64_t fileID;
		uint32_t summarySize;
		std::string metaData;

		uint32_t tagCount;

		buffer->read_uint64(&fileID);
		buffer->read_uint32(&summarySize);
		buffer->read_blob(&metaData);

		buffer->read_array_header(10, &tagCount, 0);
		buffer->set_use_data_types(false);
		std::ofstream tagFile(demo_folder +mem + ".tags", std::ios::binary);
		byte type = 8;
		tagFile.write(reinterpret_cast<const char*>(&type), 1);
		tagCount /= 2;
		std::ofstream qqqq(demo_folder +mem + "." + std::to_string(fileID), std::ios::binary);
		qqqq.close();
		tagFile.write(reinterpret_cast<const char*>(&tagCount), sizeof(tagCount));
		tagFile.write("\n", 1);

		for (uint32_t i = 0; i < tagCount; i++)
		{
			bdTag tag;

			buffer->read_uint64(&tag.m_priTag);
			buffer->read_uint64(&tag.m_secTag);
			tagFile.write(reinterpret_cast<const char*>(&tag.m_priTag), 8);
			tagFile.write("\n", 1);
			tagFile.write(reinterpret_cast<const char*>(&tag.m_secTag), 8);
			tagFile.write("\n", 1);
		}
		tagFile.close();
		buffer->set_use_data_types(true);

		std::ofstream metadataFile(demo_folder + mem + ".thumbnail", std::ios::binary);
		metadataFile << metaData;
		metadataFile.close();

		auto result = std::make_unique<bdURL>();
		result->url = std::format("{}/ex/demos/{}.summary", "http://127.0.0.1:8132", mem);
		result->serverID = 0;
		result->serverNPID = fileID;
		result->serverFilename = mem;
		auto reply = server->create_reply(this->task_id());
		reply.add(result);
		reply.send();
	}

	void bdPooledStorage::_postUploadSummary(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdPooledStorage::_preDownloadSummary(service_server* server, byte_buffer* buffer) const
	{
		uint64_t id;
		buffer->read_uint64(&id);
		auto reply = server->create_reply(this->task_id());
		auto demos = scan_folder_demo();

		auto result = std::make_unique<bdSummaryMetaHandler>();
		result->url = std::format("{}/ex/demos/{}.summary", "http://127.0.0.1:8132", demos[id]);

		std::string folderPath = demo_folder;
		std::string demoFileName = demos[id];
		std::string demoFilePath = folderPath + demoFileName;

		std::ifstream demoFileSummary(demoFilePath + ".summary");
		if (demoFileSummary.is_open()) {
			result->unknownInt = (uint32_t)std::filesystem::file_size(demoFilePath + ".summary");
			demoFileSummary.close();
		}
		reply.add(result);
		reply.send();



	}

	void bdPooledStorage::_preUploadMultiPart(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdPooledStorage::_postUploadMultiPart(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}

	void bdPooledStorage::_preDownloadMultiPart(service_server* server, byte_buffer* /*buffer*/) const
	{
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
