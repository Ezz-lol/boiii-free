#include <std_include.hpp>
#include <loader/component_loader.hpp>

#include <game/game.hpp>
#include <game/utils.hpp>
#include "name.hpp"

#include "auth.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace colors {
namespace {
utils::hook::detour cl_get_client_name_hook;

std::string decode_backtick_colors(const std::string &text) {
  bool has_backtick = false;
  std::string decoded;
  decoded.reserve(text.size() + 2);

  for (size_t i = 0; i < text.size(); ++i) {
    if (i + 1 < text.size() && text[i] == '`' && text[i + 1] >= '0' &&
        text[i + 1] <= '9') {
      decoded += '^';
      decoded += text[i + 1];
      ++i;
      has_backtick = true;
    } else {
      decoded += text[i];
    }
  }

  if (has_backtick) {
    decoded += "^7";
  }

  return decoded;
}

std::optional<int> get_color_for_xuid(const uint64_t xuid) {
  if (xuid == 0xCD02AF6448291209 || xuid == 0x10F0C433E08E1357 ||
      xuid == 0x60E0FEFE42341715) {
    return 2;
  }

  return {};
}

std::optional<int> get_color_for_client(const int client_num) {
  const game::XUID xuid = auth::get_guid(static_cast<size_t>(client_num));
  return get_color_for_xuid(xuid);
}

typedef game::vec4<uint8_t> int_color_t;
template <size_t index>
void patch_color(const uint8_t r, const uint8_t g, const uint8_t b,
                 const uint8_t a = 255) {
  int_color_t *color_table = reinterpret_cast<int_color_t *>(0x142FEFE20_g);
  game::vec4_t *g_color_table = reinterpret_cast<game::vec4_t *>(0x142FB5CF0_g);

  color_table[index] = {r, g, b, a};
  g_color_table[index] = {
      static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f,
      static_cast<float>(b) / 255.0f, static_cast<float>(a) / 255.0f};
}

bool cl_get_client_name_stub(const int local_client_num, const int index,
                             char *buf, const int size,
                             const bool add_clan_name) {
  if (!buf || size <= 0) {
    return cl_get_client_name_hook.invoke<bool>(local_client_num, index, buf,
                                                size, add_clan_name);
  }

  const bool res = cl_get_client_name_hook.invoke<bool>(
      local_client_num, index, buf, size, add_clan_name);

  std::string packed_name(buf);
  std::optional<std::string> override_name;
  std::optional<std::string> override_tag;
  const game::ClientNum_t client_num = static_cast<game::ClientNum_t>(index);

  if (game::valid_client_num(client_num)) {
    if (name::has_name_override(client_num))
      override_name = name::get_name_override(client_num);
    if (name::has_clan_abbrev_override(client_num))
      override_tag = name::get_clan_abbrev_override(client_num);
  }

  std::string name_part = packed_name;
  std::string tag_part;
  const size_t pipe = packed_name.find('|');
  if (pipe != std::string::npos) {
    name_part = packed_name.substr(0, pipe);
    tag_part = packed_name.substr(pipe + 1);
  }

  if (override_name.has_value()) {
    name_part = *override_name;
  }

  if (override_tag.has_value()) {
    tag_part = *override_tag;
  }

  std::string effective_name = name_part;
  if (!tag_part.empty() && add_clan_name) {
    effective_name += "|";
    effective_name += tag_part;
  }

  const std::string decoded = decode_backtick_colors(effective_name);
  utils::string::copy(buf, size, decoded.c_str());

  if (_ReturnAddress() == reinterpret_cast<void *>(0x1406A7B56_g)) {
    return res;
  }

  const std::optional<int32_t> color = get_color_for_client(index);
  if (!color) {
    return res;
  }

  const char *val = utils::string::va("^%d%s", *color, buf);
  utils::string::copy(buf, size, val);

  return res;
}
} // namespace

struct component final : client_component {
  void post_unpack() override {
    patch_color<1>(255, 49, 49);  // 1  - Red
    patch_color<2>(134, 192, 0);  // 2  - Green
    patch_color<3>(255, 173, 34); // 3  - Yellow
    patch_color<4>(0, 135, 193);  // 4  - Blue
    patch_color<5>(32, 197, 255); // 5  - Light Blue
    patch_color<6>(151, 80, 221); // 6  - Pink

    cl_get_client_name_hook.create(game::cl::CL_GetClientName,
                                   cl_get_client_name_stub);
  }
};
} // namespace colors

REGISTER_COMPONENT(colors::component)