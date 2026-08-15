#include <std_include.hpp>
#include "../steam.hpp"

#include "component/network.hpp"
#include "component/server_list.hpp"
#include <game/game.hpp>

namespace steam {
namespace {
constexpr uint32_t k_unFavoriteFlag = 1u;
constexpr uint32_t k_unHistoryFlag = 2u;
constexpr uint32_t BO3_APP_ID = 311210;

// combined list of favorites + recents for GetFavoriteGame enumeration
struct favorite_entry {
  game::net::netadr_t addr;
  uint32_t flags;
};

std::vector<favorite_entry> build_favorites_list() {
  std::vector<favorite_entry> result;

  server_list::get_favorite_servers().access(
      [&](const server_list::server_list &favs) {
        for (const game::net::netadr_t &addr : favs) {
          result.push_back({addr, k_unFavoriteFlag});
        }
      });

  server_list::get_recent_servers().access(
      [&](const server_list::recent_list &recents) {
        for (const game::net::netadr_t &addr : recents) {
          result.push_back({addr, k_unHistoryFlag});
        }
      });

  return result;
}
} // namespace

int32_t matchmaking::GetFavoriteGameCount() {
  const std::vector<favorite_entry> list = build_favorites_list();
  return static_cast<int>(list.size());
}

bool matchmaking::GetFavoriteGame(int32_t iGame, uint32_t *pnAppID,
                                  uint32_t *pnIP, uint16_t *pnConnPort,
                                  uint16_t *pnQueryPort, uint32_t *punFlags,
                                  uint32_t *pRTime32LastPlayedOnServer) {
  const std::vector<favorite_entry> list = build_favorites_list();
  if (iGame < 0 || iGame >= static_cast<int>(list.size()))
    return false;

  const favorite_entry &entry = list[iGame];
  if (pnAppID)
    *pnAppID = BO3_APP_ID;

  // pack IP as network-order uint32
  if (pnIP) {
    uint32_t ip = 0;
    memcpy(&ip, &entry.addr.ipv4.a, 4);
    *pnIP = ntohl(ip);
  }

  if (pnConnPort)
    *pnConnPort = entry.addr.port;
  if (pnQueryPort)
    *pnQueryPort = entry.addr.port;
  if (punFlags)
    *punFlags = entry.flags;
  if (pRTime32LastPlayedOnServer)
    *pRTime32LastPlayedOnServer = static_cast<uint32_t>(std::time(nullptr));

  return true;
}

int32_t matchmaking::AddFavoriteGame(uint32_t nAppID, uint32_t nIP,
                                     uint16_t nConnPort, uint16_t nQueryPort,
                                     uint32_t unFlags,
                                     uint32_t rTime32LastPlayedOnServer) {
  const game::net::netadr_t addr =
      network::address_from_ip(htonl(nIP), nConnPort);
  if ((unFlags & k_unFavoriteFlag) != 0u) {
    server_list::add_favorite_server(addr);
  } else if ((unFlags & k_unHistoryFlag) != 0u || unFlags == 0u) {
    server_list::add_recent_server(addr);
  }
  return 0;
}

bool matchmaking::RemoveFavoriteGame(uint32_t nAppID, uint32_t nIP,
                                     uint16_t nConnPort, uint16_t nQueryPort,
                                     uint32_t unFlags) {
  const game::net::netadr_t addr =
      network::address_from_ip(htonl(nIP), nConnPort);
  if ((unFlags & k_unFavoriteFlag) != 0u) {
    server_list::remove_favorite_server(addr);
  } else if ((unFlags & k_unHistoryFlag) != 0u || unFlags == 0u) {
    server_list::remove_recent_server(addr);
  }
  return false;
}

uint64_t matchmaking::RequestLobbyList() { return 0; }

void matchmaking::AddRequestLobbyListStringFilter(const char *pchKeyToMatch,
                                                  const char *pchValueToMatch,
                                                  int32_t eComparisonType) {}

void matchmaking::AddRequestLobbyListNumericalFilter(const char *pchKeyToMatch,
                                                     int32_t nValueToMatch,
                                                     int32_t eComparisonType) {}

void matchmaking::AddRequestLobbyListNearValueFilter(
    const char *pchKeyToMatch, int32_t nValueToBeCloseTo) {}

void matchmaking::AddRequestLobbyListFilterSlotsAvailable(
    int32_t nSlotsAvailable) {}

void matchmaking::AddRequestLobbyListDistanceFilter(
    int32_t eLobbyDistanceFilter) {}

void matchmaking::AddRequestLobbyListResultCountFilter(int32_t cMaxResults) {}

void matchmaking::AddRequestLobbyListCompatibleMembersFilter(steam_id steamID) {
}

steam_id matchmaking::GetLobbyByIndex(int32_t iLobby) {
  steam_id id;

  id.raw.account_id = SteamUser()->GetSteamID().raw.account_id;
  id.raw.universe = 1;
  id.raw.account_type = 8;
  id.raw.account_instance = 0x40000;

  return id;
}

uint64_t matchmaking::CreateLobby(int32_t eLobbyType, int32_t cMaxMembers) {
  const uint64_t result = callbacks::register_call();
  lobby_created *retvals =
      static_cast<lobby_created *>(calloc(1, sizeof(lobby_created)));
  //::Utils::Memory::AllocateArray<LobbyCreated>();
  steam_id id;

  id.raw.account_id = SteamUser()->GetSteamID().raw.account_id;
  id.raw.universe = 1;
  id.raw.account_type = 8;
  id.raw.account_instance = 0x40000;

  retvals->m_e_result = 1;
  retvals->m_ul_steam_id_lobby = id;

  callbacks::return_call(retvals, sizeof(lobby_created),
                         lobby_created::callback_id, result);

  matchmaking::JoinLobby(id);

  return result;
}

uint64_t matchmaking::JoinLobby(steam_id steamIDLobby) {
  const uint64_t result = callbacks::register_call();
  lobby_enter *retvals =
      static_cast<lobby_enter *>(calloc(1, sizeof(lobby_enter)));
  //::Utils::Memory::AllocateArray<LobbyEnter>();
  retvals->m_b_locked = false;
  retvals->m_e_chat_room_enter_response = 1;
  retvals->m_rgf_chat_permissions = 0xFFFFFFFF;
  retvals->m_ul_steam_id_lobby = steamIDLobby;

  callbacks::return_call(retvals, sizeof(lobby_enter), lobby_enter::callback_id,
                         result);

  return result;
}

void matchmaking::LeaveLobby(steam_id steamIDLobby) {}

bool matchmaking::InviteUserToLobby(steam_id steamIDLobby,
                                    steam_id steamIDInvitee) {
  return true;
}

int32_t matchmaking::GetNumLobbyMembers(steam_id steamIDLobby) { return 1; }

steam_id matchmaking::GetLobbyMemberByIndex(steam_id steamIDLobby,
                                            int32_t iMember) {
  return SteamUser()->GetSteamID();
}

const char *matchmaking::GetLobbyData(steam_id steamIDLobby,
                                      const char *pchKey) {
  return "";
}

bool matchmaking::SetLobbyData(steam_id steamIDLobby, const char *pchKey,
                               const char *pchValue) {
  return true;
}

int32_t matchmaking::GetLobbyDataCount(steam_id steamIDLobby) { return 0; }

bool matchmaking::GetLobbyDataByIndex(steam_id steamIDLobby, int32_t iLobbyData,
                                      char *pchKey, int32_t cchKeyBufferSize,
                                      char *pchValue,
                                      int32_t cchValueBufferSize) {
  return true;
}

bool matchmaking::DeleteLobbyData(steam_id steamIDLobby, const char *pchKey) {
  return true;
}

const char *matchmaking::GetLobbyMemberData(steam_id steamIDLobby,
                                            steam_id steamIDUser,
                                            const char *pchKey) {
  return "";
}

void matchmaking::SetLobbyMemberData(steam_id steamIDLobby, const char *pchKey,
                                     const char *pchValue) {}

bool matchmaking::SendLobbyChatMsg(steam_id steamIDLobby, const void *pvMsgBody,
                                   int32_t cubMsgBody) {
  return true;
}

int32_t matchmaking::GetLobbyChatEntry(steam_id steamIDLobby, int32_t iChatID,
                                       steam_id *pSteamIDUser, void *pvData,
                                       int32_t cubData,
                                       int32_t *peChatEntryType) {
  return 0;
}

bool matchmaking::RequestLobbyData(steam_id steamIDLobby) { return true; }

void matchmaking::SetLobbyGameServer(steam_id steamIDLobby,
                                     uint32_t unGameServerIP,
                                     uint16_t unGameServerPort,
                                     steam_id steamIDGameServer) {}

bool matchmaking::GetLobbyGameServer(steam_id steamIDLobby,
                                     uint32_t *punGameServerIP,
                                     uint16_t *punGameServerPort,
                                     steam_id *psteamIDGameServer) {
  return true;
}

bool matchmaking::SetLobbyMemberLimit(steam_id steamIDLobby,
                                      int32_t cMaxMembers) {
  return true;
}

int32_t matchmaking::GetLobbyMemberLimit(steam_id steamIDLobby) { return 0; }

bool matchmaking::SetLobbyType(steam_id steamIDLobby, int32_t eLobbyType) {
  return true;
}

bool matchmaking::SetLobbyJoinable(steam_id steamIDLobby, bool bLobbyJoinable) {
  return true;
}

steam_id matchmaking::GetLobbyOwner(steam_id steamIDLobby) {
  return SteamUser()->GetSteamID();
}

bool matchmaking::SetLobbyOwner(steam_id steamIDLobby,
                                steam_id steamIDNewOwner) {
  return true;
}

bool matchmaking::SetLinkedLobby(steam_id steamIDLobby,
                                 steam_id steamIDLobby2) {
  return true;
}
} // namespace steam