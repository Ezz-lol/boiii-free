#include <std_include.hpp>

#include <functional>

#include "dump.hpp"

#include "utils/cryptography.hpp"

#include "component/exception.hpp"

struct DumpEntry {
  game::db::xasset::XAssetType type;
  std::string name;
  std::string hash;

  bool operator==(const DumpEntry &other) const {
    return type == other.type && name == other.name && hash == other.hash;
  }

  constexpr DumpEntry(game::db::xasset::XAssetType asset_type, std::string n,
                      std::string h)
      : type(asset_type), name(std::move(n)), hash(std::move(h)) {}
};

namespace std {
template <> struct hash<DumpEntry> {
  std::size_t operator()(const DumpEntry &p) const noexcept {
    // Compute individual hashes
    std::size_t h1 = std::hash<int32_t>{}(static_cast<int32_t>(p.type));
    std::size_t h2 = std::hash<std::string>{}(p.name);
    std::size_t h3 = std::hash<std::string>{}(p.hash);

    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};
} // namespace std

namespace dump {
using namespace game;
using namespace game::db::xasset;

AssetBytes asset_bytes(const XAssetType type, const char *name,
                       XAssetHeader header) {
  AssetBytes result = {.buffers = {}, .name = name, .size = 0};
  switch (type) {
  case XAssetType::RAWFILE: {
    if (game::readable_ptr(header.rawfile->buffer)) {
      result.name = header.rawfile->name;
      result.size = header.rawfile->len;
      result.buffers.emplace_back(header.rawfile->buffer, header.rawfile->len);
    }
    break;
  }
  case XAssetType::SCRIPTPARSETREE:
    if (game::readable_ptr(header.scriptParseTree->buffer)) {
      result.name = header.scriptParseTree->name;
      result.size = header.scriptParseTree->len;
      result.buffers.emplace_back(
          reinterpret_cast<uint8_t *>(header.scriptParseTree->buffer),
          header.scriptParseTree->len);
    }
    break;
  case XAssetType::LOCALIZE_ENTRY: {
    if (game::readable_ptr(header.localize->value)) {
      result.name = std::format("localization/{}", header.localize->name);
      size_t size = std::strlen(header.localize->value);
      result.size = size;
      result.buffers.emplace_back(
          reinterpret_cast<const uint8_t *>(header.localize->value), size);
      break;
    }
  }
  case XAssetType::STRINGTABLE: {
    if (game::readable_ptr(header.stringTable->values)) {
      const std::string csv = header.stringTable->to_csv();

      result.name = header.stringTable->name;
      result.size = csv.size();
      result.buffers.emplace_back(csv);
      break;
    }
  } break;
  // TODO
  default:
    break;
  }
  return result;
}

static std::unordered_set<DumpEntry> dumped;
static std::unordered_set<std::string> dumped_names;
std::recursive_mutex dump_lock;

void dump_requested_assets(const XAssetType type, const char *name,
                           XAssetHeader header) {

  exception::known_blocking_call.fetch_add(1, std::memory_order_acquire);
  if (extract_assets() && game::readable_ptr(name) && name[0] != NULL &&
      game::readable_ptr(header.named) &&
      game::readable_ptr(header.named->name) && header.named->name[0] != NULL &&
      std::regex_match(name, extract_pattern())) {
    std::scoped_lock<std::recursive_mutex> lock(dump_lock);
    AssetBytes asset = asset_bytes(type, name, header);
    if (!asset.buffers.empty()) {
      std::string hash{};
      for (const AssetBuffer &buffer : asset.buffers) {
        hash += utils::cryptography::sha256::compute(buffer.span(), true);
      }
      const DumpEntry map_entry = DumpEntry(type, name, hash);
      if (dumped.contains(map_entry)) {
        const std::string log = std::format(
            R"([DB][XAsset][Extraction] Skipping extraction of asset "{}" - asset was already extracted.)",
            map_entry.name);
        fprintf(stdout, "%s\n", log.c_str());
        fflush(stdout);
        game::trace("{}", log);
      } else {
        if (dumped_names.contains(asset.name)) {
          const std::filesystem::path asset_path = asset.name;
          const std::filesystem::path asset_ext = asset_path.extension();
          const std::filesystem::path asset_stem = asset_path.stem();
          if (asset_path.has_parent_path()) {
            const std::filesystem::path asset_parent = asset_path.parent_path();
            asset.name = std::format(
                "{}/{}-{}{}", asset_parent.generic_string(),
                asset_stem.generic_string(), hash, asset_ext.generic_string());
          } else {
            asset.name = std::format("{}-{}{}", asset_stem.generic_string(),
                                     hash, asset_ext.generic_string());
          }
        } else {
          dumped_names.insert(asset.name);
        }

        dumped.insert(map_entry);
        const std::filesystem::path output_tree_root = asset_output();
        if (!std::filesystem::exists(output_tree_root)) {
          std::filesystem::create_directories(output_tree_root);
        }
        if (!asset.buffers.empty()) {
          const std::filesystem::path output_path =
              output_tree_root / asset.name;

          // Ensure parent path exists
          if (!std::filesystem::exists(output_path.parent_path())) {
            std::filesystem::create_directories(output_path.parent_path());
          }
          if (std::filesystem::exists(output_path)) {
            std::filesystem::remove(output_path);
          }

          bool successful = true;
          for (const AssetBuffer &buffer : asset.buffers) {

            if (!utils::io::write_file_bytes(output_path.string(),
                                             buffer.span(), true)) {
              successful = false;
              break;
            }
          }

          if (successful) {
            const std::string log = std::format(
                R"([DB][XAsset][Extraction] "{}" ({} bytes) -> "{}")",
                asset.name, asset.size, output_path.generic_string());
            fprintf(stdout, "%s\n", log.c_str());
            fflush(stdout);
            game::trace("{}", log);
          } else {
            const std::string log = std::format(
                R"([DB][XAsset][Extraction] failed for "{}" ({} bytes))",
                asset.name, asset.size);
            fprintf(stdout, "%s\n", log.c_str());
            fflush(stdout);
            game::trace("{}", log);
            if (std::filesystem::exists(output_path)) {
              std::filesystem::remove(output_path);
            }
          }
        }
      }
    }
  }
  exception::known_blocking_call.fetch_sub(1, std::memory_order_acquire);
}
} // namespace dump
