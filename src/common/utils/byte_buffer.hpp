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

		void write_string(const char* str, const size_t length)
		{
			this->write<uint32_t>(static_cast<uint32_t>(length));
			this->write(str, length);
		}

		void write_string(const std::string& str)
		{
			this->write_string(str.data(), str.size());
		}

		void write_string(const char* str)
		{
			this->write_string(str, strlen(str));
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
			this->write(static_cast<uint32_t>(vec.size()));
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
			const auto size = this->read<uint32_t>();
			const auto totalSize = size * sizeof(T);

			if (this->offset_ + totalSize > this->buffer_.size())
			{
				throw std::runtime_error("Out of bounds read from byte buffer");
			}

			result.resize(size);
			this->read(result.data(), totalSize);

			return result;
		}

		std::string read_string()
		{
			std::string result{};
			const auto size = this->read<uint32_t>();

			if (this->offset_ + size > this->buffer_.size())
			{
				throw std::runtime_error("Out of bounds read from byte buffer");
			}

			result.resize(size);
			this->read(result.data(), size);

			return result;
		}

		std::vector<uint8_t> read_data(size_t length);

	private:
		bool writing_{false};
		size_t offset_{0};
		std::string buffer_{};
	};
}
