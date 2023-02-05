#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "party.hpp"
#include "network.hpp"
#include "scheduler.hpp"

#include <utils/hook.hpp>
#include <utils/info_string.hpp>
#include <utils/cryptography.hpp>
#include <utils/concurrency.hpp>

namespace party
{
	namespace
	{
		std::atomic_bool is_connecting_to_dedi{false};
		game::netadr_t connect_host{{}, {}, game::NA_BAD, {}};

		struct server_query
		{
			bool sent{false};
			game::netadr_t host{};
			std::string challenge{};
			query_callback callback{};
			std::chrono::high_resolution_clock::time_point query_time{};
		};

		utils::concurrency::container<std::vector<server_query>>& get_server_queries()
		{
			static utils::concurrency::container<std::vector<server_query>> server_queries;
			return server_queries;
		}

		void stop_zombies_intro_if_needed()
		{
			if (game::Com_SessionMode_GetMode() != game::MODE_ZOMBIES)
			{
				return;
			}

			scheduler::once([]
			{
				scheduler::schedule([]
				{
					if (!game::Sys_IsDatabaseReady())
					{
						return scheduler::cond_continue;
					}

					game::Cinematic_StopPlayback(0, true);
					return scheduler::cond_end;
				}, scheduler::main);
			}, scheduler::main, 15s);
		}

		void connect_to_lobby(const game::netadr_t& addr, const std::string& mapname, const std::string& gamemode)
		{
			game::XSESSION_INFO info{};
			game::CL_ConnectFromLobby(0, &info, &addr, 1, 0, mapname.data(), gamemode.data());

			stop_zombies_intro_if_needed();
		}

		void launch_mode(const game::eModes mode)
		{
			scheduler::once([=]
			{
				const auto local_client = *reinterpret_cast<DWORD*>(0x14342355C_g);
				const auto current_mode = game::Com_SessionMode_GetMode();
				game::Com_SwitchMode(local_client, current_mode, mode, 6);
			}, scheduler::main);
		}

		void connect_to_lobby_with_mode(const game::netadr_t& addr, const game::eModes mode, const std::string& mapname,
		                                const std::string& gametype, const bool was_retried = false)
		{
			if (game::Com_SessionMode_IsMode(mode))
			{
				connect_to_lobby(addr, mapname, gametype);
				return;
			}

			if (!was_retried)
			{
				scheduler::once([=]
				{
					connect_to_lobby_with_mode(addr, mode, mapname, gametype, true);
				}, scheduler::main, 5s);

				launch_mode(mode);
			}
		}

		game::LobbyMainMode convert_mode(const game::eModes mode)
		{
			switch (mode)
			{
			case game::MODE_CAMPAIGN:
				return game::LOBBY_MAINMODE_CP;
			case game::MODE_MULTIPLAYER:
				return game::LOBBY_MAINMODE_MP;
			case game::MODE_ZOMBIES:
				return game::LOBBY_MAINMODE_ZM;
			default:
				return game::LOBBY_MAINMODE_INVALID;
			}
		}

		void connect_to_session(const game::netadr_t& addr, const std::string& hostname, const uint64_t xuid,
		                        const game::eModes mode)
		{
			const auto LobbyJoin_Begin = reinterpret_cast<bool(*)(int actionId, game::ControllerIndex_t controllerIndex,
			                                                      game::LobbyType sourceLobbyType,
			                                                      game::LobbyType targetLobbyType)>(0x141ED9540_g);

			if (!LobbyJoin_Begin(0, game::CONTROLLER_INDEX_FIRST, game::LOBBY_TYPE_PRIVATE, game::LOBBY_TYPE_PRIVATE))
			{
				return;
			}

			auto& join = *game::s_join;

			auto& host = join.hostList[0];
			memset(&host, 0, sizeof(host));

			host.info.netAdr = addr;
			host.info.xuid = xuid;
			strcpy_s(host.info.name, hostname.data());

			host.lobbyType = game::LOBBY_TYPE_PRIVATE;
			host.lobbyParams.networkMode = game::LOBBY_NETWORKMODE_LIVE;
			host.lobbyParams.mainMode = convert_mode(mode);

			host.retryCount = 0;
			host.retryTime = game::Sys_Milliseconds();

			join.potentialHost = host;
			join.hostCount = 1;
			join.processedCount = 1;
			join.state = game::JOIN_SOURCE_STATE_ASSOCIATING;
			join.startTime = game::Sys_Milliseconds();

			/*join.targetLobbyType = game::LOBBY_TYPE_PRIVATE;
			join.sourceLobbyType = game::LOBBY_TYPE_PRIVATE;
			join.controllerIndex = game::CONTROLLER_INDEX_FIRST;
			join.joinType = game::JOIN_TYPE_NORMAL;
			join.joinResult = game::JOIN_RESULT_INVALID;
			join.isFinalized = false;*/

			// LobbyJoinSource_Finalize
			join.isFinalized = true;
		}

		void handle_connect_query_response(const bool success, const game::netadr_t& target,
		                                   const utils::info_string& info, uint32_t ping)
		{
			if (!success)
			{
				return;
			}

			is_connecting_to_dedi = info.get("dedicated") == "1";

			const auto gamename = info.get("gamename");
			if (gamename != "T7"s)
			{
				const auto str = "Invalid gamename.";
				printf("%s\n", str);
				return;
			}

			const auto mapname = info.get("mapname");
			if (mapname.empty())
			{
				const auto str = "Invalid map.";
				printf("%s\n", str);
				return;
			}

			const auto gametype = info.get("gametype");
			if (gametype.empty())
			{
				const auto str = "Invalid gametype.";
				printf("%s\n", str);
				return;
			}

			//const auto hostname = info.get("sv_hostname");
			const auto playmode = info.get("playmode");
			const auto mode = game::eModes(std::atoi(playmode.data()));
			//const auto xuid = strtoull(info.get("xuid").data(), nullptr, 16);

			scheduler::once([=]
			{
				//connect_to_session(target, hostname, xuid, mode);
				connect_to_lobby_with_mode(target, mode, mapname, gametype);
			}, scheduler::main);
		}

		void connect_stub(const char* address)
		{
			const auto target = network::address_from_string(address);
			if (target.type == game::NA_BAD)
			{
				return;
			}

			connect_host = target;
			query_server(target, handle_connect_query_response);
		}

		void send_server_query(server_query& query)
		{
			query.sent = true;
			query.query_time = std::chrono::high_resolution_clock::now();
			query.challenge = utils::cryptography::random::get_challenge();

			network::send(query.host, "getInfo", query.challenge);
		}
	}

	void query_server(const game::netadr_t& host, query_callback callback)
	{
		server_query query{};
		query.sent = false;
		query.host = host;
		query.callback = std::move(callback);

		get_server_queries().access([&](std::vector<server_query>& server_queries)
		{
			server_queries.emplace_back(std::move(query));
		});
	}

	int should_transfer_stub(uint8_t* storage_file_info)
	{
		auto should_transfer = game::ShouldTransfer(storage_file_info);

		const auto offset = storage_file_info - reinterpret_cast<uint8_t*>(0x14343CDF0_g);
		const auto index = offset / 120;

		if (is_connecting_to_dedi && index >= 12 && index <= 15)
		{
			should_transfer = !should_transfer;
		}

		return should_transfer;
	}

	struct component final : client_component
	{
		void post_unpack() override
		{
			utils::hook::jump(0x141EE6030_g, connect_stub);
			utils::hook::call(0x1422781E3_g, should_transfer_stub);

			network::on("infoResponse", [](const game::netadr_t& target, const network::data_view& data)
			{
				bool found_query = false;
				server_query query{};

				const utils::info_string info{data};

				get_server_queries().access([&](std::vector<server_query>& server_queries)
				{
					for (auto i = server_queries.begin(); i != server_queries.end(); ++i)
					{
						if (i->host == target && i->challenge == info.get("challenge"))
						{
							found_query = true;
							query = std::move(*i);
							i = server_queries.erase(i);
							break;
						}
					}
				});

				if (found_query)
				{
					const auto ping = std::chrono::high_resolution_clock::now() - query.query_time;
					const auto ping_ms = std::chrono::duration_cast<std::chrono::milliseconds>(ping).count();

					query.callback(true, query.host, info, static_cast<uint32_t>(ping_ms));
				}
			});

			scheduler::loop([]
			{
				std::vector<server_query> removed_queries{};

				get_server_queries().access([&](std::vector<server_query>& server_queries)
				{
					size_t sent_queries = 0;

					const auto now = std::chrono::high_resolution_clock::now();
					for (auto i = server_queries.begin(); i != server_queries.end();)
					{
						if (!i->sent)
						{
							if (++sent_queries < 10)
							{
								send_server_query(*i);
							}

							++i;
							continue;
						}

						if ((now - i->query_time) < 2s)
						{
							++i;
							continue;
						}

						removed_queries.emplace_back(std::move(*i));
						i = server_queries.erase(i);
					}
				});

				const utils::info_string empty{};
				for (const auto& query : removed_queries)
				{
					query.callback(false, query.host, empty, 0);
				}
			}, scheduler::async, 200ms);
		}

		void pre_destroy() override
		{
			get_server_queries().access([](std::vector<server_query>& s)
			{
				s = {};
			});
		}
	};
}

REGISTER_COMPONENT(party::component)
