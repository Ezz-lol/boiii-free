#include <std_include.hpp>

#include <game/game.hpp>

#include <component/command.hpp>
#include <component/getinfo.hpp>
#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <str.hpp>

namespace bots {
namespace {
constexpr const char bot_format_string[] =
    "connect "
    "\"\\invited\\1\\cg_predictItems\\1\\cl_"
    "anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\"
    "5000\\name\\%s\\clanAbbrev\\%s\\xuid\\%s\\xnaddr\\%"
    "s\\natType\\2\\protocol\\%d\\netfieldchk\\%d\\sessionmode\\%s\\qport\\%"
    "d\"";

struct BotName {
  game::playerName_t name;
  game::clanAbbrev_t clan_abbrev;
};

std::vector<BotName> load_bots_names() {
  constexpr const BotName DEFAULT_BOT_NAMES[] = {{"Skwll", "DEV"},
                                                 {"amkillam", "DEV"},
                                                 {"Frozedy", "DEV"},

                                                 {"Ava", "WIFE"},

                                                 {"Solar", "Ezz"},
                                                 {"Catsby", "Ezz"},
                                                 {"Hellcat", "Ezz"},
                                                 {"Zyrow", "Ezz"},
                                                 {"ZACK", "Ezz"},
                                                 {"Aranella", "Ezz"},
                                                 {"Clyde", "Ezz"},
                                                 {"Misty", "Ezz"},
                                                 {"TheBot", "Ezz"},
                                                 {"Alenski", "Ezz"},
                                                 {"Boiii", "Ezz"},
                                                 {"Sabino", "Ezz"},
                                                 {"KST", "Ezz"},
                                                 {"Faisal", "Ezz"},

                                                 {"Richtofen", "ZMB"},
                                                 {"Dempsy", "ZMB"},
                                                 {"Samantha", "ZMB"},
                                                 {"Takeo", "ZMB"},
                                                 {"ShadowMan", "ZMB"},

                                                 {"Aimbot.exe", "EXE"},
                                                 {"Wallhack.exe", "EXE"},
                                                 {"NotAScammer.exe", "EXE"},

                                                 {"ThunderCockKiller12", "TF"},

                                                 {"1stPlaceBtw", "YUH"},

                                                 {"NoIamFirstPlace", "FR"},

                                                 {"IDied", "LOL"},

                                                 {"TheLegend27", "GOW"}};

  std::string buffer;
  if (!utils::io::read_file("boiii/bots.txt", &buffer) || buffer.empty()) {
    return std::vector(std::begin(DEFAULT_BOT_NAMES),
                       std::end(DEFAULT_BOT_NAMES));
  }

  std::vector<BotName> bot_names;
  std::vector<std::string> data = utils::string::split(buffer, '\n');
  for (std::string &entry : data) {
    utils::string::replace(entry, "\r", "");
    utils::string::trim(entry);

    if (!entry.empty()) {
      std::string clan_abbrev;
      // Check if there is a clan tag
      const size_t pos = entry.find(',');
      if (pos != std::string::npos) {
        // Only start copying over from non-null characters (otherwise it can be
        // "<=")
        if ((pos + 1) < entry.size()) {
          clan_abbrev = entry.substr(pos + 1);
        }

        entry = entry.substr(0, pos);
      }

      BotName name;
      strscpy(name.name, entry);
      strscpy(name.clan_abbrev, clan_abbrev);

      bot_names.emplace_back(name);
    }
  }

  return bot_names.empty() ? std::vector(std::begin(DEFAULT_BOT_NAMES),
                                         std::end(DEFAULT_BOT_NAMES))
                           : bot_names;
}

const std::vector<BotName> &get_bot_names() {
  static const std::vector<BotName> bot_names = [] {
    std::vector<BotName> names = load_bots_names();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::ranges::shuffle(names, gen);
    return names;
  }();

  return bot_names;
}

const char *get_bot_name() {
  static size_t current = 0;
  const std::vector<BotName> &names = get_bot_names();

  current = (current + 1) % names.size();
  return names.at(current).name;
}
const char *find_clan_name(const std::string &needle) {
  for (const BotName &entry : get_bot_names()) {
    if (entry.name == needle) {
      return entry.clan_abbrev;
    }
  }

  return "3arc";
}

int format_bot_string(char *buffer, [[maybe_unused]] const char *format,
                      const char *name, const char *xuid, const char *xnaddr,
                      int protocol, int net_field_chk, const char *session_mode,
                      int qport) {

  return sprintf_s(buffer, 1024, bot_format_string, name, find_clan_name(name),
                   xuid, xnaddr, protocol, net_field_chk, session_mode, qport);
}
} // namespace

struct component final : generic_component {
#ifndef NDEBUG
  std::string name() override { return "bots"; }
#endif

  void post_unpack() override {
    utils::hook::jump(game::select(0x141653B90, 0x141653B70, 0x1402732E0),
                      get_bot_name);
    utils::hook::call(game::select(0x1421EC547, 0x142249097, 0x14052E53A),
                      format_bot_string);

    if (game::is_client()) {
      utils::hook::jump(game::select(0x1416542A0, 0x141654280, 0x0),
                        get_bot_name); // SV_ZombieNameRandom
    }

    command::add("spawnBot", [](const command::params &params) {
      if (!getinfo::is_host()) {
        return;
      }

      size_t count = 1;
      if (params.size() > 1) {
        if (params[1] == "all"s) {
          count = 18;
        } else {
          count = atoi(params[1]);
        }
      }

      scheduler::once(
          [count] {
            for (size_t i = 0; i < count; ++i) {
              if (!game::sv::SV_AddTestClient()) {
                break;
              }
            }
          },
          scheduler::server);
    });
  }
};
} // namespace bots

REGISTER_COMPONENT(bots::component)
