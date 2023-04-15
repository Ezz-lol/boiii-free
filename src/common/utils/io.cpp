#include "io.hpp"
#include "nt.hpp"
#include <fstream>

namespace utils::io
{
	bool remove_file(const std::filesystem::path& file)
	{
		if (DeleteFileW(file.wstring().data()) != FALSE)
		{
			return true;
		}

		return GetLastError() == ERROR_FILE_NOT_FOUND;
	}

	bool move_file(const std::filesystem::path& src, const std::filesystem::path& target)
	{
		return MoveFileW(src.wstring().data(), target.wstring().data()) == TRUE;
	}

	bool file_exists(const std::string& file)
	{
		return std::ifstream(file).good();
	}

	bool write_file(const std::string& file, const std::string& data, const bool append)
	{
		const auto pos = file.find_last_of("/\\");
		if (pos != std::string::npos)
		{
			create_directory(file.substr(0, pos));
		}

		std::ofstream stream(
			file, std::ios::binary | std::ofstream::out | (append ? std::ofstream::app : 0));

		if (stream.is_open())
		{
			stream.write(data.data(), static_cast<std::streamsize>(data.size()));
			stream.close();
			return true;
		}

		return false;
	}

	std::string read_file(const std::string& file)
	{
		std::string data;
		read_file(file, &data);
		return data;
	}

	bool read_file(const std::string& file, std::string* data)
	{
		if (!data) return false;
		data->clear();

		if (file_exists(file))
		{
			std::ifstream stream(file, std::ios::binary);
			if (!stream.is_open()) return false;

			stream.seekg(0, std::ios::end);
			const std::streamsize size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				data->resize(static_cast<std::uint32_t>(size));
				stream.read(data->data(), size);
				stream.close();
				return true;
			}
		}

		return false;
	}

	std::size_t file_size(const std::string& file)
	{
		if (file_exists(file))
		{
			std::ifstream stream(file, std::ios::binary);

			if (stream.good())
			{
				stream.seekg(0, std::ios::end);
				return static_cast<std::size_t>(stream.tellg());
			}
		}

		return 0;
	}

	bool create_directory(const std::filesystem::path& directory)
	{
		return std::filesystem::create_directories(directory);
	}

	bool directory_exists(const std::filesystem::path& directory)
	{
		return std::filesystem::is_directory(directory);
	}

	bool directory_is_empty(const std::filesystem::path& directory)
	{
		return std::filesystem::is_empty(directory);
	}

	void copy_folder(const std::filesystem::path& src, const std::filesystem::path& target)
	{
		std::filesystem::copy(src, target,
		                      std::filesystem::copy_options::overwrite_existing |
		                      std::filesystem::copy_options::recursive);
	}


	bool file_exists(const std::wstring& file)
	{
		return std::ifstream(file).good();
	}

	bool write_file(const std::wstring& file, const std::string& data, const bool append)
	{
		const auto pos = file.find_last_of(L"/\\");
		if (pos != std::string::npos)
		{
			create_directory(file.substr(0, pos));
		}

		std::ofstream stream(
			file, std::ios::binary | std::ofstream::out | (append ? std::ofstream::app : 0));

		if (stream.is_open())
		{
			stream.write(data.data(), static_cast<std::streamsize>(data.size()));
			stream.close();
			return true;
		}

		return false;
	}

	std::string read_file(const std::wstring& file)
	{
		std::string data;
		read_file(file, &data);
		return data;
	}

	bool read_file(const std::wstring& file, std::string* data)
	{
		if (!data) return false;
		data->clear();

		if (file_exists(file))
		{
			std::ifstream stream(file, std::ios::binary);
			if (!stream.is_open()) return false;

			stream.seekg(0, std::ios::end);
			const std::streamsize size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				data->resize(static_cast<uint32_t>(size));
				stream.read(data->data(), size);
				stream.close();
				return true;
			}
		}

		return false;
	}

	std::size_t file_size(const std::wstring& file)
	{
		if (file_exists(file))
		{
			std::ifstream stream(file, std::ios::binary);

			if (stream.good())
			{
				stream.seekg(0, std::ios::end);
				return static_cast<std::size_t>(stream.tellg());
			}
		}

		return 0;
	}

	std::vector<std::filesystem::path> list_files(const std::filesystem::path& directory, const bool recursive)
	{
		std::error_code code{};
		std::vector<std::filesystem::path> files;

		if (recursive)
		{
			for (auto& file : std::filesystem::recursive_directory_iterator(directory, code))
			{
				files.push_back(file.path());
			}
		}
		else
		{
			for (auto& file : std::filesystem::directory_iterator(directory, code))
			{
				files.push_back(file.path());
			}
		}

		return files;
	}
}
