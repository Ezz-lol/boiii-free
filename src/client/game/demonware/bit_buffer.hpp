#pragma once

namespace demonware
{
	class bit_buffer final
	{
	public:
		bit_buffer() = default;

		explicit bit_buffer(std::string buffer) : buffer_(std::move(buffer))
		{
		}

		bool read_bytes(unsigned int bytes, unsigned char* output);
		bool read_bool(bool* output);
		bool read_uint32(unsigned int* output);
		bool read_data_type(char expected);

		bool write_bytes(unsigned int bytes, const char* data);
		bool write_bytes(unsigned int bytes, const unsigned char* data);
		bool write_bool(bool data);
		bool write_int32(int data);
		bool write_uint32(unsigned int data);
		bool write_data_type(char data);

		bool read(unsigned int bits, void* output);
		bool write(unsigned int bits, const void* data);

		void set_use_data_types(bool use_data_types);

		unsigned int size() const;

		std::string& get_buffer();

	private:
		std::string buffer_{};
		unsigned int current_bit_ = 0;
		bool use_data_types_ = true;
	};
}
