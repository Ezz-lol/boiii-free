#include <std_include.hpp>

#include "dump.hpp"

namespace dump {
using namespace game;
using namespace game::db::xasset;

AssetBytes asset_bytes(const XAssetType type, const char *name,
                       XAssetHeader header) {
  AssetBytes result = {.buffers = {}, .name = name, .size = 0};
  switch (type) {
  case XAssetType::RAWFILE: {
    if (readable_ptr(header.rawfile) && readable_ptr(header.rawfile->name) &&
        header.rawfile->name[0] && readable_ptr(header.rawfile->buffer)) {
      result.name = header.rawfile->name;
      result.size = header.rawfile->len;
      result.buffers.push_back({header.rawfile->buffer, header.rawfile->len});
    }
    break;
  }
  case XAssetType::SCRIPTPARSETREE:
    if (readable_ptr(header.scriptParseTree) &&
        readable_ptr(header.scriptParseTree->name) &&
        header.scriptParseTree->name[0] &&
        readable_ptr(header.scriptParseTree->buffer)) {
      result.name = header.scriptParseTree->name;
      result.size = header.scriptParseTree->len;
      result.buffers.push_back(
          {reinterpret_cast<uint8_t *>(header.scriptParseTree->buffer),
           static_cast<size_t>(header.scriptParseTree->len)});
    }
    break;
  case XAssetType::LOCALIZE_ENTRY:
    if (readable_ptr(header.localize) && readable_ptr(header.localize->name) &&
        readable_ptr(header.localize->value)) {
      result.name = std::string("localization/") + header.localize->name;
      size_t size = std::strlen(header.localize->value);
      result.buffers.push_back(
          {reinterpret_cast<const uint8_t *>(header.localize->value), size});
    }
    break;
  // TODO
  default:
    break;
  }
  return result;
}

void dump_requested_assets(const XAssetType type, const char *name,
                           XAssetHeader header) {

  if (extract_assets() && readable_ptr(name) && name[0] &&
      readable_ptr(header.named) && readable_ptr(header.named->name) &&
      header.named->name[0] && std::regex_match(name, extract_pattern())) {
    const std::filesystem::path output_tree_root = asset_output();
    if (!std::filesystem::exists(output_tree_root)) {
      std::filesystem::create_directories(output_tree_root);
    }
    const AssetBytes asset = asset_bytes(type, name, header);
    if (asset.buffers.size() > 0) {
      const std::filesystem::path output_path = output_tree_root / asset.name;

      // Ensure parent path exists
      if (!std::filesystem::exists(output_path.parent_path())) {
        std::filesystem::create_directories(output_path.parent_path());
      }
      if (std::filesystem::exists(output_path)) {
        std::filesystem::remove(output_path);
      }

      bool successful = true;
      for (const AssetBuffer &buffer : asset.buffers) {

        if (!utils::io::write_file_bytes(
                output_path.string(),
                reinterpret_cast<const uint8_t *>(buffer.buffer), buffer.size,
                true)) {

          successful = false;
          break;
        }
      }

      if (successful) {
        fprintf(stdout, "[DUMP] \"%s\" (%llu bytes) -> \"%s\"\n ",
                asset.name.c_str(), asset.size,
                output_path.generic_string().c_str());
        fflush(stdout);
      } else {
        fprintf(stdout, "[DUMP] failed for \"%s\" (%llu bytes)\n",
                asset.name.c_str(), asset.size);
        fflush(stdout);
        if (std::filesystem::exists(output_path)) {
          std::filesystem::remove(output_path);
        }
      }
    }
  }
}
} // namespace dump