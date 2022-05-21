#include "binary_resource.hpp"

#include <utility>
#include "nt.hpp"
#include "io.hpp"

namespace utils
{
	namespace
	{
		std::string get_temp_folder()
		{
			char path[MAX_PATH] = {0};
			if (!GetTempPathA(sizeof(path), path))
			{
				throw std::runtime_error("Unable to get temp path");
			}

			return path;
		}

		std::string write_existing_temp_file(const std::string& file, const std::string& data,
		                                       const bool fatal_if_overwrite_fails)
		{
			const auto temp = get_temp_folder();
			auto file_path = temp + file;

			std::string current_data;
			if (!io::read_file(file_path, &current_data))
			{
				if (!io::write_file(file_path, data))
				{
					throw std::runtime_error("Failed to write file: " + file_path);
				}

				return file_path;
			}

			if (current_data == data || io::write_file(file_path, data) || !fatal_if_overwrite_fails)
			{
				return file_path;
			}

			throw std::runtime_error(
				"Temporary file was already written, but differs. It can't be overwritten as it's still in use: " +
				file_path);
		}
	}

	binary_resource::binary_resource(const int id, std::string file)
		: filename_(std::move(file))
	{
		this->resource_ = nt::load_resource(id);

		if (this->resource_.empty())
		{
			throw std::runtime_error("Unable to load resource: " + std::to_string(id));
		}
	}

	std::string binary_resource::get_extracted_file(const bool fatal_if_overwrite_fails)
	{
		if (this->path_.empty())
		{
			this->path_ = write_existing_temp_file(this->filename_, this->resource_, fatal_if_overwrite_fails);
		}

		return this->path_;
	}

	const std::string& binary_resource::get_data() const
	{
		return this->resource_;
	}
}
