#pragma once

#include "bit_buffer.hpp"
#include "byte_buffer.hpp"
#include "data_types.hpp"

namespace demonware
{
	class reply
	{
	public:
		reply() = default;

		reply(reply&&) = delete;
		reply(const reply&) = delete;
		reply& operator=(reply&&) = delete;
		reply& operator=(const reply&) = delete;

		virtual ~reply() = default;
		virtual std::string data() = 0;
	};

	class raw_reply : public reply
	{
	protected:
		std::string buffer_;

	public:
		raw_reply() = default;

		explicit raw_reply(std::string data) : buffer_(std::move(data))
		{
		}

		std::string data() override
		{
			return this->buffer_;
		}
	};

	class typed_reply : public raw_reply
	{
	public:
		typed_reply(const uint8_t _type) : type_(_type)
		{
		}

	protected:
		uint8_t type() const { return this->type_; }

	private:
		uint8_t type_;
	};

	class encrypted_reply final : public typed_reply
	{
	public:
		encrypted_reply(const uint8_t type, bit_buffer* bbuffer) : typed_reply(type)
		{
			this->buffer_.append(bbuffer->get_buffer());
		}

		encrypted_reply(const uint8_t type, byte_buffer* bbuffer) : typed_reply(type)
		{
			this->buffer_.append(bbuffer->get_buffer());
		}

		std::string data() override;
	};

	class unencrypted_reply final : public typed_reply
	{
	public:
		unencrypted_reply(const uint8_t _type, bit_buffer* bbuffer) : typed_reply(_type)
		{
			this->buffer_.append(bbuffer->get_buffer());
		}

		unencrypted_reply(const uint8_t _type, byte_buffer* bbuffer) : typed_reply(_type)
		{
			this->buffer_.append(bbuffer->get_buffer());
		}

		std::string data() override;
	};

	class service_server;

	class remote_reply final
	{
	public:
		remote_reply(service_server* server, uint8_t _type) : type_(_type), server_(server)
		{
		}

		void send(bit_buffer* buffer, bool encrypted);
		void send(byte_buffer* buffer, bool encrypted);

		uint8_t type() const { return this->type_; }

	private:
		uint8_t type_;
		service_server* server_;
	};

	class service_reply final
	{
	public:
		service_reply(service_server* _server, const uint8_t _type, const uint32_t _error)
			: type_(_type), error_(_error), reply_(_server, 1)
		{
		}

		uint64_t send()
		{
			static uint64_t id = 0x0000000000000000;
			const auto transaction_id = ++id;

			byte_buffer buffer;
			buffer.write_uint64(transaction_id);
			buffer.write_uint32(this->error_);
			buffer.write_ubyte(this->type_);

			if (!this->error_)
			{
				buffer.write_uint32(static_cast<uint32_t>(this->objects_.size()));
				if (!this->objects_.empty())
				{
					buffer.write_uint32(static_cast<uint32_t>(this->objects_.size()));

					for (auto& object : this->objects_)
					{
						object->serialize(&buffer);
					}

					this->objects_.clear();
				}
			}
			else
			{
				buffer.write_uint64(transaction_id);
			}

			this->reply_.send(&buffer, true);
			return transaction_id;
		}

		template<typename T>
		void add(std::unique_ptr<T>& object)
		{
			static_assert(std::is_base_of_v<bdTaskResult, T>);
			this->objects_.emplace_back(std::move(object));
		}

	private:
		uint8_t type_;
		uint32_t error_;
		remote_reply reply_;
		std::vector<std::unique_ptr<bdTaskResult>> objects_;
	};
}
