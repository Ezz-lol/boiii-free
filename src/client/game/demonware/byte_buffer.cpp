#include <std_include.hpp>
#include "byte_buffer.hpp"
#include <utils/string.hpp>

namespace demonware
{
	bool byte_buffer::read_bool(bool* output)
	{
		if (!this->read_data_type(1)) return false;
		return this->read(1, output);
	}

	bool byte_buffer::read_byte(char* output)
	{
		if (!this->read_data_type(2)) return false;
		return this->read(1, output);
	}

	bool byte_buffer::read_ubyte(unsigned char* output)
	{
		if (!this->read_data_type(3)) return false;
		return this->read(1, output);
	}

	bool byte_buffer::read_int16(short* output)
	{
		if (!this->read_data_type(5)) return false;
		return this->read(2, output);
	}

	bool byte_buffer::read_uint16(unsigned short* output)
	{
		if (!this->read_data_type(6)) return false;
		return this->read(2, output);
	}

	bool byte_buffer::read_int32(int* output)
	{
		if (!this->read_data_type(7)) return false;
		return this->read(4, output);
	}

	bool byte_buffer::read_uint32(unsigned int* output)
	{
		if (!this->read_data_type(8)) return false;
		return this->read(4, output);
	}

	bool byte_buffer::read_int64(__int64* output)
	{
		if (!this->read_data_type(9)) return false;
		return this->read(8, output);
	}

	bool byte_buffer::read_uint64(unsigned __int64* output)
	{
		if (!this->read_data_type(10)) return false;
		return this->read(8, output);
	}

	bool byte_buffer::read_float(float* output)
	{
		if (!this->read_data_type(13)) return false;
		return this->read(4, output);
	}

	bool byte_buffer::read_string(std::string* output)
	{
		char* out_data;
		if (this->read_string(&out_data))
		{
			output->clear();
			output->append(out_data);
			return true;
		}

		return false;
	}

	bool byte_buffer::read_string(char** output)
	{
		if (!this->read_data_type(16)) return false;

		*output = const_cast<char*>(this->buffer_.data()) + this->current_byte_;
		this->current_byte_ += strlen(*output) + 1;

		return true;
	}

	bool byte_buffer::read_string(char* output, const int length)
	{
		if (!this->read_data_type(16)) return false;

		utils::string::copy(output, static_cast<size_t>(length), const_cast<char*>(this->buffer_.data()) + this->current_byte_);
		this->current_byte_ += strlen(output) + 1;

		return true;
	}

	bool byte_buffer::read_blob(std::string* output)
	{
		char* out_data;
		int length;
		if (this->read_blob(&out_data, &length))
		{
			output->clear();
			output->append(out_data, length);
			return true;
		}

		return false;
	}

	bool byte_buffer::read_blob(char** output, int* length)
	{
		if (!this->read_data_type(0x13))
		{
			return false;
		}

		unsigned int size;
		this->read_uint32(&size);

		*output = const_cast<char*>(this->buffer_.data()) + this->current_byte_;
		*length = static_cast<int>(size);

		this->current_byte_ += size;

		return true;
	}

	bool byte_buffer::read_data_type(const char expected)
	{
		if (!this->use_data_types_) return true;

		char type;
		this->read(1, &type);
		return type == expected;
	}

	bool byte_buffer::read_array_header(const unsigned char expected, unsigned int* element_count,
	                                    unsigned int* element_size)
	{
		if (element_count) *element_count = 0;
		if (element_size) *element_size = 0;

		if (!this->read_data_type(expected + 100)) return false;

		uint32_t array_size, el_count;
		if (!this->read_uint32(&array_size)) return false;

		this->set_use_data_types(false);
		this->read_uint32(&el_count);
		this->set_use_data_types(true);

		if (element_count) *element_count = el_count;
		if (element_size) *element_size = array_size / el_count;

		return true;
	}

	bool byte_buffer::write_bool(bool data)
	{
		this->write_data_type(1);
		return this->write(1, &data);
	}

	bool byte_buffer::write_byte(char data)
	{
		this->write_data_type(2);
		return this->write(1, &data);
	}

	bool byte_buffer::write_ubyte(unsigned char data)
	{
		this->write_data_type(3);
		return this->write(1, &data);
	}

	bool byte_buffer::write_int16(short data)
	{
		this->write_data_type(5);
		return this->write(2, &data);
	}

	bool byte_buffer::write_uint16(unsigned short data)
	{
		this->write_data_type(6);
		return this->write(2, &data);
	}

	bool byte_buffer::write_int32(int data)
	{
		this->write_data_type(7);
		return this->write(4, &data);
	}

	bool byte_buffer::write_uint32(unsigned int data)
	{
		this->write_data_type(8);
		return this->write(4, &data);
	}

	bool byte_buffer::write_int64(__int64 data)
	{
		this->write_data_type(9);
		return this->write(8, &data);
	}

	bool byte_buffer::write_uint64(unsigned __int64 data)
	{
		this->write_data_type(10);
		return this->write(8, &data);
	}

	bool byte_buffer::write_data_type(char data)
	{
		if (!this->use_data_types_) return true;
		return this->write(1, &data);
	}

	bool byte_buffer::write_float(float data)
	{
		this->write_data_type(13);
		return this->write(4, &data);
	}

	bool byte_buffer::write_string(const std::string& data)
	{
		return this->write_string(data.data());
	}

	bool byte_buffer::write_string(const char* data)
	{
		this->write_data_type(16);
		return this->write(static_cast<int>(strlen(data)) + 1, data);
	}

	bool byte_buffer::write_blob(const std::string& data)
	{
		return this->write_blob(data.data(), INT(data.size()));
	}

	bool byte_buffer::write_blob(const char* data, const int length)
	{
		this->write_data_type(0x13);
		this->write_uint32(length);

		return this->write(length, data);
	}

	bool byte_buffer::write_array_header(const unsigned char type, const unsigned int element_count,
	                                     const unsigned int element_size)
	{
		const auto using_types = this->is_using_data_types();
		this->set_use_data_types(false);

		auto result = this->write_ubyte(type + 100);

		this->set_use_data_types(true);
		result &= this->write_uint32(element_count * element_size);
		this->set_use_data_types(false);

		result &= this->write_uint32(element_count);

		this->set_use_data_types(using_types);
		return result;
	}

	bool byte_buffer::read(const int bytes, void* output)
	{
		if (bytes + this->current_byte_ > this->buffer_.size()) return false;

		std::memmove(output, this->buffer_.data() + this->current_byte_, bytes);
		this->current_byte_ += bytes;

		return true;
	}

	bool byte_buffer::write(const int bytes, const void* data)
	{
		this->buffer_.append(static_cast<const char*>(data), bytes);
		this->current_byte_ += bytes;
		return true;
	}

	bool byte_buffer::write(const std::string& data)
	{
		return this->write(static_cast<int>(data.size()), data.data());
	}

	void byte_buffer::set_use_data_types(const bool use_data_types)
	{
		this->use_data_types_ = use_data_types;
	}

	size_t byte_buffer::size() const
	{
		return this->buffer_.size();
	}

	bool byte_buffer::is_using_data_types() const
	{
		return use_data_types_;
	}

	std::string& byte_buffer::get_buffer()
	{
		return this->buffer_;
	}

	std::string byte_buffer::get_remaining()
	{
		return std::string(this->buffer_.begin() + this->current_byte_, this->buffer_.end());
	}

	bool byte_buffer::has_more_data() const
	{
		return this->buffer_.size() > this->current_byte_;
	}
}
