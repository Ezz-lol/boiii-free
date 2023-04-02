#include <std_include.hpp>
#include "../services.hpp"

#include <utils/nt.hpp>
#include <utils/io.hpp>
#include <utils/cryptography.hpp>
#include <utils/compression.hpp>

#include "resource.hpp"
#include "game/game.hpp"

namespace demonware
{
	bdStorage::bdStorage() : service(10, "bdStorage")
	{
		this->register_task(20, &bdStorage::list_publisher_files);
		this->register_task(21, &bdStorage::get_publisher_file);
		this->register_task(24, &bdStorage::upload_files);
		this->register_task(18, &bdStorage::upload_files_new);
		this->register_task(16, &bdStorage::get_files);
		this->register_task(12, &bdStorage::unk12);
		this->register_task(10, &bdStorage::set_user_file);

		this->map_publisher_resource("motd-.*\\.gz", DW_MOTD);
		this->map_publisher_resource("playlists(_.+)?\\.gz", DW_PLAYLISTS);
		this->map_publisher_resource("featured_cards(.+)?\\.gz", DW_CARDS);
		this->map_publisher_resource(".*ffotd.*\\.ff", DW_FASTFILE);
		this->map_publisher_resource("keys\\.txt", DW_KEYS);
		this->map_publisher_resource("qosconfig4\\.csv", DW_QOSCONFIG);
	}

	void bdStorage::map_publisher_resource(const std::string& expression, const INT id)
	{
		auto data = utils::nt::load_resource(id);
		this->map_publisher_resource_variant(expression, std::move(data));
	}

	void bdStorage::map_publisher_resource_variant(const std::string& expression, resource_variant resource)
	{
		if (resource.valueless_by_exception())
		{
			throw std::runtime_error("Publisher resource variant is empty!");
		}

		this->publisher_resources_.emplace_back(std::regex{expression}, std::move(resource));
	}

	bool bdStorage::load_publisher_resource(const std::string& name, std::string& buffer)
	{
		for (const auto& resource : this->publisher_resources_)
		{
			if (std::regex_match(name, resource.first))
			{
				if (std::holds_alternative<std::string>(resource.second))
				{
					buffer = std::get<std::string>(resource.second);
				}
				else
				{
					buffer = std::get<callback>(resource.second)();
				}

				if (utils::string::ends_with(name, ".gz"))
				{
					buffer = utils::compression::zlib::compress(buffer);
				}

				return true;
			}
		}

#ifndef NDEBUG
		printf("[DW]: [bdStorage]: missing publisher file: %s\n", name.data());
#endif

		return false;
	}

	void bdStorage::list_publisher_files(service_server* server, byte_buffer* buffer)
	{
		uint32_t date;
		uint16_t num_results, offset;
		std::string unk, filename, data;

		buffer->read_string(&unk);
		buffer->read_uint32(&date);
		buffer->read_uint16(&num_results);
		buffer->read_uint16(&offset);
		buffer->read_string(&filename);

#ifndef NDEBUG
		printf("[DW]: [bdStorage]: list publisher files: %s\n", filename.data());
#endif

		auto reply = server->create_reply(this->task_id());

		if (this->load_publisher_resource(filename, data))
		{
			auto info = std::make_unique<bdFileInfo>();

			info->file_id = *reinterpret_cast<const uint64_t*>(utils::cryptography::sha1::compute(filename).data());
			info->filename = filename;
			info->create_time = 0;
			info->modified_time = info->create_time;
			info->file_size = static_cast<uint32_t>(data.size());
			info->owner_id = 0;
			info->priv = false;

			reply.add(info);
		}

		reply.send();
	}

	void bdStorage::get_publisher_file(service_server* server, byte_buffer* buffer)
	{
		std::string unk, filename;
		buffer->read_string(&unk);
		buffer->read_string(&filename);

#ifndef NDEBUG
		printf("[DW]: [bdStorage]: loading publisher file: %s\n", filename.data());
#endif

		std::string data;

		if (this->load_publisher_resource(filename, data))
		{
#ifndef NDEBUG
			printf("[DW]: [bdStorage]: sending publisher file: %s, size: %lld\n", filename.data(), data.size());
#endif

			auto reply = server->create_reply(this->task_id());
			auto result = std::make_unique<bdFileData>(data);
			reply.add(result);
			reply.send();
		}
		else
		{
			server->create_reply(this->task_id(), game::BD_NO_FILE).send();
		}
	}

	void bdStorage::set_user_file(service_server* server, byte_buffer* buffer) const
	{
		bool priv;
		uint64_t owner;
		std::string game, filename, data;

		buffer->read_string(&game);
		buffer->read_string(&filename);
		buffer->read_bool(&priv);
		buffer->read_blob(&data);
		buffer->read_uint64(&owner);

		const auto path = get_user_file_path(filename);
		utils::io::write_file(path, data);

		auto info = std::make_unique<bdFileInfo>();

		info->file_id = *reinterpret_cast<const uint64_t*>(utils::cryptography::sha1::compute(filename).data());
		info->filename = filename;
		info->create_time = uint32_t(time(nullptr));
		info->modified_time = info->create_time;
		info->file_size = uint32_t(data.size());
		info->owner_id = owner;
		info->priv = priv;

		auto reply = server->create_reply(this->task_id());
		reply.add(info);
		reply.send();
	}

	std::string bdStorage::get_user_file_path(const std::string& name)
	{
		return "players/user/" + name;
	}

	void bdStorage::upload_files(service_server* server, byte_buffer* buffer) const
	{
		uint64_t owner;
		uint32_t numfiles;
		std::string game, platform;

		buffer->read_string(&game);
		buffer->read_uint64(&owner);
		buffer->read_string(&platform);
		buffer->read_uint32(&numfiles);

		auto reply = server->create_reply(this->task_id());

		for (uint32_t i = 0; i < numfiles; i++)
		{
			std::string filename, data;
			uint32_t unk;
			bool priv;

			buffer->read_string(&filename);
			buffer->read_blob(&data);
			buffer->read_uint32(&unk);
			buffer->read_bool(&priv);

			const auto path = get_user_file_path(filename);
			utils::io::write_file(path, data);

			auto info = std::make_unique<bdFile2>();

			info->unk1 = 0;
			info->unk2 = 0;
			info->unk3 = 0;
			info->priv = false;
			info->owner_id = owner;
			info->platform = platform;
			info->filename = filename;
			info->data = data;

#ifndef NDEBUG
			printf("[DW]: [bdStorage]: set user file: %s\n", filename.data());
#endif

			reply.add(info);
		}

		reply.send();
	}

	void bdStorage::upload_files_new(service_server* server, byte_buffer* buffer) const
	{
		uint64_t owner;
		uint32_t numfiles;
		std::string game, platform;

		buffer->read_string(&game);
		buffer->read_uint64(&owner);
		buffer->read_string(&platform);
		buffer->read_uint32(&numfiles);

		auto reply = server->create_reply(this->task_id());

		for (uint32_t i = 0; i < numfiles; i++)
		{
			std::string filename, data;
			uint32_t version;
			bool priv;

			buffer->read_string(&filename);
			buffer->read_blob(&data);
			buffer->read_uint32(&version);
			buffer->read_bool(&priv);

			const auto path = get_user_file_path(filename);
			utils::io::write_file(path, data);

			auto info = std::make_unique<bdContextUserStorageFileInfo>();

			info->modifed_time = static_cast<uint32_t>(time(nullptr));
			info->create_time = info->modifed_time;
			info->priv = priv;
			info->owner_id = owner;
			info->account_type = platform;
			info->filename = filename;

#ifndef NDEBUG
			printf("[DW]: [bdStorage]: set user file: %s\n", filename.data());
#endif

			reply.add(info);
		}

		reply.send();
	}

	void bdStorage::get_files(service_server* server, byte_buffer* buffer) const
	{
		std::string context;
		buffer->read_string(&context);

		uint32_t count;
		buffer->read_uint32(&count);

		std::vector<std::pair<uint64_t, std::string>> user_ctxs;

		for (auto i = 0u; i < count; i++)
		{
			uint64_t user_id;
			std::string acc_type;
			buffer->read_uint64(&user_id);
			buffer->read_string(&acc_type);

			user_ctxs.emplace_back(user_id, acc_type);
		}

		buffer->read_uint32(&count);

		std::vector<std::string> filenames;

		for (auto i = 0u; i < count; i++)
		{
			std::string filename;
			buffer->read_string(&filename);
			filenames.push_back(std::move(filename));
		}

		auto reply = server->create_reply(this->task_id());
		for (size_t i = 0u; i < filenames.size(); i++)
		{
			auto entry = std::make_unique<bdFileQueryResult>();
			entry->user_id = user_ctxs.at(i).first;
			entry->platform = user_ctxs.at(i).second;
			entry->filename = filenames.at(i);
			entry->errorcode = 0;

			auto& name = filenames.at(i);
			std::string filedata;
			if (utils::io::read_file(get_user_file_path(name), &filedata))
			{
				entry->filedata = filedata;
#ifndef NDEBUG
				printf("[DW]: [bdStorage]: get user file: %s\n", name.data());
#endif
			}
			else
			{
				entry->errorcode = game::BD_NO_FILE;
#ifndef NDEBUG
				printf("[DW]: [bdStorage]: missing user file: %s\n", name.data());
#endif
			}

			reply.add(entry);
		}

		reply.send();
	}

	void bdStorage::unk12(service_server* server, byte_buffer* buffer) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply.send();
	}
}
