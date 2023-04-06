#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace utils
{
	class byte_buffer
	{
	public:
		byte_buffer();
		byte_buffer(std::string buffer);

		template <typename T>
		byte_buffer(const std::basic_string_view<T>& buffer)
			: byte_buffer(std::string(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(T)))
		{
		}

		void write(const void* buffer, size_t length);

		void write(const std::string& string, const bool null_terminate = false)
		{
			const size_t addend = null_terminate ? 1 : 0;
			this->write(string.data(), string.size() + addend);
		}

		void write_string(const std::string& string)
		{
			this->write(string, true);
		}

		template <typename T>
		void write(const T& object)
		{
			this->write(&object, sizeof(object));
		}

		template <typename T>
		void write(const std::vector<T>& vec)
		{
			this->write(vec.data(), vec.size() * sizeof(T));
		}

		template <typename T>
		void write_vector(const std::vector<T>& vec)
		{
			this->write<uint32_t>(static_cast<uint32_t>(vec.size()));
			this->write(vec);
		}

		const std::string& get_buffer() const
		{
			return this->buffer_;
		}

		std::string move_buffer()
		{
			return std::move(this->buffer_);
		}

		void read(void* data, size_t length);

		template <typename T>
		T read()
		{
			T object{};
			this->read(&object, sizeof(object));
			return object;
		}

		template <typename T>
		std::vector<T> read_vector()
		{
			std::vector<T> result{};
			const auto size = read<uint32_t>();
			const auto totalSize = size * sizeof(T);

			if (this->offset_ + totalSize > this->buffer_.size())
			{
				throw std::runtime_error("Out of bounds read from byte buffer");
			}

			result.resize(size);
			this->read(result.data(), totalSize);

			return result;
		}

		std::string read_string();
		std::string read_string(size_t length);
		std::vector<uint8_t> read_data(size_t length);

	private:
		bool writing_{false};
		size_t offset_{0};
		std::string buffer_{};
	};
}
