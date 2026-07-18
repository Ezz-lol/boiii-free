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
  const auto xuid = auth::get_guid(static_cast<size_t>(client_num));
  return get_color_for_xuid(xuid);
}

template <size_t index>
void patch_color(const uint8_t r, const uint8_t g, const uint8_t b,
                 const uint8_t a = 255) {
  auto *color_table = reinterpret_cast<uint8_t *>(0x142FEFE20_g);
  auto *g_color_table = reinterpret_cast<float *>(0x142FB5CF0_g);

  uint8_t color_int[4];
  color_int[0] = r;
  color_int[1] = g;
  color_int[2] = b;
  color_int[3] = a;

  utils::hook::copy(color_table + index * 4, color_int, sizeof(color_int));

  float color_float[4];
  color_float[0] = static_cast<float>(r) / 255.0f;
  color_float[1] = static_cast<float>(g) / 255.0f;
  color_float[2] = static_cast<float>(b) / 255.0f;
  color_float[3] = static_cast<float>(a) / 255.0f;

  utils::hook::copy(g_color_table + index * 4, color_float,
                    sizeof(color_float));
}

bool cl_get_client_name_stub(const int local_client_num, const int index,
                             char *buf, const int size,
                             const bool add_clan_name) {
  if (!buf || size <= 0) {
    return cl_get_client_name_hook.invoke<bool>(local_client_num, index, buf,
                                                size, add_clan_name);
  }

  const auto res = cl_get_client_name_hook.invoke<bool>(
      local_client_num, index, buf, size, add_clan_name);

  std::string packed_name(buf);
  std::optional<std::string> override_name;
  std::optional<std::string> override_tag;
  const auto client_num = static_cast<game::ClientNum_t>(index);

  if (game::valid_client_num(client_num)) {
    if (name::has_name_override(client_num))
      override_name = name::get_name_override(client_num);
    if (name::has_clan_abbrev_override(client_num))
      override_tag = name::get_clan_abbrev_override(client_num);
  }

  std::string name_part = packed_name;
  std::string tag_part;
  const auto pipe = packed_name.find('|');
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

/*const char* get_gamer_tag_stub(const uint32_t num)
{
  const auto color = get_color_for_xuid(steam::SteamUser()->GetSteamID().bits);
  const auto name = reinterpret_cast<const char*
(*)(uint32_t)>(0x141EC6E80)(num) + 8;

  if (!color || num)
  {
    return name;
  }

  return utils::string::va("^1%s", *color, name);
}*/
} // namespace

struct component final : client_component {
  void post_unpack() override {
    patch_color<1>(255, 49, 49);  // 1  - Red
    patch_color<2>(134, 192, 0);  // 2  - Green
    patch_color<3>(255, 173, 34); // 3  - Yellow
    patch_color<4>(0, 135, 193);  // 4  - Blue
    patch_color<5>(32, 197, 255); // 5  - Light Blue
    patch_color<6>(151, 80, 221); // 6  - Pink

    // Old addresses
    cl_get_client_name_hook.create(game::cl::CL_GetClientName,
                                   cl_get_client_name_stub);
    // utils::hook::jump(0x141EC72E0_g, get_gamer_tag_stub);
  }
};
} // namespace colors

REGISTER_COMPONENT(colors::component)