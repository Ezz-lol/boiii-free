#pragma once

#include "byte_buffer.hpp"

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
}
