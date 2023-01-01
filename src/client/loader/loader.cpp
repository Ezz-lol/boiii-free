#include <std_include.hpp>
#include "loader.hpp"
#include "tls.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace loader
{
	namespace
	{
		template <typename T>
		T offset_pointer(void* data, const ptrdiff_t offset)
		{
			return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(data) + offset);
		}

		void load_imports(const utils::nt::library& target)
		{
			const auto* const import_directory = &target.get_optional_header()->DataDirectory[
				IMAGE_DIRECTORY_ENTRY_IMPORT];

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

		void load_relocations(const utils::nt::library& target)
		{
			if (!utils::nt::is_wine())
			{
				return;
			}

			auto* current_base = target.get_ptr();
			const auto initial_base = target.get_optional_header()->ImageBase;
			const auto delta = reinterpret_cast<ptrdiff_t>(current_base) - initial_base;

			PIMAGE_DATA_DIRECTORY directory = &target.get_optional_header()->DataDirectory[
				IMAGE_DIRECTORY_ENTRY_BASERELOC];
			if (directory->Size == 0)
			{
				return;
			}

			auto* relocation = reinterpret_cast<PIMAGE_BASE_RELOCATION>(current_base + directory->VirtualAddress);
			while (relocation->VirtualAddress > 0)
			{
				unsigned char* dest = current_base + relocation->VirtualAddress;

				auto* rel_info = offset_pointer<uint16_t*>(relocation, sizeof(IMAGE_BASE_RELOCATION));
				const auto* rel_info_end = offset_pointer<uint16_t*>(
					rel_info, relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION));

				for (; rel_info < rel_info_end; ++rel_info)
				{
					const int type = *rel_info >> 12;
					const int offset = *rel_info & 0xfff;

					switch (type)
					{
					case IMAGE_REL_BASED_ABSOLUTE:
						break;

					case IMAGE_REL_BASED_HIGHLOW:
					{
						auto* patch_address = reinterpret_cast<DWORD*>(dest + offset);
						utils::hook::set(patch_address, *patch_address + static_cast<DWORD>(delta));
						break;
					}

					case IMAGE_REL_BASED_DIR64:
					{
						auto* patch_address = reinterpret_cast<ULONGLONG*>(dest + offset);
						utils::hook::set(patch_address, *patch_address + static_cast<ULONGLONG>(delta));
						break;
					}

					default:
						throw std::runtime_error("Unknown relocation type: " + std::to_string(type));
					}
				}

				relocation = offset_pointer<PIMAGE_BASE_RELOCATION>(relocation, relocation->SizeOfBlock);
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

				auto* target_tls_start = PVOID(target_tls->StartAddressOfRawData);
				auto* tls_start = PVOID(source_tls->StartAddressOfRawData);
				const auto tls_size = source_tls->EndAddressOfRawData - source_tls->StartAddressOfRawData;
				const auto tls_index = *reinterpret_cast<DWORD*>(target_tls->AddressOfIndex);

				utils::hook::set<DWORD>(source_tls->AddressOfIndex, tls_index);

				if (target_tls->AddressOfCallBacks)
				{
					utils::hook::set<void*>(target_tls->AddressOfCallBacks, nullptr);
				}

				DWORD old_protect;
				VirtualProtect(target_tls_start, tls_size, PAGE_READWRITE, &old_protect);

				auto* const tls_base = *reinterpret_cast<LPVOID*>(__readgsqword(0x58) + 8ull * tls_index);
				std::memmove(tls_base, tls_start, tls_size);
				std::memmove(target_tls_start, tls_start, tls_size);

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

		load_relocations(target);
		load_imports(target);
		load_tls(target);

		return target;
	}
}
