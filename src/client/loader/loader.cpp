#include <std_include.hpp>
#include "loader.hpp"
#include "tls.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace loader
{
	namespace
	{
		void load_imports(const utils::nt::library& target) 
		{
			const auto* const import_directory = &target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

			auto* descriptor = PIMAGE_IMPORT_DESCRIPTOR(target.get_ptr() + import_directory->VirtualAddress);

			while (descriptor->Name)
			{
				std::string name = LPSTR(target.get_ptr() + descriptor->Name);

				auto* name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->
					OriginalFirstThunk);
				auto* address_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);

				if (!descriptor->OriginalFirstThunk)
				{
					name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);
				}

				while (*name_table_entry)
				{
					FARPROC function = nullptr;
					std::string function_name;
					const char* function_procname;

					if (IMAGE_SNAP_BY_ORDINAL(*name_table_entry))
					{
						function_name = "#" + std::to_string(IMAGE_ORDINAL(*name_table_entry));
						function_procname = MAKEINTRESOURCEA(IMAGE_ORDINAL(*name_table_entry));
					}
					else
					{
						const auto* import = PIMAGE_IMPORT_BY_NAME(target.get_ptr() + *name_table_entry);
						function_name = import->Name;
						function_procname = function_name.data();
					}

					auto library = utils::nt::library::load(name);
					if (library)
					{
						function = GetProcAddress(library, function_procname);
					}

					if (!function)
					{
						throw std::runtime_error(utils::string::va("Unable to load import '%s' from library '%s'",
						                                           function_name.data(), name.data()));
					}

					utils::hook::set(address_table_entry, reinterpret_cast<uintptr_t>(function));

					name_table_entry++;
					address_table_entry++;
				}

				descriptor++;
			}
		}

		void load_tls(const utils::nt::library& target)
		{
			if (target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
			{
				auto* target_tls = tls::allocate_tls_index();
				const auto* const source_tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(target.get_ptr() + target.
					get_optional_header()
					->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

				const auto tls_size = source_tls->EndAddressOfRawData - source_tls->StartAddressOfRawData;
				const auto tls_index = *reinterpret_cast<DWORD*>(target_tls->AddressOfIndex);
				utils::hook::set<DWORD>(source_tls->AddressOfIndex, tls_index);

				if (target_tls->AddressOfCallBacks)
				{
					utils::hook::set<void*>(target_tls->AddressOfCallBacks, nullptr);
				}

				DWORD old_protect;
				VirtualProtect(PVOID(target_tls->StartAddressOfRawData),
				               source_tls->EndAddressOfRawData - source_tls->StartAddressOfRawData, PAGE_READWRITE,
				               &old_protect);

				auto* const tls_base = *reinterpret_cast<LPVOID*>(__readgsqword(0x58) + 8ull * tls_index);
				std::memmove(tls_base, PVOID(source_tls->StartAddressOfRawData), tls_size);
				std::memmove(PVOID(target_tls->StartAddressOfRawData), PVOID(source_tls->StartAddressOfRawData),
				             tls_size);

				VirtualProtect(target_tls, sizeof(*target_tls), PAGE_READWRITE, &old_protect);
				*target_tls = *source_tls;
			}
		}
	}

	utils::nt::library load_binary(const std::string& filename)
	{
		const auto target = utils::nt::library::load(filename);
		if (!target)
		{
			throw std::runtime_error{"Failed to map: " + filename};
		}

		load_imports(target);
		load_tls(target);

		return target;
	}
}
