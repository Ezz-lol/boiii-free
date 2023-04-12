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
		utils::hook::detour lua_serverinfo_to_table_hook;

		struct state
		{
			game::netadr_t address{};
			bool requesting{false};
			std::chrono::high_resolution_clock::time_point query_start{};
			callback callback{};
		};

		utils::concurrency::container<state> master_state;

		std::vector<game::netadr_t> favorite_servers{};

		void handle_server_list_response(const game::netadr_t& target,
		                                 const network::data_view& data, state& s)
		{
			if (!s.requesting || s.address != target)
			{
				return;
			}

			s.requesting = false;
			const auto callback = std::move(s.callback);

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
				callback(true, {});
				return;
			}

			std::unordered_set<game::netadr_t> result{};

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

			callback(true, result);
		}

		void lua_serverinfo_to_table_stub(game::hks::lua_State* state, game::ServerInfo serverInfo, int index)
		{
			lua_serverinfo_to_table_hook.invoke(state, serverInfo, index);

			if (state)
			{
				auto botCount = atoi(game::Info_ValueForKey(serverInfo.tags, "bots"));
				game::Lua_SetTableInt("botCount", botCount, state);
			}
		}

		std::string get_favorite_servers_file_path()
		{
			return "players/user/favorite_servers.csv";
		}

		void write_favorite_servers()
		{
			std::string servers_buffer = "";
			for (auto itr : favorite_servers)
			{
				servers_buffer.append(utils::string::va("%u,%u\n", itr.addr, itr.port));
			}

			if (servers_buffer.empty())
			{
				return;
			}

			utils::io::write_file(get_favorite_servers_file_path(), servers_buffer);
		}

		void read_favorite_servers()
		{
			const std::string path = get_favorite_servers_file_path();
			if (!utils::io::file_exists(path))
			{
				return;
			}

			favorite_servers.clear();

			std::string filedata;
			if (utils::io::read_file(path, &filedata))
			{
				auto servers = utils::string::split(filedata, '\n');
				for (auto server_data : servers)
				{
					auto data = utils::string::split(server_data, ',');
					auto addr = std::stoul(data[0].c_str());
					auto port = (uint16_t)atoi(data[1].c_str());
					auto server = network::address_from_ip(addr, port);
					favorite_servers.push_back(server);
				}
			}
		}
	}

	bool get_master_server(game::netadr_t& address)
	{
		address = network::address_from_string("server.boiii.re:20810");
		return address.type != game::NA_BAD;
	}

	void request_servers(callback callback)
	{
		master_state.access([&callback](state& s)
		{
			game::netadr_t addr{};
			if (!get_master_server(addr))
			{
				return;
			}

			s.requesting = true;
			s.address = addr;
			s.callback = std::move(callback);
			s.query_start = std::chrono::high_resolution_clock::now();

			network::send(s.address, "getservers", utils::string::va("T7 %i full empty", PROTOCOL));
		});
	}

	void add_favorite_server(game::netadr_t addr)
	{
		if (has_favorited_server(addr))
		{
			return;
		}

		favorite_servers.push_back(addr);
		write_favorite_servers();
	}

	void remove_favorite_server(game::netadr_t addr)
	{
		for (auto it = favorite_servers.begin(); it != favorite_servers.end(); ++it)
		{
			if (network::are_addresses_equal(*it, addr))
			{
				favorite_servers.erase(it);
				break;
			}
		}

		write_favorite_servers();
	}

	bool has_favorited_server(game::netadr_t addr)
	{
		auto it = std::find_if(favorite_servers.begin(), favorite_servers.end(), [&addr](const game::netadr_t& obj) { return network::are_addresses_equal(addr, obj); });
		return it != favorite_servers.end();
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

					s.requesting = false;
					s.callback(false, {});
					s.callback = {};
				});
			}, scheduler::async, 200ms);

			lua_serverinfo_to_table_hook.create(0x141F1FD10_g, lua_serverinfo_to_table_stub);

			read_favorite_servers();
		}

		void pre_destroy() override
		{
			master_state.access([](state& s)
			{
				s.requesting = false;
				s.callback = {};
			});
		}
	};
}

REGISTER_COMPONENT(server_list::component)
