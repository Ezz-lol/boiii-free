#include <std_include.hpp>
#include "bit_buffer.hpp"

namespace demonware
{
	bool bit_buffer::read_bytes(const unsigned int bytes, unsigned char* output)
	{
		return this->read(bytes * 8, output);
	}

	bool bit_buffer::read_bool(bool* output)
	{
		if (!this->read_data_type(1))
		{
			return false;
		}

		return this->read(1, output);
	}

	bool bit_buffer::read_uint32(unsigned int* output)
	{
		if (!this->read_data_type(8))
		{
			return false;
		}

		return this->read(32, output);
	}

	bool bit_buffer::read_data_type(const char expected)
	{
		char data_type = 0;

		if (!this->use_data_types_) return true;
		if (this->read(5, &data_type))
		{
			return (data_type == expected);
		}

		return false;
	}

	bool bit_buffer::write_bytes(const unsigned int bytes, const char* data)
	{
		return this->write_bytes(bytes, reinterpret_cast<const unsigned char*>(data));
	}

	bool bit_buffer::write_bytes(const unsigned int bytes, const unsigned char* data)
	{
		return this->write(bytes * 8, data);
	}

	bool bit_buffer::write_bool(bool data)
	{
		if (this->write_data_type(1))
		{
			return this->write(1, &data);
		}

		return false;
	}

	bool bit_buffer::write_int32(int data)
	{
		if (this->write_data_type(7))
		{
			return this->write(32, &data);
		}

		return false;
	}

	bool bit_buffer::write_uint32(unsigned int data)
	{
		if (this->write_data_type(8))
		{
			return this->write(32, &data);
		}

		return false;
	}

	bool bit_buffer::write_data_type(char data)
	{
		if (!this->use_data_types_)
		{
			return true;
		}

		return this->write(5, &data);
	}

	bool bit_buffer::read(unsigned int bits, void* output)
	{
		if (bits == 0) return false;
		if ((this->current_bit_ + bits) > (this->buffer_.size() * 8)) return false;

		int cur_byte = this->current_bit_ >> 3;
		auto cur_out = 0;

		const char* bytes = this->buffer_.data();
		const auto output_bytes = static_cast<unsigned char*>(output);

		while (bits > 0)
		{
			const int min_bit = (bits < 8) ? bits : 8;
			const auto this_byte = bytes[cur_byte++] & 0xFF;
			const int remain = this->current_bit_ & 7;

			if ((min_bit + remain) <= 8)
			{
				output_bytes[cur_out] = BYTE((0xFF >> (8 - min_bit)) & (this_byte >> remain));
			}
			else
			{
				output_bytes[cur_out] = BYTE(
					(0xFF >> (8 - min_bit)) & (bytes[cur_byte] << (8 - remain)) | (this_byte >> remain));
			}

			cur_out++;
			this->current_bit_ += min_bit;
			bits -= min_bit;
		}

		return true;
	}

	bool bit_buffer::write(const unsigned int bits, const void* data)
	{
		if (bits == 0) return false;
		this->buffer_.resize(this->buffer_.size() + (bits >> 3) + 1);

		int bit = bits;
		const auto bytes = const_cast<char*>(this->buffer_.data());
		const auto* input_bytes = static_cast<const unsigned char*>(data);

		while (bit > 0)
		{
			const int bit_pos = this->current_bit_ & 7;
			auto rem_bit = 8 - bit_pos;
			const auto this_write = (bit < rem_bit) ? bit : rem_bit;

			const BYTE mask = ((0xFF >> rem_bit) | (0xFF << (bit_pos + this_write)));
			const int byte_pos = this->current_bit_ >> 3;

			const BYTE temp_byte = (mask & bytes[byte_pos]);
			const BYTE this_bit = ((bits - bit) & 7);
			const auto this_byte = (bits - bit) >> 3;

			auto this_data = input_bytes[this_byte];

			const auto next_byte = (((bits - 1) >> 3) > this_byte) ? input_bytes[this_byte + 1] : 0;

			this_data = BYTE((next_byte << (8 - this_bit)) | (this_data >> this_bit));

			const BYTE out_byte = (~mask & (this_data << bit_pos) | temp_byte);
			bytes[byte_pos] = out_byte;

			this->current_bit_ += this_write;
			bit -= this_write;
		}

		return true;
	}

	void bit_buffer::set_use_data_types(const bool use_data_types)
	{
		this->use_data_types_ = use_data_types;
	}

	unsigned int bit_buffer::size() const
	{
		return this->current_bit_ / 8 + (this->current_bit_ % 8 ? 1 : 0);
	}

	std::string& bit_buffer::get_buffer()
	{
		this->buffer_.resize(this->size());
		return this->buffer_;
	}
}
