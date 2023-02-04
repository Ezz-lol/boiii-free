#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "game/game.hpp"
#include "steam/steam.hpp"

#include "network.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/info_string.hpp>

#include <version.hpp>

#include "command.hpp"

namespace getinfo
{
	namespace
	{
		std::string get_dvar_string(const char* dvar_name)
		{
			const auto dvar = game::Dvar_FindVar(dvar_name);
			if (!dvar)
			{
				return {};
			}

			return game::Dvar_GetString(dvar);
		}


		int Com_SessionMode_GetGameMode()
		{
			return *reinterpret_cast<int*>(game::select(0x1568EF7F4, 0x14948DB04)) << 14 >> 28;
		}
	}

	int get_assigned_team()
	{
		return (rand() % 2) + 1;
	}


	utils::hook::detour xxx;

	struct DDLDef
	{
		char* name;
		uint16_t version;
		uint32_t checksum;
		byte flags;
		int bitSize;
		int byteSize;
		void* structList;
		int structCount;
		void* enumList;
		int enumCount;
		DDLDef* next;
		int headerBitSize;
		int headerByteSize;
		int reserveSize;
		int userFlagsSize;
		bool paddingUsed;
	};

	struct DDLRoot
	{
		const char* name;
		DDLDef* ddlDef;
	};

	void p__rint(const char* fileName, int version, DDLRoot* ddlRoot)
	{
		if (ddlRoot)
		{
			printf("DDL: %s\n", ddlRoot->name);
			DDLDef* currDef;
			for (currDef = ddlRoot->ddlDef; currDef; currDef = currDef->next)
			{
				//if (currDef->version == version)
				//	return currDef;
				printf("Version: %d\n", currDef->version);
			}
		}


		MessageBoxA(0, fileName, std::to_string(version).data(), 0);
	}

	void* DDL_LoadAssetWithVersion(const char* fileName, int version)
	{
		auto ddlRoot = (DDLRoot*)game::DB_FindXAssetHeader(game::ASSET_TYPE_DDL, fileName, 1, -1).data;
		p__rint(fileName, version, ddlRoot);

		return xxx.invoke<void*>(fileName, version);
	}

	enum StorageFileType
	{
		STORAGE_COMMON_SETTINGS = 0x0,
		STORAGE_PROFILE_SHOUTCASTER = 0x1,
		STORAGE_CP_STATS_ONLINE = 0x2,
		STORAGE_CP_STATS_OFFLINE = 0x3,
		STORAGE_CP_STATS_NIGHTMARE = 0x4,
		STORAGE_CP_LOADOUTS = 0x5,
		STORAGE_CP_LOADOUTS_OFFLINE = 0x6,
		STORAGE_MP_STATS_ONLINE = 0x7,
		STORAGE_MP_STATS_OFFLINE = 0x8,
		STORAGE_MP_LOADOUTS = 0x9,
		STORAGE_MP_LOADOUTS_OFFLINE = 0xA,
		STORAGE_ZM_STATS_ONLINE = 0xB,
		STORAGE_ZM_STATS_OFFLINE = 0xC,
		STORAGE_ZM_LOADOUTS = 0xD,
		STORAGE_ZM_LOADOUTS_OFFLINE = 0xE,
		STORAGE_PAINTSHOP_DATA = 0xF,
		STORAGE_GUNSMITH = 0x10,
		STORAGE_PAINTJOBS = 0x11,
		STORAGE_EMBLEMS = 0x12,
		STORAGE_EXTERNAL_DATA = 0x13,
		STORAGE_FILE_COUNT = 0x14,
		STORAGE_FILE_FIRST = 0x0,
		STORAGE_FILE_INVALID = 0xFFFFFFFF,
	};

	struct StorageFileInfo
	{
		const char* name;
		const char* ddlPath;
		const char** fileNames;
		void* files;
		int size;
		int slots;
		StorageFileType fileType;
		/*StorageTargetType targetType;
		game::eModes sessionMode;
		CampaignMode campaignMode;
		eNetworkModes networkMode;
		eGameModes gameMode;
		bool readOnLogin;
		bool readOnly;
		bool sendToServer;
		bool useScratch;
		bool optional;*/
	};

	struct StorageFileMap
	{
		StorageFileInfo* info;
		/*StorageFileCallbacks callbacks;
		StorageFileDDL ddl;
		UIModelIndex uiFileTypeModel;*/
	};


	bool should_transfer_stub(void* storage_file_info)
	{
		auto should_transfer = game::ShouldTransfer(storage_file_info);

		if(i >= 12 && i <= 15)
		{
			def = !def;
		}

		return should_transfer;
	}

	DDLDef* core_get_ddl(int a)
	{
		auto def = reinterpret_cast<decltype(core_get_ddl)*>(game::select(0x141EA9730, 0x14046EC20))(a);
		//MessageBoxA(0, (def->name + (" - core - " + std::to_string(a))).data(), std::to_string(def->version).data(), 0);

		if (a == 1)
		{
			def = ((DDLRoot*)game::DB_FindXAssetHeader(game::ASSET_TYPE_DDL, "gamedata/ddl/mp/mp_stats.ddl", true, -1).data)->ddlDef;
			//for (int i = 0;; ++i) {
				//def = reinterpret_cast<decltype(core_get_ddl)*>(game::select(0x141EA9730, 0x14046EC20))(i);
			//	MessageBoxA(0, (def->name + (" - core - " + std::to_string(0))).data(), std::to_string(def->version).data(), 0);
			//}
		}
		return def;
	}

	DDLDef* Loadouts_get_ddl(uint64_t a)
	{
		auto def = reinterpret_cast<decltype(Loadouts_get_ddl)*>(game::select(0x141EAF250, 0x140472AA0))(a);
		//MessageBoxA(0, (def->name + (" - load - " + std::to_string(a))).data(), std::to_string(def->version).data(), 0);

		if (a == 3)
		{
			def = ((DDLRoot*)game::DB_FindXAssetHeader(game::ASSET_TYPE_DDL, "gamedata/ddl/loadouts/mp_loadouts.ddl", true, -1).data)->ddlDef;
			//for (int i = 0;; ++i) {
				//def = reinterpret_cast<decltype(core_get_ddl)*>(game::select(0x141EA9730, 0x14046EC20))(i);
			//MessageBoxA(0, (def->name + (" - load - " + std::to_string(0))).data(), std::to_string(def->version).data(), 0);
			//}
		}

		return def;
	}

	struct component final : generic_component
	{
		void post_unpack() override
		{
			//xxx.create(game::select(0x142522EE0, 0x140618AB0), DDL_LoadAssetWithVersion);


			command::add("dlll", []()
			{
				game::DB_EnumXAssets(game::ASSET_TYPE_DDL, [](game::XAssetHeader h, void*)
				{
					auto* ddlr = ((DDLRoot*)h.data);
					p__rint(ddlr->name, 0, ddlr);
				}, nullptr, true);
			});

			command::add("lel", []()
				{
				for(int i = 0; i < 16;++i)
				{
					core_get_ddl(i);
				}
				});

			if (game::is_server())
			{
				/*utils::hook::call(0x140467F54_g, core_get_ddl);
				utils::hook::call(0x14052D518_g, core_get_ddl);
				utils::hook::call(0x14052E62E_g, core_get_ddl);
				utils::hook::call(0x14054B5E1_g, core_get_ddl);

				utils::hook::call(game::select(0, 0x14052E66D), Loadouts_get_ddl);
				utils::hook::call(game::select(0x142277447, 0x14054B607), Loadouts_get_ddl);*/
			}
			else
			{
				
				utils::hook::call(0x1422781E3_g, ShouldTransfer);
			}

			//utils::hook::jump(game::select(0x142254EF0, 0x140537730), get_assigned_team);

			network::on("getInfo", [](const game::netadr_t& target, const network::data_view& data)
			{
				utils::info_string info{};
				info.set("challenge", std::string(data.begin(), data.end()));
				info.set("gamename", "T7");
				info.set("hostname", get_dvar_string("sv_hostname"));
				info.set("gametype", get_dvar_string("g_gametype"));
				//info.set("sv_motd", get_dvar_string("sv_motd"));
				info.set("xuid", utils::string::va("%llX", steam::SteamUser()->GetSteamID().bits));
				info.set("mapname", get_dvar_string("mapname"));
				//info.set("isPrivate", get_dvar_string("g_password").empty() ? "0" : "1");
				//info.set("clients", utils::string::va("%i", get_client_count()));
				//info.set("bots", utils::string::va("%i", get_bot_count()));
				//info.set("sv_maxclients", utils::string::va("%i", *game::mp::svs_numclients));
				info.set("protocol", utils::string::va("%i", 1/*PROTOCOL*/));
				info.set("playmode", utils::string::va("%i", game::Com_SessionMode_GetMode()));
				info.set("gamemode", utils::string::va("%i", Com_SessionMode_GetGameMode()));
				//info.set("sv_running", utils::string::va("%i", get_dvar_bool("sv_running")));
				info.set("dedicated", utils::string::va("%i", game::is_server() ? 1 : 0));
				info.set("shortversion", SHORTVERSION);

				network::send(target, "infoResponse", info.build(), '\n');
			});
		}
	};
}

REGISTER_COMPONENT(getinfo::component)
