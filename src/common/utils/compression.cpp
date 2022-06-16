#include "memory.hpp"
#include "compression.hpp"

#include <zlib.h>
#include <zip.h>
#include <unzip.h>

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

		namespace
		{
			std::optional<std::pair<std::string, std::string>> read_zip_file_entry(unzFile& zip_file)
			{
				char filename[1024]{};
				unz_file_info file_info{};
				if (unzGetCurrentFileInfo(zip_file, &file_info, filename, sizeof(filename), nullptr, 0, nullptr, 0) !=
					UNZ_OK)
				{
					return {};
				}

				if (unzOpenCurrentFile(zip_file) != UNZ_OK)
				{
					return {};
				}

				auto _ = finally([&zip_file]
				{
					unzCloseCurrentFile(zip_file);
				});

				int error = UNZ_OK;
				std::string out_buffer{};
				static thread_local char buffer[0x2000];

				do
				{
					error = unzReadCurrentFile(zip_file, buffer, sizeof(buffer));
					if (error < 0)
					{
						return {};
					}

					// Write data to file.
					if (error > 0)
					{
						out_buffer.append(buffer, error);
					}
				}
				while (error > 0);

				return std::pair<std::string, std::string>{filename, out_buffer};
			}

			class memory_file
			{
			public:
				memory_file(const std::string& data)
					: data_(data)
				{
					func_def_.opaque = this;
					func_def_.zopen64_file = open_file_static;
					func_def_.zseek64_file = seek_file_static;
					func_def_.ztell64_file = tell_file_static;
					func_def_.zread_file = read_file_static;
					func_def_.zwrite_file = write_file_static;
					func_def_.zclose_file = close_file_static;
					func_def_.zerror_file = testerror_file_static;
				}

				const char* get_name() const
				{
					return "blub";
				}

				zlib_filefunc64_def* get_func_def()
				{
					return &this->func_def_;
				}

			private:
				const std::string& data_;
				size_t offset_{0};
				zlib_filefunc64_def func_def_{};

				voidpf open_file(const void* filename, const int mode) const
				{
					if (mode != (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_EXISTING))
					{
						return nullptr;
					}

					if (strcmp(static_cast<const char*>(filename), get_name()) != 0)
					{
						return nullptr;
					}

					return reinterpret_cast<voidpf>(1);
				}

				long seek_file(const voidpf stream, const ZPOS64_T offset, const int origin)
				{
					if (stream != reinterpret_cast<voidpf>(1))
					{
						return -1;
					}

					size_t target_base = this->data_.size();
					if (origin == ZLIB_FILEFUNC_SEEK_CUR)
					{
						target_base = this->offset_;
					}
					else if (origin == ZLIB_FILEFUNC_SEEK_SET)
					{
						target_base = 0;
					}

					const auto target_offset = target_base + offset;
					if (target_offset > this->data_.size())
					{
						return -1;
					}

					this->offset_ = target_offset;
					return 0;
				}

				ZPOS64_T tell_file(const voidpf stream) const
				{
					if (stream != reinterpret_cast<voidpf>(1))
					{
						return static_cast<ZPOS64_T>(-1);
					}

					return this->offset_;
				}

				uLong read_file(const voidpf stream, void* buf, const uLong size)
				{
					if (stream != reinterpret_cast<voidpf>(1))
					{
						return 0;
					}

					const auto file_end = this->data_.size();
					const auto start = this->offset_;
					const auto end = std::min(this->offset_ + size, file_end);
					const auto length = end - start;

					memcpy(buf, this->data_.data() + start, length);
					this->offset_ = end;

					return static_cast<uLong>(length);
				}

				static voidpf open_file_static(const voidpf opaque, const void* filename, const int mode)
				{
					return static_cast<memory_file*>(opaque)->open_file(filename, mode);
				}

				static long seek_file_static(const voidpf opaque, const voidpf stream, const ZPOS64_T offset,
				                             const int origin)
				{
					return static_cast<memory_file*>(opaque)->seek_file(stream, offset, origin);
				}

				static ZPOS64_T tell_file_static(const voidpf opaque, const voidpf stream)
				{
					return static_cast<memory_file*>(opaque)->tell_file(stream);
				}

				static uLong read_file_static(const voidpf opaque, const voidpf stream, void* buf, const uLong size)
				{
					return static_cast<memory_file*>(opaque)->read_file(stream, buf, size);
				}

				static uLong write_file_static(voidpf, voidpf, const void*, uLong)
				{
					return 0;
				}

				static int close_file_static(voidpf, voidpf)
				{
					return 0;
				}

				static int testerror_file_static(voidpf, voidpf)
				{
					return 0;
				}
			};
		}

		std::unordered_map<std::string, std::string> extract(const std::string& data)
		{
			memory_file mem_file(data);

			auto zip_file = unzOpen2_64(mem_file.get_name(), mem_file.get_func_def());
			auto _ = finally([&zip_file]
			{
				if (zip_file)
				{
					unzClose(zip_file);
				}
			});

			if (!zip_file)
			{
				return {};
			}

			unz_global_info global_info{};
			if (unzGetGlobalInfo(zip_file, &global_info) != UNZ_OK)
			{
				return {};
			}

			std::unordered_map<std::string, std::string> files{};
			files.reserve(global_info.number_entry);

			for (auto i = 0ul; i < global_info.number_entry; ++i)
			{
				if (i > 0 && unzGoToNextFile(zip_file) != UNZ_OK)
				{
					break;
				}

				auto file = read_zip_file_entry(zip_file);
				if (!file)
				{
					continue;
				}

				files[std::move(file->first)] = std::move(file->second);
			}

			return files;
		}
	}
}
