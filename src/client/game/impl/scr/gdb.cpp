#include <std_include.hpp>

#include "gdb.hpp"

#include <game/impl/ugc/ugc.hpp>
#include <unzip.h>

namespace game {
namespace scr {
namespace {
// RAII wrapper around a minizip unzFile handle.
class ZipFile {
public:
  explicit ZipFile(const char *path) : m_handle(unzOpen(path)) {}
  ~ZipFile() {
    if (m_handle != nullptr) {
      unzClose(m_handle);
    }
  }

  // Non-copyable, non-movable
  ZipFile(const ZipFile &) = delete;
  ZipFile &operator=(const ZipFile &) = delete;

  [[nodiscard]] bool IsOpen() const { return m_handle != nullptr; }
  [[nodiscard]] unzFile GetHandle() const { return m_handle; }

private:
  unzFile m_handle;
};
} // namespace

/// @brief  Process a raw GSC_GDB buffer and populate the debug file info.
///
/// Validates the magic number, converts relative line‑info offsets to absolute
/// addresses, stores the line table and its size, and canonicalises the string
/// table entries. If the buffer is invalid the buffer is freed and the gdb
/// pointer is cleared.
static void LoadGDBDataForDebugInfo(objFileInfo_t *const info,
                                    const size_t gdbSize) {
  debugFileInfo_t *const debug = &info->debugInfo;
  GSC_GDB *header = debug->gdb;

  if (gdbSize >= sizeof(GSC_GDB) && header->hasMagic(GSC_GDB::T7_MAGIC) &&
      header->lineinfo_offset <= gdbSize &&
      header->lineinfo_count <=
          (gdbSize - header->lineinfo_offset) / sizeof(uintptr_t) &&
      header->stringtable_offset <= gdbSize) {
    const char *table = header->stringtable();
    const char *const end = reinterpret_cast<const char *>(header) + gdbSize;
    std::vector<const char *> strings;
    strings.reserve(header->stringtable_count);

    for (uint32_t i = 0; i < header->stringtable_count; ++i) {
      if (table >= end) {
        strings.clear();
        break;
      }

      const void *terminator = memchr(table, '\0', end - table);
      if (terminator == nullptr) {
        strings.clear();
        break;
      }

      strings.push_back(table);
      table = static_cast<const char *>(terminator) + 1;
    }

    if (strings.size() != header->stringtable_count) {
      std::println(std::cerr, "Invalid GDB string table for script '{}'",
                   debug->filename == nullptr ? "<unknown>" : debug->filename);
      debug->free_gdb();
    } else {

      /*
         Convert relative line‑info offsets to absolute addresses.
         The line‑info table resides at header->lineinfo_offset and must be
         8‑byte aligned. Each entry is a 32‑bit offset that becomes an
         absolute pointer into the gdbBuffer.
      */
      uintptr_t *lineInfo = align(header->lineinfo().data(), sizeof(uintptr_t));

      const uint32_t lineinfoCount = header->lineinfo_count;

      for (uint32_t i = 0; i < lineinfoCount; ++i) {
        lineInfo[i] += reinterpret_cast<uintptr_t>(info->activeVersion);
      }

      // Store the converted line table and its entry count.
      debug->lineStartAddr =
          std::bit_cast<uint8_t **>(lineInfo); // uintptr_t* array
      debug->lineStartAddrCount = static_cast<int32_t>(lineinfoCount);

      // Canonicalise every string in the string table (required for later
      // lookups).
      for (const char *name : strings) {
        const ScrVarCanonicalName_t hash = builtin::fnv1a(name);
        const char *existing = sl::SL_LookupCanonicalString(hash);
        if (existing != nullptr && strcmp(existing, name) != 0) {
          std::println(
              std::cerr,
              "Canonical name hash collision in script '{}': '{}' and '{}'",
              debug->filename == nullptr ? "<unknown>" : debug->filename,
              existing, name);
          std::cerr.flush();
        }
        sl::SL_GenerateCanonicalString(name);
      }
    }
  } else {
    // Invalid magic – free the buffer and clear the pointer.
    debug->free_gdb();
  }
}

/// @brief  Load GSC debug data (GDB) for all known script objects.
///
/// First reads GDB files from the global scriptgdb.zip archive, then attempts
/// to load per‑object GDB files from User‑Generated Content (UGC) zones if a
/// UGC mod is active. The resulting line‑to‑address mappings and string tables
/// are attached to the global objFileInfo records.
void LoadScriptGDB2_Impl(const scriptInstance_t inst) {
  // Load GDBs from the global script debug ZIP archive
  scr_path_t zipPath;
  fs::FS_JoinPath(zipPath, std::size(zipPath), sys::Sys_GetAbsZoneDir(),
                  "scriptgdb.zip");

  ZipFile zipFile(zipPath);
  if (zipFile.IsOpen()) {
    // Disable streaming while we hold the ZIP open and modify global data.
    r::StreamGuard streamGuard;

    const uint32_t objCount = gObjFileInfoCount->instance[inst];

    for (uint32_t idx = 0; idx < objCount; ++idx) {
      objFileInfo_t *const info = &gObjFileInfo->instance[inst][idx];

      // Build the expected filename inside the ZIP: "<original>.gdb"
      scr_path_t gdbFileName;
      snformatf(gdbFileName, "{}.gdb", info->debugInfo.filename);

      // Normalise backslashes to forward slashes (required by ZIP loader)
      for (char *p = gdbFileName; *p != '\0'; ++p) {
        if (*p == '\\') {
          *p = '/';
        }
      }

      // Locate the file inside the archive
      const int32_t locateResult =
          unzLocateFile(zipFile.GetHandle(), gdbFileName, 0);
      if (locateResult == UNZ_OK) {
        // Retrieve the uncompressed size
        unz_file_info file_info;
        unzGetCurrentFileInfo(zipFile.GetHandle(), &file_info, nullptr, 0,
                              nullptr, 0, nullptr, 0);
        const uint32_t fileSize = file_info.uncompressed_size;

        // Allocate memory with malloc – freed later via free()
        void *const rawGdb = malloc(fileSize);
        if (rawGdb != nullptr) {
          // Read the entire file into the buffer
          if (unzOpenCurrentFile(zipFile.GetHandle()) == UNZ_OK) {
            unzReadCurrentFile(zipFile.GetHandle(), rawGdb, fileSize);
            unzCloseCurrentFile(zipFile.GetHandle());
          }

          info->debugInfo.gdb = static_cast<GSC_GDB *>(rawGdb);
          LoadGDBDataForDebugInfo(info, fileSize);
        }
      }
    }
  } // zipFile destroyed here

  // If a mod is active, also try to load from per‑zone GDB files
  if (ugc::active_mod->publisherId[0] != NULL) {
    const uint32_t objCount = gObjFileInfoCount->instance[inst];

    for (ZoneType zoneType = ZoneType::MOD; zoneType < ZoneType::COUNT;
         ++zoneType) {

      for (uint32_t idx = 0; idx < objCount; ++idx) {
        objFileInfo_t *const info = &gObjFileInfo->instance[inst][idx];

        // Build the virtual path: "all/scriptgdb/<filename>"
        scr_path_t relPath;
        snformatf(relPath, "all/scriptgdb/{}", info->debugInfo.filename);

        scr_path_t fullPath;
        const char *const pubId = ugc::active_mod->publisherId;

        // Resolve the virtual path into a concrete filesystem location
        if (ugc::UGC_ZoneSourcePath_Impl(relPath, ".gdb", std::size(fullPath),
                                         fullPath, zoneType, pubId) != 0) {
          std::ifstream file(fullPath, std::ios::binary);
          if (file.is_open()) {
            file.seekg(0, std::ios::end);
            const std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            // Allocate memory with malloc – freed later via free()
            uint8_t *const buffer = static_cast<uint8_t *>(
                malloc(static_cast<size_t>(fileSize + 1)));
            if (buffer != nullptr) {
              file.read(reinterpret_cast<char *>(buffer), fileSize);
              buffer[fileSize] = '\0';
              info->debugInfo.gdb = reinterpret_cast<GSC_GDB *>(buffer);
              LoadGDBDataForDebugInfo(info, static_cast<size_t>(fileSize));
            }
            // file closed automatically by ifstream destructor
          }
        }
      }
    }
  }
}

/// @brief  Loads GSC debug data (GDB) for a single script object.
///
/// Attempts to read the per‑file GDB first from the global scriptgdb.zip,
/// then from user‑generated content zones if a mod is active.
void LoadScriptGDB_Impl([[maybe_unused]] const scriptInstance_t inst,
                        objFileInfo_t *const file_info) {

  // Try to load from the global script debug ZIP archive.
  scr_path_t zipPath;
  fs::FS_JoinPath(zipPath, std::size(zipPath), sys::Sys_GetAbsZoneDir(),
                  "scriptgdb.zip");

  ZipFile zipFile(zipPath);
  if (zipFile.IsOpen()) {
    r::StreamGuard streamGuard;

    // Build the expected filename inside the ZIP: "<script>.gdb"
    scr_path_t gdbFileName;
    snformatf(gdbFileName, "{}.gdb", file_info->debugInfo.filename);

    // Normalise backslashes to forward slashes.
    for (char *p = gdbFileName; *p != '\0'; ++p) {
      if (*p == '\\') {
        *p = '/';
      }
    }

    if (unzLocateFile(zipFile.GetHandle(), gdbFileName, 0) == UNZ_OK) {
      unz_file_info file_infoZ;
      unzGetCurrentFileInfo(zipFile.GetHandle(), &file_infoZ, nullptr, 0,
                            nullptr, 0, nullptr, 0);
      const uint32_t fileSize = file_infoZ.uncompressed_size;

      void *const rawGdb = malloc(fileSize);
      if (rawGdb != nullptr) {
        if (unzOpenCurrentFile(zipFile.GetHandle()) == UNZ_OK) {
          unzReadCurrentFile(zipFile.GetHandle(), rawGdb, fileSize);
          unzCloseCurrentFile(zipFile.GetHandle());
        }

        file_info->debugInfo.gdb = static_cast<GSC_GDB *>(rawGdb);
        LoadGDBDataForDebugInfo(file_info, fileSize);
      }
    }
  }

  // If a UGC mod is active, attempt to load from per‑zone GDB files.
  if (ugc::active_mod->publisherId[0] != NULL) {

    for (ZoneType zoneType = ZoneType::MOD; zoneType < ZoneType::COUNT;
         ++zoneType) {

      // Build the virtual path: "all/scriptgdb/<filename>"
      scr_path_t relPath;
      snformatf(relPath, "all/scriptgdb/{}", file_info->debugInfo.filename);

      scr_path_t fullPath;
      const char *const pubId = ugc::active_mod->publisherId;

      if (ugc::UGC_ZoneSourcePath_Impl(relPath, ".gdb", std::size(fullPath),
                                       fullPath, zoneType, pubId) != 0) {
        std::ifstream file(fullPath, std::ios::binary);
        if (file.is_open()) {
          file.seekg(0, std::ios::end);
          const std::streamsize fileSize = file.tellg();
          file.seekg(0, std::ios::beg);

          uint8_t *const buffer =
              static_cast<uint8_t *>(malloc(static_cast<size_t>(fileSize + 1)));
          if (buffer != nullptr) {
            file.read(reinterpret_cast<char *>(buffer), fileSize);
            buffer[fileSize] = '\0';
            file_info->debugInfo.gdb = reinterpret_cast<GSC_GDB *>(buffer);
            LoadGDBDataForDebugInfo(file_info, static_cast<size_t>(fileSize));
          }
        }
      }
    }
  }
}

/// @brief  Locate the objFileInfo that contains the given bytecode address.
///
/// Iterates over all active script objects for the specified instance and
/// returns the first with bytecode containing the given `addr`ess.
/// Returns nullptr if no match is found.
objFileInfo_t *Scr_FindObjFileInfo_Impl(const scriptInstance_t inst,
                                        void *const addr) {
  for (uint32_t i = 0; i < gObjFileInfoCount->instance[inst]; ++i) {
    objFileInfo_t *const info = &gObjFileInfo->instance[inst][i];
    const GSC_OBJ *const obj = info->activeVersion;
    if (obj != nullptr && contains(obj->cseg(), addr)) {
      return info;
    }
  }

  return nullptr;
}

/// @brief  For a given code address, retrieve source filename, line number,
///         and the text of that source line.
///
/// Attempts to load the GDB debug data for the given bytecode position. If
/// successful, maps the address to a line in the original script file.
void Scr_GetFileAndLineNum_Impl(const scriptInstance_t inst, uint8_t *const pos,
                                const char **const filename,
                                int32_t *const line_num,
                                const char **const source_line) {
  if (filename != nullptr) {
    *filename = "<ERROR>";
  }
  if (line_num != nullptr) {
    *line_num = -1;
  }
  if (source_line != nullptr) {
    *source_line = "";
  }

  // Find the script object containing the given bytecode pointer.
  objFileInfo_t *file_info = Scr_FindObjFileInfo_Impl(inst, pos);
  if (file_info != nullptr) {
    // Always provide the filename if it exists.
    if (filename != nullptr) {
      *filename = file_info->debugInfo.filename;
    }

    if (line_num != nullptr &&
        (game::is_client() || sv_detailedScriptErrors->get_bool())) {
      LoadScriptGDB_Impl(inst, file_info);

      const int32_t line_addr_count = file_info->debugInfo.lineStartAddrCount;
      if (line_addr_count > 0) {
        // The line table contains absolute addresses of the start of each line.
        uintptr_t *const lineTable = reinterpret_cast<uintptr_t *const>(
            file_info->debugInfo.lineStartAddr);
        int32_t line_idx = 0;

        // Find the last line with start address <= 'pos'.
        while (line_idx < line_addr_count &&
               pos > reinterpret_cast<uint8_t *>(lineTable[line_idx])) {
          ++line_idx;
        }

        // If we advanced at least once, the previous entry is the desired line.
        *line_num = (line_idx > 0) ? (line_idx - 1) : -1;
      } else {
        *line_num = -1;
      }

      // If a valid line number was found and a source line is requested,
      // extract the corresponding text from the source buffer.
      if (source_line != nullptr && *line_num >= 0 &&
          file_info->debugInfo.source != nullptr) {
        char *src = file_info->debugInfo.source;
        const int32_t src_len = file_info->debugInfo.sourceLen;
        int32_t current_line = 0;
        int32_t line_start_off = 0;
        int32_t offset = 0;

        // Lines are separated by null characters in the source buffer.
        while (offset < src_len && current_line < *line_num) {
          if (src[offset] == '\0') {
            ++current_line;
          }
          ++offset;
          if (current_line == *line_num) {
            line_start_off = offset;
          }
        }

        *source_line = &src[line_start_off];
      }

      // Release the GDB buffer immediately – it is only needed for the
      // lookup.
      file_info->debugInfo.free_gdb();
    }
  }
}

std::vector<int32_t> Scr_GetImportLineNumbers(const scriptInstance_t inst,
                                              const GSC_OBJ *const obj,
                                              objFileInfo_t *const file_info,
                                              const GSC_IMPORT_ITEM *import) {
  std::vector<int32_t> lines;
  if (file_info == nullptr || obj == nullptr || import == nullptr) {
    return lines;
  }

  LoadScriptGDB(inst, file_info);

  const int32_t line_addr_count = file_info->debugInfo.lineStartAddrCount;
  const uintptr_t *const lineTable =
      reinterpret_cast<const uintptr_t *>(file_info->debugInfo.lineStartAddr);

  if (line_addr_count > 0 && lineTable != nullptr) {
    for (const uint32_t offset : import->addresses()) {
      const uintptr_t pos = reinterpret_cast<uintptr_t>(obj) + offset;
      int32_t line_idx = 0;
      for (; line_idx < line_addr_count && pos > lineTable[line_idx];
           ++line_idx) {
      }
      if (line_idx > 0 && std::ranges::find(lines, line_idx) == lines.end()) {
        lines.push_back(line_idx);
      }
    }
  }

  file_info->debugInfo.free_gdb();

  return lines;
}

void ReportObjLinkError_Impl(scriptInstance_t inst, GSC_OBJ *prime_obj,
                             objFileInfo_t *file_info, GSC_IMPORT_ITEM *import,
                             char *errorString, int errorStringLength) {
  if (file_info != nullptr && errorString != nullptr && errorStringLength > 0) {
    const std::vector<int32_t> lines =
        Scr_GetImportLineNumbers(inst, prime_obj, file_info, import);

    std::string lines_buffer;
    if (lines.empty()) {
      lines_buffer = "?";
    } else {
      lines_buffer = resolve_string(lines[0]);
      for (size_t line_idx = 1; line_idx < lines.size(); ++line_idx) {
        lines_buffer += std::format(", {}", lines[line_idx]);
      }
    }

    const char *lookupResult = sl::SL_LookupCanonicalString(import->name);
    const std::string functionName =
        lookupResult == nullptr || lookupResult[0] == NULL
            ? std::format("{:X}", import->name)
            : lookupResult;

    const std::string errorMessage =
        std::format(" \"{}\" with {} parameters in \"{}\" at {} {} ****\n",
                    functionName, import->param_count, prime_obj->get_name(),
                    lines.size() > 1 ? "lines" : "line", lines_buffer);

    const size_t currentLen = std::strlen(errorString);
    if (currentLen + 1 < static_cast<size_t>(errorStringLength)) {
      std::strncat(errorString, errorMessage.c_str(),
                   static_cast<size_t>(errorStringLength) - currentLen - 1);
    }
  }
}
} // namespace scr
} // namespace game
