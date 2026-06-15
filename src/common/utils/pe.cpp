#include "pe.hpp"
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <cwchar>

#if !(defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) ||             \
      defined(__MINGW64__)) &&                                                 \
    !defined(_MSC_VER)
namespace utils::pe {

wchar_t MultiByteToWideChar(unsigned int CodePage, unsigned long dwFlags,
                            const char *lpMultiByteStr, int cbMultiByte,
                            wchar_t *lpWideCharStr, int cchWideChar) {
  size_t convertedChars = 0;
  mbstowcs(lpWideCharStr, lpMultiByteStr, static_cast<size_t>(cbMultiByte));
  return static_cast<wchar_t>(convertedChars);
}

uint64_t __readgsqword(unsigned long Offset) {
  uint64_t Value;
  __asm__ volatile("mov %%gs:(%1), %0" : "=r"(Value) : "r"(Offset));
  return Value;
}

HMODULE GetModuleHandleA(const char *name) {

  int wide_str_len = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
  wchar_t *wide_str = new wchar_t[wide_str_len];
  MultiByteToWideChar(CP_UTF8, 0, name, -1, wide_str, wide_str_len);
  LPCWSTR lp_wide_str = wide_str;

#if defined(_WIN64) || !defined(_WIN32) // assume 64-bit if not windows
  PPEB p_peb = (PPEB)(__readgsqword(0x60));
#else
  PPEB p_peb = (PPEB)(__readfsdword(0x 30));
#endif

  PPEB_LDR_DATA p_ldr = (PPEB_LDR_DATA)(p_peb->Ldr);
  PLDR_DATA_TABLE_ENTRY p_dte =
      (PLDR_DATA_TABLE_ENTRY)(p_ldr->InMemoryOrderModuleList.Flink);
  while (p_dte) {
    if (p_dte->FullDllName.Length != 0) {
      if (std::wcsncmp(p_dte->FullDllName.Buffer, lp_wide_str, wide_str_len) ==
          0) {
        return (HMODULE)p_dte->Reserved2[0];
      }
    } else {
      break;
    }
    p_dte = *(PLDR_DATA_TABLE_ENTRY *)(p_dte);
  }

  return nullptr;
}
} // namespace utils::pe
#endif

namespace utils::pe {
bool loaded_pe(std::vector<uint8_t> &pe_buffer) {
  try {
    // Get the base address of the current module
    const uint8_t *base_address =
        reinterpret_cast<uint8_t *>(GetModuleHandleA(nullptr));
    if (base_address) {

      // Read DOS header
      const PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(
          const_cast<uint8_t *>(base_address));
      if (dos_header->e_magic == IMAGE_DOS_SIGNATURE) {

        // Read NT headers
        const PIMAGE_NT_HEADERS nt_headers =
            reinterpret_cast<PIMAGE_NT_HEADERS>(
                const_cast<uint8_t *>(base_address + dos_header->e_lfanew));
        if (nt_headers->Signature == IMAGE_NT_SIGNATURE) {

          // Get size of image
          const uint32_t image_size = nt_headers->OptionalHeader.SizeOfImage;

          // Allocate buffer for the PE file
          pe_buffer.resize(image_size);

          // Copy headers
          const uint32_t headers_size =
              nt_headers->OptionalHeader.SizeOfHeaders;
          memcpy(pe_buffer.data(), base_address, headers_size);

          // Copy each section to its proper file offset
          const PIMAGE_SECTION_HEADER section_header =
              IMAGE_FIRST_SECTION(nt_headers);
          for (uint16_t i = 0; i < nt_headers->FileHeader.NumberOfSections;
               i++) {
            const auto &section = section_header[i];

            if (section.SizeOfRawData > 0 && section.PointerToRawData > 0) {
              const uint8_t *source = base_address + section.VirtualAddress;
              const uint64_t dest_offset = section.PointerToRawData;
              const uint64_t size =
                  (std::min)(section.SizeOfRawData, section.Misc.VirtualSize);

              if (dest_offset + size > pe_buffer.size()) {
                pe_buffer.resize(dest_offset + size);
              }
              memcpy(&pe_buffer[dest_offset], source, size);
            }
          }

          return true;
        }
      }
    }

    return false;
  } catch (...) {
    return false;
  }
}

bool dump_loaded_pe(const std::filesystem::path &output_path,
                    const std::vector<uint8_t> &pe_buffer) {
  try {
    std::ofstream output_file(output_path, std::ios::binary);
    if (output_file &&
        loaded_pe(const_cast<std::vector<uint8_t> &>(pe_buffer))) {
      output_file.write(reinterpret_cast<const char *>(pe_buffer.data()),
                        pe_buffer.size());
      return output_file.good();
    }

    return false;
  } catch (...) {
    return false;
  }
}
} // namespace utils::pe