#include "byte_buffer.hpp"

#include <cstring>

namespace utils
{
	byte_buffer::byte_buffer()
		: writing_(true)
	{
	}

	byte_buffer::byte_buffer(std::string buffer)
		: writing_(false)
		  , buffer_(std::move(buffer))
	{
	}

	void byte_buffer::write(const void* buffer, const size_t length)
	{
		if (!this->writing_)
		{
			throw std::runtime_error("Writing to readable byte buffer");
		}

		this->buffer_.append(static_cast<const char*>(buffer), length);
	}

	void byte_buffer::read(void* data, const size_t length)
	{
		if (this->writing_)
		{
			throw std::runtime_error("Reading from writable byte buffer");
		}

		if (this->offset_ + length > this->buffer_.size())
		{
			throw std::runtime_error("Out of bounds read from byte buffer");
		}

		memcpy(data, this->buffer_.data() + this->offset_, length);
		this->offset_ += length;
	}

	std::string byte_buffer::read_string()
	{
		std::string result{};

		while (true)
		{
			const auto b = this->read<char>();
			if (!b)
			{
				break;
			}

			result.push_back(b);
		}

		return result;
	}

	std::string byte_buffer::read_string(const size_t length)
	{
		std::string result{};
		result.reserve(length);

		for (size_t i = 0; i < length; ++i)
		{
			result.push_back(this->read<char>());
		}

		return result;
	}

	std::vector<uint8_t> byte_buffer::read_data(const size_t length)
	{
		std::vector<uint8_t> result{};
		result.resize(length);

		this->read(result.data(), result.size());

		return result;
	}
}
