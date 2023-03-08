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
		typedef rapidjson::GenericDocument<rapidjson::UTF16LE<>> WDocument;
		typedef rapidjson::GenericValue<rapidjson::UTF16LE<>> WValue;

		typedef rapidjson::EncodedOutputStream<rapidjson::UTF16LE<>, rapidjson::FileWriteStream> OutputStream;
		typedef rapidjson::EncodedInputStream<rapidjson::UTF16LE<>, rapidjson::FileReadStream> InputStream;

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

		WDocument load_properties()
		{
			WDocument default_doc{};
			default_doc.SetObject();

			char read_buffer[256]; // Raw buffer for reading

			const std::wstring& props = get_properties_file();

			FILE* fp;
			auto err = _wfopen_s(&fp, props.data(), L"rb");
			if (err || !fp)
			{
				return default_doc;
			}

			// This will handle the BOM
			rapidjson::FileReadStream bis(fp, read_buffer, sizeof(read_buffer));
			InputStream eis(bis);

			WDocument doc{};
			const rapidjson::ParseResult result = doc.ParseStream<rapidjson::kParseNoFlags, rapidjson::UTF16LE<>>(eis);

			fclose(fp);

			if (!result || !doc.IsObject())
			{
				return default_doc;
			}

			return doc;
		}

		void store_properties(const WDocument& doc)
		{
			char write_buffer[256]; // Raw buffer for writing

			const std::wstring& props = get_properties_file();
			io::create_directory(get_properties_folder());

			FILE* fp;
			auto err = _wfopen_s(&fp, props.data(), L"wb");
			if (err || !fp)
			{
				return;
			}

			rapidjson::FileWriteStream bos(fp, write_buffer, sizeof(write_buffer));
			OutputStream eos(bos, true); // Write BOM

			rapidjson::Writer<OutputStream, rapidjson::UTF16LE<>, rapidjson::UTF16LE<>> writer(eos);
			doc.Accept(writer);

			fclose(fp);
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

	std::optional<std::wstring> load(const std::wstring& name)
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

		return {std::wstring{value.GetString()}};
	}

	std::optional<std::string> load(const std::string& name)
	{
		const auto result = load(string::convert(name));
		if (!result)
		{
			return {};
		}

		return {string::convert(*result)};
	}

	void store(const std::wstring& name, const std::wstring& value)
	{
		const auto _ = lock();
		auto doc = load_properties();

		while (doc.HasMember(name))
		{
			doc.RemoveMember(name);
		}

		WValue key{};
		key.SetString(name, doc.GetAllocator());

		WValue member{};
		member.SetString(value, doc.GetAllocator());

		doc.AddMember(key, member, doc.GetAllocator());

		store_properties(doc);
	}

	void store(const std::string& name, const std::string& value)
	{
		store(string::convert(name), string::convert(value));
	}
}
