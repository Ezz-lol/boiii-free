#include <std_include.hpp>
#include "gdb.hpp"

#include <game/impl/ugc/ugc.hpp>
#include <unzip.h> // minizip

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

  bool IsOpen() const { return m_handle != nullptr; }
  unzFile GetHandle() const { return m_handle; }

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
static void LoadGDBDataForDebugInfo(objFileInfo_t *const info) {
  debugFileInfo_t *const debug = &info->debugInfo;
  GSC_GDB *header = debug->gdb;

  if (!memcmp(&debug->gdb->magic, &GSC_GDB::T7_MAGIC,
              sizeof(debug->gdb->magic))) {

    // Convert relative line‑info offsets to absolute addresses.
    // The line‑info table resides at header->lineinfo_offset and must be
    // 8‑byte aligned. Each entry is a 32‑bit offset that becomes an
    // absolute pointer into the gdbBuffer.
    uint64_t *lineInfo = align(header->lineinfo(), sizeof(uint64_t));

    const uint32_t lineinfoCount = header->lineinfo_count;

    for (uint32_t i = 0; i < lineinfoCount; ++i) {
      lineInfo[i] += reinterpret_cast<uint64_t>(info->activeVersion);
    }

    // Store the converted line table and its entry count.
    debug->lineStartAddr =
        std::bit_cast<uint8_t **>(lineInfo); // uint64_t* array
    debug->lineStartAddrCount = static_cast<int32_t>(lineinfoCount);

    // Canonicalise every string in the string table (required for later
    // lookups).
    const char *table = header->stringtable();

    for (uint32_t i = 0; i < header->stringtable_count; ++i) {
      sl::SL_GenerateCanonicalString(table);
      table += strlen(table) + 1;
    }
  } else {
    // Invalid magic – free the buffer and clear the pointer.
    free(debug->gdb);
    debug->gdb = nullptr;
  }
}

/// @brief  Load GSC debug data (GDB) for all known script objects.
///
/// First reads GDB files from the global scriptgdb.zip archive, then attempts
/// to load per‑object GDB files from User‑Generated Content (UGC) zones if a
/// UGC mod is active. The resulting line‑to‑address mappings and string tables
/// are attached to the global objFileInfo records.
void LoadScriptGDB2_Impl(const scriptInstance_t inst) {
  // 1. Load GDBs from the global script debug ZIP archive
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
      snprintf(gdbFileName, std::size(gdbFileName), "%s.gdb",
               info->debugInfo.filename);

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
        unz_file_info fileInfo;
        unzGetCurrentFileInfo(zipFile.GetHandle(), &fileInfo, nullptr, 0,
                              nullptr, 0, nullptr, 0);
        const uint32_t fileSize = fileInfo.uncompressed_size;

        // Allocate memory with malloc – freed later via free()
        void *const rawGdb = malloc(fileSize);
        if (rawGdb != nullptr) {
          // Read the entire file into the buffer
          if (unzOpenCurrentFile(zipFile.GetHandle()) == UNZ_OK) {
            unzReadCurrentFile(zipFile.GetHandle(), rawGdb, fileSize);
            unzCloseCurrentFile(zipFile.GetHandle());
          }

          info->debugInfo.gdb = static_cast<GSC_GDB *>(rawGdb);
          LoadGDBDataForDebugInfo(info);
        }
      }
    }
  } // zipFile destroyed here

  // 2. If a UGC mod is active, also try per‑zone GDB files
  if (ugc::active_mod->publisherId[0]) {
    const uint32_t objCount = gObjFileInfoCount->instance[inst];

    for (ZoneType zoneType = ZoneType::MOD; zoneType < ZoneType::COUNT;
         ++zoneType) {

      for (uint32_t idx = 0; idx < objCount; ++idx) {
        objFileInfo_t *const info = &gObjFileInfo->instance[inst][idx];

        // Build the virtual path: "all/scriptgdb/<filename>"
        scr_path_t relPath;
        snprintf(relPath, std::size(relPath), "all/scriptgdb/%s",
                 info->debugInfo.filename);

        scr_path_t fullPath;
        const char *const pubId = ugc::active_mod->publisherId;

        // Resolve the virtual path into a concrete filesystem location
        if (ugc::UGC_ZoneSourcePath_Impl(relPath, ".gdb", std::size(fullPath),
                                         fullPath, zoneType, pubId)) {
          // Open the file using a modern C++ stream
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
              LoadGDBDataForDebugInfo(info);
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
                        objFileInfo_t *const fileInfo) {

  // 1. Try to load from the global script debug ZIP archive.
  scr_path_t zipPath;
  fs::FS_JoinPath(zipPath, std::size(zipPath), sys::Sys_GetAbsZoneDir(),
                  "scriptgdb.zip");

  ZipFile zipFile(zipPath);
  if (zipFile.IsOpen()) {
    r::StreamGuard streamGuard;

    // Build the expected filename inside the ZIP: "<script>.gdb"
    scr_path_t gdbFileName;
    snprintf(gdbFileName, std::size(gdbFileName), "%s.gdb",
             fileInfo->debugInfo.filename);

    // Normalise backslashes to forward slashes.
    for (char *p = gdbFileName; *p != '\0'; ++p) {
      if (*p == '\\') {
        *p = '/';
      }
    }

    if (unzLocateFile(zipFile.GetHandle(), gdbFileName, 0) == UNZ_OK) {
      unz_file_info fileInfoZ;
      unzGetCurrentFileInfo(zipFile.GetHandle(), &fileInfoZ, nullptr, 0,
                            nullptr, 0, nullptr, 0);
      const uint32_t fileSize = fileInfoZ.uncompressed_size;

      void *const rawGdb = malloc(fileSize);
      if (rawGdb != nullptr) {
        if (unzOpenCurrentFile(zipFile.GetHandle()) == UNZ_OK) {
          unzReadCurrentFile(zipFile.GetHandle(), rawGdb, fileSize);
          unzCloseCurrentFile(zipFile.GetHandle());
        }

        fileInfo->debugInfo.gdb = static_cast<GSC_GDB *>(rawGdb);
        LoadGDBDataForDebugInfo(fileInfo);
      }
    }
  }

  // 2. If a UGC mod is active, attempt to load from per‑zone GDB files.
  if (ugc::active_mod->publisherId[0]) {

    for (ZoneType zoneType = ZoneType::MOD; zoneType < ZoneType::COUNT;
         ++zoneType) {

      // Build the virtual path: "all/scriptgdb/<filename>"
      scr_path_t relPath;
      snprintf(relPath, std::size(relPath), "all/scriptgdb/%s",
               fileInfo->debugInfo.filename);

      scr_path_t fullPath;
      const char *const pubId = ugc::active_mod->publisherId;

      if (ugc::UGC_ZoneSourcePath_Impl(relPath, ".gdb", std::size(fullPath),
                                       fullPath, zoneType, pubId)) {
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
            fileInfo->debugInfo.gdb = reinterpret_cast<GSC_GDB *>(buffer);
            LoadGDBDataForDebugInfo(fileInfo);
          }
        }
      }
    }
  }
}

/// @brief  Locate the objFileInfo that contains the given code address.
///
/// Iterates over all active versions of script objects for the specified
/// instance and returns the first whose code segment encompasses 'addr'.
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
/// Uses the GDB debug data (loaded on demand) to map the address to a line
/// in the original script file. The GDB buffer is freed immediately after
/// the lookup to conserve memory.
void Scr_GetFileAndLineNum_Impl(const scriptInstance_t inst, uint8_t *const pos,
                                const char **const filename,
                                int32_t *const lineNum,
                                const char **const sourceLine) {
  if (filename != nullptr) {
    *filename = "<ERROR>";
  }
  if (lineNum != nullptr) {
    *lineNum = -1;
  }
  if (sourceLine != nullptr) {
    *sourceLine = "";
  }

  // 2. Find the owning script object.
  objFileInfo_t *fileInfo = Scr_FindObjFileInfo_Impl(inst, pos);
  if (fileInfo != nullptr) {
    // 3. Always provide the filename if it exists.
    if (filename != nullptr) {
      *filename = fileInfo->debugInfo.filename;
    }

    if (lineNum != nullptr &&
        (game::is_client() || sv_detailedScriptErrors->get_bool())) {
      LoadScriptGDB_Impl(inst, fileInfo);

      const int32_t lineAddrCount = fileInfo->debugInfo.lineStartAddrCount;
      if (lineAddrCount > 0) {
        // The line table contains absolute addresses of the start of each line.
        uint64_t *const lineTable = reinterpret_cast<uint64_t *const>(
            fileInfo->debugInfo.lineStartAddr);
        int32_t lineIdx = 0;

        // Find the last line whose starting address is <= 'pos'.
        while (lineIdx < lineAddrCount &&
               pos > reinterpret_cast<uint8_t *>(lineTable[lineIdx])) {
          ++lineIdx;
        }

        // If we advanced at least once, the previous entry is the desired line.
        *lineNum = (lineIdx > 0) ? (lineIdx - 1) : -1;
      } else {
        *lineNum = -1;
      }

      // 5. If a valid line number was found and a source line is requested,
      //    extract the corresponding text from the source buffer.
      if (sourceLine != nullptr && *lineNum >= 0 &&
          fileInfo->debugInfo.source != nullptr) {
        char *src = fileInfo->debugInfo.source;
        const int32_t srcLen = fileInfo->debugInfo.sourceLen;
        int32_t currentLine = 0;
        int32_t lineStartOff = 0;
        int32_t offset = 0;

        // Lines are separated by null characters in the source buffer.
        while (offset < srcLen && currentLine < *lineNum) {
          if (src[offset] == '\0') {
            ++currentLine;
          }
          ++offset;
          if (currentLine == *lineNum) {
            lineStartOff = offset;
          }
        }

        *sourceLine = &src[lineStartOff];
      }

      // 6. Release the GDB buffer immediately – it is only needed for the
      // lookup.
      if (fileInfo->debugInfo.gdb != nullptr) {
        free(fileInfo->debugInfo.gdb);
        fileInfo->debugInfo.gdb = nullptr;
      }
    }
  }
}

void ReportObjLinkError_Impl(scriptInstance_t inst, GSC_OBJ *prime_obj,
                             objFileInfo_t *fileInfo, GSC_IMPORT_ITEM *import,
                             char *errorString, int errorStringLength) {
  if (fileInfo) {
    LoadScriptGDB(inst, fileInfo);

    std::string linesBuffer;

    // The addresses are stored contiguously in memory immediately following the
    // GSC_IMPORT_ITEM.
    std::span<const uint32_t> addressOffsets = import->addresses();

    // Calculate line numbers for each address
    for (size_t i = 0; i < addressOffsets.size(); ++i) {
      const int32_t lineAddrCount = fileInfo->debugInfo.lineStartAddrCount;
      int32_t lineIdx = 0;
      if (lineAddrCount > 0) {
        // The line table contains absolute addresses of the start of each line.
        uint64_t *const lineTable = reinterpret_cast<uint64_t *const>(
            fileInfo->debugInfo.lineStartAddr);

        // Find the last line whose starting address is <= 'pos'.
        while (lineIdx<lineAddrCount &&reinterpret_cast<uint8_t *>(
                   static_cast<uint64_t>(
                       addressOffsets[i]))> reinterpret_cast<uint8_t
                                                                 *>(
            lineTable[lineIdx])) {
          ++lineIdx;
        }

        // If we advanced at least once, the previous entry is the desired line.
        lineIdx = (lineIdx > 0) ? (lineIdx - 1) : -1;
      } else {
        lineIdx = -1;
      }

      linesBuffer += std::to_string(lineIdx);
    }

    // Free GDB debug context if it exists
    if (fileInfo->debugInfo.gdb != nullptr) {
      free(fileInfo->debugInfo.gdb);
      fileInfo->debugInfo.gdb = nullptr;
    }

    // Lookup function name in the global linked-list hashmap
    std::string functionName;
    const char *lookupResult = sl::SL_LookupCanonicalString(import->name);
    if (lookupResult && lookupResult[0]) {
      functionName = lookupResult;
    } else {
      // Fallback: format name ID as Hex if string hash isn't found
      functionName = std::format("{:X}", import->name);
    }

    // Format the final error message (replaces 'va' and 'Com_sprintf')
    std::string errorMessage =
        std::format(" \"{}\" with {} parameters in \"{}\" at {} {} ****\n",
                    functionName, import->param_count, prime_obj->get_name(),
                    (import->num_address > 1) ? "lines" : "line", linesBuffer);

    // Append to the provided C-style buffer safely (replaces 'I_strcat')
    if (errorString != nullptr && errorStringLength > 0) {
      size_t currentLen = std::strlen(errorString);
      size_t spaceLeft =
          static_cast<size_t>(errorStringLength) - currentLen - 1;
      std::strncat(errorString, errorMessage.c_str(), spaceLeft);
    }

    // Print to the engine console
    com::Com_Printf(8, consoleLabel_e::CHANNEL_ERROR, "%s",
                    errorMessage.c_str());
  }
}
} // namespace scr
} // namespace game