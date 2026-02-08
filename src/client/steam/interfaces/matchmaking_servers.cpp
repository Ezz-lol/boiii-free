#include <std_include.hpp>
#include "../steam.hpp"

#include "game/game.hpp"

#include "component/party.hpp"
#include "component/network.hpp"
#include "component/server_list.hpp"

#include <utils/string.hpp>
#include <utils/concurrency.hpp>

#include <utils/io.hpp>

#include <iphlpapi.h>

#include <algorithm>
#include <unordered_set>

namespace steam
{
	namespace
	{
		struct server
		{
			bool handled{false};
			game::netadr_t address{};
			gameserveritem_t server_item{};
		};

		auto* const internet_request = reinterpret_cast<void*>(1);
		auto* const lan_request = reinterpret_cast<void*>(2);
		auto* const favorites_request = reinterpret_cast<void*>(4);
		auto* const history_request = reinterpret_cast<void*>(5);

		using servers = std::vector<server>;

		::utils::concurrency::container<servers> internet_servers{};
		::utils::concurrency::container<servers> lan_servers{};
		::utils::concurrency::container<servers> favorites_servers{};
		::utils::concurrency::container<servers> history_servers{};
		std::atomic<matchmaking_server_list_response*> internet_response{};
		std::atomic<matchmaking_server_list_response*> lan_response{};
		std::atomic<matchmaking_server_list_response*> favorites_response{};
		std::atomic<matchmaking_server_list_response*> history_response{};

		std::string get_lan_servers_file_path()
		{
			return "boiii_players/user/lan_servers.txt";
		}

		std::unordered_set<uint32_t> get_local_ipv4_addrs()
		{
			std::unordered_set<uint32_t> out{};
			out.emplace(htonl(INADDR_LOOPBACK));

			ULONG size = 0;
			if (GetAdaptersAddresses(AF_INET, 0, nullptr, nullptr, &size) != ERROR_BUFFER_OVERFLOW || size == 0)
			{
				return out;
			}

			std::string buffer;
			buffer.resize(size);
			auto* addrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
			if (GetAdaptersAddresses(AF_INET, 0, nullptr, addrs, &size) != NO_ERROR)
			{
				return out;
			}

			for (auto* a = addrs; a; a = a->Next)
			{
				for (auto* u = a->FirstUnicastAddress; u; u = u->Next)
				{
					if (!u->Address.lpSockaddr || u->Address.lpSockaddr->sa_family != AF_INET)
					{
						continue;
					}

					const auto* in = reinterpret_cast<const sockaddr_in*>(u->Address.lpSockaddr);
					out.emplace(in->sin_addr.s_addr);
				}
			}

			return out;
		}

		std::vector<game::netadr_t> get_lan_targets()
		{
			std::vector<game::netadr_t> out{};
			out.reserve(1100);

			const auto local_addrs = get_local_ipv4_addrs();

			const auto add_target = [&out, &local_addrs](const std::string& in)
			{
				if (in.empty())
				{
					return;
				}

				std::string addr_str = in;
				addr_str.erase(std::remove(addr_str.begin(), addr_str.end(), '\r'), addr_str.end());

				const auto has_port = addr_str.find(':') != std::string::npos;
				if (!has_port)
				{
					addr_str.append(":27017");
				}

				auto addr = network::address_from_string(addr_str);
				if (addr.type == game::NA_BAD)
				{
					return;
				}

				if (addr.type == game::NA_RAWIP && local_addrs.contains(addr.addr))
				{
					return;
				}

				for (const auto& existing : out)
				{
					if (existing == addr)
					{
						return;
					}
				}

				out.emplace_back(addr);
			};

			{
				std::string data;
				if (::utils::io::read_file(get_lan_servers_file_path(), &data))
				{
					const auto lines = ::utils::string::split(data, '\n');
					for (const auto& line : lines)
					{
						add_target(line);
					}
				}
			}

			const auto add_range_24 = [&out, &local_addrs](const uint8_t a, const uint8_t b, const uint8_t c)
			{
				for (uint16_t d = 1; d <= 254; ++d)
				{
					game::netadr_t addr{};
					addr.localNetID = game::NS_SERVER;
					addr.type = game::NA_RAWIP;
					addr.port = 27017;
					addr.addr = htonl((static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) |
						(static_cast<uint32_t>(c) << 8) | static_cast<uint32_t>(d));

					if (local_addrs.contains(addr.addr))
					{
						continue;
					}

					out.emplace_back(addr);
				}
			};

			add_range_24(192, 168, 0);
			add_range_24(192, 168, 1);
			add_range_24(10, 0, 0);
			add_range_24(26, 0, 0);

			return out;
		}

		template <typename T>
		void copy_safe(T& dest, const char* in)
		{
			::utils::string::copy(dest, in);
			::utils::string::strip_material(dest, dest, std::extent_v<T>);
		}

		gameserveritem_t create_server_item(const game::netadr_t& address, const ::utils::info_string& info,
		                                    const uint32_t ping, const bool success)
		{
			const auto sub_protocol = atoi(info.get("sub_protocol").data());

			gameserveritem_t server{};
			server.m_NetAdr.m_usConnectionPort = address.port;
			server.m_NetAdr.m_usQueryPort = address.port;
			server.m_NetAdr.m_unIP = ntohl(address.addr);
			server.m_nPing = static_cast<int>(ping);
			server.m_bHadSuccessfulResponse = success;
			server.m_bDoNotRefresh = false;

			copy_safe(server.m_szGameDir, "");
			copy_safe(server.m_szMap, info.get("mapname").data());
			copy_safe(server.m_szGameDescription, info.get("description").data());

			server.m_nAppID = (sub_protocol == SUB_PROTOCOL || sub_protocol == (SUB_PROTOCOL - 1)) ? 311210 : 0;
			server.m_nPlayers = atoi(info.get("clients").data());
			server.m_nMaxPlayers = atoi(info.get("sv_maxclients").data());
			server.m_nBotPlayers = atoi(info.get("bots").data());
			server.m_bPassword = info.get("isPrivate") == "1";
			server.m_bSecure = true;
			server.m_ulTimeLastPlayed = 0;
			server.m_nServerVersion = 1000;

			copy_safe(server.m_szServerName, info.get("hostname").data());

			const auto playmode = info.get("playmode");
			const auto mode = static_cast<game::eModes>(std::atoi(playmode.data()));

			const auto* tags = ::utils::string::va(
				R"(\gametype\%s\dedicated\%s\ranked\false\hardcore\%s\zombies\%s\playerCount\%d\bots\%d\modName\%s\)",
				info.get("gametype").data(),
				info.get("dedicated") == "1" ? "true" : "false",
				info.get("hc") == "1" ? "true" : "false",
				mode == game::MODE_ZOMBIES ? "true" : "false",
				server.m_nPlayers,
				atoi(info.get("bots").data()),
				info.get("modName").data());

			copy_safe(server.m_szGameTags, tags);

			server.m_steamID.bits = strtoull(info.get("xuid").data(), nullptr, 16);

			return server;
		}

		void handle_server_respone(const bool success, const game::netadr_t& host, const ::utils::info_string& info,
		                           const uint32_t ping, ::utils::concurrency::container<servers>& server_list,
		                           std::atomic<matchmaking_server_list_response*>& response, void* request)
		{
			bool all_handled = false;
			std::optional<int> index{};
			server_list.access([&](servers& srvs)
			{
				size_t i = 0;
				for (; i < srvs.size(); ++i)
				{
					if (srvs[i].address == host)
					{
						break;
					}
				}

				if (i >= srvs.size())
				{
					return;
				}

				index = static_cast<int>(i);

				auto& srv = srvs[i];
				srv.handled = true;
				srv.server_item = create_server_item(host, info, ping, success);


				for (const auto& entry : srvs)
				{
					if (!entry.handled)
					{
						return;
					}
				}

				all_handled = true;
			});

			const auto res = response.load();
			if (!index || !res)
			{
				return;
			}

			if (success)
			{
				res->ServerResponded(request, *index);
			}
			else
			{
				res->ServerFailedToRespond(request, *index);
			}

			if (all_handled)
			{
				res->RefreshComplete(request, eServerResponded);
			}
		}

		void handle_internet_server_response(const bool success, const game::netadr_t& host,
		                                     const ::utils::info_string& info,
		                                     const uint32_t ping)
		{
			handle_server_respone(success, host, info, ping, internet_servers, internet_response, internet_request);
		}

		void handle_lan_server_response(const bool success, const game::netadr_t& host,
		                                const ::utils::info_string& info,
		                                const uint32_t ping)
		{
			handle_server_respone(success, host, info, ping, lan_servers, lan_response, lan_request);
		}


		void handle_favorites_server_response(const bool success, const game::netadr_t& host,
		                                      const ::utils::info_string& info,
		                                      const uint32_t ping)
		{
			handle_server_respone(success, host, info, ping, favorites_servers, favorites_response, favorites_request);
		}

		void handle_history_server_response(const bool success, const game::netadr_t& host,
		                                    const ::utils::info_string& info,
		                                    const uint32_t ping)
		{
			handle_server_respone(success, host, info, ping, history_servers, history_response, history_request);
		}

		void ping_server(const game::netadr_t& server, party::query_callback callback)
		{
			party::query_server(server, callback);
		}
	}

	void* matchmaking_servers::RequestInternetServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                     matchmaking_server_list_response* pRequestServersResponse)
	{
		internet_response = pRequestServersResponse;

		server_list::request_servers([](const bool success, const std::unordered_set<game::netadr_t>& s)
		{
			const auto res = internet_response.load();
			if (!res)
			{
				return;
			}

			if (!success)
			{
				res->RefreshComplete(internet_request, eServerFailedToRespond);
				return;
			}

			if (s.empty())
			{
				res->RefreshComplete(internet_request, eNoServersListedOnMasterServer);
				return;
			}

			internet_servers.access([&s](servers& srvs)
			{
				srvs = {};
				srvs.reserve(s.size());

				for (auto& address : s)
				{
					server new_server{};
					new_server.address = address;
					new_server.server_item = create_server_item(address, {}, 0, false);

					srvs.push_back(new_server);
				}
			});

			for (auto& srv : s)
			{
				ping_server(srv, handle_internet_server_response);
			}
		});

		return internet_request;
	}

	void* matchmaking_servers::RequestLANServerList(unsigned int iApp,
	                                                matchmaking_server_list_response* pRequestServersResponse)
	{
		lan_response = pRequestServersResponse;

		auto targets = get_lan_targets();
		const auto res = lan_response.load();
		if (!res)
		{
			return lan_request;
		}

		if (targets.empty())
		{
			res->RefreshComplete(lan_request, eNoServersListedOnMasterServer);
			return lan_request;
		}

		lan_servers.access([&targets](servers& srvs)
		{
			srvs = {};
			srvs.reserve(targets.size());

			for (auto& address : targets)
			{
				server new_server{};
				new_server.address = address;
				new_server.server_item = create_server_item(address, {}, 0, false);
				srvs.push_back(new_server);
			}
		});

		for (auto& srv : targets)
		{
			ping_server(srv, handle_lan_server_response);
		}

		return lan_request;
	}

	void* matchmaking_servers::RequestFriendsServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                    matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(3);
	}

	void* matchmaking_servers::RequestFavoritesServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                      matchmaking_server_list_response* pRequestServersResponse)
	{
		favorites_response = pRequestServersResponse;

		auto& srvs = server_list::get_favorite_servers();
		srvs.access([&](std::unordered_set<game::netadr_t> s)
		{
			const auto res = favorites_response.load();
			if (!res)
			{
				return;
			}

			if (s.empty())
			{
				res->RefreshComplete(favorites_request, eNoServersListedOnMasterServer);
				return;
			}

			favorites_servers.access([s](servers& srvs)
			{
				srvs = {};
				srvs.reserve(s.size());

				for (auto& address : s)
				{
					server new_server{};
					new_server.address = address;
					new_server.server_item = create_server_item(address, {}, 0, false);

					srvs.push_back(new_server);
				}
			});

			for (auto& srv : s)
			{
				ping_server(srv, handle_favorites_server_response);
			}
		});

		return favorites_request;
	}

	void* matchmaking_servers::RequestHistoryServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                    matchmaking_server_list_response* pRequestServersResponse)
	{
		history_response = pRequestServersResponse;

		auto& srvs = server_list::get_recent_servers();
		srvs.access([&](std::vector<game::netadr_t> s)
		{
			const auto res = history_response.load();
			if (!res)
			{
				return;
			}

			if (s.empty())
			{
				res->RefreshComplete(history_request, eNoServersListedOnMasterServer);
				return;
			}

			history_servers.access([s](servers& srvs)
			{
				srvs = {};
				srvs.reserve(s.size());

				for (auto& address : s)
				{
					server new_server{};
					new_server.address = address;
					new_server.server_item = create_server_item(address, {}, 0, false);
					srvs.push_back(new_server);
				}
			});

			for (auto& srv : s)
			{
				ping_server(srv, handle_history_server_response);
			}
		});

		return history_request;
	}

	void* matchmaking_servers::RequestSpectatorServerList(unsigned int iApp, void** ppchFilters, unsigned int nFilters,
	                                                      matchmaking_server_list_response* pRequestServersResponse)
	{
		return reinterpret_cast<void*>(6);
	}

	void matchmaking_servers::ReleaseRequest(void* hServerListRequest)
	{
		if (internet_request == hServerListRequest)
		{
			internet_response = nullptr;
		}
		if (lan_request == hServerListRequest)
		{
			lan_response = nullptr;
		}
		if (favorites_request == hServerListRequest)
		{
			favorites_response = nullptr;
		}
		if (history_request == hServerListRequest)
		{
			history_response = nullptr;
		}
	}

	gameserveritem_t* matchmaking_servers::GetServerDetails(void* hRequest, int iServer)
	{
		if (internet_request != hRequest && lan_request != hRequest && favorites_request != hRequest && history_request != hRequest)
		{
			return nullptr;
		}

		auto& servers_list = hRequest == favorites_request
			? favorites_servers
			: (hRequest == history_request ? history_servers : (hRequest == lan_request ? lan_servers : internet_servers));

		static thread_local gameserveritem_t server_item{};
		return servers_list.access<gameserveritem_t*>([iServer](const servers& s) -> gameserveritem_t*
		{
			if (iServer < 0 || static_cast<size_t>(iServer) >= s.size())
			{
				return nullptr;
			}

			server_item = s[iServer].server_item;
			return &server_item;
		});
	}

	void matchmaking_servers::CancelQuery(void* hRequest)
	{
	}

	void matchmaking_servers::RefreshQuery(void* hRequest)
	{
	}

	bool matchmaking_servers::IsRefreshing(void* hRequest)
	{
		return false;
	}

	int matchmaking_servers::GetServerCount(void* hRequest)
	{
		if (internet_request != hRequest && lan_request != hRequest && favorites_request != hRequest && history_request != hRequest)
		{
			return 0;
		}

		auto& servers_list = hRequest == favorites_request
			? favorites_servers
			: (hRequest == history_request ? history_servers : (hRequest == lan_request ? lan_servers : internet_servers));
		return servers_list.access<int>([](const servers& s)
		{
			return static_cast<int>(s.size());
		});
	}

	void matchmaking_servers::RefreshServer(void* hRequest, const int iServer)
	{
		if (internet_request != hRequest && lan_request != hRequest && favorites_request != hRequest && history_request != hRequest)
		{
			return;
		}

		std::optional<game::netadr_t> address{};
		auto& servers_list = hRequest == favorites_request
			? favorites_servers
			: (hRequest == history_request ? history_servers : (hRequest == lan_request ? lan_servers : internet_servers));
		servers_list.access([&](const servers& s)
		{
			if (iServer < 0 || static_cast<size_t>(iServer) >= s.size())
			{
				return;
			}

			address = s[iServer].address;
		});

		if (address)
		{
			auto callback = hRequest == favorites_request
				? handle_favorites_server_response
				: (hRequest == history_request
					? handle_history_server_response
					: (hRequest == lan_request ? handle_lan_server_response : handle_internet_server_response));
			ping_server(*address, callback);
		}
	}

	void* matchmaking_servers::PingServer(const unsigned int unIP, const unsigned short usPort,
	                                      matchmaking_ping_response* pRequestServersResponse)
	{
		auto response = pRequestServersResponse;
		const auto addr = network::address_from_ip(htonl(unIP), usPort);

		party::query_server(
			addr, [response](const bool success, const game::netadr_t& host, const ::utils::info_string& info,
			                 const uint32_t ping)
			{
				if (success)
				{
					auto server_item = create_server_item(host, info, ping, success);
					response->ServerResponded(server_item);
				}
				else
				{
					response->ServerFailedToRespond();
				}
			});

		return reinterpret_cast<void*>(static_cast<uint64_t>(7 + rand()));
	}

	int matchmaking_servers::PlayerDetails(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse)
	{
		return 0;
	}

	int matchmaking_servers::ServerRules(unsigned int unIP, unsigned short usPort, void* pRequestServersResponse)
	{
		return 0;
	}

	void matchmaking_servers::CancelServerQuery(int hServerQuery)
	{
	}
}
