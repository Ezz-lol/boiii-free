#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "server_list.hpp"

#include "game/game.hpp"

#include <utils/string.hpp>
#include <utils/concurrency.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>

#include "network.hpp"
#include "scheduler.hpp"

namespace server_list
{
	namespace
	{
		utils::hook::detour lua_server_info_to_table_hook;

		struct master_query
		{
			game::netadr_t address{};
			bool responded{false};
			std::unordered_set<game::netadr_t> results{};
		};

		struct state
		{
			std::vector<master_query> masters{};
			bool requesting{false};
			std::chrono::high_resolution_clock::time_point query_start{};
			callback callback{};
		};

		utils::concurrency::container<state> master_state;

		utils::concurrency::container<server_list> favorite_servers{};

		std::unordered_set<game::netadr_t> parse_server_list_data(const network::data_view& data)
		{
			std::unordered_set<game::netadr_t> result{};

			std::optional<size_t> start{};
			for (size_t i = 0; i + 6 < data.size(); ++i)
			{
				if (data[i + 6] == '\\')
				{
					start.emplace(i);
					break;
				}
			}

			if (!start.has_value())
			{
				return result;
			}

			for (auto i = start.value(); i + 6 < data.size(); i += 7)
			{
				if (data[i + 6] != '\\')
				{
					break;
				}

				game::netadr_t address{};
				address.type = game::NA_RAWIP;
				address.localNetID = game::NS_CLIENT1;
				memcpy(&address.ipv4.a, data.data() + i + 0, 4);
				memcpy(&address.port, data.data() + i + 4, 2);
				address.port = ntohs(address.port);

				result.emplace(address);
			}

			return result;
		}

		bool all_masters_done(const state& s)
		{
			for (const auto& m : s.masters)
			{
				if (!m.responded)
				{
					return false;
				}
			}
			return true;
		}

		void finalize_master_query(state& s)
		{
			s.requesting = false;
			auto cb = std::move(s.callback);

			std::unordered_set<game::netadr_t> merged{};
			bool any_success = false;

			for (const auto& m : s.masters)
			{
				if (!m.results.empty())
				{
					any_success = true;
				}
				for (const auto& addr : m.results)
				{
					merged.insert(addr);
				}
			}

			s.masters.clear();
			cb(any_success, merged);
		}

		void handle_server_list_response(const game::netadr_t& target,
		                                 const network::data_view& data, state& s)
		{
			if (!s.requesting)
			{
				return;
			}

			master_query* matched = nullptr;
			for (auto& m : s.masters)
			{
				if (!m.responded && m.address == target)
				{
					matched = &m;
					break;
				}
			}

			if (!matched)
			{
				return;
			}

			matched->responded = true;
			matched->results = parse_server_list_data(data);

			if (all_masters_done(s))
			{
				finalize_master_query(s);
			}
		}

		void lua_server_info_to_table_stub(game::hks::lua_State* state, game::ServerInfo server_info, int index)
		{
			lua_server_info_to_table_hook.invoke(state, server_info, index);

			if (state)
			{
				const auto bot_count = atoi(game::Info_ValueForKey(server_info.tags, "bots"));
				game::Lua_SetTableInt("botCount", bot_count, state);
			}
		}

		std::string get_favorite_servers_file_path()
		{
			return "boiii_players/user/favorite_servers.txt";
		}

		void write_favorite_servers()
		{
			favorite_servers.access([](const std::unordered_set<game::netadr_t>& servers)
			{
				std::string servers_buffer{};
				for (const auto& itr : servers)
				{
					servers_buffer.append(utils::string::va("%i.%i.%i.%i:%hu\n", itr.ipv4.a, itr.ipv4.b, itr.ipv4.c,
					                                        itr.ipv4.d, itr.port));
				}

				utils::io::write_file(get_favorite_servers_file_path(), servers_buffer);
			});
		}

		void read_favorite_servers()
		{
			const std::string path = get_favorite_servers_file_path();
			if (!utils::io::file_exists(path))
			{
				return;
			}

			favorite_servers.access([&path](std::unordered_set<game::netadr_t>& servers)
			{
				servers.clear();

				std::string data;
				if (utils::io::read_file(path, &data))
				{
					const auto srv = utils::string::split(data, '\n');
					for (const auto& server_address : srv)
					{
						auto server = network::address_from_string(server_address);
						servers.insert(server);
					}
				}
			});
		}
	}

	std::vector<game::netadr_t> get_master_servers()
	{
		std::vector<game::netadr_t> servers;
		const char* hosts[] = {"master.ezz.lol:20810", "m.ezz.lol:20810"};
		for (const auto* host : hosts)
		{
			auto addr = network::address_from_string(host);
			if (addr.type != game::NA_BAD)
			{
				servers.push_back(addr);
			}
		}
		return servers;
	}

	void request_servers(callback callback)
	{
		master_state.access([&callback](state& s)
		{
			auto masters = get_master_servers();
			if (masters.empty())
			{
				return;
			}

			s.requesting = true;
			s.masters.clear();
			s.callback = std::move(callback);
			s.query_start = std::chrono::high_resolution_clock::now();

			for (const auto& addr : masters)
			{
				master_query mq{};
				mq.address = addr;
				s.masters.push_back(mq);

				network::send(addr, "getservers", utils::string::va("T7 %i full empty", PROTOCOL));
			}
		});
	}

	void add_favorite_server(game::netadr_t addr)
	{
		favorite_servers.access([&addr](std::unordered_set<game::netadr_t>& servers)
		{
			servers.insert(addr);
		});
		write_favorite_servers();
	}

	void remove_favorite_server(game::netadr_t addr)
	{
		favorite_servers.access([&addr](std::unordered_set<game::netadr_t>& servers)
		{
			for (auto it = servers.begin(); it != servers.end(); ++it)
			{
				if (network::are_addresses_equal(*it, addr))
				{
					servers.erase(it);
					break;
				}
			}
		});
		write_favorite_servers();
	}

	utils::concurrency::container<server_list>& get_favorite_servers()
	{
		return favorite_servers;
	}

	struct component final : client_component
	{
		void post_unpack() override
		{
			network::on("getServersResponse", [](const game::netadr_t& target, const network::data_view& data)
			{
				master_state.access([&](state& s)
				{
					handle_server_list_response(target, data, s);
				});
			});

			scheduler::loop([]
			{
				master_state.access([](state& s)
				{
					if (!s.requesting)
					{
						return;
					}

					const auto now = std::chrono::high_resolution_clock::now();
					if ((now - s.query_start) < 2s)
					{
						return;
					}

					// Timeout: mark all non-responded masters as done
					for (auto& m : s.masters)
					{
						m.responded = true;
					}

					finalize_master_query(s);
				});
			}, scheduler::async, 200ms);

			lua_server_info_to_table_hook.create(0x141F1FD10_g, lua_server_info_to_table_stub);

			scheduler::once([]
			{
				read_favorite_servers();
			}, scheduler::main);
		}

		void pre_destroy() override
		{
			master_state.access([](state& s)
			{
				s.requesting = false;
				s.masters.clear();
				s.callback = {};
			});
		}
	};
}

REGISTER_COMPONENT(server_list::component)
