#pragma once

#include "byte_buffer.hpp"
#include "game/structs.hpp"

namespace demonware
{
	class bdTaskResult
	{
	public:
		virtual ~bdTaskResult() = default;

		virtual void serialize(byte_buffer*)
		{
		}

		virtual void deserialize(byte_buffer*)
		{
		}
	};

	class bdFileData final : public bdTaskResult
	{
	public:
		std::string file_data;

		explicit bdFileData(std::string buffer) : file_data(std::move(buffer))
		{
		}

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_blob(this->file_data);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_blob(&this->file_data);
		}
	};

	class bdFileInfo final : public bdTaskResult
	{
	public:
		uint64_t file_id;
		uint32_t create_time;
		uint32_t modified_time;
		bool priv;
		uint64_t owner_id;
		std::string filename;
		uint32_t file_size;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint32(this->file_size);
			buffer->write_uint64(this->file_id);
			buffer->write_uint32(this->create_time);
			buffer->write_uint32(this->modified_time);
			buffer->write_bool(this->priv);
			buffer->write_uint64(this->owner_id);
			buffer->write_string(this->filename);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint32(&this->file_size);
			buffer->read_uint64(&this->file_id);
			buffer->read_uint32(&this->create_time);
			buffer->read_uint32(&this->modified_time);
			buffer->read_bool(&this->priv);
			buffer->read_uint64(&this->owner_id);
			buffer->read_string(&this->filename);
		}
	};

	struct bdFileQueryResult final : public bdTaskResult
	{
		std::uint64_t user_id;
		std::string platform;
		std::string filename;
		std::uint32_t errorcode;
		std::string filedata;

		void serialize(byte_buffer* data) override
		{
			data->write_uint64(user_id);
			data->write_string(platform);
			data->write_string(filename);
			data->write_uint32(errorcode);
			data->write_blob(filedata);
		}

		void deserialize(byte_buffer* data) override
		{
			data->read_uint64(&user_id);
			data->read_string(&platform);
			data->read_string(&filename);
			data->read_uint32(&errorcode);
			data->read_blob(&filedata);
		}
	};

	class bdTimeStamp final : public bdTaskResult
	{
	public:
		uint32_t unix_time;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint32(this->unix_time);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint32(&this->unix_time);
		}
	};

	class bdDMLInfo : public bdTaskResult
	{
	public:
		std::string country_code; // Char [3]
		std::string country; // Char [65]
		std::string region; // Char [65]
		std::string city; // Char [129]
		float latitude;
		float longitude;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_string(this->country_code);
			buffer->write_string(this->country);
			buffer->write_string(this->region);
			buffer->write_string(this->city);
			buffer->write_float(this->latitude);
			buffer->write_float(this->longitude);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_string(&this->country_code);
			buffer->read_string(&this->country);
			buffer->read_string(&this->region);
			buffer->read_string(&this->city);
			buffer->read_float(&this->latitude);
			buffer->read_float(&this->longitude);
		}
	};

	class bdDMLRawData final : public bdDMLInfo
	{
	public:
		uint32_t asn; // Autonomous System Number.
		std::string timezone;

		void serialize(byte_buffer* buffer) override
		{
			bdDMLInfo::serialize(buffer);

			buffer->write_uint32(this->asn);
			buffer->write_string(this->timezone);
		}

		void deserialize(byte_buffer* buffer) override
		{
			bdDMLInfo::deserialize(buffer);

			buffer->read_uint32(&this->asn);
			buffer->read_string(&this->timezone);
		}
	};

	// made up name
	class bdFile final : public bdTaskResult
	{
	public:
		uint64_t owner_id;
		std::string platform;
		std::string filename;
		uint32_t unk;
		std::string data;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint64(this->owner_id);
			buffer->write_string(this->platform);
			buffer->write_string(this->filename);
			buffer->write_uint32(this->unk);
			buffer->write_blob(this->data);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint64(&this->owner_id);
			buffer->read_string(&this->platform);
			buffer->read_string(&this->filename);
			buffer->read_uint32(&this->unk);
			buffer->read_blob(&this->data);
		}
	};

	class bdFile2 final : public bdTaskResult
	{
	public:
		uint32_t unk1;
		uint32_t unk2;
		uint32_t unk3;
		bool priv;
		uint64_t owner_id;
		std::string platform;
		std::string filename;
		std::string data;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint32(this->unk1);
			buffer->write_uint32(this->unk2);
			buffer->write_uint32(this->unk3);
			buffer->write_bool(this->priv);
			buffer->write_uint64(this->owner_id);
			buffer->write_string(this->platform);
			buffer->write_string(this->filename);
			buffer->write_blob(this->data);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint32(&this->unk1);
			buffer->read_uint32(&this->unk2);
			buffer->read_uint32(&this->unk3);
			buffer->read_bool(&this->priv);
			buffer->read_uint64(&this->owner_id);
			buffer->read_string(&this->platform);
			buffer->read_string(&this->filename);
			buffer->read_blob(&this->data);
		}
	};

	class bdContextUserStorageFileInfo final : public bdTaskResult
	{
	public:
		uint32_t create_time;
		uint32_t modifed_time;
		bool priv;
		uint64_t owner_id;
		std::string account_type;
		std::string filename;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint32(this->create_time);
			buffer->write_uint32(this->modifed_time);
			buffer->write_bool(this->priv);
			buffer->write_uint64(this->owner_id);
			buffer->write_string(this->account_type);
			buffer->write_string(this->filename);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint32(&this->create_time);
			buffer->read_uint32(&this->modifed_time);
			buffer->read_bool(&this->priv);
			buffer->read_uint64(&this->owner_id);
			buffer->read_string(&this->account_type);
			buffer->read_string(&this->filename);
		}
	};

	class bdPerformanceValue final : public bdTaskResult
	{
	public:
		uint64_t user_id;
		int64_t performance;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint64(this->user_id);
			buffer->write_int64(this->performance);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint64(&this->user_id);
			buffer->read_int64(&this->performance);
		}
	};

	class bdDDLChecksumResult final : public bdTaskResult
	{
	public:
		std::string checksum{};
		bool checksum_matched{};

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_string(this->checksum);
			buffer->write_bool(this->checksum_matched);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_string(&this->checksum);
		}
	};

	struct bdSockAddr final
	{
		bdSockAddr() : in_un(), m_family(AF_INET)
		{
		}

		union
		{
			struct
			{
				char m_b1;
				char m_b2;
				char m_b3;
				char m_b4;
			} m_caddr;

			unsigned int m_iaddr;

			struct
			{
				unsigned __int16 m_w1;
				unsigned __int16 m_w2;
				unsigned __int16 m_w3;
				unsigned __int16 m_w4;
				unsigned __int16 m_w5;
				unsigned __int16 m_w6;
				unsigned __int16 m_w7;
				unsigned __int16 m_w8;
			} m_caddr6;

			char m_iaddr6[16];
			char m_sockaddr_storage[128];
		} in_un;

		unsigned __int16 m_family;
	};

	struct bdInetAddr final : bdTaskResult
	{
		bdSockAddr m_addr;

		bool is_valid() const
		{
			return (this->m_addr.m_family == AF_INET /*|| this->m_addr.m_family == AF_INET6*/);
		}

		void serialize(byte_buffer* buffer) override
		{
			const auto data_types = buffer->is_using_data_types();
			buffer->set_use_data_types(false);

			if (this->m_addr.m_family == AF_INET)
			{
				buffer->write(4, &this->m_addr.in_un.m_caddr);
			}

			buffer->set_use_data_types(data_types);
		}

		void deserialize(byte_buffer* buffer) override
		{
			const auto data_types = buffer->is_using_data_types();
			buffer->set_use_data_types(false);

			if (this->m_addr.m_family == AF_INET)
			{
				buffer->read(4, &this->m_addr.in_un.m_caddr);
			}

			buffer->set_use_data_types(data_types);
		}
	};

	struct bdAddr final : bdTaskResult
	{
		bdInetAddr m_address;
		unsigned __int16 m_port{};

		void serialize(byte_buffer* buffer) override
		{
			const bool data_types = buffer->is_using_data_types();
			buffer->set_use_data_types(false);

			this->m_address.serialize(buffer);
			buffer->write_uint16(this->m_port);

			buffer->set_use_data_types(data_types);
		}

		void deserialize(byte_buffer* buffer) override
		{
			const auto data_types = buffer->is_using_data_types();
			buffer->set_use_data_types(false);

			this->m_address.deserialize(buffer);
			buffer->read_uint16(&this->m_port);

			buffer->set_use_data_types(data_types);
		}
	};

	struct bdCommonAddr : bdTaskResult
	{
		bdAddr m_local_addrs[5];
		bdAddr m_public_addr;
		game::bdNATType m_nat_type;
		unsigned int m_hash;
		bool m_is_loopback;

		void serialize(byte_buffer* buffer) override
		{
			const auto data_types = buffer->is_using_data_types();
			buffer->set_use_data_types(false);

			auto valid = true;
			for (uint32_t i = 0; i < 5 && i < ARRAYSIZE(this->m_local_addrs) && valid; ++i)
			{
				this->m_local_addrs[i].serialize(buffer);
				valid = this->m_local_addrs[i].m_address.is_valid();
			}

			if (valid)
			{
				this->m_public_addr.serialize(buffer);
				buffer->write_byte(this->m_nat_type);
			}

			buffer->set_use_data_types(data_types);
		}

		void deserialize(byte_buffer* buffer) override
		{
			const auto data_types = buffer->is_using_data_types();
			buffer->set_use_data_types(false);

			auto valid = true;
			for (uint32_t i = 0; i < ARRAYSIZE(this->m_local_addrs) && valid; ++i)
			{
				bdAddr addr;
				addr.deserialize(buffer);
				this->m_local_addrs[i] = addr;
				valid = this->m_local_addrs[i].m_address.is_valid();
			}

			if (valid)
			{
				this->m_public_addr.deserialize(buffer);
				buffer->read_ubyte(reinterpret_cast<uint8_t*>(&this->m_nat_type));
			}

			buffer->set_use_data_types(data_types);
		}
	};

	class bdSessionID final : public bdTaskResult
	{
	public:
		uint64_t session_id;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_blob(LPSTR(&this->session_id), sizeof this->session_id);
		}

		void deserialize(byte_buffer* buffer) override
		{
			int size{};
			char* data{};
			buffer->read_blob(&data, &size);

			if (data && uint32_t(size) >= sizeof(this->session_id))
			{
				this->session_id = *reinterpret_cast<uint64_t*>(data);
			}
		}
	};

	class bdMatchMakingInfo final : bdTaskResult
	{
		bdSessionID m_sessionID;
		std::string m_hostAddr;
		uint32_t m_gameType;
		uint32_t m_maxPlayers;
		uint32_t m_numPlayers;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_blob(this->m_hostAddr);
			this->m_sessionID.serialize(buffer);
			buffer->write_uint32(this->m_gameType);
			buffer->write_uint32(this->m_maxPlayers);
			buffer->write_uint32(this->m_numPlayers);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_blob(&this->m_hostAddr);
			buffer->read_uint32(&this->m_gameType);
			buffer->read_uint32(&this->m_maxPlayers);
		}
	};

	class bdPublicProfileInfo final : public bdTaskResult
	{
	public:
		uint64_t m_entityID;
		int32_t m_VERSION;
		std::string m_ddl;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint64(this->m_entityID);
			buffer->write_int32(this->m_VERSION);
			buffer->write_blob(this->m_ddl);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint64(&this->m_entityID);
			buffer->read_int32(&this->m_VERSION);
			buffer->read_blob(&this->m_ddl);
		}
	};
}
