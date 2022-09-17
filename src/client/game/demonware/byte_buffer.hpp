#pragma once

namespace demonware
{
	class byte_buffer final
	{
	public:
		byte_buffer() = default;

		explicit byte_buffer(std::string buffer) : buffer_(std::move(buffer))
		{
		}

		bool read_bool(bool* output);
		bool read_byte(char* output);
		bool read_ubyte(unsigned char* output);
		bool read_int16(short* output);
		bool read_uint16(unsigned short* output);
		bool read_int32(int* output);
		bool read_uint32(unsigned int* output);
		bool read_int64(__int64* output);
		bool read_uint64(unsigned __int64* output);
		bool read_float(float* output);
		bool read_string(char** output);
		bool read_string(char* output, int length);
		bool read_string(std::string* output);
		bool read_blob(char** output, int* length);
		bool read_blob(std::string* output);
		bool read_data_type(char expected);

		bool read_array_header(unsigned char expected, unsigned int* element_count,
		                       unsigned int* element_size = nullptr);

		bool write_bool(bool data);
		bool write_byte(char data);
		bool write_ubyte(unsigned char data);
		bool write_int16(short data);
		bool write_uint16(unsigned short data);
		bool write_int32(int data);
		bool write_uint32(unsigned int data);
		bool write_int64(__int64 data);
		bool write_uint64(unsigned __int64 data);
		bool write_data_type(char data);
		bool write_float(float data);
		bool write_string(const char* data);
		bool write_string(const std::string& data);
		bool write_blob(const char* data, int length);
		bool write_blob(const std::string& data);

		bool write_array_header(unsigned char type, unsigned int element_count, unsigned int element_size);

		bool read(int bytes, void* output);
		bool write(int bytes, const void* data);
		bool write(const std::string& data);

		void set_use_data_types(bool use_data_types);
		size_t size() const;

		bool is_using_data_types() const;

		std::string& get_buffer();
		std::string get_remaining();

		bool has_more_data() const;

	private:
		std::string buffer_;
		size_t current_byte_ = 0;
		bool use_data_types_ = true;
	};
}
