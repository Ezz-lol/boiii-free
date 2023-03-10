#include "properties.hpp"

#include "finally.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/encodedstream.h"

#include "io.hpp"
#include "com.hpp"
#include "string.hpp"

namespace utils::properties
{
	namespace
	{
		typedef rapidjson::EncodedOutputStream<rapidjson::UTF8<>, rapidjson::FileWriteStream> OutputStream;
		typedef rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::FileReadStream> InputStream;

		std::filesystem::path get_properties_folder()
		{
			static auto props = get_appdata_path() / "user";
			return props;
		}

		std::filesystem::path get_properties_file()
		{
			static auto props = get_properties_folder() / "properties.json";
			return props;
		}

		rapidjson::Document load_properties()
		{
			rapidjson::Document default_doc{};
			default_doc.SetObject();

			char read_buffer[256]{0}; // Raw buffer for reading

			const std::wstring& props = get_properties_file();

			FILE* fp;
			auto err = _wfopen_s(&fp, props.data(), L"rb");
			if (err || !fp)
			{
				return default_doc;
			}

			const auto _ = finally([&]
			{
				if (fp)
				{
					fclose(fp);
				}
			});

			// This will handle the BOM
			rapidjson::FileReadStream bis(fp, read_buffer, sizeof(read_buffer));
			InputStream eis(bis);

			rapidjson::Document doc{};
			const rapidjson::ParseResult result = doc.ParseStream<rapidjson::kParseNoFlags, rapidjson::UTF8<>>(eis);

			if (!result || !doc.IsObject())
			{
				return default_doc;
			}

			return doc;
		}

		void store_properties(const rapidjson::Document& doc)
		{
			char write_buffer[256]{0}; // Raw buffer for writing

			const std::wstring& props = get_properties_file();
			io::create_directory(get_properties_folder());

			FILE* fp;
			auto err = _wfopen_s(&fp, props.data(), L"wb");
			if (err || !fp)
			{
				return;
			}

			const auto _ = finally([&]
			{
				if (fp)
				{
					fclose(fp);
				}
			});

			rapidjson::FileWriteStream bos(fp, write_buffer, sizeof(write_buffer));
			OutputStream eos(bos);

			rapidjson::Writer writer(eos);
			doc.Accept(writer);
		}
	}

	std::filesystem::path get_appdata_path()
	{
		PWSTR path;
		if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path)))
		{
			throw std::runtime_error("Failed to read APPDATA path!");
		}

		auto _ = utils::finally([&path]
		{
			CoTaskMemFree(path);
		});

		static auto appdata = std::filesystem::path(path) / "boiii";
		return appdata;
	}

	std::unique_lock<named_mutex> lock()
	{
		static named_mutex mutex{"boiii-properties-lock"};
		return std::unique_lock{mutex};
	}

	std::optional<std::string> load(const std::string& name)
	{
		const auto _ = lock();
		const auto doc = load_properties();

		if (!doc.HasMember(name))
		{
			return {};
		}

		const auto& value = doc[name];
		if (!value.IsString())
		{
			return {};
		}

		return {std::string{value.GetString()}};
	}

	void store(const std::string& name, const std::string& value)
	{
		const auto _ = lock();
		auto doc = load_properties();

		while (doc.HasMember(name))
		{
			doc.RemoveMember(name);
		}

		rapidjson::Value key{};
		key.SetString(name, doc.GetAllocator());

		rapidjson::Value member{};
		member.SetString(value, doc.GetAllocator());

		doc.AddMember(key, member, doc.GetAllocator());

		store_properties(doc);
	}
}
