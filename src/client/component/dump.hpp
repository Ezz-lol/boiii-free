#pragma once

#include <game/game.hpp>
#include <utility>
#include <utils/io.hpp>

namespace dump {
using namespace game;
using namespace game::db::xasset;

enum class AssetBufferType : uint8_t { Pointer = 0x0, Serialized = 0x1 };

struct AssetBuffer {
  AssetBufferType type;
  union {
    struct {
      const uint8_t *buffer;
      size_t size;
    };
    std::string serialized;
  };

  inline constexpr AssetBuffer(const uint8_t *buff,
                               const size_t buf_size) noexcept
      : type(AssetBufferType::Pointer), buffer(buff), size(buf_size) {}

  inline AssetBuffer(std::string ser)
      : type(AssetBufferType::Serialized), serialized(std::move(ser)) {}

  ~AssetBuffer() { destroy(); }

  AssetBuffer(const AssetBuffer &other) : type(other.type) {
    if (type == AssetBufferType::Pointer) {
      buffer = other.buffer;
      size = other.size;
    } else {
      ::new (static_cast<void *>(&serialized)) std::string(other.serialized);
    }
  }

  AssetBuffer(AssetBuffer &&other) noexcept : type(other.type) {
    if (type == AssetBufferType::Pointer) {
      buffer = other.buffer;
      size = other.size;
    } else {
      ::new (static_cast<void *>(&serialized))
          std::string(std::move(other.serialized));
      other.type = AssetBufferType::Pointer;
    }
  }

  AssetBuffer &operator=(const AssetBuffer &other) {
    if (this != &other) {
      destroy();
      type = other.type;
      if (type == AssetBufferType::Pointer) {
        buffer = other.buffer;
        size = other.size;
      } else {
        ::new (static_cast<void *>(&serialized)) std::string(other.serialized);
      }
    }
    return *this;
  }

  AssetBuffer &operator=(AssetBuffer &&other) noexcept {
    if (this != &other) {
      destroy();
      type = other.type;
      if (type == AssetBufferType::Pointer) {
        buffer = other.buffer;
        size = other.size;
      } else {
        ::new (static_cast<void *>(&serialized))
            std::string(std::move(other.serialized));
        other.type = AssetBufferType::Pointer;
      }
    }
    return *this;
  }

  [[nodiscard]] std::span<const uint8_t> span() const noexcept {
    switch (type) {
    case AssetBufferType::Pointer:
      return {buffer, size};
    case AssetBufferType::Serialized:
      return {reinterpret_cast<const uint8_t *>(serialized.data()),
              serialized.size()};
    }
    return {};
  }

private:
  void destroy() noexcept {
    if (type == AssetBufferType::Serialized) {
      serialized.~basic_string();
    }
  }
};

struct AssetBytes {
  std::vector<AssetBuffer> buffers;
  std::string name;
  size_t size; // total
};

AssetBytes asset_bytes(const XAssetType type, const char *name,
                       XAssetHeader header);

void dump_requested_assets(const XAssetType type, const char *name,
                           XAssetHeader header);

} // namespace dump
