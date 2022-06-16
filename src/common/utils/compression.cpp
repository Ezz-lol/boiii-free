#include "memory.hpp"
#include "compression.hpp"

#include <zlib.h>
#include <zip.h>

#include "io.hpp"
#include "finally.hpp"

namespace utils::compression
{
	namespace zlib
	{
		namespace
		{
			class zlib_stream
			{
			public:
				zlib_stream()
				{
					memset(&stream_, 0, sizeof(stream_));
					valid_ = inflateInit(&stream_) == Z_OK;
				}

				zlib_stream(zlib_stream&&) = delete;
				zlib_stream(const zlib_stream&) = delete;
				zlib_stream& operator=(zlib_stream&&) = delete;
				zlib_stream& operator=(const zlib_stream&) = delete;

				~zlib_stream()
				{
					if (valid_)
					{
						inflateEnd(&stream_);
					}
				}

				z_stream& get()
				{
					return stream_; //
				}

				bool is_valid() const
				{
					return valid_;
				}

			private:
				bool valid_{false};
				z_stream stream_{};
			};
		}

		std::string decompress(const std::string& data)
		{
			std::string buffer{};
			zlib_stream stream_container{};
			if (!stream_container.is_valid())
			{
				return {};
			}

			int ret{};
			size_t offset = 0;
			static thread_local uint8_t dest[CHUNK] = {0};
			auto& stream = stream_container.get();

			do
			{
				const auto input_size = std::min(sizeof(dest), data.size() - offset);
				stream.avail_in = static_cast<uInt>(input_size);
				stream.next_in = reinterpret_cast<const Bytef*>(data.data()) + offset;
				offset += stream.avail_in;

				do
				{
					stream.avail_out = sizeof(dest);
					stream.next_out = dest;

					ret = inflate(&stream, Z_NO_FLUSH);
					if (ret != Z_OK && ret != Z_STREAM_END)
					{
						return {};
					}

					buffer.insert(buffer.end(), dest, dest + sizeof(dest) - stream.avail_out);
				}
				while (stream.avail_out == 0);
			}
			while (ret != Z_STREAM_END);

			return buffer;
		}

		std::string compress(const std::string& data)
		{
			std::string result{};
			auto length = compressBound(static_cast<uLong>(data.size()));
			result.resize(length);

			if (compress2(reinterpret_cast<Bytef*>(result.data()), &length,
			              reinterpret_cast<const Bytef*>(data.data()), static_cast<uLong>(data.size()),
			              Z_BEST_COMPRESSION) != Z_OK)
			{
				return {};
			}

			result.resize(length);
			return result;
		}
	}

	namespace zip
	{
		namespace
		{
			bool add_file(zipFile& zip_file, const std::string& filename, const std::string& data)
			{
				const auto zip_64 = data.size() > 0xffffffff ? 1 : 0;
				if (ZIP_OK != zipOpenNewFileInZip64(zip_file, filename.data(), nullptr, nullptr, 0, nullptr, 0, nullptr,
				                                    Z_DEFLATED, Z_BEST_COMPRESSION, zip_64))
				{
					return false;
				}

				const auto _ = finally([&zip_file]()
				{
					zipCloseFileInZip(zip_file);
				});

				return ZIP_OK == zipWriteInFileInZip(zip_file, data.data(), static_cast<unsigned>(data.size()));
			}
		}

		void archive::add(std::string filename, std::string data)
		{
			this->files_[std::move(filename)] = std::move(data);
		}

		bool archive::write(const std::string& filename, const std::string& comment)
		{
			// Hack to create the directory :3
			io::write_file(filename, {});
			io::remove_file(filename);

			auto* zip_file = zipOpen64(filename.data(), 0);
			if (!zip_file)
			{
				return false;
			}

			const auto _ = finally([&zip_file, &comment]()
			{
				zipClose(zip_file, comment.empty() ? nullptr : comment.data());
			});

			for (const auto& file : this->files_)
			{
				if (!add_file(zip_file, file.first, file.second))
				{
					return false;
				}
			}

			return true;
		}
	}
}
