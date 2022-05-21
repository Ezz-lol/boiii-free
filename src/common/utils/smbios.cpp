#include "smbios.hpp"
#include "memory.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <intrin.h>

namespace utils::smbios
{
	namespace
	{
#pragma warning(push)
#pragma warning(disable: 4200)
		struct RawSMBIOSData
		{
			BYTE Used20CallingMethod;
			BYTE SMBIOSMajorVersion;
			BYTE SMBIOSMinorVersion;
			BYTE DmiRevision;
			DWORD Length;
			BYTE SMBIOSTableData[];
		};

		typedef struct
		{
			BYTE type;
			BYTE length;
			WORD handle;
		} dmi_header;
#pragma warning(pop)

		std::vector<uint8_t> get_smbios_data()
		{
			DWORD size = 0;
			std::vector<uint8_t> data{};

			size = GetSystemFirmwareTable('RSMB', 0, nullptr, size);
			data.resize(size);
			GetSystemFirmwareTable('RSMB', 0, data.data(), size);

			return data;
		}

		std::string parse_uuid(const uint8_t* data)
		{
			if (utils::memory::is_set(data, 0, 16) || utils::memory::is_set(data, -1, 16))
			{
				return {};
			}

			char uuid[16] = {0};
			*reinterpret_cast<unsigned long*>(uuid + 0) =
				_byteswap_ulong(*reinterpret_cast<const unsigned long*>(data + 0));
			*reinterpret_cast<unsigned short*>(uuid + 4) =
				_byteswap_ushort(*reinterpret_cast<const unsigned short*>(data + 4));
			*reinterpret_cast<unsigned short*>(uuid + 6) =
				_byteswap_ushort(*reinterpret_cast<const unsigned short*>(data + 6));
			memcpy(uuid + 8, data + 8, 8);

			return std::string(uuid, sizeof(uuid));
		}
	}

	std::string get_uuid()
	{
		auto smbios_data = get_smbios_data();
		auto* raw_data = reinterpret_cast<RawSMBIOSData*>(smbios_data.data());

		auto* data = raw_data->SMBIOSTableData;
		for (DWORD i = 0; i + sizeof(dmi_header) < raw_data->Length;)
		{
			auto* header = reinterpret_cast<dmi_header*>(data + i);
			if (header->length < 4)
			{
				return {};
			}

			if (header->type == 0x01 && header->length >= 0x19)
			{
				return parse_uuid(data + i + 0x8);
			}

			i += header->length;
			while ((i + 1) < raw_data->Length && *reinterpret_cast<uint16_t*>(data + i) != 0)
			{
				++i;
			}

			i += 2;
		}

		return {};
	}
}
