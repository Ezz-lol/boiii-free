#include "image.hpp"
#include <stdexcept>

#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 4244)
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include "finally.hpp"

namespace utils::image
{
	image load_image(const std::string& data)
	{
		stbi_uc* buffer{};
		const auto _ = finally([&]
		{
			if (buffer)
			{
				stbi_image_free(buffer);
			}
		});

		constexpr int channels = 4;
		int x, y, channels_in_file;
		buffer = stbi_load_from_memory(reinterpret_cast<const uint8_t*>(data.data()),
		                               static_cast<int>(data.size()), &x, &y, &channels_in_file, channels);
		if (!buffer)
		{
			throw std::runtime_error("Failed to load image");
		}

		image res{};
		res.width = static_cast<size_t>(x);
		res.height = static_cast<size_t>(y);
		res.data.assign(reinterpret_cast<const char*>(buffer), res.width * res.height * channels);

		return res;
	}

	object create_bitmap(const image& img)
	{
		auto copy = img.data;

		for (size_t i = 0; i < (img.width * img.height); ++i)
		{
			auto& r = copy[i * 4 + 0];
			auto& b = copy[i * 4 + 2];
			std::swap(r, b);
		}

		return CreateBitmap(static_cast<int>(img.width), static_cast<int>(img.height), 4, 8, copy.data());
	}
}
